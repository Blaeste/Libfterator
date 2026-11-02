/* Faux tests : chaque malloc échoue à son tour. */
#include <stdlib.h>
#include <string.h>
#include "lft_test.h"

static char	*good_three(void)
{
	char	*a = malloc(1);
	char	*b;
	char	*c;

	if (!a)
		return (NULL);
	b = malloc(2);
	if (!b)
		return (free(a), NULL);
	c = malloc(3);
	if (!c)
		return (free(a), free(b), NULL);
	free(a);
	free(b);
	return (c);
}

static char	*leaks_when_third_fails(void)
{
	char	*a = malloc(1);
	char	*b;
	char	*c;

	if (!a)
		return (NULL);
	b = malloc(2);
	if (!b)
		return (free(a), NULL);
	c = malloc(3);
	if (!c)
		return (free(a), NULL);
	free(a);
	free(b);
	return (c);
}

static char	*unchecked(void)
{
	char	*a = malloc(4);

	a[0] = 'x';
	return (a);
}

static char	*returns_static_on_failure(void)
{
	static char	fallback[4];
	char		*a = malloc(4);

	return (a ? a : fallback);
}

static int	g_loops;

TEST(mf, correct_function)
{
	MALLOC_FAIL_LOOP
	{
		char	*r = good_three();

		g_loops++;
		if (lft_mf_injected())
			ASSERT_NULL(r);
		else
			free(r);
	}
	/* un passage pour compter, puis un par malloc */
	ASSERT_EQ(g_loops, 4);
}

TEST(mf, leak_is_reported_with_the_malloc_number)
{
	MALLOC_FAIL_LOOP
	{
		char	*r = leaks_when_third_fails();

		if (lft_mf_injected())
			ASSERT_NULL(r);
		else
			free(r);
	}
}

TEST(mf, unchecked_malloc_crashes)
{
	MALLOC_FAIL_LOOP
	{
		char	*r = unchecked();

		free(r);
	}
}

TEST(mf, non_null_after_failure_fails)
{
	MALLOC_FAIL_LOOP
	{
		char	*r = returns_static_on_failure();

		if (lft_mf_injected())
			ASSERT_NULL(r);
		else
			free(r);
	}
}

TEST(mf, function_without_malloc_fails)
{
	MALLOC_FAIL_LOOP
	{
		lft_mf_injected();
	}
}

TEST(mf, test_blocks_are_never_injected)
{
	MALLOC_FAIL_LOOP
	{
		char	*own = lft_alloc(8);
		char	*r = good_three();

		ASSERT_NOT_NULL(own);
		free(own);
		free(r);
	}
}
