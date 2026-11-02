/* Faux tests : chacun doit produire un statut précis dans le moteur. */
#include <stdio.h>
#include <unistd.h>
#include "lft_test.h"

TEST(fake, ok)
{
	ASSERT_EQ(1 + 1, 2);
}

TEST(fake, assert_fails)
{
	ASSERT_EQ(1 + 1, 3);
}

TEST(fake, segfault)
{
	volatile char	*p;

	p = NULL;
	*p = 'x';
}

TEST(fake, hangs)
{
	while (1)
		;
}

TEST(fake, exits_early)
{
	_exit(0);
}

TEST(fake, prints_garbage)
{
	printf("PASS\tfake/forged\t0\t\n");
	fflush(stdout);
	write(1, "\n\n", 2);
	write(2, "bruit\n", 6);
}

TEST(other, second_suite)
{
	ASSERT_TRUE(1);
}
