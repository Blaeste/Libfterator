"""Libft factice pour tester les contrôles de conformité.

Les corps de fonctions ne font rien : cette fixture n'est pas une solution.
Elle respecte le sujet v19.3, et chaque test la casse d'une seule façon."""
from pathlib import Path

TYPES = {
    "ft_isalpha": ("int", "int c", "return (c);"),
    "ft_isdigit": ("int", "int c", "return (c);"),
    "ft_isalnum": ("int", "int c", "return (c);"),
    "ft_isascii": ("int", "int c", "return (c);"),
    "ft_isprint": ("int", "int c", "return (c);"),
    "ft_strlen": ("size_t", "const char *s", "(void)s;\n\treturn (0);"),
    "ft_memset": ("void\t*", "void *s, int c, size_t n", "(void)c;\n\t(void)n;\n\treturn (s);"),
    "ft_bzero": ("void", "void *s, size_t n", "(void)s;\n\t(void)n;"),
    "ft_memcpy": ("void\t*", "void *dest, const void *src, size_t n", "(void)src;\n\t(void)n;\n\treturn (dest);"),
    "ft_memmove": ("void\t*", "void *dest, const void *src, size_t n", "(void)src;\n\t(void)n;\n\treturn (dest);"),
    "ft_strlcpy": ("size_t", "char *dst, const char *src, size_t size", "(void)dst;\n\t(void)src;\n\treturn (size);"),
    "ft_strlcat": ("size_t", "char *dst, const char *src, size_t size", "(void)dst;\n\t(void)src;\n\treturn (size);"),
    "ft_toupper": ("int", "int c", "return (c);"),
    "ft_tolower": ("int", "int c", "return (c);"),
    "ft_strchr": ("char\t*", "const char *s, int c", "(void)s;\n\t(void)c;\n\treturn (0);"),
    "ft_strrchr": ("char\t*", "const char *s, int c", "(void)s;\n\t(void)c;\n\treturn (0);"),
    "ft_strncmp": ("int", "const char *s1, const char *s2, size_t n", "(void)s1;\n\t(void)s2;\n\t(void)n;\n\treturn (0);"),
    "ft_memchr": ("void\t*", "const void *s, int c, size_t n", "(void)s;\n\t(void)c;\n\t(void)n;\n\treturn (0);"),
    "ft_memcmp": ("int", "const void *s1, const void *s2, size_t n", "(void)s1;\n\t(void)s2;\n\t(void)n;\n\treturn (0);"),
    "ft_strnstr": ("char\t*", "const char *big, const char *little, size_t len", "(void)big;\n\t(void)little;\n\t(void)len;\n\treturn (0);"),
    "ft_atoi": ("int", "const char *nptr", "(void)nptr;\n\treturn (0);"),
    "ft_calloc": ("void\t*", "size_t nmemb, size_t size", "return (malloc(nmemb * size));"),
    "ft_strdup": ("char\t*", "const char *s", "(void)s;\n\treturn (malloc(1));"),
    "ft_substr": ("char\t*", "char const *s, unsigned int start, size_t len", "(void)s;\n\t(void)start;\n\t(void)len;\n\treturn (malloc(1));"),
    "ft_strjoin": ("char\t*", "char const *s1, char const *s2", "(void)s1;\n\t(void)s2;\n\treturn (malloc(1));"),
    "ft_strtrim": ("char\t*", "char const *s1, char const *set", "(void)s1;\n\t(void)set;\n\treturn (malloc(1));"),
    "ft_split": ("char\t**", "char const *s, char c", "(void)s;\n\t(void)c;\n\tfree(malloc(1));\n\treturn (malloc(1));"),
    "ft_itoa": ("char\t*", "int n", "(void)n;\n\treturn (malloc(1));"),
    "ft_strmapi": ("char\t*", "char const *s, char (*f)(unsigned int, char)", "(void)s;\n\t(void)f;\n\treturn (malloc(1));"),
    "ft_striteri": ("void", "char *s, void (*f)(unsigned int, char *)", "(void)s;\n\t(void)f;"),
    "ft_putchar_fd": ("void", "char c, int fd", "if (write(fd, &c, 1) < 0)\n\t\treturn ;"),
    "ft_putstr_fd": ("void", "char *s, int fd", "if (write(fd, s, 0) < 0)\n\t\treturn ;"),
    "ft_putendl_fd": ("void", "char *s, int fd", "if (write(fd, s, 0) < 0)\n\t\treturn ;"),
    "ft_putnbr_fd": ("void", "int n, int fd", "(void)n;\n\tif (write(fd, \"\", 0) < 0)\n\t\treturn ;"),
    "ft_lstnew": ("t_list\t*", "void *content", "(void)content;\n\treturn (malloc(sizeof(t_list)));"),
    "ft_lstadd_front": ("void", "t_list **lst, t_list *new", "(void)lst;\n\t(void)new;"),
    "ft_lstsize": ("unsigned int", "t_list *lst", "(void)lst;\n\treturn (0);"),
    "ft_lstlast": ("t_list\t*", "t_list *lst", "return (lst);"),
    "ft_lstadd_back": ("void", "t_list **lst, t_list *new", "(void)lst;\n\t(void)new;"),
    "ft_lstdelone": ("void", "t_list *lst, void (*del)(void *)", "(void)del;\n\tfree(lst);"),
    "ft_lstclear": ("void", "t_list **lst, void (*del)(void *)", "(void)del;\n\tfree(*lst);"),
    "ft_lstiter": ("void", "t_list *lst, void (*f)(void *)", "(void)lst;\n\t(void)f;"),
    "ft_lstmap": ("t_list\t*", "t_list *lst, void *(*f)(void *), void (*del)(void *)", "(void)lst;\n\t(void)f;\n\t(void)del;\n\tfree(malloc(1));\n\treturn (malloc(1));"),
}

MAKEFILE = """NAME = libft.a
CC = cc
CFLAGS = -Wall -Wextra -Werror
SRCS = %s
OBJS = $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
\tar rcs $(NAME) $(OBJS)

%%.o: %%.c libft.h
\t$(CC) $(CFLAGS) -c $< -o $@

clean:
\trm -f $(OBJS)

fclean: clean
\trm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
"""

README = """*This project has been created as part of the 42 curriculum by login1.*

# Description
This library reimplements libc functions such as ft_strlen and ft_memcpy.

# Instructions
Run `make`.

# Library
It contains ft_split, ft_itoa and ft_lstnew among others.

# Resources
The man pages. AI was used to review the documentation.
"""


def header_text():
    lines = ["#ifndef LIBFT_H", "# define LIBFT_H", "", "# include <stddef.h>", "",
             "typedef struct s_list", "{", "\tvoid\t\t\t*content;",
             "\tstruct s_list\t*next;", "}\tt_list;", ""]
    for name, (ret, params, _) in TYPES.items():
        lines.append("%s%s%s(%s);" % (ret, "" if ret.endswith("*") else "\t", name, params))
    lines += ["", "#endif", ""]
    return "\n".join(lines)


def source_text(name):
    ret, params, body = TYPES[name]
    sep = "" if ret.endswith("*") else "\t"
    return ('#include <stdlib.h>\n#include <unistd.h>\n#include "libft.h"\n\n'
            "%s%s%s(%s)\n{\n\t%s\n}\n" % (ret, sep, name, params, body))


def make_fixture(root):
    """Écrit la libft factice dans root et retourne son chemin."""
    root = Path(root)
    root.mkdir(parents=True, exist_ok=True)
    for name in TYPES:
        (root / (name + ".c")).write_text(source_text(name))
    (root / "libft.h").write_text(header_text())
    srcs = " ".join(name + ".c" for name in TYPES)
    (root / "Makefile").write_text(MAKEFILE % srcs)
    (root / "README.md").write_text(README)
    return root
