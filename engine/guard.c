/* Buffers protégés pour les tests, et diagnostic des accès hors limites. */
#define _GNU_SOURCE
#include <signal.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <ucontext.h>
#include "lft_test.h"
#include "lft_internal.h"

#define ALT_STACK_SIZE (64 * 1024)

static void	*guard_or_die(size_t size, int kind, int front)
{
	char	*p;

	p = lft_region_new(size, kind, front);
	if (p == NULL)
		lft_report(LFT_INTERNAL, TR("mmap impossible pour un buffer protégé", "mmap failed for a guarded buffer"));
	return (p);
}

char	*lft_guard_buf(size_t n)
{
	return (guard_or_die(n, LFT_R_GUARD, 0));
}

char	*lft_guard_buf_front(size_t n)
{
	return (guard_or_die(n, LFT_R_GUARD, 1));
}

static const char	*guard_readonly(const void *src, size_t n, int front)
{
	char	*p;

	p = guard_or_die(n, LFT_R_GUARD_RO, front);
	memcpy(p, src, n);
	lft_region_protect(p, PROT_READ);
	return (p);
}

const char	*lft_guard_str(const char *s)
{
	return (guard_readonly(s, strlen(s) + 1, 0));
}

const char	*lft_guard_str_front(const char *s)
{
	return (guard_readonly(s, strlen(s) + 1, 1));
}

const char	*lft_guard_mem(const void *src, size_t n)
{
	return (guard_readonly(src, n, 0));
}

void	lft_guard_free(const void *p)
{
	const t_lft_region	*r;

	r = lft_region_of(p);
	if (r == NULL || r->map == NULL)
		return ;
	munmap(r->map, r->map_len);
	lft_region_forget(p);
}

/* ----------------------- Diagnostic des fautes ----------------------- */

static const char	*access_word(void *uc)
{
#if defined(__x86_64__) && defined(REG_ERR)
	return ((((ucontext_t *)uc)->uc_mcontext.gregs[REG_ERR] & 2)
		? TR("écriture", "write") : TR("lecture", "read"));
#else
	(void)uc;
	return (TR("accès", "access"));
#endif
}

static const char	*container_word(int kind)
{
	if (kind == LFT_R_GUARD)
		return (TR("d'un buffer", "of a buffer"));
	if (kind == LFT_R_GUARD_RO)
		return (TR("d'une entrée", "of an input"));
	return (TR("d'un bloc malloc", "of a malloc block"));
}

static void	describe_fault(const t_lft_region *r, const char *addr, void *uc)
{
	const char	*data;
	const char	*word;

	data = (const char *)~r->hidden;
	word = access_word(uc);
	if (!r->live)
		lft_report_fmt(LFT_OOB, TR("%s dans un bloc de %zu octet(s) déjà libéré", "%s in a block of %zu byte(s) already freed"),
			word, r->size);
	if (addr >= data + r->size)
		lft_report_fmt(LFT_OOB, TR("%s %zu octet(s) après la fin %s de "
			"%zu octet(s)", "%s %zu byte(s) past the end %s of "
			"%zu byte(s)"), word, (size_t)(addr - (data + r->size)) + 1,
			container_word(r->kind), r->size);
	if (addr < data)
		lft_report_fmt(LFT_OOB, TR("%s %zu octet(s) avant le début %s de "
			"%zu octet(s)", "%s %zu byte(s) before the start %s of "
			"%zu byte(s)"), word, (size_t)(data - addr),
			container_word(r->kind), r->size);
	lft_report_fmt(LFT_OOB, TR("écriture dans une entrée en lecture seule "
		"(paramètre const) de %zu octet(s)", "write into a read-only input "
		"(const parameter) of %zu byte(s)"), r->size);
}

/* Si la faute ne touche aucune zone connue, on la rejoue sans handler : le
** processus meurt normalement et le runner voit le vrai signal. */
static void	on_fault(int sig, siginfo_t *info, void *uc)
{
	const t_lft_region	*r;
	long				failed;

	r = lft_region_at(info->si_addr);
	if (r)
		describe_fault(r, info->si_addr, uc);
	failed = lft_mf_failed_malloc();
	if (failed)
		lft_report_fmt(LFT_CRASH, TR("%s quand le malloc n°%ld échoue "
			"(retour de malloc non vérifié ?)", "%s when malloc #%ld fails "
			"(return of malloc not checked?)"),
			sig == SIGBUS ? "SIGBUS" : "SIGSEGV", failed);
}

void	lft_install_fault_handler(void)
{
	static char			stack[ALT_STACK_SIZE];
	stack_t				alt;
	struct sigaction	sa;

	alt.ss_sp = stack;
	alt.ss_size = sizeof(stack);
	alt.ss_flags = 0;
	sigaltstack(&alt, NULL);
	memset(&sa, 0, sizeof(sa));
	sa.sa_sigaction = on_fault;
	sa.sa_flags = SA_SIGINFO | SA_ONSTACK | SA_RESETHAND;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGSEGV, &sa, NULL);
	sigaction(SIGBUS, &sa, NULL);
}
