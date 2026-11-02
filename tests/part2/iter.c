/* ft_strmapi, ft_striteri. */
#include <stdlib.h>
#include <string.h>
#include "lft_test.h"

static unsigned int	g_calls;
static unsigned int	g_indexes[64];
static char			g_chars[64];
static char			*g_addresses[64];

static char	map_next(unsigned int i, char c)
{
	(void)i;
	return ((char)(c + 1));
}

static char	map_add_index(unsigned int i, char c)
{
	return ((char)(c + i));
}

static char	map_upper(unsigned int i, char c)
{
	(void)i;
	return ((c >= 'a' && c <= 'z') ? (char)(c - 32) : c);
}

static char	map_same(unsigned int i, char c)
{
	(void)i;
	return (c);
}

static char	map_alternate(unsigned int i, char c)
{
	if (i % 2 == 0)
		return ((c >= 'a' && c <= 'z') ? (char)(c - 32) : c);
	return ((c >= 'A' && c <= 'Z') ? (char)(c + 32) : c);
}

static char	map_digit_to_letter(unsigned int i, char c)
{
	(void)i;
	return ((c >= '0' && c <= '9') ? (char)('a' + (c - '0')) : c);
}

static char	map_alpha_only(unsigned int i, char c)
{
	(void)i;
	return (((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) ? (char)(c + 1) : c);
}

static char	map_record(unsigned int i, char c)
{
	if (g_calls < 64)
	{
		g_indexes[g_calls] = i;
		g_chars[g_calls] = c;
	}
	g_calls++;
	return (c);
}

static char	map_second_to_nul(unsigned int i, char c)
{
	return (i == 1 ? '\0' : c);
}

#define CHECK_MAP(s, f, want) \
	do { \
		char	*lft_r = ft_strmapi((s), (f)); \
		\
		ASSERT_STR_AS("ft_strmapi(" #s ", " #f ")", lft_r, (want)); \
		free(lft_r); \
	} while (0)

TEST(strmapi, basic)
{
	CHECK_MAP("abc", map_next, "bcd");
}

TEST(strmapi, with_index)
{
	CHECK_MAP("aaaa", map_add_index, "abcd");
}

TEST(strmapi, toupper_func)
{
	CHECK_MAP("hello world", map_upper, "HELLO WORLD");
}

TEST(strmapi, empty_string)
{
	CHECK_MAP("", map_same, "");
}

TEST(strmapi, alternating_case)
{
	CHECK_MAP("hello world", map_alternate, "HeLlO WoRlD");
}

TEST(strmapi, numbers_to_letters)
{
	CHECK_MAP("123abc", map_digit_to_letter, "bcdabc");
}

TEST(strmapi, single_char)
{
	CHECK_MAP("a", map_next, "b");
}

TEST(strmapi, preserve_non_alpha)
{
	CHECK_MAP("a1b2c!", map_alpha_only, "b1c2d!");
}

TEST(strmapi, f_gets_each_index_in_order)
{
	char			*r = ft_strmapi("wxyz", map_record);
	unsigned int	i;

	ASSERT_NOT_NULL(r);
	ASSERT_EQ(g_calls, 4);
	for (i = 0; i < 4; i++)
	{
		ASSERT_EQ(g_indexes[i], i);
		ASSERT_EQ(g_chars[i], "wxyz"[i]);
	}
	free(r);
}

TEST(strmapi, f_is_never_called_on_an_empty_string)
{
	char	*r = ft_strmapi("", map_record);

	ASSERT_NOT_NULL(r);
	ASSERT_EQ(g_calls, 0);
	free(r);
}

TEST(strmapi, result_keeps_its_length_when_f_returns_nul)
{
	char	*r = ft_strmapi("abc", map_second_to_nul);

	ASSERT_NOT_NULL(r);
	ASSERT_MEM(r, "a\0c", 4);
	free(r);
}

TEST(strmapi, input_is_read_only)
{
	CHECK_MAP(lft_guard_str("abc"), map_next, "bcd");
	CHECK_MAP(lft_guard_str(""), map_next, "");
}

TEST(strmapi, result_is_a_new_block)
{
	char	s[] = "abc";
	char	*r = ft_strmapi(s, map_same);

	ASSERT_NOT_NULL(r);
	ASSERT_TRUE(r != s);
	free(r);
}

TEST(strmapi, high_bytes)
{
	CHECK_MAP("\xff\x80", map_same, "\xff\x80");
}

TEST(strmapi, malloc_failure_returns_null)
{
	MALLOC_FAIL_LOOP
	{
		char	*r = ft_strmapi("abc", map_next);

		if (lft_mf_injected())
			ASSERT_NULL(r);
		else
			free(r);
	}
}

static void	iter_next(unsigned int i, char *c)
{
	(void)i;
	*c = (char)(*c + 1);
}

static void	iter_add_index(unsigned int i, char *c)
{
	*c = (char)(*c + i);
}

static void	iter_upper(unsigned int i, char *c)
{
	*c = map_upper(i, *c);
}

static void	iter_alternate(unsigned int i, char *c)
{
	*c = map_alternate(i, *c);
}

static void	iter_set_index(unsigned int i, char *c)
{
	*c = (char)('0' + i % 10);
}

static void	iter_alphabet(unsigned int i, char *c)
{
	*c = (char)('a' + i % 26);
}

static void	iter_alpha_only(unsigned int i, char *c)
{
	*c = map_alpha_only(i, *c);
}

static void	iter_record(unsigned int i, char *c)
{
	if (g_calls < 64)
	{
		g_indexes[g_calls] = i;
		g_chars[g_calls] = *c;
		g_addresses[g_calls] = c;
	}
	g_calls++;
}

TEST(striteri, basic)
{
	char	s[] = "abc";

	ft_striteri(s, iter_next);
	ASSERT_STR(s, "bcd");
}

TEST(striteri, with_index)
{
	char	s[] = "aaaa";

	ft_striteri(s, iter_add_index);
	ASSERT_STR(s, "abcd");
}

TEST(striteri, toupper_func)
{
	char	s[] = "hello world";

	ft_striteri(s, iter_upper);
	ASSERT_STR(s, "HELLO WORLD");
}

TEST(striteri, empty_string)
{
	char	s[] = "";

	ft_striteri(s, iter_next);
	ASSERT_EQ(s[0], '\0');
}

TEST(striteri, alternating_case)
{
	char	s[] = "hello world";

	ft_striteri(s, iter_alternate);
	ASSERT_STR(s, "HeLlO WoRlD");
}

TEST(striteri, set_to_index)
{
	char	s[] = "abcdef";

	ft_striteri(s, iter_set_index);
	ASSERT_STR(s, "012345");
}

TEST(striteri, single_char)
{
	char	s[] = "a";

	ft_striteri(s, iter_next);
	ASSERT_STR(s, "b");
}

TEST(striteri, preserve_non_alpha)
{
	char	s[] = "a1b2c!";

	ft_striteri(s, iter_alpha_only);
	ASSERT_STR(s, "b1c2d!");
}

TEST(striteri, long_string)
{
	char	s[] = "xxxxxxxxxx";

	ft_striteri(s, iter_alphabet);
	ASSERT_STR(s, "abcdefghij");
}

TEST(striteri, f_gets_each_index_and_address_in_order)
{
	char			s[] = "wxyz";
	unsigned int	i;

	ft_striteri(s, iter_record);
	ASSERT_EQ(g_calls, 4);
	for (i = 0; i < 4; i++)
	{
		ASSERT_EQ(g_indexes[i], i);
		ASSERT_EQ(g_chars[i], "wxyz"[i]);
		ASSERT_PTR(g_addresses[i], &s[i]);
	}
}

TEST(striteri, f_is_never_called_on_an_empty_string)
{
	char	s[] = "";

	ft_striteri(s, iter_record);
	ASSERT_EQ(g_calls, 0);
}

TEST(striteri, reads_nothing_after_the_terminator)
{
	char	*s = lft_guard_buf(4);

	memcpy(s, "abc", 4);
	ft_striteri(s, iter_next);
	ASSERT_STR(s, "bcd");
}

TEST(striteri, allocates_nothing)
{
	char	s[] = "abc";

	ft_striteri(s, iter_next);
	ASSERT_EQ(lft_malloc_calls(), 0);
}
