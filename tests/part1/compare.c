/* ft_strncmp, ft_memcmp : seul le signe du résultat est spécifié. */
#include <string.h>
#include "lft_test.h"

TEST(strncmp, basic_equal)
{
	ASSERT_EQ(ft_strncmp("Hello", "Hello", 5), 0);
}

TEST(strncmp, basic_different)
{
	ASSERT_SIGN(ft_strncmp("Hello", "World", 5), -1);
	ASSERT_SIGN(ft_strncmp("World", "Hello", 5), 1);
}

TEST(strncmp, partial_compare)
{
	ASSERT_EQ(ft_strncmp("Hello World", "Hello Universe", 5), 0);
	ASSERT_EQ(ft_strncmp("Hello World", "Hello Universe", 6), 0);
	ASSERT_SIGN(ft_strncmp("Hello World", "Hello Universe", 7), 1);
}

TEST(strncmp, zero_length)
{
	ASSERT_EQ(ft_strncmp("Hello", "World", 0), 0);
}

TEST(strncmp, zero_length_reads_nothing)
{
	ASSERT_EQ(ft_strncmp(lft_guard_buf(0), lft_guard_buf(0), 0), 0);
}

TEST(strncmp, empty_strings)
{
	ASSERT_EQ(ft_strncmp("", "", 1), 0);
}

TEST(strncmp, one_empty)
{
	ASSERT_SIGN(ft_strncmp("Hello", "", 3), 1);
	ASSERT_SIGN(ft_strncmp("", "Hello", 3), -1);
}

TEST(strncmp, different_lengths)
{
	ASSERT_EQ(ft_strncmp("Hello", "Hello World", 5), 0);
	ASSERT_SIGN(ft_strncmp("Hello", "Hello World", 10), -1);
	ASSERT_SIGN(ft_strncmp("Hello World", "Hello", 10), 1);
}

TEST(strncmp, case_sensitive)
{
	ASSERT_SIGN(ft_strncmp("Hello", "hello", 5), -1);
}

TEST(strncmp, unsigned_char)
{
	ASSERT_SIGN(ft_strncmp("\xff", "\x01", 1), 1);
	ASSERT_SIGN(ft_strncmp("\x01", "\xff", 1), -1);
	ASSERT_SIGN(ft_strncmp("\x80", "\x7f", 1), 1);
	ASSERT_SIGN(ft_strncmp("a\200", "a", 2), 1);
	ASSERT_SIGN(ft_strncmp("a", "a\200", 2), -1);
}

TEST(strncmp, stops_after_terminator)
{
	ASSERT_EQ(ft_strncmp("abc\0x", "abc\0y", 5), 0);
	ASSERT_EQ(ft_strncmp("", "\0a", 2), 0);
}

TEST(strncmp, difference_at_the_last_allowed_char)
{
	ASSERT_SIGN(ft_strncmp("abcd", "abce", 4), -1);
	ASSERT_EQ(ft_strncmp("abcd", "abce", 3), 0);
}

TEST(strncmp, reads_nothing_after_the_terminator)
{
	const char	*a = lft_guard_str("abc");
	const char	*b = lft_guard_str("abc");

	ASSERT_EQ(ft_strncmp(a, b, 4), 0);
	ASSERT_EQ(ft_strncmp(a, b, SIZE_MAX), 0);
	ASSERT_SIGN(ft_strncmp(a, lft_guard_str("ab"), 100), 1);
	ASSERT_SIGN(ft_strncmp(lft_guard_str("ab"), b, 100), -1);
}

TEST(strncmp, reads_exactly_n_chars)
{
	const char	*a = lft_guard_mem("abc", 3);
	const char	*b = lft_guard_mem("abc", 3);

	ASSERT_EQ(ft_strncmp(a, b, 3), 0);
}

TEST(strncmp, fuzz_against_libc)
{
	const char	*alphabet = "ab\x80\xff";
	char		sa[17];
	char		sb[17];
	const char	*a;
	const char	*b;
	int			round;
	size_t		i;
	size_t		n;

	for (round = 0; round < 3000; round++)
	{
		for (i = 0; i < 16; i++)
		{
			sa[i] = alphabet[lft_rand() % 4];
			sb[i] = lft_rand() % 4 ? sa[i] : alphabet[lft_rand() % 4];
		}
		sa[lft_rand() % 17] = '\0';
		sb[lft_rand() % 17] = '\0';
		sa[16] = '\0';
		sb[16] = '\0';
		a = lft_guard_str(sa);
		b = lft_guard_str(sb);
		n = lft_rand() % 20;
		ASSERT_SIGN(ft_strncmp(a, b, n), strncmp(a, b, n));
		lft_guard_free(a);
		lft_guard_free(b);
	}
}

TEST(memcmp, basic_equal)
{
	ASSERT_EQ(ft_memcmp("Hello", "Hello", 5), 0);
}

TEST(memcmp, basic_different)
{
	ASSERT_SIGN(ft_memcmp("Hello", "World", 5), -1);
	ASSERT_SIGN(ft_memcmp("World", "Hello", 5), 1);
}

TEST(memcmp, partial_equal)
{
	ASSERT_EQ(ft_memcmp("Hello World", "Hello Universe", 5), 0);
}

TEST(memcmp, zero_length)
{
	ASSERT_EQ(ft_memcmp("Hello", "World", 0), 0);
}

TEST(memcmp, zero_length_reads_nothing)
{
	ASSERT_EQ(ft_memcmp(lft_guard_mem("", 0), lft_guard_mem("", 0), 0), 0);
}

TEST(memcmp, binary_data)
{
	unsigned char	a[] = {0, 1, 2, 255, 254};
	unsigned char	b[] = {0, 1, 2, 255, 253};

	ASSERT_SIGN(ft_memcmp(a, b, 5), 1);
	ASSERT_SIGN(ft_memcmp(b, a, 5), -1);
	ASSERT_EQ(ft_memcmp(a, b, 4), 0);
}

TEST(memcmp, with_null_bytes)
{
	ASSERT_SIGN(ft_memcmp("Hello\0World", "Hello\0Universe", 12), 1);
}

TEST(memcmp, negative_bytes)
{
	char	a[] = {-1, 0};
	char	b[] = {1, 0};

	ASSERT_SIGN(ft_memcmp(a, b, 2), 1);
	ASSERT_SIGN(ft_memcmp(b, a, 2), -1);
}

TEST(memcmp, struct_data)
{
	struct s_v { int x; char y; }	a;
	struct s_v						b;

	memset(&a, 0, sizeof(a));
	memset(&b, 0, sizeof(b));
	a.x = 42;
	b.x = 42;
	a.y = 'A';
	b.y = 'B';
	ASSERT_SIGN(ft_memcmp(&a, &b, sizeof(a)), -1);
}

TEST(memcmp, large_data)
{
	char	a[1000];
	char	b[1000];

	memset(a, 'A', 1000);
	memset(b, 'A', 1000);
	ASSERT_EQ(ft_memcmp(a, b, 1000), 0);
	a[500] = 'B';
	ASSERT_SIGN(ft_memcmp(a, b, 1000), 1);
	ASSERT_EQ(ft_memcmp(a, b, 500), 0);
}

TEST(memcmp, null_bytes_beyond_string)
{
	char	a[20] = "teste";
	char	b[20] = "test";

	ASSERT_SIGN(ft_memcmp(a, b, 10), 1);
}

TEST(memcmp, signed_char_edge)
{
	char	a[10] = "abcdef";
	char	b[10] = "abc\xfdxx";

	ASSERT_SIGN(ft_memcmp(a, b, 5), -1);
}

TEST(memcmp, continues_after_null)
{
	char	a[20] = "abc";
	char	b[20] = "abc";

	a[4] = 'x';
	b[4] = 'y';
	ASSERT_SIGN(ft_memcmp(a, b, 7), -1);
}

TEST(memcmp, first_difference_decides)
{
	unsigned char	a[] = {1, 255};
	unsigned char	b[] = {2, 0};

	ASSERT_SIGN(ft_memcmp(a, b, 2), -1);
}

TEST(memcmp, reads_exactly_n_bytes)
{
	const char	*a = lft_guard_mem("abcdef", 6);
	const char	*b = lft_guard_mem("abcdef", 6);

	ASSERT_EQ(ft_memcmp(a, b, 6), 0);
}

TEST(memcmp, fuzz_against_libc)
{
	unsigned char	a[32];
	unsigned char	b[32];
	int				round;
	size_t			i;
	size_t			n;

	for (round = 0; round < 3000; round++)
	{
		for (i = 0; i < 32; i++)
		{
			a[i] = (unsigned char)lft_rand();
			b[i] = lft_rand() % 8 ? a[i] : (unsigned char)lft_rand();
		}
		n = lft_rand() % 33;
		ASSERT_SIGN(ft_memcmp(a, b, n), memcmp(a, b, n));
	}
}
