/* Faux tests : une fonction absente ne fait échouer que les tests qui l'appellent. */
#include "lft_test.h"

TEST(missing, calls_absent_function)
{
	ASSERT_EQ(ft_lstsize(NULL), 0);
}

TEST(missing, calls_present_function)
{
	ASSERT_EQ(ft_isalpha('a'), 'a');
}
