/* Faux tests : accès hors limites détectés à l'octet près. */
#include <stdlib.h>
#include "lft_test.h"

static volatile char	g_sink;

TEST(guard, block_in_bounds)
{
	char	*p = malloc(5);
	int		i;

	for (i = 0; i < 5; i++)
		p[i] = 'x';
	g_sink = p[4];
	free(p);
}

TEST(guard, block_write_one_past)
{
	char	*p = malloc(5);

	p[5] = 'x';
	free(p);
}

TEST(guard, block_read_one_past)
{
	char	*p = malloc(5);

	g_sink = p[5];
	free(p);
}

TEST(guard, block_write_before_start)
{
	char	*p = malloc(5);

	p[-1] = 'x';
	free(p);
}

TEST(guard, zero_size_access)
{
	char	*p = malloc(0);

	g_sink = p[0];
	free(p);
}

TEST(guard, use_after_free)
{
	char	*p = malloc(5);

	free(p);
	g_sink = p[0];
}

TEST(guard, buf_in_bounds)
{
	char	*p = lft_guard_buf(5);

	p[0] = 'a';
	p[4] = 'b';
	lft_guard_free(p);
}

TEST(guard, buf_write_one_past)
{
	char	*p = lft_guard_buf(5);

	p[5] = 'x';
}

TEST(guard, buf_front_write_before)
{
	char	*p = lft_guard_buf_front(5);

	p[-1] = 'x';
}

TEST(guard, buf_front_in_bounds)
{
	char	*p = lft_guard_buf_front(5);

	p[0] = 'a';
	p[4] = 'b';
	lft_guard_free(p);
}

TEST(guard, str_copy_is_correct)
{
	const char	*s = lft_guard_str("hello");
	const char	*f = lft_guard_str_front("hello");

	ASSERT_STR(s, "hello");
	ASSERT_STR(f, "hello");
	lft_guard_free(s);
	lft_guard_free(f);
}

TEST(guard, str_read_past_terminator)
{
	const char	*s = lft_guard_str("hi");

	g_sink = s[3];
}

TEST(guard, str_write_is_refused)
{
	char	*s = (char *)lft_guard_str("hi");

	s[0] = 'x';
}

TEST(guard, str_front_read_before)
{
	const char	*s = lft_guard_str_front("hi");

	g_sink = s[-1];
}

TEST(guard, mem_read_past)
{
	const char	*s = lft_guard_mem("abc", 3);

	g_sink = s[2];
	g_sink = s[3];
}

static void	recurse(volatile int depth)
{
	volatile char	pad[512];

	pad[0] = (char)depth;
	if (depth >= 0)
		recurse(depth + 1);
	g_sink = pad[0];
}

TEST(guard, stack_overflow_is_plain_crash)
{
	recurse(1);
}
