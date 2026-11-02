/* Suivi mémoire : --wrap=malloc,free, registre des blocs, échecs simulés.
** Chaque bloc est collé à une page protégée, rempli d'un motif non nul, et
** devient inaccessible à sa libération. En mode réel (valgrind, ASan), le vrai
** malloc est utilisé et les adresses sont masquées dans le registre pour que
** valgrind voie les fuites comme perdues. */
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include "lft_test.h"
#include "lft_internal.h"

#define POISON 0xA5
#define CANARY 0xCA
#define QUARANTINE 4096
#define HUGE_ALLOC (64UL * 1024 * 1024)

void	*__real_malloc(size_t size);
void	__real_free(void *ptr);

static t_lft_region	*g_regions;
static size_t		g_nregions;
static size_t		g_cap_regions;
static uint32_t		*g_index;
static size_t		g_cap_index;
static uint32_t		g_quarantine[QUARANTINE];
static size_t		g_quarantine_next;
static size_t		g_quarantine_used;
static int			g_real;
static long			g_live_blocks;
static size_t		g_live_bytes;
static long			g_calls;
static size_t		g_last_size;
static long			g_fail_at;
static int			g_injected;
static long			g_mf_pass;
static long			g_mf_total;
static long			g_mf_base_blocks;
static size_t		g_mf_base_bytes;

void	lft_set_real_malloc(int real)
{
	g_real = real;
}

int	lft_real_malloc(void)
{
	return (g_real);
}

static size_t	page_size(void)
{
	static size_t	size;

	if (size == 0)
		size = (size_t)sysconf(_SC_PAGESIZE);
	return (size);
}

static void	*raw_map(size_t len)
{
	void	*p;

	p = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS,
			-1, 0);
	if (p == MAP_FAILED)
		lft_report(LFT_INTERNAL, "mmap impossible pour le registre");
	return (p);
}

/* ------------------------------ Registre ------------------------------ */

static size_t	index_slot(uintptr_t hidden)
{
	uint64_t	h;

	h = (uint64_t)hidden * 11400714819323198485ULL;
	return ((size_t)(h >> 20) & (g_cap_index - 1));
}

static void	index_insert(uint32_t region)
{
	size_t	slot;

	slot = index_slot(g_regions[region].hidden);
	while (g_index[slot] != 0)
		slot = (slot + 1) & (g_cap_index - 1);
	g_index[slot] = region + 1;
}

static void	grow_registry(void)
{
	size_t			cap;
	t_lft_region	*regions;
	size_t			i;

	cap = g_cap_regions ? g_cap_regions * 2 : 1024;
	regions = raw_map(cap * sizeof(*regions));
	if (g_regions)
	{
		memcpy(regions, g_regions, g_nregions * sizeof(*regions));
		munmap(g_regions, g_cap_regions * sizeof(*regions));
		munmap(g_index, g_cap_index * sizeof(*g_index));
	}
	g_regions = regions;
	g_cap_regions = cap;
	g_cap_index = cap * 4;
	g_index = raw_map(g_cap_index * sizeof(*g_index));
	for (i = 0; i < g_nregions; i++)
		index_insert((uint32_t)i);
}

static t_lft_region	*registry_find(const void *data)
{
	uintptr_t	hidden;
	size_t		slot;

	if (g_index == NULL)
		return (NULL);
	hidden = ~(uintptr_t)data;
	slot = index_slot(hidden);
	while (g_index[slot] != 0)
	{
		if (g_regions[g_index[slot] - 1].hidden == hidden)
			return (&g_regions[g_index[slot] - 1]);
		slot = (slot + 1) & (g_cap_index - 1);
	}
	return (NULL);
}

static t_lft_region	*registry_add(const void *data)
{
	t_lft_region	*r;

	r = registry_find(data);
	if (r)
		return (r);
	if (g_nregions == g_cap_regions)
		grow_registry();
	r = &g_regions[g_nregions];
	r->hidden = ~(uintptr_t)data;
	index_insert((uint32_t)g_nregions);
	g_nregions++;
	return (r);
}

const t_lft_region	*lft_region_at(const void *addr)
{
	const char	*a;
	size_t		i;

	a = addr;
	for (i = 0; i < g_nregions; i++)
		if (g_regions[i].map && a >= g_regions[i].map
			&& a < g_regions[i].map + g_regions[i].map_len)
			return (&g_regions[i]);
	return (NULL);
}

const t_lft_region	*lft_region_of(const void *data)
{
	return (registry_find(data));
}

/* ------------------------ Régions protégées ------------------------ */

/* [page protégée][pages utilisables][page protégée]. Les données sont collées
** à la page de fin, ou à celle du début si front est vrai. Le reste de la zone
** utilisable reçoit un motif de contrôle pour voir les écritures hors limites. */
char	*lft_region_new(size_t size, int kind, int front)
{
	size_t			page;
	size_t			rw;
	char			*map;
	char			*data;
	t_lft_region	*r;

	page = page_size();
	rw = ((size ? size : 1) + page - 1) / page * page;
	map = mmap(NULL, rw + 2 * page, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS,
			-1, 0);
	if (map == MAP_FAILED)
		return (NULL);
	if (mprotect(map + page, rw, PROT_READ | PROT_WRITE) != 0)
		lft_report(LFT_INTERNAL, "mprotect impossible");
	if (front)
	{
		data = map + page;
		memset(data + size, CANARY, rw - size);
	}
	else
	{
		data = map + page + rw - size;
		memset(map + page, CANARY, rw - size);
	}
	memset(data, POISON, size);
	r = registry_add(data);
	r->map = map;
	r->map_len = rw + 2 * page;
	r->size = size;
	r->kind = (unsigned char)kind;
	r->live = 1;
	return (data);
}

void	lft_region_protect(const void *data, int prot)
{
	const t_lft_region	*r;
	size_t				page;

	r = registry_find(data);
	page = page_size();
	if (r && r->map)
		mprotect(r->map + page, r->map_len - 2 * page, prot);
}

/* L'adresse redevient inconnue : un free() dessus sera refusé. */
void	lft_region_forget(const void *data)
{
	t_lft_region	*r;

	r = registry_find(data);
	if (r)
	{
		r->map = NULL;
		r->live = 0;
		r->kind = LFT_R_GUARD;
	}
}

static void	check_canary(const t_lft_region *r)
{
	const unsigned char	*data;
	const unsigned char	*start;
	const unsigned char	*p;

	data = (const unsigned char *)~r->hidden;
	start = (const unsigned char *)r->map + page_size();
	for (p = start; p < data; p++)
		if (*p != CANARY)
			lft_report_fmt(LFT_MEM,
				"écriture %zu octet(s) avant le début d'un bloc malloc de "
				"%zu octet(s)", (size_t)(data - p), r->size);
}

static void	quarantine_push(t_lft_region *r)
{
	t_lft_region	*old;

	if (g_quarantine_used == QUARANTINE)
	{
		old = &g_regions[g_quarantine[g_quarantine_next]];
		munmap(old->map, old->map_len);
		old->map = NULL;
	}
	else
		g_quarantine_used++;
	g_quarantine[g_quarantine_next] = (uint32_t)(r - g_regions);
	g_quarantine_next = (g_quarantine_next + 1) % QUARANTINE;
}

/* --------------------------------- Blocs -------------------------------- */

static void	*block_new(size_t size, int kind)
{
	char			*data;
	t_lft_region	*r;

	if (g_real)
	{
		data = __real_malloc(size);
		if (data == NULL)
			return (NULL);
		r = registry_add(data);
		r->map = NULL;
		r->size = size;
		r->kind = (unsigned char)kind;
		r->live = 1;
	}
	else
	{
		if (size > SIZE_MAX / 4)
			return (NULL);
		data = lft_region_new(size, kind, 0);
		if (data == NULL)
		{
			if (size >= HUGE_ALLOC)
				return (NULL);
			lft_report(LFT_INTERNAL, "mmap impossible pour un petit bloc");
		}
	}
	g_live_blocks++;
	g_live_bytes += size;
	return (data);
}

void	*__wrap_malloc(size_t size)
{
	g_calls++;
	if (g_fail_at != 0 && g_calls == g_fail_at)
	{
		g_injected = 1;
		return (NULL);
	}
	g_last_size = size;
	return (block_new(size, LFT_R_BLOCK));
}

void	__wrap_free(void *ptr)
{
	t_lft_region	*r;

	if (ptr == NULL)
		return ;
	r = registry_find(ptr);
	if (r == NULL || r->kind == LFT_R_GUARD || r->kind == LFT_R_GUARD_RO)
		lft_report_fmt(LFT_MEM,
			"free() d'un pointeur qui n'est pas le début d'un bloc malloc (%p)",
			ptr);
	if (!r->live)
		lft_report_fmt(LFT_MEM, "double free d'un bloc de %zu octet(s)",
			r->size);
	r->live = 0;
	g_live_blocks--;
	g_live_bytes -= r->size;
	if (g_real)
	{
		__real_free(ptr);
		return ;
	}
	check_canary(r);
	mprotect(r->map, r->map_len, PROT_NONE);
	quarantine_push(r);
}

void	*lft_alloc(size_t size)
{
	void	*p;

	p = block_new(size, LFT_R_TEST_BLOCK);
	if (p == NULL)
		lft_report(LFT_INTERNAL, "lft_alloc impossible");
	return (p);
}

long	lft_malloc_calls(void)
{
	return (g_calls);
}

size_t	lft_last_malloc_size(void)
{
	return (g_last_size);
}

long	lft_live_blocks(void)
{
	return (g_live_blocks);
}

void	lft_check_leaks(void)
{
	if (g_live_blocks > 0)
		lft_report_fmt(LFT_LEAK, "%ld bloc(s) non libéré(s), %zu octet(s)",
			g_live_blocks, g_live_bytes);
}

/* ------------------------- Échecs de malloc ------------------------- */

void	lft_mf_begin(void)
{
	g_mf_pass = 0;
	g_mf_total = 0;
	g_mf_base_blocks = g_live_blocks;
	g_mf_base_bytes = g_live_bytes;
}

static void	mf_end_of_pass(void)
{
	long	failed;

	failed = g_injected ? g_fail_at : 0;
	g_fail_at = 0;
	g_injected = 0;
	if (g_live_blocks != g_mf_base_blocks)
	{
		if (failed)
			lft_report_fmt(LFT_LEAK, "%ld bloc(s) non libéré(s), %zu octet(s) "
				"quand le malloc n°%ld échoue",
				g_live_blocks - g_mf_base_blocks,
				g_live_bytes - g_mf_base_bytes, failed);
		lft_report_fmt(LFT_LEAK, "%ld bloc(s) non libéré(s), %zu octet(s)",
			g_live_blocks - g_mf_base_blocks,
			g_live_bytes - g_mf_base_bytes);
	}
	if (g_mf_pass == 1)
	{
		g_mf_total = g_calls;
		if (g_mf_total == 0)
			lft_report(LFT_FAIL, "aucun appel à malloc pendant le scénario");
	}
}

/* Premier passage : aucune panne, on compte les malloc. Ensuite, un passage
** par malloc, en faisant échouer celui-là seulement. */
int	lft_mf_next(void)
{
	if (g_mf_pass > 0)
		mf_end_of_pass();
	g_mf_pass++;
	if (g_mf_pass > 1 && g_mf_pass - 1 > g_mf_total)
		return (0);
	g_calls = 0;
	g_fail_at = g_mf_pass - 1;
	return (1);
}

int	lft_mf_injected(void)
{
	return (g_injected);
}

long	lft_mf_failed_malloc(void)
{
	return (g_injected ? g_fail_at : 0);
}
