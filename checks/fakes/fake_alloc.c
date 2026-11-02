/* Faux tests : suivi des blocs alloués pendant un test. */
#include <stdlib.h>
#include "lft_test.h"

TEST(alloc, clean)
{
	char	*p = malloc(10);

	ASSERT_NOT_NULL(p);
	free(p);
}

TEST(alloc, leak)
{
	char	*p = malloc(10);

	ASSERT_NOT_NULL(p);
}

TEST(alloc, leak_two_blocks)
{
	ASSERT_NOT_NULL(malloc(10));
	ASSERT_NOT_NULL(malloc(5));
}

TEST(alloc, double_free)
{
	char	*p = malloc(10);

	free(p);
	free(p);
}

TEST(alloc, free_inside_block)
{
	char	*p = malloc(10);

	free(p + 1);
}

TEST(alloc, free_stack)
{
	char			c;
	char *volatile	p = &c;

	free(p);
}

TEST(alloc, free_null_is_fine)
{
	free(NULL);
}

TEST(alloc, test_block_counts_for_leaks)
{
	ASSERT_NOT_NULL(lft_alloc(4));
}

TEST(alloc, test_block_can_be_freed)
{
	char	*p = lft_alloc(4);

	ASSERT_NOT_NULL(p);
	free(p);
}

TEST(alloc, counters)
{
	char	*a = malloc(3);
	char	*b = malloc(7);
	char	*c = lft_alloc(99);

	ASSERT_EQ(lft_malloc_calls(), 2);
	ASSERT_EQ(lft_last_malloc_size(), 7);
	ASSERT_EQ(lft_live_blocks(), 3);
	free(a);
	free(b);
	free(c);
	ASSERT_EQ(lft_live_blocks(), 0);
}

TEST(alloc, memory_is_not_zeroed)
{
	unsigned char	*p = malloc(16);
	int				i;

	for (i = 0; i < 16; i++)
		ASSERT_TRUE(p[i] != 0);
	free(p);
}

TEST(alloc, zero_size_is_unique_and_freeable)
{
	char	*a = malloc(0);
	char	*b = malloc(0);

	ASSERT_NOT_NULL(a);
	ASSERT_NOT_NULL(b);
	ASSERT_TRUE(a != b);
	free(a);
	free(b);
}

TEST(alloc, huge_size_returns_null)
{
	{ volatile size_t huge = SIZE_MAX; ASSERT_NULL(malloc(huge)); ASSERT_NULL(malloc(huge / 2)); }
	ASSERT_EQ(lft_live_blocks(), 0);
}

TEST(alloc, many_blocks)
{
	static char	*p[20000];
	int			i;

	for (i = 0; i < 20000; i++)
	{
		p[i] = malloc((size_t)(i % 32));
		ASSERT_NOT_NULL(p[i]);
	}
	for (i = 0; i < 20000; i++)
		free(p[i]);
}

TEST(alloc, many_cycles_do_not_exhaust_mappings)
{
	long	i;
	char	*p;

	for (i = 0; i < 100000; i++)
	{
		p = malloc(8);
		ASSERT_NOT_NULL(p);
		free(p);
	}
}
