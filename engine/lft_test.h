/* API des tests : TEST(), assertions, et prototypes du sujet libft v19.3.
** Ce header ne dépend pas du libft.h testé : les tests compilent même si
** celui-ci est incomplet. Les écarts sont signalés par driver/conformity.py. */
#ifndef LFT_TEST_H
# define LFT_TEST_H

# include <limits.h>
# include <stddef.h>
# include <stdint.h>
# include <string.h>

/* ------------------------- Enregistrement ------------------------- */

void	lft_register(const char *suite, const char *name, void (*fn)(void),
			int line);

# define TEST(suite, name) \
	static void	lft_t_##suite##_##name(void); \
	__attribute__((constructor)) static void	lft_r_##suite##_##name(void) \
	{ \
		lft_register(#suite, #name, lft_t_##suite##_##name, __LINE__); \
	} \
	static void	lft_t_##suite##_##name(void)

/* ---------------------------- Assertions --------------------------- */

void		lft_fail(const char *file, int line, const char *fmt, ...)
			__attribute__((noreturn, format(printf, 3, 4)));
void		lft_info(const char *fmt, ...)
			__attribute__((noreturn, format(printf, 1, 2)));
const char	*lft_quote(const char *s);
const char	*lft_quote_n(const void *s, size_t n);
void		lft_assert_str(const char *file, int line, const char *expr,
				const char *got, const char *exp);
void		lft_assert_mem(const char *file, int line, const char *expr,
				const void *got, const void *exp, size_t n);
void		lft_assert_ptr(const char *file, int line, const char *expr,
				const void *got, const void *exp);
void		lft_assert_sign(const char *file, int line, const char *expr,
				long long got, long long ref);

# define FAILF(...) lft_fail(__FILE__, __LINE__, __VA_ARGS__)
# define INFO(...) lft_info(__VA_ARGS__)

# define ASSERT_TRUE(cond) \
	do { \
		if (!(cond)) \
			FAILF("%s : faux, vrai attendu", #cond); \
	} while (0)

# define ASSERT_FALSE(cond) \
	do { \
		if (cond) \
			FAILF("%s : vrai, faux attendu", #cond); \
	} while (0)

# define ASSERT_EQ(got, exp) \
	do { \
		long long	lft_g = (long long)(got); \
		long long	lft_e = (long long)(exp); \
		\
		if (lft_g != lft_e) \
			FAILF("%s : obtenu %lld, attendu %lld", #got, lft_g, lft_e); \
	} while (0)

# define ASSERT_STR(got, exp) \
	lft_assert_str(__FILE__, __LINE__, #got, (got), (exp))
# define ASSERT_STR_AS(text, got, exp) \
	lft_assert_str(__FILE__, __LINE__, (text), (got), (exp))
# define ASSERT_MEM(got, exp, n) \
	lft_assert_mem(__FILE__, __LINE__, #got, (got), (exp), (n))
# define ASSERT_PTR(got, exp) \
	lft_assert_ptr(__FILE__, __LINE__, #got, (got), (exp))
# define ASSERT_NULL(got) \
	lft_assert_ptr(__FILE__, __LINE__, #got, (got), NULL)
# define ASSERT_SIGN(got, ref) \
	lft_assert_sign(__FILE__, __LINE__, #got, (long long)(got), (long long)(ref))
# define ASSERT_NOT_NULL(got) \
	do { \
		if ((got) == NULL) \
			FAILF("%s : NULL, un pointeur attendu", #got); \
	} while (0)

/* ----------------------------- Mémoire ----------------------------- */

/* Bloc du test (contenu de liste, résultat d'un f...). Il compte pour les
** fuites et se libère avec free(), mais n'est jamais visé par les échecs. */
void		*lft_alloc(size_t size);
long		lft_malloc_calls(void);
size_t		lft_last_malloc_size(void);
long		lft_live_blocks(void);

/* Zones protégées : l'octet juste après (ou juste avant pour _front) provoque
** un statut OOB. Les zones _str et _mem sont en lecture seule. */
char		*lft_guard_buf(size_t n);
char		*lft_guard_buf_front(size_t n);
const char	*lft_guard_str(const char *s);
const char	*lft_guard_str_front(const char *s);
const char	*lft_guard_mem(const void *src, size_t n);
void		lft_guard_free(const void *p);

/* Rejoue le bloc une fois pour compter les malloc, puis une fois par malloc en
** le faisant échouer. Les fuites sont vérifiées à la fin de chaque passage. */
void		lft_mf_begin(void);
int			lft_mf_next(void);
int			lft_mf_injected(void);

# define MALLOC_FAIL_LOOP for (lft_mf_begin(); lft_mf_next(); )

/* Fichier en mémoire pour lire ce que les ft_put*_fd écrivent. Pas de limite
** de taille contrairement à un pipe, donc jamais de blocage. */
int			lft_capture_begin(void);
size_t		lft_capture_end(int fd, char *buf, size_t size);

/* ---------------------------- Aléatoire ---------------------------- */

/* Générateur propre à chaque test, reproductible avec --seed. */
unsigned int	lft_rand(void);
unsigned int	lft_seed(void);

/* ------------------------ Fonctions testées ------------------------ */

typedef struct s_list
{
	void			*content;
	struct s_list	*next;
}	t_list;

int				ft_isalpha(int c);
int				ft_isdigit(int c);
int				ft_isalnum(int c);
int				ft_isascii(int c);
int				ft_isprint(int c);
size_t			ft_strlen(const char *s);
void			*ft_memset(void *s, int c, size_t n);
void			ft_bzero(void *s, size_t n);
void			*ft_memcpy(void *dest, const void *src, size_t n);
void			*ft_memmove(void *dest, const void *src, size_t n);
size_t			ft_strlcpy(char *dst, const char *src, size_t size);
size_t			ft_strlcat(char *dst, const char *src, size_t size);
int				ft_toupper(int c);
int				ft_tolower(int c);
char			*ft_strchr(const char *s, int c);
char			*ft_strrchr(const char *s, int c);
int				ft_strncmp(const char *s1, const char *s2, size_t n);
void			*ft_memchr(const void *s, int c, size_t n);
int				ft_memcmp(const void *s1, const void *s2, size_t n);
char			*ft_strnstr(const char *big, const char *little, size_t len);
int				ft_atoi(const char *nptr);
void			*ft_calloc(size_t nmemb, size_t size);
char			*ft_strdup(const char *s);

char			*ft_substr(char const *s, unsigned int start, size_t len);
char			*ft_strjoin(char const *s1, char const *s2);
char			*ft_strtrim(char const *s1, char const *set);
char			**ft_split(char const *s, char c);
char			*ft_itoa(int n);
char			*ft_strmapi(char const *s, char (*f)(unsigned int, char));
void			ft_striteri(char *s, void (*f)(unsigned int, char *));
void			ft_putchar_fd(char c, int fd);
void			ft_putstr_fd(char *s, int fd);
void			ft_putendl_fd(char *s, int fd);
void			ft_putnbr_fd(int n, int fd);

t_list			*ft_lstnew(void *content);
void			ft_lstadd_front(t_list **lst, t_list *new);
unsigned int	ft_lstsize(t_list *lst);
t_list			*ft_lstlast(t_list *lst);
void			ft_lstadd_back(t_list **lst, t_list *new);
void			ft_lstdelone(t_list *lst, void (*del)(void *));
void			ft_lstclear(t_list **lst, void (*del)(void *));
void			ft_lstiter(t_list *lst, void (*f)(void *));
t_list			*ft_lstmap(t_list *lst, void *(*f)(void *), void (*del)(void *));

#endif
