/* ft_split. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lft_test.h"

static void	free_split(char **r)
{
	size_t	i;

	for (i = 0; r[i]; i++)
		free(r[i]);
	free(r);
}

/* words se termine par NULL. Toute fuite ou double libération est vue à la fin. */
static void	check_split(const char *s, char c, const char *const *words)
{
	char	**r = ft_split(s, c);
	size_t	i;

	if (r == NULL)
		FAILF("ft_split(%s, %d) : NULL", lft_quote(s), c);
	for (i = 0; words[i]; i++)
	{
		if (r[i] == NULL)
			FAILF("ft_split(%s, %d) : %zu mot(s), au moins %zu attendu(s)",
				lft_quote(s), c, i, i + 1);
		if (strcmp(r[i], words[i]) != 0)
			FAILF("ft_split(%s, %d) : mot %zu : obtenu %s, attendu %s",
				lft_quote(s), c, i, lft_quote(r[i]), lft_quote(words[i]));
	}
	if (r[i] != NULL)
		FAILF("ft_split(%s, %d) : mot en trop après %zu mot(s) : %s", lft_quote(s),
			c, i, lft_quote(r[i]));
	free_split(r);
}

#define WORDS(...) ((const char *const []){__VA_ARGS__, NULL})
#define NO_WORD ((const char *const []){NULL})

TEST(split, basic)
{
	check_split("Hello World Test", ' ', WORDS("Hello", "World", "Test"));
}

TEST(split, multiple_delimiters)
{
	check_split("Hello  World   Test", ' ', WORDS("Hello", "World", "Test"));
}

TEST(split, leading_trailing_delim)
{
	check_split(" Hello World ", ' ', WORDS("Hello", "World"));
}

TEST(split, empty_string)
{
	check_split("", ' ', NO_WORD);
}

TEST(split, only_delimiters)
{
	check_split("   ", ' ', NO_WORD);
}

TEST(split, single_word)
{
	check_split("Hello", ' ', WORDS("Hello"));
}

TEST(split, no_delimiter_found)
{
	check_split("HelloWorld", ' ', WORDS("HelloWorld"));
}

TEST(split, different_delimiter)
{
	check_split("one,two,three,four", ',', WORDS("one", "two", "three", "four"));
}

TEST(split, single_char_words)
{
	check_split("a b c d", ' ', WORDS("a", "b", "c", "d"));
}

TEST(split, complex_example)
{
	check_split("::Hello::World::Test::", ':', WORDS("Hello", "World", "Test"));
}

TEST(split, single_char)
{
	check_split("a", ' ', WORDS("a"));
}

TEST(split, mixed_delimiters)
{
	check_split("a,b,c,d,e", ',', WORDS("a", "b", "c", "d", "e"));
}

TEST(split, consecutive_delimiters_start)
{
	check_split("   hello world", ' ', WORDS("hello", "world"));
}

TEST(split, consecutive_delimiters_end)
{
	check_split("hello world   ", ' ', WORDS("hello", "world"));
}

TEST(split, special_chars)
{
	check_split("hello\nworld\ttest", '\n', WORDS("hello", "world\ttest"));
}

TEST(split, one_delimiter_alone)
{
	check_split(",", ',', NO_WORD);
}

TEST(split, word_and_one_delimiter)
{
	check_split("a,", ',', WORDS("a"));
	check_split(",a", ',', WORDS("a"));
	check_split(",a,", ',', WORDS("a"));
}

TEST(split, nul_delimiter)
{
	check_split("hello world", '\0', WORDS("hello world"));
	check_split("", '\0', NO_WORD);
}

TEST(split, high_byte_delimiter)
{
	check_split("ab\xff" "cd\xff\xff" "ef", '\xff', WORDS("ab", "cd", "ef"));
	check_split("a\x80\xff" "b", '\x80', WORDS("a", "\xff" "b"));
}

TEST(split, delimiter_is_not_a_set)
{
	check_split("a,b;c", ',', WORDS("a", "b;c"));
}

TEST(split, reads_nothing_after_the_terminator)
{
	check_split(lft_guard_str("aa bb"), ' ', WORDS("aa", "bb"));
	check_split(lft_guard_str("aa bb "), ' ', WORDS("aa", "bb"));
	check_split(lft_guard_str("   "), ' ', NO_WORD);
	check_split(lft_guard_str(""), ' ', NO_WORD);
}

TEST(split, input_is_not_modified)
{
	char	s[] = " aa bb ";

	check_split(s, ' ', WORDS("aa", "bb"));
	ASSERT_STR(s, " aa bb ");
}

TEST(split, words_are_independent_blocks)
{
	char	**r = ft_split("aa bb cc", ' ');

	ASSERT_NOT_NULL(r);
	ASSERT_EQ(lft_live_blocks(), 4);
	free(r[1]);
	ASSERT_STR(r[0], "aa");
	ASSERT_STR(r[2], "cc");
	free(r[0]);
	free(r[2]);
	free(r);
}

TEST(split, empty_result_is_one_block)
{
	char	**r = ft_split("   ", ' ');

	ASSERT_NOT_NULL(r);
	ASSERT_NULL(r[0]);
	ASSERT_EQ(lft_live_blocks(), 1);
	free(r);
}

TEST(split, every_delimiter_position)
{
	char	buf[16];
	size_t	pos;

	for (pos = 0; pos < 8; pos++)
	{
		memcpy(buf, "abcdefgh", 9);
		buf[pos] = ',';
		if (pos == 0)
			check_split(buf, ',', WORDS("bcdefgh"));
		else if (pos == 7)
			check_split(buf, ',', WORDS("abcdefg"));
		else
		{
			char	first[9];
			char	second[9];

			memcpy(first, buf, pos);
			first[pos] = '\0';
			strcpy(second, buf + pos + 1);
			check_split(buf, ',', WORDS(first, second));
		}
	}
}

TEST(split, five_thousand_words)
{
	static char	s[10001];
	char		**r;
	size_t		i;

	for (i = 0; i < 5000; i++)
	{
		s[i * 2] = (char)('a' + i % 26);
		s[i * 2 + 1] = ' ';
	}
	r = ft_split(s, ' ');
	ASSERT_NOT_NULL(r);
	for (i = 0; i < 5000; i++)
		ASSERT_EQ(r[i][0], 'a' + i % 26);
	ASSERT_NULL(r[5000]);
	free_split(r);
}

TEST(split, one_word_of_sixteen_kilobytes)
{
	size_t	size = 1 << 14;
	char	*s = lft_guard_buf(size + 1);
	char	**r;

	memset(s, 'a', size);
	s[size] = '\0';
	r = ft_split(s, ' ');
	ASSERT_NOT_NULL(r);
	ASSERT_EQ(strlen(r[0]), size);
	ASSERT_NULL(r[1]);
	free_split(r);
}

TEST(split, malloc_failure_frees_everything)
{
	MALLOC_FAIL_LOOP
	{
		char	**r = ft_split("aa bb cc dd", ' ');

		if (lft_mf_injected())
			ASSERT_NULL(r);
		else
			free_split(r);
	}
}

TEST(split, malloc_failure_on_many_words)
{
	MALLOC_FAIL_LOOP
	{
		char	**r = ft_split("a b c d e f g h i j k l m n o p q r s t", ' ');

		if (lft_mf_injected())
			ASSERT_NULL(r);
		else
			free_split(r);
	}
}

TEST(split, malloc_failure_on_empty_result)
{
	MALLOC_FAIL_LOOP
	{
		char	**r = ft_split("", ' ');

		if (lft_mf_injected())
			ASSERT_NULL(r);
		else
			free_split(r);
	}
}

TEST(split, fuzz_against_a_simple_model)
{
	const char	*alphabet = "ab,";
	char		s[33];
	char		copy[33];
	char		*words[17];
	int			round;
	size_t		i;
	size_t		len;
	size_t		count;
	char		**r;
	const char	*guarded;

	for (round = 0; round < 1500; round++)
	{
		len = lft_rand() % 33;
		for (i = 0; i < len; i++)
			s[i] = alphabet[lft_rand() % 3];
		s[len] = '\0';
		strcpy(copy, s);
		count = 0;
		for (i = 0; i < len; i++)
		{
			if (copy[i] != ',' && (i == 0 || copy[i - 1] == '\0' || copy[i - 1] == ','))
				words[count++] = &copy[i];
			if (copy[i] == ',')
				copy[i] = '\0';
		}
		guarded = lft_guard_str(s);
		r = ft_split(guarded, ',');
		ASSERT_NOT_NULL(r);
		for (i = 0; i < count; i++)
		{
			if (r[i] == NULL || strcmp(r[i], words[i]) != 0)
				FAILF("ft_split(%s, ',') : mot %zu : obtenu %s, attendu %s",
					lft_quote(s), i, lft_quote(r[i]), lft_quote(words[i]));
		}
		if (r[count] != NULL)
			FAILF("ft_split(%s, ',') : mot en trop : %s", lft_quote(s),
				lft_quote(r[count]));
		free_split(r);
		lft_guard_free(guarded);
	}
}
