/* ft_strnstr : sémantique du man BSD. */
#include <string.h>
#include "lft_test.h"

TEST(strnstr, basic_found)
{
	char	h[] = "Hello World";

	ASSERT_PTR(ft_strnstr(h, "World", strlen(h)), &h[6]);
}

TEST(strnstr, basic_not_found)
{
	ASSERT_NULL(ft_strnstr("Hello World", "Universe", 11));
}

TEST(strnstr, empty_needle)
{
	char	h[] = "Hello World";

	ASSERT_PTR(ft_strnstr(h, "", strlen(h)), h);
}

TEST(strnstr, needle_longer_than_len)
{
	ASSERT_NULL(ft_strnstr("Hello World", "World", 3));
}

TEST(strnstr, exact_match)
{
	char	h[] = "Hello";

	ASSERT_PTR(ft_strnstr(h, "Hello", 5), h);
}

TEST(strnstr, partial_len)
{
	char	h[] = "Hello World Program";

	ASSERT_NULL(ft_strnstr(h, "World", 8));
	ASSERT_PTR(ft_strnstr(h, "World", 11), &h[6]);
}

TEST(strnstr, at_beginning)
{
	char	h[] = "HelloWorld";

	ASSERT_PTR(ft_strnstr(h, "Hello", 10), h);
}

TEST(strnstr, at_end)
{
	char	h[] = "HelloWorld";

	ASSERT_PTR(ft_strnstr(h, "World", 10), &h[5]);
}

TEST(strnstr, single_char)
{
	char	h[] = "Hello World";

	ASSERT_PTR(ft_strnstr(h, "W", 11), &h[6]);
}

TEST(strnstr, multiple_occurrences)
{
	char	h[] = "Hello Hello World";

	ASSERT_PTR(ft_strnstr(h, "Hello", 17), h);
}

TEST(strnstr, zero_len)
{
	ASSERT_NULL(ft_strnstr("Hello World", "Hello", 0));
}

TEST(strnstr, empty_needle_empty_haystack)
{
	char	h[] = "";

	ASSERT_PTR(ft_strnstr(h, "", 0), h);
	ASSERT_PTR(ft_strnstr(h, "", 1), h);
	ASSERT_PTR(ft_strnstr(h, "", 2), h);
}

TEST(strnstr, empty_haystack_with_needle)
{
	ASSERT_NULL(ft_strnstr("", "teste", 0));
	ASSERT_NULL(ft_strnstr("", "teste", 1));
	ASSERT_NULL(ft_strnstr("", "teste", 2));
}

TEST(strnstr, haystack_empty_needle)
{
	char	h[] = "teste";

	ASSERT_PTR(ft_strnstr(h, "", 0), h);
	ASSERT_PTR(ft_strnstr(h, "", 1), h);
	ASSERT_PTR(ft_strnstr(h, "", 2), h);
}

TEST(strnstr, partial_match_boundary)
{
	char	h[] = "abcdefgh";

	ASSERT_NULL(ft_strnstr(h, "abc", 2));
	ASSERT_PTR(ft_strnstr(h, "abc", 3), h);
	ASSERT_PTR(ft_strnstr(h, "abc", 4), h);
}

TEST(strnstr, needle_longer_than_haystack)
{
	ASSERT_NULL(ft_strnstr("abc", "abcdef", 2));
	ASSERT_NULL(ft_strnstr("abc", "abcdef", 3));
	ASSERT_NULL(ft_strnstr("abc", "abcdef", 4));
	ASSERT_NULL(ft_strnstr("abc", "abcdef", 5));
}

TEST(strnstr, overlapping_pattern)
{
	char	h[] = "aaxx";

	ASSERT_NULL(ft_strnstr(h, "xx", 2));
	ASSERT_NULL(ft_strnstr(h, "xx", 3));
	ASSERT_PTR(ft_strnstr(h, "xx", 4), &h[2]);
	ASSERT_PTR(ft_strnstr(h, "xx", 5), &h[2]);
	ASSERT_PTR(ft_strnstr(h, "xx", 6), &h[2]);
}

TEST(strnstr, signed_chars)
{
	ASSERT_NULL(ft_strnstr("abcdef", "abc\xfdxx", 0xffffffff));
}

TEST(strnstr, high_bytes)
{
	char	h[] = "ab\xfd\xfe" "cd";

	ASSERT_PTR(ft_strnstr(h, "\xfd\xfe", 6), &h[2]);
}

TEST(strnstr, restart_after_a_partial_match)
{
	char	a[] = "aaab";
	char	b[] = "abababc";
	char	c[] = "mississippi";

	ASSERT_PTR(ft_strnstr(a, "aab", 4), &a[1]);
	ASSERT_PTR(ft_strnstr(b, "ababc", 7), &b[2]);
	ASSERT_PTR(ft_strnstr(c, "issip", 11), &c[4]);
}

TEST(strnstr, needle_tail_past_len)
{
	ASSERT_NULL(ft_strnstr("abcdef", "def", 5));
	ASSERT_NULL(ft_strnstr("abcdef", "ef", 5));
}

TEST(strnstr, nothing_found_after_a_nul)
{
	ASSERT_NULL(ft_strnstr("abc\0def", "def", 7));
}

TEST(strnstr, huge_len_stops_at_terminator)
{
	const char	*h = lft_guard_str("abcdef");

	ASSERT_PTR(ft_strnstr(h, "def", SIZE_MAX), h + 3);
	ASSERT_NULL(ft_strnstr(h, "xyz", SIZE_MAX));
	ASSERT_NULL(ft_strnstr(h, "defg", SIZE_MAX));
	ASSERT_NULL(ft_strnstr(h, "xyz", 1000));
}

TEST(strnstr, reads_at_most_len_chars)
{
	const char	*h = lft_guard_mem("abcdef", 6);

	ASSERT_NULL(ft_strnstr(h, "xyz", 6));
	ASSERT_NULL(ft_strnstr(h, "efg", 6));
	ASSERT_PTR(ft_strnstr(h, "ef", 6), h + 4);
}

TEST(strnstr, reads_nothing_past_the_needle)
{
	const char	*n = lft_guard_str("cd");
	const char	*h = lft_guard_str("abcde");

	ASSERT_PTR(ft_strnstr(h, n, 5), h + 2);
	ASSERT_PTR(ft_strnstr(h, lft_guard_str(""), 0), h);
}

static const char	*model(const char *h, const char *n, size_t len)
{
	size_t	nl;
	size_t	i;

	nl = strlen(n);
	if (nl == 0)
		return (h);
	for (i = 0; h[i] && i + nl <= len; i++)
		if (strncmp(h + i, n, nl) == 0)
			return (h + i);
	return (NULL);
}

TEST(strnstr, fuzz_against_the_man_definition)
{
	char		hbuf[17];
	char		nbuf[5];
	const char	*h;
	const char	*n;
	int			round;
	size_t		i;
	size_t		hl;
	size_t		nl;
	size_t		len;

	for (round = 0; round < 3000; round++)
	{
		hl = lft_rand() % 17;
		nl = lft_rand() % 5;
		for (i = 0; i < hl; i++)
			hbuf[i] = (char)('a' + lft_rand() % 2);
		hbuf[hl] = '\0';
		for (i = 0; i < nl; i++)
			nbuf[i] = (char)('a' + lft_rand() % 2);
		nbuf[nl] = '\0';
		h = lft_guard_str(hbuf);
		n = lft_guard_str(nbuf);
		len = lft_rand() % 20;
		if (ft_strnstr(h, n, len) != model(h, n, len))
			FAILF("ft_strnstr(%s, %s, %zu) : résultat incorrect",
				lft_quote(hbuf), lft_quote(nbuf), len);
		lft_guard_free(h);
		lft_guard_free(n);
	}
}
