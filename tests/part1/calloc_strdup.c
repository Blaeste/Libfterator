/* ft_calloc, ft_strdup. */
#include <stdlib.h>
#include <string.h>
#include "lft_test.h"

TEST(calloc, basic)
{
	int	*p = ft_calloc(10, sizeof(int));
	int	i;

	ASSERT_NOT_NULL(p);
	for (i = 0; i < 10; i++)
		ASSERT_EQ(p[i], 0);
	free(p);
}

TEST(calloc, single_element)
{
	int	*p = ft_calloc(1, sizeof(int));

	ASSERT_NOT_NULL(p);
	ASSERT_EQ(*p, 0);
	free(p);
}

TEST(calloc, char_array)
{
	char	*p = ft_calloc(100, sizeof(char));
	int		i;

	ASSERT_NOT_NULL(p);
	for (i = 0; i < 100; i++)
		ASSERT_EQ(p[i], 0);
	free(p);
}

TEST(calloc, large_allocation)
{
	int	*p = ft_calloc(1000, sizeof(int));
	int	i;

	ASSERT_NOT_NULL(p);
	for (i = 0; i < 1000; i++)
		ASSERT_EQ(p[i], 0);
	free(p);
}

TEST(calloc, struct_allocation)
{
	struct s_v { int x; char y; double z; }	*p;
	int										i;

	p = ft_calloc(5, sizeof(*p));
	ASSERT_NOT_NULL(p);
	for (i = 0; i < 5; i++)
	{
		ASSERT_EQ(p[i].x, 0);
		ASSERT_EQ(p[i].y, 0);
		ASSERT_TRUE(p[i].z == 0.0);
	}
	free(p);
}

TEST(calloc, write_and_read)
{
	int	*p = ft_calloc(10, sizeof(int));
	int	i;

	ASSERT_NOT_NULL(p);
	for (i = 0; i < 10; i++)
		p[i] = i * 2;
	for (i = 0; i < 10; i++)
		ASSERT_EQ(p[i], i * 2);
	free(p);
}

TEST(calloc, every_byte_is_zero_for_every_size)
{
	size_t			n;
	size_t			i;
	unsigned char	*p;

	for (n = 1; n <= 64; n++)
	{
		p = ft_calloc(n, 1);
		ASSERT_NOT_NULL(p);
		for (i = 0; i < n; i++)
			ASSERT_EQ(p[i], 0);
		free(p);
	}
}

TEST(calloc, zero_count)
{
	void	*p = ft_calloc(0, sizeof(int));

	ASSERT_NOT_NULL(p);
	free(p);
}

TEST(calloc, zero_size)
{
	void	*p = ft_calloc(10, 0);

	ASSERT_NOT_NULL(p);
	free(p);
}

TEST(calloc, both_zero)
{
	void	*p = ft_calloc(0, 0);

	ASSERT_NOT_NULL(p);
	free(p);
}

TEST(calloc, zero_returns_a_unique_pointer)
{
	void	*a = ft_calloc(0, 1);
	void	*b = ft_calloc(1, 0);
	void	*c = ft_calloc(0, 0);

	ASSERT_NOT_NULL(a);
	ASSERT_NOT_NULL(b);
	ASSERT_NOT_NULL(c);
	ASSERT_TRUE(a != b);
	ASSERT_TRUE(a != c);
	ASSERT_TRUE(b != c);
	free(a);
	free(b);
	free(c);
}

TEST(calloc, zero_with_a_huge_other_argument)
{
	void	*a = ft_calloc(0, SIZE_MAX);
	void	*b = ft_calloc(SIZE_MAX, 0);

	ASSERT_NOT_NULL(a);
	ASSERT_NOT_NULL(b);
	free(a);
	free(b);
}

TEST(calloc, overflow_returns_null)
{
	ASSERT_NULL(ft_calloc(SIZE_MAX, SIZE_MAX));
	ASSERT_NULL(ft_calloc(SIZE_MAX, 2));
	ASSERT_NULL(ft_calloc(2, SIZE_MAX));
}

TEST(calloc, overflow_wrapping_to_a_small_size_returns_null)
{
	ASSERT_NULL(ft_calloc(SIZE_MAX / 2 + 1, 2));
	ASSERT_NULL(ft_calloc((size_t)1 << 32, (size_t)1 << 32));
	ASSERT_NULL(ft_calloc(SIZE_MAX / 4 + 2, 4));
}

TEST(calloc, malloc_failure_returns_null)
{
	MALLOC_FAIL_LOOP
	{
		char	*p = ft_calloc(4, 4);

		if (lft_mf_injected())
			ASSERT_NULL(p);
		else
			free(p);
	}
}

TEST(calloc, independent_blocks)
{
	char	*a = ft_calloc(4, 1);
	char	*b = ft_calloc(4, 1);

	ASSERT_TRUE(a != b);
	a[0] = 'x';
	ASSERT_EQ(b[0], 0);
	free(a);
	free(b);
}

TEST(strdup, basic)
{
	char	*r = ft_strdup("Hello World");

	ASSERT_STR(r, "Hello World");
	free(r);
}

TEST(strdup, empty_string)
{
	char	*r = ft_strdup("");

	ASSERT_STR(r, "");
	free(r);
}

TEST(strdup, single_char)
{
	char	*r = ft_strdup("A");

	ASSERT_STR(r, "A");
	free(r);
}

TEST(strdup, long_string)
{
	const char	*s = "This is a very long string to test the strdup function "
		"implementation with various characters and spaces";
	char		*r = ft_strdup(s);

	ASSERT_STR(r, s);
	free(r);
}

TEST(strdup, special_chars)
{
	char	*r = ft_strdup("Hello\tWorld\n!@#$%^&*()");

	ASSERT_STR(r, "Hello\tWorld\n!@#$%^&*()");
	free(r);
}

TEST(strdup, independent_copy)
{
	char	s[] = "Hello World";
	char	*r = ft_strdup(s);

	ASSERT_NOT_NULL(r);
	ASSERT_TRUE(r != s);
	s[0] = 'X';
	ASSERT_EQ(r[0], 'H');
	r[6] = 'X';
	ASSERT_EQ(s[6], 'W');
	free(r);
}

TEST(strdup, null_terminator)
{
	char	*r = ft_strdup("Hello");

	ASSERT_NOT_NULL(r);
	ASSERT_EQ(r[5], '\0');
	ASSERT_EQ(strlen(r), 5);
	free(r);
}

TEST(strdup, numbers_and_symbols)
{
	const char	*s = "123456789!@#$%^&*()_+-=[]{}|;:,.<>?";
	char		*r = ft_strdup(s);

	ASSERT_STR(r, s);
	free(r);
}

TEST(strdup, high_bytes)
{
	char	*r = ft_strdup("\xff\x80\xc3\xa9");

	ASSERT_STR(r, "\xff\x80\xc3\xa9");
	free(r);
}

TEST(strdup, stops_at_first_nul)
{
	char	*r = ft_strdup("ab\0cd");

	ASSERT_STR(r, "ab");
	free(r);
}

TEST(strdup, reads_nothing_after_the_terminator)
{
	char	*r = ft_strdup(lft_guard_str("abc"));

	ASSERT_STR(r, "abc");
	free(r);
	r = ft_strdup(lft_guard_str(""));
	ASSERT_STR(r, "");
	free(r);
}

TEST(strdup, malloc_failure_returns_null)
{
	MALLOC_FAIL_LOOP
	{
		char	*r = ft_strdup("hello");

		if (lft_mf_injected())
			ASSERT_NULL(r);
		else
		{
			ASSERT_STR(r, "hello");
			free(r);
		}
	}
}
