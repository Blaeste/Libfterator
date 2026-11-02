/* ft_atoi. Le dépassement d'un int est indéfini : voir tests/info. */
#include <stdio.h>
#include <stdlib.h>
#include "lft_test.h"

TEST(atoi, basic_positive)
{
	ASSERT_EQ(ft_atoi("123"), 123);
}

TEST(atoi, basic_negative)
{
	ASSERT_EQ(ft_atoi("-123"), -123);
}

TEST(atoi, with_plus)
{
	ASSERT_EQ(ft_atoi("+42"), 42);
}

TEST(atoi, zero)
{
	ASSERT_EQ(ft_atoi("0"), 0);
	ASSERT_EQ(ft_atoi("-0"), 0);
	ASSERT_EQ(ft_atoi("+0"), 0);
}

TEST(atoi, leading_whitespace)
{
	ASSERT_EQ(ft_atoi("   \t\n  42"), 42);
}

TEST(atoi, trailing_chars)
{
	ASSERT_EQ(ft_atoi("123abc"), 123);
}

TEST(atoi, no_digits)
{
	ASSERT_EQ(ft_atoi("abc"), 0);
}

TEST(atoi, empty_string)
{
	ASSERT_EQ(ft_atoi(""), 0);
}

TEST(atoi, only_whitespace)
{
	ASSERT_EQ(ft_atoi("   \t\n  "), 0);
}

TEST(atoi, multiple_signs)
{
	ASSERT_EQ(ft_atoi("+-42"), 0);
}

TEST(atoi, large_number)
{
	ASSERT_EQ(ft_atoi("2147483647"), INT_MAX);
}

TEST(atoi, negative_large)
{
	ASSERT_EQ(ft_atoi("-2147483648"), INT_MIN);
}

TEST(atoi, double_signs)
{
	ASSERT_EQ(ft_atoi("--123"), 0);
}

TEST(atoi, minus_plus)
{
	ASSERT_EQ(ft_atoi("-+123"), 0);
}

TEST(atoi, plus_plus)
{
	ASSERT_EQ(ft_atoi("++123"), 0);
}

TEST(atoi, space_between_sign)
{
	ASSERT_EQ(ft_atoi("- 123"), 0);
}

TEST(atoi, plus_space)
{
	ASSERT_EQ(ft_atoi("+ 123"), 0);
}

TEST(atoi, plus_newline)
{
	ASSERT_EQ(ft_atoi("+\n123"), 0);
}

TEST(atoi, all_whitespace)
{
	ASSERT_EQ(ft_atoi(" \t\v\n\r\f123"), 123);
}

TEST(atoi, each_whitespace_alone)
{
	ASSERT_EQ(ft_atoi(" 7"), 7);
	ASSERT_EQ(ft_atoi("\t7"), 7);
	ASSERT_EQ(ft_atoi("\n7"), 7);
	ASSERT_EQ(ft_atoi("\v7"), 7);
	ASSERT_EQ(ft_atoi("\f7"), 7);
	ASSERT_EQ(ft_atoi("\r7"), 7);
}

TEST(atoi, other_control_chars_are_not_whitespace)
{
	ASSERT_EQ(ft_atoi("\x01" "7"), 0);
	ASSERT_EQ(ft_atoi("\x08" "7"), 0);
	ASSERT_EQ(ft_atoi("\x0e" "7"), 0);
	ASSERT_EQ(ft_atoi("\x7f" "7"), 0);
}

TEST(atoi, high_bytes_are_not_digits_nor_whitespace)
{
	ASSERT_EQ(ft_atoi("\xff" "7"), 0);
	ASSERT_EQ(ft_atoi("\x80" "7"), 0);
	ASSERT_EQ(ft_atoi("7\xb0"), 7);
}

TEST(atoi, leading_zeros)
{
	ASSERT_EQ(ft_atoi("+0000000000000000000000000000000000000000000000000000123"), 123);
	ASSERT_EQ(ft_atoi("-0000000000000000000000000000000000000000000000000000123"), -123);
}

TEST(atoi, slash_separator)
{
	ASSERT_EQ(ft_atoi("12/3"), 12);
}

TEST(atoi, semicolon_separator)
{
	ASSERT_EQ(ft_atoi("12;3"), 12);
}

TEST(atoi, sign_alone)
{
	ASSERT_EQ(ft_atoi("+"), 0);
	ASSERT_EQ(ft_atoi("-"), 0);
	ASSERT_EQ(ft_atoi("   -"), 0);
}

TEST(atoi, sign_after_digits)
{
	ASSERT_EQ(ft_atoi("12-3"), 12);
	ASSERT_EQ(ft_atoi("12+3"), 12);
}

TEST(atoi, whitespace_after_digits)
{
	ASSERT_EQ(ft_atoi("12 3"), 12);
}

TEST(atoi, whitespace_then_sign)
{
	ASSERT_EQ(ft_atoi(" \t-42"), -42);
	ASSERT_EQ(ft_atoi("\n+42"), 42);
}

TEST(atoi, not_decimal_notations)
{
	ASSERT_EQ(ft_atoi("12.5"), 12);
	ASSERT_EQ(ft_atoi("0x10"), 0);
	ASSERT_EQ(ft_atoi("1e3"), 1);
}

TEST(atoi, reads_nothing_after_the_terminator)
{
	ASSERT_EQ(ft_atoi(lft_guard_str("42")), 42);
	ASSERT_EQ(ft_atoi(lft_guard_str("-")), 0);
	ASSERT_EQ(ft_atoi(lft_guard_str("  ")), 0);
	ASSERT_EQ(ft_atoi(lft_guard_str("")), 0);
}

TEST(atoi, every_power_of_ten)
{
	char	buf[16];
	int		value;
	int		i;

	value = 1;
	for (i = 0; i < 10; i++)
	{
		snprintf(buf, sizeof(buf), "%d", value);
		ASSERT_EQ(ft_atoi(buf), value);
		snprintf(buf, sizeof(buf), "%d", -value);
		ASSERT_EQ(ft_atoi(buf), -value);
		if (i < 9)
			value *= 10;
	}
}

TEST(atoi, fuzz_on_valid_integers)
{
	char	buf[32];
	int		round;
	int		value;

	for (round = 0; round < 3000; round++)
	{
		value = (int)lft_rand();
		snprintf(buf, sizeof(buf), "%d", value);
		ASSERT_EQ(ft_atoi(buf), value);
	}
}

TEST(atoi, fuzz_against_libc_on_arbitrary_text)
{
	const char	*alphabet = " \t\n\v\f\r+-0123456789a\x7f\xff";
	char		buf[10];
	const char	*s;
	int			round;
	size_t		i;
	size_t		len;

	for (round = 0; round < 3000; round++)
	{
		len = lft_rand() % 10;
		for (i = 0; i < len; i++)
			buf[i] = alphabet[lft_rand() % 21];
		buf[len] = '\0';
		s = lft_guard_str(buf);
		if (ft_atoi(s) != atoi(s))
			FAILF("ft_atoi(%s) : obtenu %d, atoi donne %d", lft_quote(buf),
				ft_atoi(s), atoi(s));
		lft_guard_free(s);
	}
}
