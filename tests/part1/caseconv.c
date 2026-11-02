/* ft_toupper, ft_tolower. */
#include "lft_test.h"

static int	want_upper(int c)
{
	return ((c >= 'a' && c <= 'z') ? c - 32 : c);
}

static int	want_lower(int c)
{
	return ((c >= 'A' && c <= 'Z') ? c + 32 : c);
}

TEST(toupper, basic)
{
	ASSERT_EQ(ft_toupper('a'), 'A');
	ASSERT_EQ(ft_toupper('z'), 'Z');
	ASSERT_EQ(ft_toupper('A'), 'A');
	ASSERT_EQ(ft_toupper('Z'), 'Z');
}

TEST(toupper, non_alpha)
{
	ASSERT_EQ(ft_toupper('1'), '1');
	ASSERT_EQ(ft_toupper('!'), '!');
	ASSERT_EQ(ft_toupper(' '), ' ');
	ASSERT_EQ(ft_toupper('\n'), '\n');
	ASSERT_EQ(ft_toupper('@'), '@');
}

TEST(toupper, boundaries)
{
	ASSERT_EQ(ft_toupper('@'), '@');
	ASSERT_EQ(ft_toupper('['), '[');
	ASSERT_EQ(ft_toupper('`'), '`');
	ASSERT_EQ(ft_toupper('{'), '{');
}

TEST(toupper, numbers_and_special)
{
	const char	*s = "0123456789!@#$%^&*()_+-=[]{}|;:,.<>?";
	int			i;

	for (i = 0; s[i]; i++)
		ASSERT_EQ(ft_toupper(s[i]), s[i]);
}

TEST(toupper, all_letters)
{
	int	c;

	for (c = 'a'; c <= 'z'; c++)
		ASSERT_EQ(ft_toupper(c), c - 32);
	for (c = 'A'; c <= 'Z'; c++)
		ASSERT_EQ(ft_toupper(c), c);
}

TEST(toupper, limits)
{
	ASSERT_EQ(ft_toupper(0), 0);
	ASSERT_EQ(ft_toupper(127), 127);
	ASSERT_EQ(ft_toupper(-1), -1);
	ASSERT_EQ(ft_toupper(128), 128);
	ASSERT_EQ(ft_toupper(255), 255);
}

TEST(toupper, every_value)
{
	int	c;

	for (c = -1; c <= 255; c++)
		if (ft_toupper(c) != want_upper(c))
			FAILF("ft_toupper(%d) : obtenu %d, attendu %d", c, ft_toupper(c),
				want_upper(c));
}

TEST(tolower, basic)
{
	ASSERT_EQ(ft_tolower('A'), 'a');
	ASSERT_EQ(ft_tolower('Z'), 'z');
	ASSERT_EQ(ft_tolower('a'), 'a');
	ASSERT_EQ(ft_tolower('z'), 'z');
}

TEST(tolower, non_alpha)
{
	ASSERT_EQ(ft_tolower('1'), '1');
	ASSERT_EQ(ft_tolower('!'), '!');
	ASSERT_EQ(ft_tolower(' '), ' ');
	ASSERT_EQ(ft_tolower('\n'), '\n');
	ASSERT_EQ(ft_tolower('['), '[');
}

TEST(tolower, boundaries)
{
	ASSERT_EQ(ft_tolower('@'), '@');
	ASSERT_EQ(ft_tolower('['), '[');
	ASSERT_EQ(ft_tolower('`'), '`');
	ASSERT_EQ(ft_tolower('{'), '{');
}

TEST(tolower, numbers_and_special)
{
	const char	*s = "0123456789!@#$%^&*()_+-=[]{}|;:,.<>?";
	int			i;

	for (i = 0; s[i]; i++)
		ASSERT_EQ(ft_tolower(s[i]), s[i]);
}

TEST(tolower, all_letters)
{
	int	c;

	for (c = 'A'; c <= 'Z'; c++)
		ASSERT_EQ(ft_tolower(c), c + 32);
	for (c = 'a'; c <= 'z'; c++)
		ASSERT_EQ(ft_tolower(c), c);
}

TEST(tolower, limits)
{
	ASSERT_EQ(ft_tolower(0), 0);
	ASSERT_EQ(ft_tolower(127), 127);
	ASSERT_EQ(ft_tolower(-1), -1);
	ASSERT_EQ(ft_tolower(128), 128);
	ASSERT_EQ(ft_tolower(255), 255);
}

TEST(tolower, every_value)
{
	int	c;

	for (c = -1; c <= 255; c++)
		if (ft_tolower(c) != want_lower(c))
			FAILF("ft_tolower(%d) : obtenu %d, attendu %d", c, ft_tolower(c),
				want_lower(c));
}
