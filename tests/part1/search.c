/* ft_strchr, ft_strrchr, ft_memchr. */
#include <string.h>
#include "lft_test.h"

TEST(strchr, basic)
{
	char	s[] = "Hello World";

	ASSERT_PTR(ft_strchr(s, 'W'), &s[6]);
}

TEST(strchr, first_char)
{
	char	s[] = "Hello";

	ASSERT_PTR(ft_strchr(s, 'H'), &s[0]);
}

TEST(strchr, last_char)
{
	char	s[] = "Hello";

	ASSERT_PTR(ft_strchr(s, 'o'), &s[4]);
}

TEST(strchr, null_terminator)
{
	char	s[] = "Hello";

	ASSERT_PTR(ft_strchr(s, '\0'), &s[5]);
}

TEST(strchr, not_found)
{
	ASSERT_NULL(ft_strchr("Hello World", 'X'));
}

TEST(strchr, empty_string)
{
	char	s[] = "";

	ASSERT_NULL(ft_strchr(s, 'a'));
	ASSERT_PTR(ft_strchr(s, '\0'), s);
}

TEST(strchr, multiple_occurrences)
{
	char	s[] = "Hello Hello";

	ASSERT_PTR(ft_strchr(s, 'l'), &s[2]);
}

TEST(strchr, special_chars)
{
	char	s[] = "Hello\tWorld\n!";

	ASSERT_PTR(ft_strchr(s, '\t'), &s[5]);
	ASSERT_PTR(ft_strchr(s, '\n'), &s[11]);
}

TEST(strchr, unsigned_char_comparison)
{
	char	s[] = "Hello\310World";

	ASSERT_PTR(ft_strchr(s, 200), &s[5]);
	ASSERT_PTR(ft_strchr(s, (char)200), &s[5]);
}

TEST(strchr, value_is_converted_to_char)
{
	char	s[] = "teste";

	ASSERT_PTR(ft_strchr(s, 't' + 256), s);
	ASSERT_PTR(ft_strchr(s, 'e' + 1024), &s[1]);
	ASSERT_PTR(ft_strchr(s, 1024), &s[5]);
	ASSERT_PTR(ft_strchr(s, 256), &s[5]);
}

TEST(strchr, stops_at_terminator)
{
	char	s[] = "abc\0xyz";

	ASSERT_NULL(ft_strchr(s, 'x'));
}

TEST(strchr, reads_nothing_after_the_terminator)
{
	const char	*s = lft_guard_str("abc");

	ASSERT_NULL(ft_strchr(s, 'x'));
	ASSERT_PTR(ft_strchr(s, 'c'), s + 2);
	ASSERT_PTR(ft_strchr(s, '\0'), s + 3);
	ASSERT_NULL(ft_strchr(lft_guard_str(""), 'x'));
}

TEST(strchr, fuzz_against_libc)
{
	char		buf[33];
	const char	*alphabet = "ab\x80\xff";
	const char	*s;
	int			round;
	size_t		len;
	size_t		i;
	int			c;

	for (round = 0; round < 2000; round++)
	{
		len = lft_rand() % 33;
		for (i = 0; i < len; i++)
			buf[i] = alphabet[lft_rand() % 4];
		buf[len] = '\0';
		s = lft_guard_str(buf);
		c = (int)(lft_rand() % 2 ? alphabet[lft_rand() % 4]
				: (int)(lft_rand() & 0xfff));
		if (ft_strchr(s, c) != strchr(s, c))
			FAILF("ft_strchr(%s, %d) différent de strchr", lft_quote(buf), c);
		lft_guard_free(s);
	}
}

TEST(strrchr, basic)
{
	char	s[] = "Hello World";

	ASSERT_PTR(ft_strrchr(s, 'o'), &s[7]);
}

TEST(strrchr, first_char)
{
	char	s[] = "Hello";

	ASSERT_PTR(ft_strrchr(s, 'H'), &s[0]);
}

TEST(strrchr, last_char)
{
	char	s[] = "Hello";

	ASSERT_PTR(ft_strrchr(s, 'o'), &s[4]);
}

TEST(strrchr, null_terminator)
{
	char	s[] = "Hello";

	ASSERT_PTR(ft_strrchr(s, '\0'), &s[5]);
}

TEST(strrchr, not_found)
{
	ASSERT_NULL(ft_strrchr("Hello World", 'X'));
}

TEST(strrchr, empty_string)
{
	char	s[] = "";

	ASSERT_NULL(ft_strrchr(s, 'a'));
	ASSERT_PTR(ft_strrchr(s, '\0'), s);
}

TEST(strrchr, multiple_occurrences)
{
	char	s[] = "Hello Hello Hello";

	ASSERT_PTR(ft_strrchr(s, 'l'), &s[15]);
}

TEST(strrchr, same_as_strchr)
{
	char	s[] = "Hello World";

	ASSERT_PTR(ft_strrchr(s, 'W'), &s[6]);
	ASSERT_PTR(ft_strrchr(s, 'W'), ft_strchr(s, 'W'));
}

TEST(strrchr, repeated_char)
{
	char	s[] = "aaaaaaa";

	ASSERT_PTR(ft_strrchr(s, 'a'), &s[6]);
}

TEST(strrchr, unsigned_char_comparison)
{
	char	s[] = "a\310b\310c";

	ASSERT_PTR(ft_strrchr(s, 200), &s[3]);
	ASSERT_PTR(ft_strrchr(s, (char)200), &s[3]);
}

TEST(strrchr, value_is_converted_to_char)
{
	char	s[] = "teste";

	ASSERT_PTR(ft_strrchr(s, 't' + 256), &s[3]);
	ASSERT_PTR(ft_strrchr(s, 1024), &s[5]);
	ASSERT_PTR(ft_strrchr(s, 256), &s[5]);
}

TEST(strrchr, stops_at_terminator)
{
	char	s[] = "abc\0xyz";

	ASSERT_NULL(ft_strrchr(s, 'x'));
}

TEST(strrchr, reads_nothing_after_the_terminator)
{
	const char	*s = lft_guard_str("abca");

	ASSERT_PTR(ft_strrchr(s, 'a'), s + 3);
	ASSERT_NULL(ft_strrchr(s, 'x'));
	ASSERT_PTR(ft_strrchr(s, '\0'), s + 4);
}

TEST(strrchr, reads_nothing_before_the_start)
{
	const char	*s = lft_guard_str_front("abca");

	ASSERT_PTR(ft_strrchr(s, 'a'), s + 3);
	ASSERT_PTR(ft_strrchr(s, 'b'), s + 1);
	ASSERT_NULL(ft_strrchr(s, 'x'));
	ASSERT_NULL(ft_strrchr(lft_guard_str_front(""), 'x'));
}

TEST(strrchr, fuzz_against_libc)
{
	char		buf[33];
	const char	*alphabet = "ab\x80\xff";
	const char	*s;
	int			round;
	size_t		len;
	size_t		i;
	int			c;

	for (round = 0; round < 2000; round++)
	{
		len = lft_rand() % 33;
		for (i = 0; i < len; i++)
			buf[i] = alphabet[lft_rand() % 4];
		buf[len] = '\0';
		s = lft_guard_str(buf);
		c = (int)(lft_rand() % 2 ? alphabet[lft_rand() % 4]
				: (int)(lft_rand() & 0xfff));
		if (ft_strrchr(s, c) != strrchr(s, c))
			FAILF("ft_strrchr(%s, %d) différent de strrchr", lft_quote(buf), c);
		lft_guard_free(s);
	}
}

TEST(memchr, basic_found)
{
	char	s[] = "Hello World";

	ASSERT_PTR(ft_memchr(s, 'W', 11), &s[6]);
}

TEST(memchr, not_found)
{
	ASSERT_NULL(ft_memchr("Hello World", 'X', 11));
}

TEST(memchr, zero_byte)
{
	char	s[] = "Hello";

	ASSERT_PTR(ft_memchr(s, '\0', 6), &s[5]);
}

TEST(memchr, binary_data)
{
	unsigned char	s[] = {0, 1, 2, 255, 254, 128, 127};

	ASSERT_PTR(ft_memchr(s, 255, 7), &s[3]);
	ASSERT_PTR(ft_memchr(s, 128, 7), &s[5]);
	ASSERT_PTR(ft_memchr(s, 0, 7), &s[0]);
}

TEST(memchr, first_byte)
{
	char	s[] = "Hello World";

	ASSERT_PTR(ft_memchr(s, 'H', 11), &s[0]);
}

TEST(memchr, last_byte)
{
	char	s[] = "Hello World";

	ASSERT_PTR(ft_memchr(s, 'd', 11), &s[10]);
}

TEST(memchr, partial_search)
{
	ASSERT_NULL(ft_memchr("Hello World", 'W', 5));
}

TEST(memchr, match_just_outside_the_range)
{
	char	s[] = "abcd";

	ASSERT_NULL(ft_memchr(s, 'c', 2));
	ASSERT_PTR(ft_memchr(s, 'c', 3), &s[2]);
}

TEST(memchr, zero_length)
{
	ASSERT_NULL(ft_memchr("Hello World", 'H', 0));
}

TEST(memchr, zero_length_reads_nothing)
{
	ASSERT_NULL(ft_memchr(lft_guard_mem("", 0), 'H', 0));
}

TEST(memchr, negative_char)
{
	char	s[] = {-1, -128, 127, 0};

	ASSERT_PTR(ft_memchr(s, -1, 4), &s[0]);
	ASSERT_PTR(ft_memchr(s, -128, 4), &s[1]);
}

TEST(memchr, value_is_converted_to_unsigned_char)
{
	char	s[] = "abc";

	ASSERT_PTR(ft_memchr(s, 'b' + 256, 3), &s[1]);
	ASSERT_PTR(ft_memchr(s, 256, 4), &s[3]);
}

TEST(memchr, struct_data)
{
	struct { int a; char b; short c; }	s;

	memset(&s, 0, sizeof(s));
	s.a = 0x41424344;
	s.b = 'X';
	s.c = 0x5959;
	ASSERT_PTR(ft_memchr(&s, 'X', sizeof(s)), &s.b);
}

TEST(memchr, continues_after_nul)
{
	char	s[] = {'a', 0, 'b', 0, 'c'};

	ASSERT_PTR(ft_memchr(s, 'c', 5), &s[4]);
}

TEST(memchr, reads_exactly_n_bytes)
{
	const char	*s = lft_guard_mem("abcdef", 6);

	ASSERT_NULL(ft_memchr(s, 'x', 6));
	ASSERT_PTR(ft_memchr(s, 'f', 6), s + 5);
}

TEST(memchr, stops_reading_at_the_match)
{
	const char	*s = lft_guard_mem("abc", 3);

	ASSERT_PTR(ft_memchr(s, 'c', 100), s + 2);
	ASSERT_PTR(ft_memchr(s, 'a', SIZE_MAX), s);
}

TEST(memchr, fuzz_against_libc)
{
	const char	alphabet[] = {'a', 'b', '\0', '\x80', '\xff'};
	char		buf[64];
	int			round;
	size_t		n;
	size_t		i;
	int			c;

	for (round = 0; round < 2000; round++)
	{
		n = lft_rand() % 65;
		for (i = 0; i < sizeof(buf); i++)
			buf[i] = alphabet[lft_rand() % sizeof(alphabet)];
		c = lft_rand() % 2 ? buf[lft_rand() % 64] : (int)(lft_rand() & 0xfff);
		if (ft_memchr(buf, c, n) != memchr(buf, c, n))
			FAILF("ft_memchr(%s, %d, %zu) différent de memchr",
				lft_quote_n(buf, n), c, n);
	}
}
