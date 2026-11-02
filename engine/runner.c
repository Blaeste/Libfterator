/* Runner : un fork par test, statut par ligne sur stdout.
** Format de sortie : STATUT<TAB>suite/nom<TAB>ms<TAB>détail */
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include "lft_test.h"
#include "lft_internal.h"

#define MAX_TESTS 8192
#define REPORT_FD 900
#define REPORT_MAX 2048
#define OUTPUT_MAX 4096
#define MAX_ADDRESS_SPACE (4UL * 1024 * 1024 * 1024)
#define SANITIZER_EXIT 98

typedef struct s_test
{
	const char	*suite;
	const char	*name;
	void		(*fn)(void);
	int			line;
	int			rank;
}	t_test;

typedef struct s_outcome
{
	char	report[REPORT_MAX];
	size_t	report_len;
	char	output[OUTPUT_MAX];
	size_t	output_len;
	int		wstatus;
}	t_outcome;

static t_test		g_tests[MAX_TESTS];
static int			g_count;
static int			g_timeout = 5;
static int			g_no_fork;
static unsigned int	g_seed = 42;
static unsigned int	g_rand_state = 1;

/* Ordre du sujet. Les suites inconnues passent après, par ordre alphabétique. */
static const char	*g_order[] = {
	"isalpha", "isdigit", "isalnum", "isascii", "isprint", "strlen", "memset",
	"bzero", "memcpy", "memmove", "strlcpy", "strlcat", "toupper", "tolower",
	"strchr", "strrchr", "strncmp", "memchr", "memcmp", "strnstr", "atoi",
	"calloc", "strdup",
	"substr", "strjoin", "strtrim", "split", "itoa", "strmapi", "striteri",
	"putchar_fd", "putstr_fd", "putendl_fd", "putnbr_fd",
	"lstnew", "lstadd_front", "lstsize", "lstlast", "lstadd_back", "lstdelone",
	"lstclear", "lstiter", "lstmap",
	"info", NULL
};

void	lft_register(const char *suite, const char *name, void (*fn)(void),
		int line)
{
	if (g_count == MAX_TESTS)
	{
		fprintf(stderr, "libfterator: trop de tests (max %d)\n", MAX_TESTS);
		exit(2);
	}
	g_tests[g_count].suite = suite;
	g_tests[g_count].name = name;
	g_tests[g_count].fn = fn;
	g_tests[g_count].line = line;
	g_count++;
}

static int	suite_rank(const char *suite)
{
	int	i;

	for (i = 0; g_order[i]; i++)
		if (strcmp(g_order[i], suite) == 0)
			return (i);
	return (i);
}

static int	compare_tests(const void *pa, const void *pb)
{
	const t_test	*a;
	const t_test	*b;
	int				cmp;

	a = pa;
	b = pb;
	if (a->rank != b->rank)
		return (a->rank - b->rank);
	cmp = strcmp(a->suite, b->suite);
	if (cmp != 0)
		return (cmp);
	return (a->line - b->line);
}

/* Appelée par les remplaçants générés pour les fonctions absentes de libft.a. */
void	lft_missing(const char *name)
{
	lft_report_fmt(LFT_MISSING, "%s absente de libft.a", name);
}

/* ------------------------------ Aléatoire ----------------------------- */

unsigned int	lft_seed(void)
{
	return (g_seed);
}

unsigned int	lft_rand(void)
{
	g_rand_state ^= g_rand_state << 13;
	g_rand_state ^= g_rand_state >> 17;
	g_rand_state ^= g_rand_state << 5;
	return (g_rand_state);
}

static void	seed_for(const t_test *t)
{
	const char		*parts[2];
	unsigned int	h;
	int				i;
	const char		*p;

	parts[0] = t->suite;
	parts[1] = t->name;
	h = 2166136261u ^ g_seed;
	for (i = 0; i < 2; i++)
		for (p = parts[i]; *p; p++)
			h = (h ^ (unsigned char)*p) * 16777619u;
	g_rand_state = h ? h : 1;
}

/* ------------------------------ Exécution ----------------------------- */

static void	child_setup(int report_w, int output_w)
{
	struct rlimit	limit;
	int				devnull;

	if (dup2(report_w, REPORT_FD) < 0)
		_exit(4);
	devnull = open("/dev/null", O_RDONLY);
	if (devnull >= 0)
		dup2(devnull, STDIN_FILENO);
	dup2(output_w, STDOUT_FILENO);
	dup2(output_w, STDERR_FILENO);
	limit.rlim_cur = 0;
	limit.rlim_max = 0;
	setrlimit(RLIMIT_CORE, &limit);
	if (!lft_real_malloc())
	{
		limit.rlim_cur = MAX_ADDRESS_SPACE;
		limit.rlim_max = MAX_ADDRESS_SPACE;
		setrlimit(RLIMIT_AS, &limit);
	}
}

static void	run_test_body(const t_test *t)
{
	seed_for(t);
	lft_install_fault_handler();
	t->fn();
	lft_check_leaks();
	lft_report(LFT_PASS, "");
}

static void	drain(int fd, char *buf, size_t max, size_t *len, int *open_fd)
{
	char	chunk[1024];
	ssize_t	got;
	size_t	room;

	got = read(fd, chunk, sizeof(chunk));
	if (got <= 0)
	{
		if (got == 0 || errno != EINTR)
			*open_fd = 0;
		return ;
	}
	room = max - *len;
	if ((size_t)got < room)
		room = (size_t)got;
	memcpy(buf + *len, chunk, room);
	*len += room;
}

static void	collect(int report_r, int output_r, t_outcome *out)
{
	struct pollfd	fds[2];
	int				open_report;
	int				open_output;

	open_report = 1;
	open_output = 1;
	while (open_report || open_output)
	{
		fds[0].fd = open_report ? report_r : -1;
		fds[1].fd = open_output ? output_r : -1;
		fds[0].events = POLLIN;
		fds[1].events = POLLIN;
		if (poll(fds, 2, -1) < 0 && errno != EINTR)
			break ;
		if (fds[0].revents)
			drain(report_r, out->report, sizeof(out->report) - 1,
				&out->report_len, &open_report);
		if (fds[1].revents)
			drain(output_r, out->output, sizeof(out->output) - 1,
				&out->output_len, &open_output);
	}
	out->report[out->report_len] = '\0';
	out->output[out->output_len] = '\0';
}

static long	now_ms(void)
{
	struct timespec	ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

static void	print_clean(const char *s)
{
	for (; *s; s++)
	{
		if (*s == '\n')
			fputs("\\n", stdout);
		else if (*s == '\t')
			fputs("\\t", stdout);
		else
			fputc(*s, stdout);
	}
}

static void	print_line(const char *status, const t_test *t, long ms,
		const char *detail)
{
	printf("%s\t%s/%s\t%ld\t", status, t->suite, t->name, ms);
	print_clean(detail);
	putchar('\n');
	fflush(stdout);
}

static const char	*status_of_report(char kind)
{
	if (kind == LFT_PASS)
		return ("PASS");
	if (kind == LFT_FAIL)
		return ("FAIL");
	if (kind == LFT_LEAK)
		return ("LEAK");
	if (kind == LFT_MEM)
		return ("MEMERR");
	if (kind == LFT_OOB)
		return ("OOB");
	if (kind == LFT_MISSING)
		return ("MISSING");
	if (kind == LFT_INFO)
		return ("INFO");
	if (kind == LFT_CRASH)
		return ("CRASH");
	return ("INTERNAL");
}

/* ------------------------ Compte rendu du test ------------------------ */

static const t_test	*g_inline_test;

void	lft_report(char kind, const char *msg)
{
	char	buf[REPORT_MAX];
	size_t	len;
	int		code;

	code = (kind == LFT_PASS || kind == LFT_INFO) ? 0 : 1;
	if (g_inline_test)
	{
		print_line(status_of_report(kind), g_inline_test, 0, msg);
		_exit(code);
	}
	buf[0] = kind;
	len = strlen(msg);
	if (len > sizeof(buf) - 1)
		len = sizeof(buf) - 1;
	memcpy(buf + 1, msg, len);
	if (write(REPORT_FD, buf, len + 1) < 0)
		_exit(3);
	_exit(code);
}

void	lft_report_fmt(char kind, const char *fmt, ...)
{
	char	buf[REPORT_MAX];
	va_list	ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	lft_report(kind, buf);
}

static const char	*signal_name(int sig)
{
	if (sig == SIGSEGV)
		return ("SIGSEGV");
	if (sig == SIGBUS)
		return ("SIGBUS");
	if (sig == SIGABRT)
		return ("SIGABRT");
	if (sig == SIGFPE)
		return ("SIGFPE");
	if (sig == SIGILL)
		return ("SIGILL");
	if (sig == SIGPIPE)
		return ("SIGPIPE");
	if (sig == SIGKILL)
		return ("SIGKILL");
	return ("signal");
}

/* Sans compte rendu du test, seul le code de fin renseigne sur ce qui s'est
** passé. Une fin sans compte rendu n'est jamais un succès. */
static int	report_outcome(const t_test *t, const t_outcome *out, long ms)
{
	char		detail[REPORT_MAX + OUTPUT_MAX + 128];
	const char	*status;
	int			wstatus;

	wstatus = out->wstatus;
	if (out->report_len > 0)
	{
		status = status_of_report(out->report[0]);
		snprintf(detail, sizeof(detail), "%s", out->report + 1);
	}
	else if (WIFSIGNALED(wstatus) && WTERMSIG(wstatus) == SIGALRM)
	{
		status = "TIMEOUT";
		snprintf(detail, sizeof(detail), "plus de %d s", g_timeout);
	}
	else if (WIFSIGNALED(wstatus))
	{
		status = "CRASH";
		snprintf(detail, sizeof(detail), "%s", signal_name(WTERMSIG(wstatus)));
	}
	else if (WEXITSTATUS(wstatus) == SANITIZER_EXIT)
	{
		status = "CRASH";
		snprintf(detail, sizeof(detail), "%s", out->output);
	}
	else
	{
		status = "CRASH";
		snprintf(detail, sizeof(detail),
			"processus terminé sans fin de test (code %d) : %s",
			WEXITSTATUS(wstatus), out->output);
	}
	if (strcmp(t->suite, "info") == 0)
	{
		if (strcmp(status, "PASS") == 0)
			snprintf(detail, sizeof(detail), "retour normal");
		status = "INFO";
	}
	print_line(status, t, ms, detail);
	return (strcmp(status, "PASS") == 0 || strcmp(status, "INFO") == 0);
}

static int	run_forked(const t_test *t)
{
	int			report[2];
	int			output[2];
	pid_t		pid;
	long		start;
	static t_outcome	outcome;

	if (pipe(report) < 0 || pipe(output) < 0)
	{
		perror("libfterator: pipe");
		exit(2);
	}
	fflush(stdout);
	start = now_ms();
	pid = fork();
	if (pid < 0)
	{
		perror("libfterator: fork");
		exit(2);
	}
	if (pid == 0)
	{
		close(report[0]);
		close(output[0]);
		child_setup(report[1], output[1]);
		close(report[1]);
		close(output[1]);
		alarm((unsigned int)g_timeout);
		run_test_body(t);
	}
	close(report[1]);
	close(output[1]);
	memset(&outcome, 0, sizeof(outcome));
	collect(report[0], output[0], &outcome);
	close(report[0]);
	close(output[0]);
	waitpid(pid, &outcome.wstatus, 0);
	return (report_outcome(t, &outcome, now_ms() - start));
}

/* Sans fork, pour gdb et valgrind : un seul test, dans ce processus. */
static void	run_inline(const t_test *t)
{
	fflush(stdout);
	g_inline_test = t;
	run_test_body(t);
}

/* ------------------------------- Entrée ------------------------------- */

static void	usage(void)
{
	fprintf(stderr, "usage: runner [--list] [--run MOTIF] [--only NOM] "
		"[--timeout S] [--seed N] [--no-fork] [--real-malloc]\n");
	exit(2);
}

static const char	*g_pattern;
static const char	*g_only;
static int			g_list;

static void	parse_args(int argc, char **argv)
{
	int	i;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--list") == 0)
			g_list = 1;
		else if (strcmp(argv[i], "--no-fork") == 0)
			g_no_fork = 1;
		else if (strcmp(argv[i], "--real-malloc") == 0)
			lft_set_real_malloc(1);
		else if (strcmp(argv[i], "--run") == 0 && i + 1 < argc)
			g_pattern = argv[++i];
		else if (strcmp(argv[i], "--only") == 0 && i + 1 < argc)
			g_only = argv[++i];
		else if (strcmp(argv[i], "--timeout") == 0 && i + 1 < argc)
			g_timeout = atoi(argv[++i]);
		else if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc)
			g_seed = (unsigned int)strtoul(argv[++i], NULL, 10);
		else
			usage();
	}
	if (g_timeout <= 0 || (g_no_fork && g_only == NULL))
		usage();
}

static int	selected(const t_test *t)
{
	char	full[256];

	snprintf(full, sizeof(full), "%s/%s", t->suite, t->name);
	if (g_only)
		return (strcmp(full, g_only) == 0);
	if (g_pattern)
		return (strstr(full, g_pattern) != NULL);
	return (1);
}

int	main(int argc, char **argv)
{
	int	i;
	int	all_ok;

	parse_args(argc, argv);
	for (i = 0; i < g_count; i++)
		g_tests[i].rank = suite_rank(g_tests[i].suite);
	qsort(g_tests, (size_t)g_count, sizeof(t_test), compare_tests);
	all_ok = 1;
	for (i = 0; i < g_count; i++)
	{
		if (!selected(&g_tests[i]))
			continue ;
		if (g_list)
			printf("%s/%s\n", g_tests[i].suite, g_tests[i].name);
		else if (g_no_fork)
			run_inline(&g_tests[i]);
		else if (!run_forked(&g_tests[i]))
			all_ok = 0;
	}
	return (all_ok ? 0 : 1);
}
