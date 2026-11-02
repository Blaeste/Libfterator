/* Partie 3 : listes chaînées.
** Les listes de test sont construites sans ft_lstnew pour qu'un bug dans une
** fonction ne fasse pas échouer les tests de toutes les autres. */
#include <stdlib.h>
#include <string.h>
#include "lft_test.h"

static int		g_del_calls;
static void		*g_del_args[32];
static int		g_f_calls;
static void		*g_f_args[32];

static t_list	*node(void *content)
{
	t_list	*n = lft_alloc(sizeof(*n));

	n->content = content;
	n->next = NULL;
	return (n);
}

/* Liste de n nœuds dont le contenu pointe sur values[i]. */
static t_list	*chain(int n, int *values)
{
	t_list	*head = NULL;
	t_list	*cur;

	while (n-- > 0)
	{
		cur = node(values ? &values[n] : NULL);
		cur->next = head;
		head = cur;
	}
	return (head);
}

static void	free_nodes(t_list *lst)
{
	t_list	*next;

	while (lst)
	{
		next = lst->next;
		free(lst);
		lst = next;
	}
}

static void	del_count(void *content)
{
	if (g_del_calls < 32)
		g_del_args[g_del_calls] = content;
	g_del_calls++;
}

static void	del_free(void *content)
{
	del_count(content);
	free(content);
}

static void	f_count(void *content)
{
	if (g_f_calls < 32)
		g_f_args[g_f_calls] = content;
	g_f_calls++;
}

static void	f_double_value(void *content)
{
	*(int *)content *= 2;
}

static char	*own_string(const char *s)
{
	char	*copy = lft_alloc(strlen(s) + 1);

	strcpy(copy, s);
	return (copy);
}

/* -------------------------------- lstnew -------------------------------- */

TEST(lstnew, basic)
{
	char	*content = "Hello";
	t_list	*n = ft_lstnew(content);

	ASSERT_NOT_NULL(n);
	ASSERT_PTR(n->content, content);
	ASSERT_NULL(n->next);
	free(n);
}

TEST(lstnew, null)
{
	t_list	*n = ft_lstnew(NULL);

	ASSERT_NOT_NULL(n);
	ASSERT_NULL(n->content);
	ASSERT_NULL(n->next);
	free(n);
}

TEST(lstnew, content_is_not_copied)
{
	int		value = 42;
	t_list	*n = ft_lstnew(&value);

	ASSERT_NOT_NULL(n);
	value = 7;
	ASSERT_EQ(*(int *)n->content, 7);
	free(n);
}

TEST(lstnew, allocates_exactly_one_node)
{
	t_list	*n = ft_lstnew("x");

	ASSERT_NOT_NULL(n);
	ASSERT_EQ(lft_malloc_calls(), 1);
	free(n);
}

TEST(lstnew, nodes_are_independent)
{
	t_list	*a = ft_lstnew("a");
	t_list	*b = ft_lstnew("b");

	ASSERT_NOT_NULL(a);
	ASSERT_NOT_NULL(b);
	ASSERT_TRUE(a != b);
	free(a);
	free(b);
}

TEST(lstnew, malloc_failure_returns_null)
{
	MALLOC_FAIL_LOOP
	{
		t_list	*n = ft_lstnew("x");

		if (lft_mf_injected())
			ASSERT_NULL(n);
		else
			free(n);
	}
}

/* ----------------------------- lstadd_front ----------------------------- */

TEST(lstadd_front, basic)
{
	t_list	*lst = node("World");
	t_list	*first = node("Hello");
	t_list	*old = lst;

	ft_lstadd_front(&lst, first);
	ASSERT_PTR(lst, first);
	ASSERT_PTR(lst->next, old);
	ASSERT_NULL(lst->next->next);
	free_nodes(lst);
}

TEST(lstadd_front, empty)
{
	t_list	*lst = NULL;
	t_list	*first = node("First");

	ft_lstadd_front(&lst, first);
	ASSERT_PTR(lst, first);
	ASSERT_NULL(lst->next);
	free_nodes(lst);
}

TEST(lstadd_front, order_after_many_calls)
{
	int		values[5] = {0, 1, 2, 3, 4};
	t_list	*lst = NULL;
	t_list	*cur;
	int		i;

	for (i = 0; i < 5; i++)
		ft_lstadd_front(&lst, node(&values[i]));
	cur = lst;
	for (i = 4; i >= 0; i--)
	{
		ASSERT_PTR(cur->content, &values[i]);
		cur = cur->next;
	}
	ASSERT_NULL(cur);
	free_nodes(lst);
}

TEST(lstadd_front, allocates_nothing)
{
	t_list	*lst = node("a");

	ft_lstadd_front(&lst, node("b"));
	ASSERT_EQ(lft_malloc_calls(), 0);
	free_nodes(lst);
}

/* -------------------------------- lstsize ------------------------------- */

TEST(lstsize, basic)
{
	t_list	*lst = chain(3, NULL);

	ASSERT_EQ(ft_lstsize(lst), 3);
	free_nodes(lst);
}

TEST(lstsize, empty)
{
	ASSERT_EQ(ft_lstsize(NULL), 0);
}

TEST(lstsize, single)
{
	t_list	*lst = chain(1, NULL);

	ASSERT_EQ(ft_lstsize(lst), 1);
	free_nodes(lst);
}

TEST(lstsize, ten_thousand_nodes)
{
	t_list	*lst = chain(10000, NULL);

	ASSERT_EQ(ft_lstsize(lst), 10000);
	free_nodes(lst);
}

TEST(lstsize, does_not_change_the_list)
{
	t_list	*lst = chain(3, NULL);
	t_list	*second = lst->next;

	ft_lstsize(lst);
	ASSERT_PTR(lst->next, second);
	ASSERT_NOT_NULL(second->next);
	free_nodes(lst);
}

/* -------------------------------- lstlast ------------------------------- */

TEST(lstlast, basic)
{
	t_list	*lst = chain(3, NULL);

	ASSERT_PTR(ft_lstlast(lst), lst->next->next);
	free_nodes(lst);
}

TEST(lstlast, single)
{
	t_list	*lst = chain(1, NULL);

	ASSERT_PTR(ft_lstlast(lst), lst);
	free_nodes(lst);
}

TEST(lstlast, null)
{
	ASSERT_NULL(ft_lstlast(NULL));
}

TEST(lstlast, does_not_change_the_list)
{
	t_list	*lst = chain(3, NULL);
	t_list	*second = lst->next;

	ft_lstlast(lst);
	ASSERT_PTR(lst->next, second);
	free_nodes(lst);
}

/* ------------------------------ lstadd_back ----------------------------- */

TEST(lstadd_back, basic)
{
	t_list	*lst = node("First");
	t_list	*head = lst;
	t_list	*last = node("Last");

	ft_lstadd_back(&lst, last);
	ASSERT_PTR(lst, head);
	ASSERT_PTR(lst->next, last);
	ASSERT_NULL(last->next);
	free_nodes(lst);
}

TEST(lstadd_back, empty)
{
	t_list	*lst = NULL;
	t_list	*first = node("First");

	ft_lstadd_back(&lst, first);
	ASSERT_PTR(lst, first);
	ASSERT_NULL(lst->next);
	free_nodes(lst);
}

TEST(lstadd_back, order_after_many_calls)
{
	int		values[5] = {0, 1, 2, 3, 4};
	t_list	*lst = NULL;
	t_list	*cur;
	int		i;

	for (i = 0; i < 5; i++)
		ft_lstadd_back(&lst, node(&values[i]));
	cur = lst;
	for (i = 0; i < 5; i++)
	{
		ASSERT_PTR(cur->content, &values[i]);
		cur = cur->next;
	}
	ASSERT_NULL(cur);
	free_nodes(lst);
}

TEST(lstadd_back, keeps_the_nodes_after_new)
{
	t_list	*lst = chain(2, NULL);
	t_list	*tail = chain(2, NULL);

	ft_lstadd_back(&lst, tail);
	ASSERT_EQ(ft_lstsize(lst), 4);
	ASSERT_PTR(lst->next->next, tail);
	free_nodes(lst);
}

TEST(lstadd_back, allocates_nothing)
{
	t_list	*lst = node("a");

	ft_lstadd_back(&lst, node("b"));
	ASSERT_EQ(lft_malloc_calls(), 0);
	free_nodes(lst);
}

/* ------------------------------- lstdelone ------------------------------ */

TEST(lstdelone, basic)
{
	char	*content = own_string("Hello");
	t_list	*n = node(content);

	ft_lstdelone(n, del_free);
	ASSERT_EQ(g_del_calls, 1);
}

TEST(lstdelone, calls_del_once_on_the_content)
{
	int		value = 1;
	t_list	*n = node(&value);

	ft_lstdelone(n, del_count);
	ASSERT_EQ(g_del_calls, 1);
	ASSERT_PTR(g_del_args[0], &value);
}

TEST(lstdelone, does_not_free_the_next_node)
{
	int		value = 1;
	t_list	*second = node(&value);
	t_list	*first = node(&value);

	first->next = second;
	ft_lstdelone(first, del_count);
	ASSERT_EQ(g_del_calls, 1);
	ASSERT_EQ(lft_live_blocks(), 1);
	ASSERT_PTR(second->content, &value);
	free_nodes(second);
}

/* Une fuite sera vue par le contrôle de fin de test. */
TEST(lstdelone, frees_the_node_itself)
{
	t_list	*n = node(NULL);

	ft_lstdelone(n, del_count);
}

/* ------------------------------- lstclear ------------------------------- */

TEST(lstclear, basic)
{
	t_list	*lst = chain(3, NULL);

	ft_lstclear(&lst, del_count);
	ASSERT_NULL(lst);
}

TEST(lstclear, calls_del_on_every_content)
{
	int		values[4] = {1, 2, 3, 4};
	t_list	*lst = chain(4, values);
	int		i;
	int		seen[4] = {0};

	ft_lstclear(&lst, del_count);
	ASSERT_EQ(g_del_calls, 4);
	for (i = 0; i < 4; i++)
	{
		ASSERT_TRUE((int *)g_del_args[i] >= values && (int *)g_del_args[i] < values + 4);
		seen[(int *)g_del_args[i] - values]++;
	}
	for (i = 0; i < 4; i++)
		ASSERT_EQ(seen[i], 1);
}

TEST(lstclear, frees_the_contents_through_del)
{
	t_list	*lst = node(own_string("a"));

	lst->next = node(own_string("b"));
	ft_lstclear(&lst, del_free);
	ASSERT_NULL(lst);
	ASSERT_EQ(g_del_calls, 2);
}

TEST(lstclear, empty_list)
{
	t_list	*lst = NULL;

	ft_lstclear(&lst, del_count);
	ASSERT_NULL(lst);
	ASSERT_EQ(g_del_calls, 0);
}

TEST(lstclear, single_node)
{
	t_list	*lst = chain(1, NULL);

	ft_lstclear(&lst, del_count);
	ASSERT_NULL(lst);
	ASSERT_EQ(g_del_calls, 1);
}

TEST(lstclear, from_the_middle_keeps_the_nodes_before)
{
	t_list	*lst = chain(3, NULL);
	t_list	*keep = lst;

	ft_lstclear(&lst->next, del_count);
	ASSERT_NULL(keep->next);
	ASSERT_EQ(g_del_calls, 2);
	ASSERT_EQ(lft_live_blocks(), 1);
	free_nodes(keep);
}

TEST(lstclear, ten_thousand_nodes)
{
	t_list	*lst = chain(10000, NULL);

	ft_lstclear(&lst, del_count);
	ASSERT_NULL(lst);
	ASSERT_EQ(g_del_calls, 10000);
}

/* -------------------------------- lstiter ------------------------------- */

TEST(lstiter, basic)
{
	t_list	*lst = chain(3, NULL);

	ft_lstiter(lst, f_count);
	ASSERT_EQ(g_f_calls, 3);
	free_nodes(lst);
}

TEST(lstiter, empty)
{
	ft_lstiter(NULL, f_count);
	ASSERT_EQ(g_f_calls, 0);
}

TEST(lstiter, order_and_content)
{
	int		values[4] = {1, 2, 3, 4};
	t_list	*lst = chain(4, values);
	int		i;

	ft_lstiter(lst, f_count);
	ASSERT_EQ(g_f_calls, 4);
	for (i = 0; i < 4; i++)
		ASSERT_PTR(g_f_args[i], &values[i]);
	free_nodes(lst);
}

TEST(lstiter, f_can_change_the_content)
{
	int		values[3] = {1, 2, 3};
	t_list	*lst = chain(3, values);

	ft_lstiter(lst, f_double_value);
	ASSERT_EQ(values[0], 2);
	ASSERT_EQ(values[1], 4);
	ASSERT_EQ(values[2], 6);
	free_nodes(lst);
}

TEST(lstiter, does_not_change_the_links)
{
	t_list	*lst = chain(3, NULL);
	t_list	*second = lst->next;
	t_list	*third = second->next;

	ft_lstiter(lst, f_count);
	ASSERT_PTR(lst->next, second);
	ASSERT_PTR(second->next, third);
	ASSERT_NULL(third->next);
	free_nodes(lst);
}

/* -------------------------------- lstmap -------------------------------- */

static int	g_map_calls;

static void	*map_own_string(void *content)
{
	g_map_calls++;
	return (own_string(content));
}

typedef struct s_box
{
	char	*inner;
}	t_box;

/* Contenu à deux blocs : un simple free() à la place de del laisse une fuite. */
static void	*map_box(void *content)
{
	t_box	*box = lft_alloc(sizeof(*box));

	g_map_calls++;
	box->inner = own_string(content);
	return (box);
}

static void	del_box(void *content)
{
	t_box	*box = content;

	del_count(content);
	free(box->inner);
	free(box);
}

static void	*map_same_pointer(void *content)
{
	g_map_calls++;
	return (content);
}

static t_list	*string_list(void)
{
	t_list	*lst = node("Hello");

	lst->next = node("World");
	lst->next->next = node("!");
	return (lst);
}

TEST(lstmap, basic)
{
	t_list	*lst = string_list();
	t_list	*r = ft_lstmap(lst, map_own_string, del_free);

	ASSERT_NOT_NULL(r);
	ASSERT_STR(r->content, "Hello");
	ASSERT_STR(r->next->content, "World");
	ASSERT_STR(r->next->next->content, "!");
	ASSERT_NULL(r->next->next->next);
	ASSERT_EQ(g_del_calls, 0);
	ft_lstclear(&r, del_free);
	free_nodes(lst);
}

TEST(lstmap, empty)
{
	ASSERT_NULL(ft_lstmap(NULL, map_same_pointer, del_count));
	ASSERT_EQ(g_map_calls, 0);
	ASSERT_EQ(g_del_calls, 0);
}

TEST(lstmap, single_node)
{
	t_list	*lst = node("only");
	t_list	*r = ft_lstmap(lst, map_own_string, del_free);

	ASSERT_NOT_NULL(r);
	ASSERT_STR(r->content, "only");
	ASSERT_NULL(r->next);
	ft_lstclear(&r, del_free);
	free_nodes(lst);
}

TEST(lstmap, new_list_uses_new_nodes)
{
	t_list	*lst = string_list();
	t_list	*r = ft_lstmap(lst, map_same_pointer, del_count);

	ASSERT_NOT_NULL(r);
	ASSERT_TRUE(r != lst);
	ASSERT_TRUE(r->next != lst->next);
	ASSERT_PTR(r->content, lst->content);
	free_nodes(r);
	free_nodes(lst);
}

TEST(lstmap, original_list_is_unchanged)
{
	t_list	*lst = string_list();
	t_list	*second = lst->next;
	t_list	*third = second->next;
	t_list	*r = ft_lstmap(lst, map_own_string, del_free);

	ASSERT_NOT_NULL(r);
	ASSERT_PTR(lst->next, second);
	ASSERT_PTR(second->next, third);
	ASSERT_NULL(third->next);
	ASSERT_STR(lst->content, "Hello");
	ft_lstclear(&r, del_free);
	free_nodes(lst);
}

TEST(lstmap, f_is_called_once_per_node_in_order)
{
	int		values[4] = {1, 2, 3, 4};
	t_list	*lst = chain(4, values);
	t_list	*r = ft_lstmap(lst, map_same_pointer, del_count);
	t_list	*cur = r;
	int		i;

	ASSERT_EQ(g_map_calls, 4);
	for (i = 0; i < 4; i++)
	{
		ASSERT_NOT_NULL(cur);
		ASSERT_PTR(cur->content, &values[i]);
		cur = cur->next;
	}
	ASSERT_NULL(cur);
	free_nodes(r);
	free_nodes(lst);
}

TEST(lstmap, del_is_not_called_on_success)
{
	t_list	*lst = string_list();
	t_list	*r = ft_lstmap(lst, map_same_pointer, del_count);

	ASSERT_EQ(g_del_calls, 0);
	free_nodes(r);
	free_nodes(lst);
}

TEST(lstmap, malloc_failure_frees_everything_through_del)
{
	t_list	*lst = string_list();

	MALLOC_FAIL_LOOP
	{
		t_list	*r;
		t_list	*next;

		g_map_calls = 0;
		g_del_calls = 0;
		r = ft_lstmap(lst, map_box, del_box);
		if (lft_mf_injected())
		{
			ASSERT_NULL(r);
			if (g_del_calls != g_map_calls)
				FAILF("f a créé %d contenu(s), del en a supprimé %d : chaque "
					"contenu doit passer par del", g_map_calls, g_del_calls);
		}
		else
		{
			while (r)
			{
				next = r->next;
				del_box(r->content);
				free(r);
				r = next;
			}
		}
	}
	free_nodes(lst);
}

TEST(lstmap, malloc_failure_leaves_the_original_list_intact)
{
	t_list	*lst = string_list();

	MALLOC_FAIL_LOOP
	{
		t_list	*r = ft_lstmap(lst, map_own_string, del_free);

		if (lft_mf_injected())
			ASSERT_NULL(r);
		else
			ft_lstclear(&r, del_free);
		ASSERT_EQ(ft_lstsize(lst), 3);
		ASSERT_STR(lst->content, "Hello");
	}
	free_nodes(lst);
}

/* ------------------------------ Scénario ------------------------------- */

TEST(lstclear, complex_operations)
{
	t_list	*lst = NULL;
	char	text[2];
	int		i;

	text[1] = '\0';
	for (i = 0; i < 5; i++)
	{
		text[0] = (char)('0' + i);
		ft_lstadd_back(&lst, ft_lstnew(own_string(text)));
	}
	ASSERT_EQ(ft_lstsize(lst), 5);
	ASSERT_STR(ft_lstlast(lst)->content, "4");
	ft_lstadd_front(&lst, ft_lstnew(own_string("X")));
	ASSERT_EQ(ft_lstsize(lst), 6);
	ASSERT_STR(lst->content, "X");
	ft_lstclear(&lst, del_free);
	ASSERT_NULL(lst);
	ASSERT_EQ(g_del_calls, 6);
}
