/* Assertions : messages lisibles avec octets non imprimables échappés. */
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "lft_test.h"
#include "lft_internal.h"

#define QUOTE_MAX 64

void	lft_fail(const char *file, int line, const char *fmt, ...)
{
	char		buf[1536];
	const char	*base;
	va_list		ap;

	base = strrchr(file, '/');
	base = base ? base + 1 : file;
	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	if (lft_mf_failed_malloc())
		lft_report_fmt(LFT_FAIL, "%s:%d: %s (quand le malloc n°%ld échoue)",
			base, line, buf, lft_mf_failed_malloc());
	lft_report_fmt(LFT_FAIL, "%s:%d: %s", base, line, buf);
}

void	lft_info(const char *fmt, ...)
{
	char	buf[1536];
	va_list	ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	lft_report(LFT_INFO, buf);
}

/* Quatre buffers tournants : un message peut citer plusieurs chaînes. */
static char	*next_buffer(void)
{
	static char	bufs[4][QUOTE_MAX * 4 + 16];
	static int	i;

	i = (i + 1) % 4;
	return (bufs[i]);
}

static const char	*quote(const unsigned char *s, size_t n)
{
	char	*out;
	size_t	len;
	size_t	i;

	out = next_buffer();
	len = 0;
	out[len++] = '"';
	for (i = 0; i < n && i < QUOTE_MAX; i++)
	{
		if (s[i] == '"' || s[i] == '\\')
			len += sprintf(out + len, "\\%c", s[i]);
		else if (s[i] >= 32 && s[i] < 127)
			out[len++] = (char)s[i];
		else
			len += sprintf(out + len, "\\x%02x", s[i]);
	}
	out[len++] = '"';
	if (n > QUOTE_MAX)
		len += sprintf(out + len, "...");
	out[len] = '\0';
	return (out);
}

const char	*lft_quote(const char *s)
{
	if (s == NULL)
		return ("NULL");
	return (quote((const unsigned char *)s, strlen(s)));
}

const char	*lft_quote_n(const void *s, size_t n)
{
	if (s == NULL)
		return ("NULL");
	return (quote(s, n));
}

void	lft_assert_str(const char *file, int line, const char *expr,
		const char *got, const char *exp)
{
	if (got == NULL)
		lft_fail(file, line, "%s : obtenu NULL, attendu %s", expr,
			lft_quote(exp));
	if (strcmp(got, exp) != 0)
		lft_fail(file, line, "%s : obtenu %s, attendu %s", expr,
			lft_quote(got), lft_quote(exp));
}

void	lft_assert_mem(const char *file, int line, const char *expr,
		const void *got, const void *exp, size_t n)
{
	const unsigned char	*g;
	const unsigned char	*e;
	size_t				i;

	g = got;
	e = exp;
	for (i = 0; i < n; i++)
		if (g[i] != e[i])
			lft_fail(file, line,
				"%s : octet %zu sur %zu : obtenu 0x%02x, attendu 0x%02x",
				expr, i, n, g[i], e[i]);
}

void	lft_assert_ptr(const char *file, int line, const char *expr,
		const void *got, const void *exp)
{
	if (got == exp)
		return ;
	if (exp == NULL)
		lft_fail(file, line, "%s : obtenu %p, attendu NULL", expr, got);
	if (got == NULL)
		lft_fail(file, line, "%s : obtenu NULL, un pointeur attendu", expr);
	lft_fail(file, line, "%s : pointeur décalé de %td octet(s)", expr,
		(const char *)got - (const char *)exp);
}

static int	sign_of(long long v)
{
	return ((v > 0) - (v < 0));
}

void	lft_assert_sign(const char *file, int line, const char *expr,
		long long got, long long ref)
{
	if (sign_of(got) != sign_of(ref))
		lft_fail(file, line, "%s : obtenu %lld, signe attendu %s", expr, got,
			ref > 0 ? "positif" : (ref < 0 ? "négatif" : "zéro"));
}
