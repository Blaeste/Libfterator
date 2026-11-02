/* ft_memcpy, ft_memmove. */
#include <string.h>
#include "lft_test.h"

TEST(memcpy, basic)
{
	char	dest[20];

	ft_memcpy(dest, "Hello World", 12);
	ASSERT_STR(dest, "Hello World");
}

TEST(memcpy, partial)
{
	char	dest[20];
	char	want[20];

	memset(dest, 'X', 20);
	memset(want, 'X', 20);
	memcpy(want, "abcde", 5);
	ft_memcpy(dest, "abcdefghij", 5);
	ASSERT_MEM(dest, want, 20);
}

TEST(memcpy, zero_size)
{
	char	dest[10] = "original";

	ASSERT_PTR(ft_memcpy(dest, "test", 0), dest);
	ASSERT_STR(dest, "original");
}

TEST(memcpy, zero_size_reads_and_writes_nothing)
{
	char		*dest = lft_guard_buf(0);
	const char	*src = lft_guard_mem("", 0);

	ASSERT_PTR(ft_memcpy(dest, src, 0), dest);
}

TEST(memcpy, return_value)
{
	char	dest[10];

	ASSERT_PTR(ft_memcpy(dest, "test", 5), dest);
}

TEST(memcpy, binary_data)
{
	unsigned char	src[] = {0, 1, 2, 255, 254, 128, 127};
	unsigned char	dest[10];

	ft_memcpy(dest, src, 7);
	ASSERT_MEM(dest, src, 7);
}

TEST(memcpy, does_not_stop_at_nul)
{
	char	dest[8];

	memset(dest, 'X', 8);
	ft_memcpy(dest, "ab\0cd\0ef", 8);
	ASSERT_MEM(dest, "ab\0cd\0ef", 8);
}

TEST(memcpy, large)
{
	char	src[1000];
	char	dest[1000];
	int		i;

	for (i = 0; i < 1000; i++)
		src[i] = (char)(i % 256);
	ft_memcpy(dest, src, 1000);
	ASSERT_MEM(dest, src, 1000);
}

TEST(memcpy, struct)
{
	struct s_v { int x; char y; double z; }	src;
	struct s_v								dest;

	memset(&src, 0, sizeof(src));
	src.x = 42;
	src.y = 'A';
	src.z = 3.14159;
	ft_memcpy(&dest, &src, sizeof(src));
	ASSERT_MEM(&dest, &src, sizeof(src));
	ASSERT_EQ(dest.x, 42);
	ASSERT_EQ(dest.y, 'A');
}

TEST(memcpy, alignment)
{
	char	src[100];
	char	dest[100];
	char	want[100];
	int		i;

	memset(dest, 0x42, 100);
	memset(want, 0x42, 100);
	for (i = 0; i < 100; i++)
		src[i] = (char)(i % 128);
	memcpy(want + 1, src + 3, 50);
	ft_memcpy(dest + 1, src + 3, 50);
	ASSERT_MEM(dest, want, 100);
}

TEST(memcpy, reads_and_writes_exactly_n_bytes)
{
	char		pattern[64];
	size_t		n;
	char		*dest;
	const char	*src;

	for (n = 0; n < 64; n++)
		pattern[n] = (char)('A' + n % 26);
	for (n = 0; n <= 64; n++)
	{
		dest = lft_guard_buf(n);
		src = lft_guard_mem(pattern, n);
		ft_memcpy(dest, src, n);
		ASSERT_MEM(dest, pattern, n);
		lft_guard_free(dest);
		lft_guard_free(src);
	}
}

TEST(memcpy, fuzz_against_libc)
{
	int		round;
	size_t	n;
	char	src[256];
	char	dest[256];
	char	want[256];
	size_t	i;

	for (round = 0; round < 2000; round++)
	{
		n = lft_rand() % 256;
		for (i = 0; i < sizeof(src); i++)
		{
			src[i] = (char)lft_rand();
			dest[i] = (char)lft_rand();
		}
		memcpy(want, dest, sizeof(want));
		memcpy(want, src, n);
		ft_memcpy(dest, src, n);
		ASSERT_MEM(dest, want, sizeof(want));
	}
}

TEST(memmove, basic)
{
	char	dest[20];

	ft_memmove(dest, "Hello World", 12);
	ASSERT_STR(dest, "Hello World");
}

TEST(memmove, return_value)
{
	char	buf[16] = "abcdefgh";

	ASSERT_PTR(ft_memmove(buf + 2, buf, 4), buf + 2);
	ASSERT_PTR(ft_memmove(buf, buf + 2, 4), buf);
}

TEST(memmove, overlap_forward)
{
	char	buf[] = "abcdefghijklmnop";

	ft_memmove(buf + 2, buf, 10);
	ASSERT_STR(buf, "ababcdefghijmnop");
}

TEST(memmove, overlap_backward)
{
	char	buf[] = "abcdefghijklmnop";

	ft_memmove(buf, buf + 2, 10);
	ASSERT_STR(buf, "cdefghijklklmnop");
}

TEST(memmove, no_overlap)
{
	char	dest[20];
	char	want[20];

	memset(dest, 'X', 20);
	memset(want, 'X', 20);
	memcpy(want, "Hello World", 12);
	ft_memmove(dest, "Hello World", 12);
	ASSERT_MEM(dest, want, 20);
}

TEST(memmove, zero_size)
{
	char	buf[10] = "original";

	ASSERT_PTR(ft_memmove(buf, "test", 0), buf);
	ASSERT_STR(buf, "original");
}

TEST(memmove, zero_size_reads_and_writes_nothing)
{
	char		*dest = lft_guard_buf(0);
	const char	*src = lft_guard_mem("", 0);

	ASSERT_PTR(ft_memmove(dest, src, 0), dest);
}

TEST(memmove, same_pointer)
{
	char	buf[] = "test string";

	ASSERT_PTR(ft_memmove(buf, buf, strlen(buf)), buf);
	ASSERT_STR(buf, "test string");
}

TEST(memmove, binary_overlap)
{
	unsigned char	buf[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	unsigned char	want[] = {1, 2, 3, 1, 2, 3, 4, 5, 9, 10};

	ft_memmove(buf + 3, buf, 5);
	ASSERT_MEM(buf, want, 10);
}

TEST(memmove, does_not_stop_at_nul)
{
	char	buf[8] = {'a', 0, 'b', 0, 'c', 0, 'd', 0};
	char	want[8] = {'a', 'a', 0, 'b', 0, 'c', 0, 'd'};

	ft_memmove(buf + 1, buf, 7);
	ASSERT_MEM(buf, want, 8);
}

TEST(memmove, large_overlap)
{
	char	buf[1000];
	char	want[1000];
	int		i;

	for (i = 0; i < 1000; i++)
		buf[i] = (char)('A' + i % 26);
	memcpy(want, buf, 1000);
	memmove(want + 100, want, 800);
	ft_memmove(buf + 100, buf, 800);
	ASSERT_MEM(buf, want, 1000);
	memmove(want, want + 100, 800);
	ft_memmove(buf, buf + 100, 800);
	ASSERT_MEM(buf, want, 1000);
}

TEST(memmove, single_byte_overlap)
{
	char	buf[] = "abcdefgh";

	ft_memmove(buf + 1, buf, 1);
	ASSERT_STR(buf, "aacdefgh");
}

TEST(memmove, every_shift_and_length)
{
	char	got[128];
	char	want[128];
	int		shift;
	int		len;
	int		i;

	for (len = 0; len <= 40; len++)
	{
		for (shift = -24; shift <= 24; shift++)
		{
			for (i = 0; i < 128; i++)
				got[i] = (char)(i * 7 + 1);
			memcpy(want, got, 128);
			memmove(want + 40 + shift, want + 40, (size_t)len);
			ASSERT_PTR(ft_memmove(got + 40 + shift, got + 40, (size_t)len),
				got + 40 + shift);
			if (memcmp(got, want, 128) != 0)
				FAILF("len=%d, décalage=%d : résultat différent de memmove",
					len, shift);
		}
	}
}

TEST(memmove, reads_and_writes_exactly_n_bytes)
{
	char		pattern[64];
	size_t		n;
	char		*dest;
	const char	*src;

	for (n = 0; n < 64; n++)
		pattern[n] = (char)('A' + n % 26);
	for (n = 0; n <= 64; n++)
	{
		dest = lft_guard_buf(n);
		src = lft_guard_mem(pattern, n);
		ft_memmove(dest, src, n);
		ASSERT_MEM(dest, pattern, n);
		lft_guard_free(dest);
		lft_guard_free(src);
	}
}

TEST(memmove, stays_inside_the_range_when_overlapping)
{
	size_t	n;
	char	*end;
	char	*front;

	for (n = 1; n <= 32; n++)
	{
		end = lft_guard_buf(n + 1);
		front = lft_guard_buf_front(n + 1);
		memset(end, 'a', n + 1);
		memset(front, 'a', n + 1);
		ft_memmove(end + 1, end, n);
		ft_memmove(end, end + 1, n);
		ft_memmove(front + 1, front, n);
		ft_memmove(front, front + 1, n);
		lft_guard_free(end);
		lft_guard_free(front);
	}
}
