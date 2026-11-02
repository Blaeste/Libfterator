/* ft_strlcpy, ft_strlcat : sémantique du man BSD. */
#include <stdlib.h>
#include <string.h>
#include "lft_test.h"

#define FILLER 'Z'

/* Vérifie retour, contenu, terminateur et octets laissés intacts. */
static void	check_strlcpy(const char *src_text, size_t size)
{
	const char	*src = lft_guard_str(src_text);
	char		*dst = lft_guard_buf(size);
	size_t		srclen = strlen(src_text);
	size_t		copied = 0;
	size_t		ret;
	size_t		i;

	memset(dst, FILLER, size);
	ret = ft_strlcpy(dst, src, size);
	if (ret != srclen)
		FAILF("strlcpy(%s, size=%zu) : retour %zu, attendu %zu",
			lft_quote(src_text), size, ret, srclen);
	if (size > 0)
	{
		copied = srclen < size - 1 ? srclen : size - 1;
		if (memcmp(dst, src_text, copied) != 0 || dst[copied] != '\0')
			FAILF("strlcpy(%s, size=%zu) : résultat %s", lft_quote(src_text),
				size, lft_quote_n(dst, size));
	}
	for (i = size > 0 ? copied + 1 : 0; i < size; i++)
		if (dst[i] != FILLER)
			FAILF("strlcpy(%s, size=%zu) : octet %zu modifié au-delà du "
				"terminateur", lft_quote(src_text), size, i);
	lft_guard_free(src);
	lft_guard_free(dst);
}

static void	check_strlcat(const char *dst_text, const char *src_text, size_t size)
{
	size_t		dstlen = strlen(dst_text);
	size_t		srclen = strlen(src_text);
	size_t		cap = size > dstlen + 1 ? size : dstlen + 1;
	char		*dst = lft_guard_buf(cap);
	char		*want = lft_alloc(cap);
	const char	*src = lft_guard_str(src_text);
	size_t		expected;
	size_t		ret;
	size_t		n;

	memset(dst, FILLER, cap);
	memcpy(dst, dst_text, dstlen + 1);
	memcpy(want, dst, cap);
	if (size <= dstlen)
		expected = size + srclen;
	else
	{
		expected = dstlen + srclen;
		n = size - dstlen - 1;
		if (srclen < n)
			n = srclen;
		memcpy(want + dstlen, src_text, n);
		want[dstlen + n] = '\0';
	}
	ret = ft_strlcat(dst, src, size);
	if (ret != expected)
		FAILF("strlcat(%s, %s, size=%zu) : retour %zu, attendu %zu",
			lft_quote(dst_text), lft_quote(src_text), size, ret, expected);
	if (memcmp(dst, want, cap) != 0)
		FAILF("strlcat(%s, %s, size=%zu) : résultat %s, attendu %s",
			lft_quote(dst_text), lft_quote(src_text), size,
			lft_quote_n(dst, cap), lft_quote_n(want, cap));
	free(want);
	lft_guard_free(src);
	lft_guard_free(dst);
}

TEST(strlcpy, basic)
{
	char	dest[20];

	ASSERT_EQ(ft_strlcpy(dest, "Hello World", 20), 11);
	ASSERT_STR(dest, "Hello World");
}

TEST(strlcpy, truncate)
{
	char	*dest = lft_guard_buf(6);

	ASSERT_EQ(ft_strlcpy(dest, "Hello World", 6), 11);
	ASSERT_STR(dest, "Hello");
}

TEST(strlcpy, size_zero)
{
	char	dest[10] = "original";

	ASSERT_EQ(ft_strlcpy(dest, "Hello", 0), 5);
	ASSERT_STR(dest, "original");
}

TEST(strlcpy, size_zero_touches_nothing)
{
	ASSERT_EQ(ft_strlcpy(lft_guard_buf(0), "Hello", 0), 5);
}

TEST(strlcpy, size_one)
{
	char	dest[10];

	memset(dest, 'X', 10);
	ASSERT_EQ(ft_strlcpy(dest, "Hello", 1), 5);
	ASSERT_EQ(dest[0], '\0');
	ASSERT_EQ(dest[1], 'X');
}

TEST(strlcpy, empty_src)
{
	char	dest[10] = "original";

	ASSERT_EQ(ft_strlcpy(dest, "", 10), 0);
	ASSERT_STR(dest, "");
	ASSERT_EQ(dest[1], 'r');
}

TEST(strlcpy, exact_size)
{
	char	*dest = lft_guard_buf(5);

	ASSERT_EQ(ft_strlcpy(dest, "test", 5), 4);
	ASSERT_STR(dest, "test");
}

TEST(strlcpy, one_byte_too_small)
{
	char	*dest = lft_guard_buf(4);

	ASSERT_EQ(ft_strlcpy(dest, "test", 4), 4);
	ASSERT_STR(dest, "tes");
}

TEST(strlcpy, long_string)
{
	const char	*src = "This is a very long string to test strlcpy behavior";
	char		*dest = lft_guard_buf(20);

	ASSERT_EQ(ft_strlcpy(dest, src, 20), strlen(src));
	ASSERT_EQ(strlen(dest), 19);
	ASSERT_MEM(dest, src, 19);
}

TEST(strlcpy, special_chars)
{
	char	dest[20];

	ASSERT_EQ(ft_strlcpy(dest, "Hello\tWorld\n!", 20), 13);
	ASSERT_STR(dest, "Hello\tWorld\n!");
}

TEST(strlcpy, does_not_pad_with_zeros)
{
	char	dest[10];

	memset(dest, 'X', 10);
	ft_strlcpy(dest, "ab", 10);
	ASSERT_MEM(dest, "ab\0XXXXXXX", 10);
}

TEST(strlcpy, huge_size_stops_at_terminator)
{
	char	*dest = lft_guard_buf(6);

	ASSERT_EQ(ft_strlcpy(dest, "hello", SIZE_MAX), 5);
	ASSERT_STR(dest, "hello");
}

TEST(strlcpy, returns_full_length_of_long_source)
{
	static char	src[5000];
	char		dest[3];

	memset(src, 'a', sizeof(src) - 1);
	ASSERT_EQ(ft_strlcpy(dest, src, sizeof(dest)), sizeof(src) - 1);
	ASSERT_STR(dest, "aa");
}

TEST(strlcpy, every_source_and_size)
{
	char	src[17];
	size_t	len;
	size_t	size;

	for (len = 0; len <= 16; len++)
	{
		memcpy(src, "0123456789abcdef", len);
		src[len] = '\0';
		for (size = 0; size <= 20; size++)
			check_strlcpy(src, size);
	}
}

TEST(strlcat, basic)
{
	char	dest[20] = "Hello ";

	ASSERT_EQ(ft_strlcat(dest, "World", 20), 11);
	ASSERT_STR(dest, "Hello World");
}

TEST(strlcat, truncate)
{
	char	dest[10] = "Hello ";

	ASSERT_EQ(ft_strlcat(dest, "World!", 10), 12);
	ASSERT_STR(dest, "Hello Wor");
}

TEST(strlcat, size_zero)
{
	char	dest[10] = "Hello";

	ASSERT_EQ(ft_strlcat(dest, " World", 0), 6);
	ASSERT_STR(dest, "Hello");
}

TEST(strlcat, size_zero_reads_nothing_from_dst)
{
	ASSERT_EQ(ft_strlcat(lft_guard_buf(0), "abc", 0), 3);
}

TEST(strlcat, empty_src)
{
	char	dest[20] = "Hello";

	ASSERT_EQ(ft_strlcat(dest, "", 20), 5);
	ASSERT_STR(dest, "Hello");
}

TEST(strlcat, empty_dest)
{
	char	dest[20] = "";

	ASSERT_EQ(ft_strlcat(dest, "Hello", 20), 5);
	ASSERT_STR(dest, "Hello");
}

TEST(strlcat, size_equal_dest)
{
	char	dest[10] = "Hello";

	ASSERT_EQ(ft_strlcat(dest, " World", 5), 11);
	ASSERT_STR(dest, "Hello");
}

TEST(strlcat, size_less_than_dest)
{
	char	dest[10] = "Hello";

	ASSERT_EQ(ft_strlcat(dest, " World", 3), 9);
	ASSERT_STR(dest, "Hello");
}

TEST(strlcat, multiple_concat)
{
	char	dest[50] = "Hello";

	ft_strlcat(dest, " ", 50);
	ASSERT_STR(dest, "Hello ");
	ft_strlcat(dest, "World", 50);
	ASSERT_STR(dest, "Hello World");
	ASSERT_EQ(ft_strlcat(dest, "!", 50), 12);
	ASSERT_STR(dest, "Hello World!");
}

TEST(strlcat, dst_not_terminated_within_size)
{
	char	*dest = lft_guard_buf(5);

	memset(dest, 'x', 5);
	ASSERT_EQ(ft_strlcat(dest, "abc", 5), 8);
	ASSERT_MEM(dest, "xxxxx", 5);
}

TEST(strlcat, exactly_one_free_byte)
{
	char	*dest = lft_guard_buf(6);

	memcpy(dest, "Hello", 6);
	ASSERT_EQ(ft_strlcat(dest, "abc", 6), 8);
	ASSERT_STR(dest, "Hello");
}

TEST(strlcat, fills_the_buffer_exactly)
{
	char	*dest = lft_guard_buf(9);

	memcpy(dest, "Hello", 6);
	ASSERT_EQ(ft_strlcat(dest, "abc", 9), 8);
	ASSERT_STR(dest, "Helloabc");
}

TEST(strlcat, does_not_pad_with_zeros)
{
	char	dest[10];

	memset(dest, 'X', 10);
	memcpy(dest, "ab", 3);
	ft_strlcat(dest, "c", 10);
	ASSERT_MEM(dest, "abc\0XXXXXX", 10);
}

TEST(strlcat, huge_size_stops_at_terminator)
{
	char	*dest = lft_guard_buf(6);

	memcpy(dest, "he", 3);
	ASSERT_EQ(ft_strlcat(dest, "llo", SIZE_MAX), 5);
	ASSERT_STR(dest, "hello");
}

TEST(strlcat, every_combination)
{
	char	dst[9];
	char	src[9];
	size_t	d;
	size_t	s;
	size_t	size;

	for (d = 0; d <= 8; d++)
	{
		memcpy(dst, "ABCDEFGH", d);
		dst[d] = '\0';
		for (s = 0; s <= 8; s++)
		{
			memcpy(src, "01234567", s);
			src[s] = '\0';
			for (size = 0; size <= 20; size++)
				check_strlcat(dst, src, size);
		}
	}
}
