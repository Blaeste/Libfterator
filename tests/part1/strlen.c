/* ft_strlen. */
#include <string.h>
#include "lft_test.h"

TEST(strlen, basic)
{
	ASSERT_EQ(ft_strlen("Hello"), 5);
}

TEST(strlen, empty)
{
	ASSERT_EQ(ft_strlen(""), 0);
}

TEST(strlen, single_char)
{
	ASSERT_EQ(ft_strlen("a"), 1);
}

TEST(strlen, long_string)
{
	const char	*s = "This is a very long string to test strlen implementation";

	ASSERT_EQ(ft_strlen(s), strlen(s));
}

TEST(strlen, with_numbers)
{
	ASSERT_EQ(ft_strlen("abc123def456"), 12);
}

TEST(strlen, special_chars)
{
	const char	*s = "!@#$%^&*()_+-=[]{}|;:,.<>?";

	ASSERT_EQ(ft_strlen(s), strlen(s));
}

TEST(strlen, spaces)
{
	ASSERT_EQ(ft_strlen("   hello   world   "), 19);
}

TEST(strlen, high_bytes)
{
	ASSERT_EQ(ft_strlen("\x80\xff\xc3\xa9"), 4);
}

TEST(strlen, stops_at_first_nul)
{
	ASSERT_EQ(ft_strlen("abc\0def"), 3);
}

TEST(strlen, reads_nothing_after_the_terminator)
{
	ASSERT_EQ(ft_strlen(lft_guard_str("")), 0);
	ASSERT_EQ(ft_strlen(lft_guard_str("abc")), 3);
	ASSERT_EQ(ft_strlen(lft_guard_str("abcdefghijklmnopqrstuvwxyz")), 26);
}

TEST(strlen, every_length_up_to_64)
{
	char		buf[65];
	const char	*s;
	size_t		len;

	memset(buf, 'x', sizeof(buf));
	for (len = 0; len <= 64; len++)
	{
		buf[len] = '\0';
		s = lft_guard_str(buf);
		ASSERT_EQ(ft_strlen(s), len);
		lft_guard_free(s);
		buf[len] = 'x';
	}
}

TEST(strlen, one_million_chars)
{
	size_t	size = 1000000;
	char	*s = lft_guard_buf(size + 1);

	memset(s, 'z', size);
	s[size] = '\0';
	ASSERT_EQ(ft_strlen(s), size);
}
