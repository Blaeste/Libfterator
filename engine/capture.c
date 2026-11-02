/* Capture de ce qui est écrit sur un descripteur. */
#define _GNU_SOURCE
#include <sys/mman.h>
#include <unistd.h>
#include "lft_test.h"
#include "lft_internal.h"

int	lft_capture_begin(void)
{
	int	fd;

	fd = memfd_create("lft_capture", 0);
	if (fd < 0)
		lft_report(LFT_INTERNAL, "memfd_create impossible");
	return (fd);
}

size_t	lft_capture_end(int fd, char *buf, size_t size)
{
	ssize_t	got;
	size_t	total;

	if (lseek(fd, 0, SEEK_SET) < 0)
		lft_report(LFT_INTERNAL, "lseek impossible sur la capture");
	total = 0;
	while (total + 1 < size)
	{
		got = read(fd, buf + total, size - 1 - total);
		if (got <= 0)
			break ;
		total += (size_t)got;
	}
	buf[total] = '\0';
	close(fd);
	return (total);
}
