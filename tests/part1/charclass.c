/* ft_isalpha, ft_isdigit, ft_isalnum, ft_isascii, ft_isprint.
** Le sujet impose un retour d'exactement 1 ou 0. */
#include "lft_test.h"

static int	want_alpha(int c)
{
	return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}

static int	want_digit(int c)
{
	return (c >= '0' && c <= '9');
}

static int	want_alnum(int c)
{
	return (want_alpha(c) || want_digit(c));
}

static int	want_ascii(int c)
{
	return (c >= 0 && c <= 127);
}

static int	want_print(int c)
{
	return (c >= 32 && c <= 126);
}

/* Le man définit ces fonctions pour unsigned char et EOF, soit -1 à 255. */
#define CHECK_EVERY_VALUE(fn, want) \
	do { \
		int	c; \
		\
		for (c = -1; c <= 255; c++) \
			if (fn(c) != want(c)) \
				FAILF(#fn "(%d) : obtenu %d, attendu exactement %d", c, \
					fn(c), want(c)); \
	} while (0)

TEST(isalpha, basic)
{
	ASSERT_EQ(ft_isalpha('a'), 1);
	ASSERT_EQ(ft_isalpha('Z'), 1);
	ASSERT_EQ(ft_isalpha('m'), 1);
	ASSERT_EQ(ft_isalpha('1'), 0);
	ASSERT_EQ(ft_isalpha(' '), 0);
	ASSERT_EQ(ft_isalpha('\n'), 0);
}

TEST(isalpha, edge)
{
	ASSERT_EQ(ft_isalpha('@'), 0);
	ASSERT_EQ(ft_isalpha('A'), 1);
	ASSERT_EQ(ft_isalpha('Z'), 1);
	ASSERT_EQ(ft_isalpha('['), 0);
	ASSERT_EQ(ft_isalpha('`'), 0);
	ASSERT_EQ(ft_isalpha('a'), 1);
	ASSERT_EQ(ft_isalpha('z'), 1);
	ASSERT_EQ(ft_isalpha('{'), 0);
	ASSERT_EQ(ft_isalpha(0), 0);
	ASSERT_EQ(ft_isalpha(127), 0);
}

TEST(isalpha, every_value_returns_exactly_1_or_0)
{
	CHECK_EVERY_VALUE(ft_isalpha, want_alpha);
}

TEST(isdigit, basic)
{
	ASSERT_EQ(ft_isdigit('0'), 1);
	ASSERT_EQ(ft_isdigit('5'), 1);
	ASSERT_EQ(ft_isdigit('9'), 1);
	ASSERT_EQ(ft_isdigit('a'), 0);
	ASSERT_EQ(ft_isdigit(' '), 0);
	ASSERT_EQ(ft_isdigit('/'), 0);
	ASSERT_EQ(ft_isdigit(':'), 0);
}

TEST(isdigit, every_value_returns_exactly_1_or_0)
{
	CHECK_EVERY_VALUE(ft_isdigit, want_digit);
}

TEST(isalnum, basic)
{
	ASSERT_EQ(ft_isalnum('a'), 1);
	ASSERT_EQ(ft_isalnum('Z'), 1);
	ASSERT_EQ(ft_isalnum('5'), 1);
	ASSERT_EQ(ft_isalnum(' '), 0);
	ASSERT_EQ(ft_isalnum('!'), 0);
	ASSERT_EQ(ft_isalnum('@'), 0);
}

TEST(isalnum, every_value_returns_exactly_1_or_0)
{
	CHECK_EVERY_VALUE(ft_isalnum, want_alnum);
}

TEST(isascii, basic)
{
	ASSERT_EQ(ft_isascii(0), 1);
	ASSERT_EQ(ft_isascii(127), 1);
	ASSERT_EQ(ft_isascii('A'), 1);
	ASSERT_EQ(ft_isascii(128), 0);
	ASSERT_EQ(ft_isascii(-1), 0);
	ASSERT_EQ(ft_isascii(200), 0);
}

TEST(isascii, every_value_returns_exactly_1_or_0)
{
	CHECK_EVERY_VALUE(ft_isascii, want_ascii);
}

TEST(isprint, basic)
{
	ASSERT_EQ(ft_isprint(' '), 1);
	ASSERT_EQ(ft_isprint('~'), 1);
	ASSERT_EQ(ft_isprint('A'), 1);
	ASSERT_EQ(ft_isprint(31), 0);
	ASSERT_EQ(ft_isprint(127), 0);
	ASSERT_EQ(ft_isprint('\n'), 0);
	ASSERT_EQ(ft_isprint('\t'), 0);
}

TEST(isprint, every_value_returns_exactly_1_or_0)
{
	CHECK_EVERY_VALUE(ft_isprint, want_print);
}

TEST(isalpha, mixed_string)
{
	const char	*s = "Hello123!\n\t";

	ASSERT_EQ(ft_isalpha(s[0]), 1);
	ASSERT_EQ(ft_isalnum(s[0]), 1);
	ASSERT_EQ(ft_isascii(s[0]), 1);
	ASSERT_EQ(ft_isprint(s[0]), 1);
	ASSERT_EQ(ft_isalpha(s[5]), 0);
	ASSERT_EQ(ft_isdigit(s[5]), 1);
	ASSERT_EQ(ft_isalnum(s[5]), 1);
	ASSERT_EQ(ft_isalpha(s[8]), 0);
	ASSERT_EQ(ft_isdigit(s[8]), 0);
	ASSERT_EQ(ft_isalnum(s[8]), 0);
	ASSERT_EQ(ft_isascii(s[8]), 1);
	ASSERT_EQ(ft_isprint(s[8]), 1);
	ASSERT_EQ(ft_isprint(s[9]), 0);
}
