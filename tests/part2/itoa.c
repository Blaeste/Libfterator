/* ft_itoa. */
#include <stdio.h>
#include <stdlib.h>
#include "lft_test.h"

#define CHECK_ITOA(n, want) \
	do { \
		char	*lft_r = ft_itoa(n); \
		\
		ASSERT_STR_AS("ft_itoa(" #n ")", lft_r, (want)); \
		free(lft_r); \
	} while (0)

TEST(itoa, positive)
{
	CHECK_ITOA(123, "123");
}

TEST(itoa, negative)
{
	CHECK_ITOA(-123, "-123");
}

TEST(itoa, zero)
{
	CHECK_ITOA(0, "0");
}

TEST(itoa, int_max)
{
	CHECK_ITOA(INT_MAX, "2147483647");
}

TEST(itoa, int_min)
{
	CHECK_ITOA(INT_MIN, "-2147483648");
}

TEST(itoa, near_the_limits)
{
	CHECK_ITOA(INT_MAX - 1, "2147483646");
	CHECK_ITOA(INT_MIN + 1, "-2147483647");
}

TEST(itoa, single_digit_positive)
{
	CHECK_ITOA(7, "7");
}

TEST(itoa, single_digit_negative)
{
	CHECK_ITOA(-7, "-7");
}

TEST(itoa, large_positive)
{
	CHECK_ITOA(987654321, "987654321");
}

TEST(itoa, large_negative)
{
	CHECK_ITOA(-987654321, "-987654321");
}

TEST(itoa, powers_of_ten)
{
	CHECK_ITOA(10, "10");
	CHECK_ITOA(100, "100");
	CHECK_ITOA(1000, "1000");
	CHECK_ITOA(-10, "-10");
	CHECK_ITOA(-100, "-100");
	CHECK_ITOA(1000000000, "1000000000");
	CHECK_ITOA(-1000000000, "-1000000000");
}

TEST(itoa, every_digit_count)
{
	char		want[16];
	const int	bounds[] = {9, 10, 99, 100, 999, 1000, 9999, 10000, 99999, 100000,
		999999, 1000000, 9999999, 10000000, 99999999, 100000000, 999999999,
		1000000000};
	size_t		i;
	char		*r;

	for (i = 0; i < sizeof(bounds) / sizeof(bounds[0]); i++)
	{
		snprintf(want, sizeof(want), "%d", bounds[i]);
		r = ft_itoa(bounds[i]);
		ASSERT_STR(r, want);
		free(r);
		snprintf(want, sizeof(want), "%d", -bounds[i]);
		r = ft_itoa(-bounds[i]);
		ASSERT_STR(r, want);
		free(r);
	}
}

TEST(itoa, results_are_independent_blocks)
{
	char	*a = ft_itoa(1);
	char	*b = ft_itoa(2);

	ASSERT_NOT_NULL(a);
	ASSERT_NOT_NULL(b);
	ASSERT_TRUE(a != b);
	ASSERT_STR(a, "1");
	ASSERT_STR(b, "2");
	free(a);
	free(b);
}

TEST(itoa, fuzz_against_printf)
{
	char	want[16];
	char	*r;
	int		round;
	int		value;

	for (round = 0; round < 3000; round++)
	{
		value = (int)lft_rand();
		if (round % 3 == 0)
			value %= 1000;
		snprintf(want, sizeof(want), "%d", value);
		r = ft_itoa(value);
		ASSERT_STR(r, want);
		free(r);
	}
}

TEST(itoa, malloc_failure_returns_null)
{
	MALLOC_FAIL_LOOP
	{
		char	*r = ft_itoa(-4242);

		if (lft_mf_injected())
			ASSERT_NULL(r);
		else
			free(r);
	}
}

TEST(itoa, malloc_failure_on_int_min)
{
	MALLOC_FAIL_LOOP
	{
		char	*r = ft_itoa(INT_MIN);

		if (lft_mf_injected())
			ASSERT_NULL(r);
		else
			free(r);
	}
}

TEST(itoa, malloc_failure_on_zero)
{
	MALLOC_FAIL_LOOP
	{
		char	*r = ft_itoa(0);

		if (lft_mf_injected())
			ASSERT_NULL(r);
		else
			free(r);
	}
}
