/* Interfaces entre les fichiers du moteur (pas pour les tests). */
#ifndef LFT_INTERNAL_H
# define LFT_INTERNAL_H

# include <stddef.h>
# include <stdint.h>

/* Lettre envoyée par le test au runner en fin d'exécution. */
# define LFT_PASS 'P'
# define LFT_FAIL 'F'
# define LFT_LEAK 'L'
# define LFT_MEM 'M'
# define LFT_OOB 'O'
# define LFT_MISSING 'N'
# define LFT_INFO 'I'
# define LFT_CRASH 'C'
# define LFT_INTERNAL 'X'

/* Types de zones suivies par le registre. */
# define LFT_R_BLOCK 0
# define LFT_R_TEST_BLOCK 1
# define LFT_R_GUARD 2
# define LFT_R_GUARD_RO 3

typedef struct s_lft_region
{
	uintptr_t		hidden;
	char			*map;
	size_t			map_len;
	size_t			size;
	unsigned char	kind;
	unsigned char	live;
}	t_lft_region;

void	lft_report(char kind, const char *msg) __attribute__((noreturn));
void	lft_report_fmt(char kind, const char *fmt, ...)
		__attribute__((noreturn, format(printf, 2, 3)));

void				lft_missing(const char *name) __attribute__((noreturn));
char				*lft_region_new(size_t size, int kind, int front);
void				lft_region_protect(const void *data, int prot);
void				lft_region_forget(const void *data);
const t_lft_region	*lft_region_at(const void *addr);
const t_lft_region	*lft_region_of(const void *data);
void				lft_set_real_malloc(int real);
int					lft_real_malloc(void);
void				lft_check_leaks(void);
long				lft_mf_failed_malloc(void);
void				lft_install_fault_handler(void);

#endif
