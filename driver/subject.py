"""Sujet libft v19.3 : les 43 fonctions, leur type et leurs fonctions externes."""
from collections import namedtuple

Function = namedtuple("Function", "name part ctype externals")

_M = ("malloc",)
_MF = ("malloc", "free")
_F = ("free",)
_W = ("write",)

FUNCTIONS = [
    Function("ft_isalpha", 1, "int (int)", ()),
    Function("ft_isdigit", 1, "int (int)", ()),
    Function("ft_isalnum", 1, "int (int)", ()),
    Function("ft_isascii", 1, "int (int)", ()),
    Function("ft_isprint", 1, "int (int)", ()),
    Function("ft_strlen", 1, "size_t (const char *)", ()),
    Function("ft_memset", 1, "void *(void *, int, size_t)", ()),
    Function("ft_bzero", 1, "void (void *, size_t)", ()),
    Function("ft_memcpy", 1, "void *(void *, const void *, size_t)", ()),
    Function("ft_memmove", 1, "void *(void *, const void *, size_t)", ()),
    Function("ft_strlcpy", 1, "size_t (char *, const char *, size_t)", ()),
    Function("ft_strlcat", 1, "size_t (char *, const char *, size_t)", ()),
    Function("ft_toupper", 1, "int (int)", ()),
    Function("ft_tolower", 1, "int (int)", ()),
    Function("ft_strchr", 1, "char *(const char *, int)", ()),
    Function("ft_strrchr", 1, "char *(const char *, int)", ()),
    Function("ft_strncmp", 1, "int (const char *, const char *, size_t)", ()),
    Function("ft_memchr", 1, "void *(const void *, int, size_t)", ()),
    Function("ft_memcmp", 1, "int (const void *, const void *, size_t)", ()),
    Function("ft_strnstr", 1, "char *(const char *, const char *, size_t)", ()),
    Function("ft_atoi", 1, "int (const char *)", ()),
    Function("ft_calloc", 1, "void *(size_t, size_t)", _M),
    Function("ft_strdup", 1, "char *(const char *)", _M),
    Function("ft_substr", 2, "char *(const char *, unsigned int, size_t)", _M),
    Function("ft_strjoin", 2, "char *(const char *, const char *)", _M),
    Function("ft_strtrim", 2, "char *(const char *, const char *)", _M),
    Function("ft_split", 2, "char **(const char *, char)", _MF),
    Function("ft_itoa", 2, "char *(int)", _M),
    Function("ft_strmapi", 2, "char *(const char *, char (*)(unsigned int, char))", _M),
    Function("ft_striteri", 2, "void (char *, void (*)(unsigned int, char *))", ()),
    Function("ft_putchar_fd", 2, "void (char, int)", _W),
    Function("ft_putstr_fd", 2, "void (char *, int)", _W),
    Function("ft_putendl_fd", 2, "void (char *, int)", _W),
    Function("ft_putnbr_fd", 2, "void (int, int)", _W),
    Function("ft_lstnew", 3, "t_list *(void *)", _M),
    Function("ft_lstadd_front", 3, "void (t_list **, t_list *)", ()),
    Function("ft_lstsize", 3, "unsigned int (t_list *)", ()),
    Function("ft_lstlast", 3, "t_list *(t_list *)", ()),
    Function("ft_lstadd_back", 3, "void (t_list **, t_list *)", ()),
    Function("ft_lstdelone", 3, "void (t_list *, void (*)(void *))", _F),
    Function("ft_lstclear", 3, "void (t_list **, void (*)(void *))", _F),
    Function("ft_lstiter", 3, "void (t_list *, void (*)(void *))", ()),
    Function("ft_lstmap", 3, "t_list *(t_list *, void *(*)(void *), void (*)(void *))", _MF),
]

BY_NAME = {f.name: f for f in FUNCTIONS}
NAMES = [f.name for f in FUNCTIONS]
