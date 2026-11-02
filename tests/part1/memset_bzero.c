/* ft_memset, ft_bzero. */
#include <string.h>
#include "lft_test.h"

TEST(memset, basic)
{
	char	buf[10];
	char	want[10];

	memset(want, 'A', 10);
	ft_memset(buf, 'A', 10);
	ASSERT_MEM(buf, want, 10);
}

TEST(memset, zero)
{
	char	buf[5] = {1, 2, 3, 4, 5};
	char	want[5] = {0};

	ft_memset(buf, 0, 5);
	ASSERT_MEM(buf, want, 5);
}

TEST(memset, partial)
{
	char	buf[10] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j'};

	ft_memset(buf + 2, 'X', 5);
	ASSERT_MEM(buf, "abXXXXXhij", 10);
}

TEST(memset, size_zero)
{
	char	buf[5] = "test";

	ASSERT_PTR(ft_memset(buf, 'X', 0), buf);
	ASSERT_STR(buf, "test");
}

TEST(memset, size_zero_touches_nothing)
{
	char	*buf = lft_guard_buf(0);

	ASSERT_PTR(ft_memset(buf, 'X', 0), buf);
}

TEST(memset, return_value)
{
	char	buf[10];

	ASSERT_PTR(ft_memset(buf, 'A', 10), buf);
}

TEST(memset, large)
{
	char	buf[1000];
	char	want[1000];

	memset(want, 42, 1000);
	ft_memset(buf, 42, 1000);
	ASSERT_MEM(buf, want, 1000);
}

TEST(memset, negative_char)
{
	unsigned char	buf[5];
	int				i;

	ft_memset(buf, -1, 5);
	for (i = 0; i < 5; i++)
		ASSERT_EQ(buf[i], 255);
}

TEST(memset, value_is_converted_to_unsigned_char)
{
	unsigned char	buf[4];

	ft_memset(buf, 256 + 'A', 4);
	ASSERT_MEM(buf, "AAAA", 4);
	ft_memset(buf, 0x1ff, 4);
	ASSERT_EQ(buf[3], 0xff);
	ft_memset(buf, 1024, 4);
	ASSERT_EQ(buf[0], 0);
}

TEST(memset, stays_inside_the_range)
{
	size_t	n;
	char	*end;
	char	*front;
	size_t	i;

	for (n = 0; n <= 32; n++)
	{
		end = lft_guard_buf(n);
		front = lft_guard_buf_front(n);
		ft_memset(end, 'x', n);
		ft_memset(front, 'x', n);
		for (i = 0; i < n; i++)
		{
			ASSERT_EQ(end[i], 'x');
			ASSERT_EQ(front[i], 'x');
		}
		lft_guard_free(end);
		lft_guard_free(front);
	}
}

TEST(memset, int_array)
{
	int	values[4] = {1, 2, 3, 4};
	int	i;

	ft_memset(values, 0, sizeof(values));
	for (i = 0; i < 4; i++)
		ASSERT_EQ(values[i], 0);
}

TEST(bzero, basic)
{
	char	buf[10] = "abcdefghi";
	char	want[10] = {0};

	ft_bzero(buf, 10);
	ASSERT_MEM(buf, want, 10);
}

TEST(bzero, partial)
{
	char	buf[10] = "abcdefghi";
	char	want[10] = {'a', 'b', 'c', 0, 0, 0, 0, 'h', 'i', 0};

	ft_bzero(buf + 3, 4);
	ASSERT_MEM(buf, want, 10);
}

TEST(bzero, size_zero)
{
	char	buf[5] = "test";

	ft_bzero(buf, 0);
	ASSERT_STR(buf, "test");
}

TEST(bzero, size_zero_touches_nothing)
{
	ft_bzero(lft_guard_buf(0), 0);
}

TEST(bzero, single_byte)
{
	char	buf[5] = "test";

	ft_bzero(buf, 1);
	ASSERT_EQ(buf[0], 0);
	ASSERT_EQ(buf[1], 'e');
}

TEST(bzero, large)
{
	char	buf[1000];
	int		i;

	memset(buf, 'A', 1000);
	ft_bzero(buf, 1000);
	for (i = 0; i < 1000; i++)
		ASSERT_EQ(buf[i], 0);
}

TEST(bzero, struct)
{
	struct { int a; char b; double c; }	s;
	char								want[sizeof(s)];

	memset(&s, 0x5a, sizeof(s));
	memset(want, 0, sizeof(want));
	ft_bzero(&s, sizeof(s));
	ASSERT_MEM(&s, want, sizeof(s));
}

TEST(bzero, stays_inside_the_range)
{
	size_t	n;
	char	*end;
	char	*front;
	size_t	i;

	for (n = 0; n <= 32; n++)
	{
		end = lft_guard_buf(n);
		front = lft_guard_buf_front(n);
		ft_bzero(end, n);
		ft_bzero(front, n);
		for (i = 0; i < n; i++)
		{
			ASSERT_EQ(end[i], 0);
			ASSERT_EQ(front[i], 0);
		}
		lft_guard_free(end);
		lft_guard_free(front);
	}
}
