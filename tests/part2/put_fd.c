/* ft_putchar_fd, ft_putstr_fd, ft_putendl_fd, ft_putnbr_fd. */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "lft_test.h"

#define CHECK_OUTPUT(fd, want) \
	do { \
		char	lft_buf[256]; \
		size_t	lft_n = lft_capture_end((fd), lft_buf, sizeof(lft_buf)); \
		\
		ASSERT_EQ(lft_n, sizeof(want) - 1); \
		ASSERT_MEM(lft_buf, (want), lft_n); \
	} while (0)

TEST(putchar_fd, basic)
{
	int	fd = lft_capture_begin();

	ft_putchar_fd('A', fd);
	ft_putchar_fd('B', fd);
	CHECK_OUTPUT(fd, "AB");
}

TEST(putchar_fd, nul_byte)
{
	int	fd = lft_capture_begin();

	ft_putchar_fd('\0', fd);
	CHECK_OUTPUT(fd, "\0");
}

TEST(putchar_fd, high_byte)
{
	int	fd = lft_capture_begin();

	ft_putchar_fd('\xff', fd);
	ft_putchar_fd('\x80', fd);
	CHECK_OUTPUT(fd, "\xff\x80");
}

TEST(putchar_fd, every_byte)
{
	int		fd = lft_capture_begin();
	char	buf[300];
	int		c;

	for (c = 0; c < 256; c++)
		ft_putchar_fd((char)c, fd);
	ASSERT_EQ(lft_capture_end(fd, buf, sizeof(buf)), 256);
	for (c = 0; c < 256; c++)
		ASSERT_EQ((unsigned char)buf[c], c);
}

TEST(putchar_fd, invalid_fd_does_not_crash)
{
	ft_putchar_fd('x', -1);
	ft_putchar_fd('x', 4242);
}

TEST(putstr_fd, basic)
{
	int	fd = lft_capture_begin();

	ft_putstr_fd("Hello World", fd);
	CHECK_OUTPUT(fd, "Hello World");
}

TEST(putstr_fd, empty)
{
	int	fd = lft_capture_begin();

	ft_putstr_fd("", fd);
	CHECK_OUTPUT(fd, "");
}

TEST(putstr_fd, no_trailing_newline)
{
	int	fd = lft_capture_begin();

	ft_putstr_fd("a", fd);
	ft_putstr_fd("b", fd);
	CHECK_OUTPUT(fd, "ab");
}

TEST(putstr_fd, keeps_inner_newlines)
{
	int	fd = lft_capture_begin();

	ft_putstr_fd("a\nb\n", fd);
	CHECK_OUTPUT(fd, "a\nb\n");
}

TEST(putstr_fd, high_bytes)
{
	int	fd = lft_capture_begin();

	ft_putstr_fd("\xff\x80z", fd);
	CHECK_OUTPUT(fd, "\xff\x80z");
}

TEST(putstr_fd, reads_nothing_after_the_terminator)
{
	int	fd = lft_capture_begin();

	ft_putstr_fd((char *)lft_guard_str("abc"), fd);
	ft_putstr_fd((char *)lft_guard_str(""), fd);
	CHECK_OUTPUT(fd, "abc");
}

TEST(putstr_fd, invalid_fd_does_not_crash)
{
	ft_putstr_fd("x", -1);
}

TEST(putstr_fd, twenty_thousand_chars)
{
	static char	s[20001];
	static char	out[20010];
	int			fd = lft_capture_begin();

	memset(s, 'q', 20000);
	ft_putstr_fd(s, fd);
	ASSERT_EQ(lft_capture_end(fd, out, sizeof(out)), 20000);
	ASSERT_MEM(out, s, 20000);
}

TEST(putstr_fd, writes_to_stdout_when_asked)
{
	int		fd = lft_capture_begin();
	int		saved = dup(STDOUT_FILENO);

	dup2(fd, STDOUT_FILENO);
	ft_putstr_fd("sortie", STDOUT_FILENO);
	dup2(saved, STDOUT_FILENO);
	close(saved);
	CHECK_OUTPUT(fd, "sortie");
}

TEST(putstr_fd, sequence_of_all_put_functions)
{
	int	fd = lft_capture_begin();

	ft_putchar_fd('X', fd);
	ft_putstr_fd("test", fd);
	ft_putendl_fd("line", fd);
	ft_putnbr_fd(42, fd);
	CHECK_OUTPUT(fd, "Xtestline\n42");
}

TEST(putendl_fd, basic)
{
	int	fd = lft_capture_begin();

	ft_putendl_fd("Hello", fd);
	CHECK_OUTPUT(fd, "Hello\n");
}

TEST(putendl_fd, empty)
{
	int	fd = lft_capture_begin();

	ft_putendl_fd("", fd);
	CHECK_OUTPUT(fd, "\n");
}

TEST(putendl_fd, string_with_newline)
{
	int	fd = lft_capture_begin();

	ft_putendl_fd("a\n", fd);
	CHECK_OUTPUT(fd, "a\n\n");
}

TEST(putendl_fd, twice)
{
	int	fd = lft_capture_begin();

	ft_putendl_fd("a", fd);
	ft_putendl_fd("b", fd);
	CHECK_OUTPUT(fd, "a\nb\n");
}

TEST(putendl_fd, reads_nothing_after_the_terminator)
{
	int	fd = lft_capture_begin();

	ft_putendl_fd((char *)lft_guard_str("abc"), fd);
	CHECK_OUTPUT(fd, "abc\n");
}

TEST(putendl_fd, invalid_fd_does_not_crash)
{
	ft_putendl_fd("x", -1);
}

#define CHECK_NBR(n, want) \
	do { \
		int	lft_fd = lft_capture_begin(); \
		\
		ft_putnbr_fd((n), lft_fd); \
		CHECK_OUTPUT(lft_fd, want); \
	} while (0)

TEST(putnbr_fd, positive)
{
	CHECK_NBR(123, "123");
}

TEST(putnbr_fd, negative)
{
	CHECK_NBR(-456, "-456");
}

TEST(putnbr_fd, zero)
{
	CHECK_NBR(0, "0");
}

TEST(putnbr_fd, int_max)
{
	CHECK_NBR(INT_MAX, "2147483647");
}

TEST(putnbr_fd, int_min)
{
	CHECK_NBR(INT_MIN, "-2147483648");
}

TEST(putnbr_fd, int_min_max_together)
{
	int	fd = lft_capture_begin();

	ft_putnbr_fd(INT_MAX, fd);
	ft_putchar_fd(' ', fd);
	ft_putnbr_fd(INT_MIN, fd);
	CHECK_OUTPUT(fd, "2147483647 -2147483648");
}

TEST(putnbr_fd, digit_boundaries)
{
	CHECK_NBR(9, "9");
	CHECK_NBR(10, "10");
	CHECK_NBR(99, "99");
	CHECK_NBR(100, "100");
	CHECK_NBR(-9, "-9");
	CHECK_NBR(-10, "-10");
	CHECK_NBR(1000000000, "1000000000");
	CHECK_NBR(-1000000000, "-1000000000");
}

TEST(putnbr_fd, no_newline_between_calls)
{
	int	fd = lft_capture_begin();

	ft_putnbr_fd(1, fd);
	ft_putnbr_fd(2, fd);
	CHECK_OUTPUT(fd, "12");
}

TEST(putnbr_fd, invalid_fd_does_not_crash)
{
	ft_putnbr_fd(42, -1);
}

TEST(putnbr_fd, fuzz_against_printf)
{
	char	want[16];
	char	got[32];
	int		round;
	int		value;
	int		fd;

	for (round = 0; round < 2000; round++)
	{
		value = (int)lft_rand();
		snprintf(want, sizeof(want), "%d", value);
		fd = lft_capture_begin();
		ft_putnbr_fd(value, fd);
		lft_capture_end(fd, got, sizeof(got));
		ASSERT_STR(got, want);
	}
}
