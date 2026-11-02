/* Cas indéfinis ou laissés libres par le sujet : on montre ce que fait la libft,
** sans jamais compter le résultat. Un crash ici est affiché, pas sanctionné. */
#include <stdlib.h>
#include <string.h>
#include "lft_test.h"

static char	map_same(unsigned int i, char c)
{
	(void)i;
	return (c);
}

static void	iter_nothing(unsigned int i, char *c)
{
	(void)i;
	(void)c;
}

static void	del_nothing(void *content)
{
	(void)content;
}

static void	*f_null(void *content)
{
	(void)content;
	return (NULL);
}

#define SHOW_PTR(call) \
	do { \
		void	*lft_r = (void *)(call); \
		\
		INFO("retourne %s", lft_r ? "un pointeur" : "NULL"); \
	} while (0)

#define SHOW_VOID(call) \
	do { \
		call; \
		INFO("ne plante pas"); \
	} while (0)

/* Arguments NULL en partie 1 */

TEST(info, strlen_null)
{
	INFO("retourne %zu", ft_strlen(NULL));
}

TEST(info, strchr_null)
{
	SHOW_PTR(ft_strchr(NULL, 'a'));
}

TEST(info, strrchr_null)
{
	SHOW_PTR(ft_strrchr(NULL, 'a'));
}

TEST(info, strncmp_null)
{
	INFO("retourne %d", ft_strncmp(NULL, "test", 4));
}

TEST(info, memcpy_null_dest)
{
	SHOW_PTR(ft_memcpy(NULL, "test", 4));
}

TEST(info, memcpy_both_null)
{
	SHOW_PTR(ft_memcpy(NULL, NULL, 4));
}

TEST(info, memset_null)
{
	SHOW_PTR(ft_memset(NULL, 'A', 5));
}

TEST(info, memmove_null_dest)
{
	SHOW_PTR(ft_memmove(NULL, "test", 4));
}

TEST(info, memmove_both_null)
{
	SHOW_PTR(ft_memmove(NULL, NULL, 4));
}

TEST(info, atoi_null)
{
	INFO("retourne %d", ft_atoi(NULL));
}

TEST(info, strdup_null)
{
	SHOW_PTR(ft_strdup(NULL));
}

TEST(info, strnstr_null_haystack_len_zero)
{
	SHOW_PTR(ft_strnstr(NULL, "a", 0));
}

TEST(info, strlcpy_null_dest_size_zero)
{
	INFO("retourne %zu", ft_strlcpy(NULL, "abc", 0));
}

TEST(info, strlcat_null_dest_size_zero)
{
	INFO("retourne %zu", ft_strlcat(NULL, "abc", 0));
}

/* Arguments NULL en partie 2 */

TEST(info, substr_null)
{
	SHOW_PTR(ft_substr(NULL, 0, 5));
}

TEST(info, strjoin_null_first)
{
	SHOW_PTR(ft_strjoin(NULL, "World"));
}

TEST(info, strjoin_null_second)
{
	SHOW_PTR(ft_strjoin("Hello", NULL));
}

TEST(info, strjoin_both_null)
{
	SHOW_PTR(ft_strjoin(NULL, NULL));
}

TEST(info, strtrim_null_string)
{
	SHOW_PTR(ft_strtrim(NULL, " "));
}

TEST(info, strtrim_null_set)
{
	SHOW_PTR(ft_strtrim("  Hello  ", NULL));
}

TEST(info, split_null)
{
	SHOW_PTR(ft_split(NULL, ' '));
}

TEST(info, strmapi_null_string)
{
	SHOW_PTR(ft_strmapi(NULL, map_same));
}

TEST(info, strmapi_null_function)
{
	SHOW_PTR(ft_strmapi("hello", NULL));
}

TEST(info, striteri_null_string)
{
	SHOW_VOID(ft_striteri(NULL, iter_nothing));
}

TEST(info, striteri_null_function)
{
	char	s[] = "hello";

	SHOW_VOID(ft_striteri(s, NULL));
}

TEST(info, putstr_fd_null)
{
	SHOW_VOID(ft_putstr_fd(NULL, -1));
}

TEST(info, putendl_fd_null)
{
	SHOW_VOID(ft_putendl_fd(NULL, -1));
}

/* Arguments NULL en partie 3 */

TEST(info, lstadd_front_null_list)
{
	t_list	node = {NULL, NULL};

	SHOW_VOID(ft_lstadd_front(NULL, &node));
}

TEST(info, lstadd_front_null_new)
{
	t_list	*lst = NULL;

	SHOW_VOID(ft_lstadd_front(&lst, NULL));
}

TEST(info, lstadd_back_null_list)
{
	t_list	node = {NULL, NULL};

	SHOW_VOID(ft_lstadd_back(NULL, &node));
}

TEST(info, lstadd_back_null_new)
{
	t_list	*lst = NULL;

	SHOW_VOID(ft_lstadd_back(&lst, NULL));
}

TEST(info, lstdelone_null_node)
{
	SHOW_VOID(ft_lstdelone(NULL, del_nothing));
}

TEST(info, lstclear_null_pointer)
{
	SHOW_VOID(ft_lstclear(NULL, del_nothing));
}

TEST(info, lstiter_null_function)
{
	t_list	node = {NULL, NULL};

	SHOW_VOID(ft_lstiter(&node, NULL));
}

/* Comportements laissés libres */

TEST(info, atoi_overflow_positive)
{
	INFO("retourne %d", ft_atoi("999999999999999999999999999999999999999999999"));
}

TEST(info, atoi_overflow_negative)
{
	INFO("retourne %d", ft_atoi("-999999999999999999999999999999999999999999999"));
}

TEST(info, atoi_just_over_int_max)
{
	INFO("retourne %d", ft_atoi("2147483648"));
}

TEST(info, substr_allocation_for_a_huge_len)
{
	char	*r = ft_substr("hola", 0, 1000000);

	INFO("alloue %zu octets pour un résultat de 5 octets", lft_last_malloc_size());
	free(r);
}

TEST(info, strtrim_allocation)
{
	char	*r = ft_strtrim("     a     ", " ");

	INFO("alloue %zu octets pour un résultat de 2 octets", lft_last_malloc_size());
	free(r);
}

TEST(info, lstmap_when_f_returns_null)
{
	t_list	node = {"x", NULL};
	t_list	*r = ft_lstmap(&node, f_null, del_nothing);

	INFO("retourne %s", r ? "une liste" : "NULL");
}
