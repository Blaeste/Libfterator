/* ft_substr, ft_strjoin, ft_strtrim. */
#include <stdlib.h>
#include <string.h>
#include "lft_test.h"

#define CHECK_SUBSTR(s, start, len, want) \
	do { \
		char	*lft_r = ft_substr((s), (start), (len)); \
		\
		ASSERT_STR_AS("ft_substr(" #s ", " #start ", " #len ")", lft_r, (want)); \
		free(lft_r); \
	} while (0)

#define CHECK_JOIN(a, b, want) \
	do { \
		char	*lft_r = ft_strjoin((a), (b)); \
		\
		ASSERT_STR_AS("ft_strjoin(" #a ", " #b ")", lft_r, (want)); \
		free(lft_r); \
	} while (0)

#define CHECK_TRIM(s, set, want) \
	do { \
		char	*lft_r = ft_strtrim((s), (set)); \
		\
		ASSERT_STR_AS("ft_strtrim(" #s ", " #set ")", lft_r, (want)); \
		free(lft_r); \
	} while (0)

TEST(substr, basic)
{
	CHECK_SUBSTR("Hello World", 6, 5, "World");
}

TEST(substr, full_string)
{
	CHECK_SUBSTR("Hello", 0, 5, "Hello");
}

TEST(substr, start_beyond_string)
{
	CHECK_SUBSTR("Hello", 10, 3, "");
}

TEST(substr, len_beyond_string)
{
	CHECK_SUBSTR("Hello", 2, 10, "llo");
}

TEST(substr, zero_length)
{
	CHECK_SUBSTR("Hello", 2, 0, "");
}

TEST(substr, empty_source)
{
	CHECK_SUBSTR("", 0, 5, "");
}

TEST(substr, single_char)
{
	CHECK_SUBSTR("Hello World", 6, 1, "W");
}

TEST(substr, middle_section)
{
	CHECK_SUBSTR("0123456789", 3, 4, "3456");
}

TEST(substr, at_end)
{
	CHECK_SUBSTR("Hello World", 10, 1, "d");
}

TEST(substr, empty_string_zero_len)
{
	CHECK_SUBSTR("", 0, 0, "");
}

TEST(substr, empty_string_with_len)
{
	CHECK_SUBSTR("", 0, 1, "");
}

TEST(substr, start_beyond_empty)
{
	CHECK_SUBSTR("", 1, 1, "");
}

TEST(substr, max_len)
{
	CHECK_SUBSTR("hola", 0, SIZE_MAX, "hola");
	CHECK_SUBSTR("hola", 2, SIZE_MAX, "la");
}

TEST(substr, large_start_max_len)
{
	CHECK_SUBSTR("hola", UINT_MAX, SIZE_MAX, "");
	CHECK_SUBSTR("hola", UINT_MAX, 3, "");
	CHECK_SUBSTR("hola", 4000000000u, 1, "");
}

TEST(substr, exact_boundaries)
{
	CHECK_SUBSTR("hola", 4, 0, "");
	CHECK_SUBSTR("hola", 4, 1, "");
	CHECK_SUBSTR("hola", 5, 1, "");
	CHECK_SUBSTR("hola", 3, 1, "a");
	CHECK_SUBSTR("hola", 3, 2, "a");
}

TEST(substr, result_is_a_new_block)
{
	char	s[] = "hello";
	char	*r = ft_substr(s, 0, 5);

	ASSERT_NOT_NULL(r);
	ASSERT_TRUE(r != s);
	r[0] = 'X';
	ASSERT_EQ(s[0], 'h');
	free(r);
}

TEST(substr, empty_result_can_be_written_and_freed)
{
	char	*a = ft_substr("hello", 10, 2);
	char	*b = ft_substr("hello", 1, 0);

	ASSERT_NOT_NULL(a);
	ASSERT_NOT_NULL(b);
	a[0] = '\0';
	b[0] = '\0';
	free(a);
	free(b);
}

TEST(substr, every_start_and_length)
{
	const char		*text = "0123456789";
	char			want[16];
	const char		*s;
	char			*r;
	unsigned int	start;
	size_t			len;
	size_t			n;

	s = lft_guard_str(text);
	for (start = 0; start <= 12; start++)
	{
		for (len = 0; len <= 12; len++)
		{
			n = 0;
			if (start < 10)
				n = (10 - start) < len ? (10 - start) : len;
			memcpy(want, text + (start < 10 ? start : 0), n);
			want[n] = '\0';
			r = ft_substr(s, start, len);
			if (r == NULL || strcmp(r, want) != 0)
				FAILF("ft_substr(%s, %u, %zu) : obtenu %s, attendu %s",
					lft_quote(text), start, len, lft_quote(r), lft_quote(want));
			free(r);
		}
	}
}

TEST(substr, high_bytes)
{
	CHECK_SUBSTR("a\xff\x80z", 1, 2, "\xff\x80");
}

TEST(substr, malloc_failure_returns_null)
{
	MALLOC_FAIL_LOOP
	{
		char	*r = ft_substr("Hello World", 6, 5);

		if (lft_mf_injected())
			ASSERT_NULL(r);
		else
			free(r);
	}
}

TEST(substr, malloc_failure_when_start_is_beyond)
{
	MALLOC_FAIL_LOOP
	{
		char	*r = ft_substr("Hello", 10, 5);

		if (lft_mf_injected())
			ASSERT_NULL(r);
		else
			free(r);
	}
}

TEST(strjoin, basic)
{
	CHECK_JOIN("Hello ", "World", "Hello World");
}

TEST(strjoin, empty_first)
{
	CHECK_JOIN("", "World", "World");
}

TEST(strjoin, empty_second)
{
	CHECK_JOIN("Hello", "", "Hello");
}

TEST(strjoin, both_empty)
{
	CHECK_JOIN("", "", "");
}

TEST(strjoin, long_strings)
{
	CHECK_JOIN("This is a very long first string that contains many characters and spaces",
		" and this is the second part that will be concatenated to form an even longer string",
		"This is a very long first string that contains many characters and spaces"
		" and this is the second part that will be concatenated to form an even longer string");
}

TEST(strjoin, special_chars)
{
	CHECK_JOIN("Hello\tWorld\n", "!@#$%^&*()", "Hello\tWorld\n!@#$%^&*()");
}

TEST(strjoin, single_chars)
{
	CHECK_JOIN("A", "B", "AB");
}

TEST(strjoin, high_bytes)
{
	CHECK_JOIN("\xff", "\x80", "\xff\x80");
}

TEST(strjoin, reads_nothing_after_the_terminators)
{
	CHECK_JOIN(lft_guard_str("abc"), lft_guard_str("def"), "abcdef");
	CHECK_JOIN(lft_guard_str(""), lft_guard_str("def"), "def");
	CHECK_JOIN(lft_guard_str("abc"), lft_guard_str(""), "abc");
	CHECK_JOIN(lft_guard_str(""), lft_guard_str(""), "");
}

TEST(strjoin, inputs_are_read_only)
{
	CHECK_JOIN(lft_guard_str_front("abc"), lft_guard_str_front("def"), "abcdef");
}

TEST(strjoin, result_is_a_new_block)
{
	char	a[] = "abc";
	char	b[] = "";
	char	*r = ft_strjoin(a, b);

	ASSERT_NOT_NULL(r);
	ASSERT_TRUE(r != a);
	ASSERT_TRUE(r != b);
	free(r);
}

TEST(strjoin, same_string_twice)
{
	char	s[] = "ab";

	CHECK_JOIN(s, s, "abab");
}

/* Assez long pour sortir d'un buffer fixe, assez court pour qu'une libft correcte
** mais lente ne parte pas en timeout. */
TEST(strjoin, sixteen_kilobytes)
{
	size_t	size = 1 << 14;
	char	*a = lft_guard_buf(size + 1);
	char	*r;

	memset(a, 'a', size);
	a[size] = '\0';
	r = ft_strjoin(a, a);
	ASSERT_NOT_NULL(r);
	ASSERT_EQ(strlen(r), 2 * size);
	free(r);
}

TEST(strjoin, malloc_failure_returns_null)
{
	MALLOC_FAIL_LOOP
	{
		char	*r = ft_strjoin("abc", "def");

		if (lft_mf_injected())
			ASSERT_NULL(r);
		else
			free(r);
	}
}

TEST(strtrim, basic)
{
	CHECK_TRIM("   Hello World   ", " ", "Hello World");
}

TEST(strtrim, multiple_chars)
{
	CHECK_TRIM("abcHello Worldcba", "abc", "Hello World");
}

TEST(strtrim, no_trim_needed)
{
	CHECK_TRIM("Hello World", "xyz", "Hello World");
}

TEST(strtrim, all_trimmed)
{
	CHECK_TRIM("aaabbbccc", "abc", "");
}

TEST(strtrim, empty_string)
{
	CHECK_TRIM("", " ", "");
}

TEST(strtrim, empty_set)
{
	CHECK_TRIM("  Hello World  ", "", "  Hello World  ");
}

TEST(strtrim, whitespace_chars)
{
	CHECK_TRIM(" \t\n Hello World \n\t ", " \t\n", "Hello World");
}

TEST(strtrim, only_beginning)
{
	CHECK_TRIM("abcHello World", "abc", "Hello World");
}

TEST(strtrim, only_end)
{
	CHECK_TRIM("Hello Worldabc", "abc", "Hello World");
}

TEST(strtrim, single_char)
{
	CHECK_TRIM("aHa", "a", "H");
}

TEST(strtrim, empty_both)
{
	CHECK_TRIM("", "", "");
}

TEST(strtrim, empty_set_nochange)
{
	CHECK_TRIM("abcd", "", "abcd");
}

TEST(strtrim, empty_string_with_set)
{
	CHECK_TRIM("", "cdef", "");
}

TEST(strtrim, prefix_only)
{
	CHECK_TRIM(" . abcd", " ", ". abcd");
}

TEST(strtrim, suffix_only)
{
	CHECK_TRIM("ab cd  f    ", " ", "ab cd  f");
}

TEST(strtrim, complex_set)
{
	CHECK_TRIM("xxxz  test with x and z and x .  zx  xx z", "z x",
		"test with x and z and x .");
}

TEST(strtrim, no_match_set)
{
	CHECK_TRIM("   abxfg  ", "x", "   abxfg  ");
}

TEST(strtrim, punctuation)
{
	CHECK_TRIM(".teste, bla ,.,.", ",.", "teste, bla ");
}

TEST(strtrim, inner_chars_are_kept)
{
	CHECK_TRIM("aXaXa", "a", "XaX");
}

TEST(strtrim, repeated_chars_in_set)
{
	CHECK_TRIM("aaHelloaa", "aaaa", "Hello");
}

TEST(strtrim, set_order_does_not_matter)
{
	CHECK_TRIM("abcHellocba", "cba", "Hello");
	CHECK_TRIM("abcHellocba", "bac", "Hello");
}

TEST(strtrim, one_char_kept_on_each_side)
{
	CHECK_TRIM("xxaxx", "x", "a");
	CHECK_TRIM("axx", "x", "a");
	CHECK_TRIM("xxa", "x", "a");
	CHECK_TRIM("a", "x", "a");
}

TEST(strtrim, single_char_all_trimmed)
{
	CHECK_TRIM("x", "x", "");
}

TEST(strtrim, high_bytes)
{
	CHECK_TRIM("\xff\x80" "abc\x80\xff", "\xff\x80", "abc");
	CHECK_TRIM("\xff" "abc", "\x80", "\xff" "abc");
}

TEST(strtrim, reads_nothing_after_the_inputs)
{
	CHECK_TRIM(lft_guard_str("  a  "), lft_guard_str(" "), "a");
	CHECK_TRIM(lft_guard_str("     "), lft_guard_str(" "), "");
	CHECK_TRIM(lft_guard_str(""), lft_guard_str(" "), "");
	CHECK_TRIM(lft_guard_str("abc"), lft_guard_str(""), "abc");
}

TEST(strtrim, reads_nothing_before_the_start)
{
	CHECK_TRIM(lft_guard_str_front("     "), " ", "");
	CHECK_TRIM(lft_guard_str_front(""), " ", "");
	CHECK_TRIM(lft_guard_str_front("x"), "x", "");
	CHECK_TRIM(lft_guard_str_front("  ab"), " ", "ab");
}

TEST(strtrim, result_is_a_new_block)
{
	char	s[] = "hello";
	char	*r = ft_strtrim(s, "x");

	ASSERT_NOT_NULL(r);
	ASSERT_TRUE(r != s);
	r[0] = 'X';
	ASSERT_EQ(s[0], 'h');
	free(r);
}

TEST(strtrim, empty_result_can_be_written_and_freed)
{
	char	*r = ft_strtrim("xxx", "x");

	ASSERT_NOT_NULL(r);
	r[0] = '\0';
	free(r);
}

TEST(strtrim, malloc_failure_returns_null)
{
	MALLOC_FAIL_LOOP
	{
		char	*r = ft_strtrim("  abc  ", " ");

		if (lft_mf_injected())
			ASSERT_NULL(r);
		else
			free(r);
	}
}

TEST(strtrim, malloc_failure_when_everything_is_trimmed)
{
	MALLOC_FAIL_LOOP
	{
		char	*r = ft_strtrim("     ", " ");

		if (lft_mf_injected())
			ASSERT_NULL(r);
		else
			free(r);
	}
}
