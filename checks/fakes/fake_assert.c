/* Faux tests : messages produits par chaque assertion. */
#include <stdlib.h>
#include "lft_test.h"

TEST(assert, all_pass)
{
	char	buf[3] = "ab";

	ASSERT_TRUE(1);
	ASSERT_FALSE(0);
	ASSERT_EQ(2, 2);
	ASSERT_STR("ab", "ab");
	ASSERT_MEM(buf, "ab", 3);
	ASSERT_PTR(buf, buf);
	ASSERT_NULL(NULL);
	ASSERT_NOT_NULL(buf);
	ASSERT_SIGN(-3, -100);
	ASSERT_SIGN(7, 1);
	ASSERT_SIGN(0, 0);
}

TEST(assert, str_diff)
{
	ASSERT_STR("abc", "abd");
}

TEST(assert, str_null)
{
	ASSERT_STR(NULL, "abd");
}

TEST(assert, str_unprintable)
{
	ASSERT_STR("a\xff\n\t\"", "b");
}

TEST(assert, mem_diff)
{
	ASSERT_MEM("abcd", "abxd", 4);
}

TEST(assert, ptr_shifted)
{
	char	buf[8] = {0};

	ASSERT_PTR(buf + 3, buf);
}

TEST(assert, null_expected)
{
	char	c = 0;

	ASSERT_NULL(&c);
}

TEST(assert, not_null_expected)
{
	char	*p = NULL;

	ASSERT_NOT_NULL(p);
}

TEST(assert, sign_opposite)
{
	ASSERT_SIGN(-3, 5);
}

TEST(assert, sign_zero_vs_positive)
{
	ASSERT_SIGN(0, 5);
}

TEST(assert, sign_negative_vs_zero)
{
	ASSERT_SIGN(-1, 0);
}

TEST(assert, eq_large_size_t)
{
	ASSERT_EQ((size_t)5, 6);
}
