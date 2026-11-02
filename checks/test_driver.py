"""Tests du lanceur : chaque règle du sujet est cassée une par une."""
import os
import shutil
import stat
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from fixture import make_fixture  # noqa: E402
from driver import build, conformity, project, subject  # noqa: E402

ROOT = Path(__file__).resolve().parent.parent
FAKES = Path(__file__).resolve().parent / "fakes"


class Base(unittest.TestCase):
    def setUp(self):
        self.tmp = Path(tempfile.mkdtemp(prefix="lft-driver-"))
        self.lib = make_fixture(self.tmp / "libft")

    def tearDown(self):
        shutil.rmtree(str(self.tmp), ignore_errors=True)

    def edit(self, name, old, new):
        path = self.lib / name
        text = path.read_text()
        self.assertIn(old, text)
        path.write_text(text.replace(old, new))

    def checks(self, **kwargs):
        kwargs.setdefault("norminette", False)
        ws = project.Workspace(self.lib, self.tmp / "work")
        result = conformity.run_all(ws, **kwargs)
        self.ws = ws
        return result

    def status(self, result, key):
        found = [c for c in result.checks if c.key == key]
        self.assertTrue(found, "contrôle %s absent de %s" % (
            key, [c.key for c in result.checks]))
        return found[0].status

    def detail(self, result, key):
        return [c for c in result.checks if c.key == key][0].detail

    def assertKo(self, result, key, *words):
        self.assertEqual(self.status(result, key), "KO",
                         "%s devrait être KO" % key)
        for word in words:
            self.assertIn(word, self.detail(result, key))


class GoodLibft(Base):
    def test_the_fixture_respects_every_rule(self):
        result = self.checks()
        bad = [(c.key, c.status, c.detail) for c in result.checks
               if c.status not in ("OK", "SKIP")]
        self.assertEqual(bad, [])

    def test_source_directory_is_never_touched(self):
        def snapshot():
            return {p.name: (p.stat().st_mtime_ns, p.stat().st_size)
                    for p in self.lib.iterdir()}
        before = snapshot()
        self.checks()
        self.assertEqual(snapshot(), before)


class Makefile(Base):
    def test_missing_rule(self):
        self.edit("Makefile", "re: fclean all", "")
        self.edit("Makefile", ".PHONY: all clean fclean re", ".PHONY: all clean fclean")
        self.assertKo(self.checks(), "make.rules", "re")

    def test_wrong_name(self):
        self.edit("Makefile", "NAME = libft.a", "NAME = other.a")
        self.assertKo(self.checks(), "make.rules", "NAME")

    def test_compiler_must_be_cc(self):
        self.edit("Makefile", "CC = cc", "CC = gcc")
        self.assertKo(self.checks(), "make.cc", "gcc")

    def test_implicit_cc_is_accepted(self):
        self.edit("Makefile", "CC = cc\n", "")
        self.assertEqual(self.status(self.checks(), "make.cc"), "OK")

    def test_all_warning_flags_are_required(self):
        self.edit("Makefile", "-Wall -Wextra -Werror", "-Wall -Werror")
        self.assertKo(self.checks(), "make.flags", "-Wextra")

    def test_std_c99_is_forbidden(self):
        self.edit("Makefile", "-Werror", "-Werror -std=c99")
        self.assertKo(self.checks(), "make.forbidden", "-std=c99")

    def test_libtool_is_forbidden(self):
        self.edit("Makefile", "\tar rcs $(NAME) $(OBJS)",
                  "\tar rcs $(NAME) $(OBJS)\n\t-@true libtool")
        self.assertKo(self.checks(), "make.forbidden", "libtool")

    def test_ar_is_required(self):
        self.assertEqual(self.status(self.checks(), "make.ar"), "OK")

    def test_build_failure_is_reported(self):
        self.edit("ft_atoi.c", "return (0);", "return (0)")
        result = self.checks()
        self.assertKo(result, "make.build")
        self.assertFalse(result.archive_ok)

    def test_unused_parameter_breaks_the_build_with_werror(self):
        self.edit("ft_atoi.c", "(void)nptr;", "")
        self.assertKo(self.checks(), "make.build")

    def test_relink_is_detected(self):
        self.edit("Makefile", ".PHONY: all clean fclean re", ".PHONY: all clean fclean re $(NAME)")
        self.assertKo(self.checks(), "make.relink")

    def test_no_rebuild_after_a_source_change_is_detected(self):
        self.edit("Makefile", "$(NAME): $(OBJS)\n\tar rcs $(NAME) $(OBJS)",
                  "$(NAME):\n\t$(MAKE) --no-print-directory objs\n\tar rcs $(NAME) $(OBJS)\n\nobjs: $(OBJS)")
        self.assertKo(self.checks(), "make.rebuild")

    def test_clean_must_remove_objects(self):
        self.edit("Makefile", "clean:\n\trm -f $(OBJS)", "clean:\n\t@true")
        self.assertKo(self.checks(), "make.clean")

    def test_clean_must_keep_the_archive(self):
        self.edit("Makefile", "clean:\n\trm -f $(OBJS)", "clean:\n\trm -f $(OBJS) $(NAME)")
        self.assertKo(self.checks(), "make.clean", "libft.a")

    def test_fclean_must_remove_the_archive(self):
        self.edit("Makefile", "fclean: clean\n\trm -f $(NAME)", "fclean: clean\n\t@true")
        self.assertKo(self.checks(), "make.fclean")

    def test_cleaning_twice_must_not_fail(self):
        self.edit("Makefile", "fclean: clean\n\trm -f $(NAME)", "fclean: clean\n\trm $(NAME)")
        self.assertKo(self.checks(), "make.fclean")

    def test_re_must_rebuild(self):
        self.edit("Makefile", "re: fclean all", "re: fclean")
        self.assertKo(self.checks(), "make.re")

    def test_lists_must_come_with_plain_make(self):
        import fixture
        names = [n for n in fixture.TYPES if n.startswith("ft_lst")]
        for name in names:
            self.edit("Makefile", " " + name + ".c", "")
        self.edit("Makefile", ".PHONY:", "bonus: $(OBJS) " + " ".join(n + ".o" for n in names)
                  + "\n\tar rcs $(NAME) $(OBJS) "
                  + " ".join(n + ".o" for n in names)
                  + "\n\n%s\n\n.PHONY:" % ("\n".join("%s.o: %s.c\n\t$(CC) $(CFLAGS) -c $< -o $@" % (n, n) for n in names)))
        result = self.checks()
        self.assertKo(result, "functions.required", "ft_lstnew")
        self.assertTrue(result.used_bonus_rule)
        self.assertTrue(result.archive_ok)
        self.assertIn("ft_lstnew", result.defined)


class Files(Base):
    def test_extra_c_file_is_ko(self):
        (self.lib / "main.c").write_text("int main(void) { return (0); }\n")
        self.assertKo(self.checks(), "files.layout", "main.c")

    def test_subfolder_with_sources_is_ko(self):
        (self.lib / "src").mkdir()
        (self.lib / "src" / "ft_extra.c").write_text("")
        self.assertKo(self.checks(), "files.layout", "src")

    def test_stray_file_is_a_warning(self):
        (self.lib / "notes.txt").write_text("x")
        result = self.checks()
        self.assertEqual(self.status(result, "files.extra"), "WARN")
        self.assertIn("notes.txt", self.detail(result, "files.extra"))

    def test_gitignore_and_build_outputs_are_fine(self):
        (self.lib / ".gitignore").write_text("*.o\n")
        (self.lib / "ft_atoi.o").write_text("")
        self.assertEqual(self.status(self.checks(), "files.extra"), "OK")

    def test_bonus_file_names_are_a_warning(self):
        os.rename(str(self.lib / "ft_lstnew.c"), str(self.lib / "ft_lstnew_bonus.c"))
        self.edit("Makefile", "ft_lstnew.c", "ft_lstnew_bonus.c")
        result = self.checks()
        self.assertEqual(self.status(result, "files.extra"), "WARN")
        self.assertIn("_bonus", self.detail(result, "files.extra"))

    def test_source_not_compiled_is_ko(self):
        (self.lib / "ft_unused.c").write_text("int ft_unused(void) { return (0); }\n")
        self.assertKo(self.checks(), "files.unused", "ft_unused.c")


class Symbols(Base):
    def add_file(self, name, code):
        (self.lib / name).write_text('#include "libft.h"\n' + code)
        self.edit("Makefile", "SRCS = ", "SRCS = %s " % name)

    def test_missing_function_is_ko(self):
        self.edit("Makefile", " ft_lstsize.c", "")
        self.assertKo(self.checks(), "functions.required", "ft_lstsize")

    def test_non_static_helper_is_ko(self):
        self.edit("ft_strlen.c", "size_t\tft_strlen", "int\tcount_stuff(void)\n{\n\treturn (0);\n}\n\nsize_t\tft_strlen")
        self.assertKo(self.checks(), "functions.static", "count_stuff")

    def test_static_helper_is_fine(self):
        self.edit("ft_strlen.c", "size_t\tft_strlen", "static int\tcount_stuff(void)\n{\n\treturn (0);\n}\n\nsize_t\tft_strlen")
        self.edit("ft_strlen.c", "(void)s;", "(void)s;\n\t(void)count_stuff;")
        self.assertEqual(self.status(self.checks(), "functions.static"), "OK")

    def test_global_variable_is_ko(self):
        self.edit("ft_strlen.c", "size_t\tft_strlen", "int\tg_count = 3;\n\nsize_t\tft_strlen")
        self.assertKo(self.checks(), "functions.globals", "g_count")

    def test_static_file_variable_is_ko(self):
        self.edit("ft_strlen.c", "size_t\tft_strlen", "static int\tg_count;\n\nsize_t\tft_strlen")
        self.edit("ft_strlen.c", "(void)s;", "(void)s;\n\tg_count++;")
        self.assertKo(self.checks(), "functions.globals", "g_count")

    def test_forbidden_external_is_ko(self):
        self.edit("ft_strlen.c", "(void)s;", "printf(\"%s\", s);")
        self.edit("ft_strlen.c", "#include <stdlib.h>", "#include <stdlib.h>\n#include <stdio.h>")
        self.assertKo(self.checks(), "functions.externals", "ft_strlen", "printf")

    def test_malloc_outside_its_functions_is_ko(self):
        self.edit("ft_strlen.c", "(void)s;", "(void)s;\n\tfree(malloc(1));")
        self.assertKo(self.checks(), "functions.externals", "ft_strlen", "malloc")

    def test_free_in_a_malloc_only_function_is_ko(self):
        self.edit("ft_itoa.c", "(void)n;", "(void)n;\n\tfree(malloc(1));")
        self.assertKo(self.checks(), "functions.externals", "ft_itoa", "free")

    def test_allowed_externals_are_fine(self):
        self.assertEqual(self.status(self.checks(), "functions.externals"), "OK")

    def test_ft_call_reaching_a_forbidden_function_is_a_warning(self):
        self.edit("ft_putnbr_fd.c", "(void)n;", "free(ft_itoa(n));")
        result = self.checks()
        self.assertEqual(self.status(result, "functions.indirect"), "WARN")
        self.assertIn("ft_putnbr_fd", self.detail(result, "functions.indirect"))

    def test_optimized_builds_downgrade_the_builtin_replacements(self):
        self.edit("Makefile", "-Werror", "-Werror -O2")
        self.edit("ft_strdup.c", "(void)s;\n\treturn (malloc(1));",
                  "char\t*p;\n\n\tp = malloc(8);\n\tmemset(p, 0, (size_t)s);\n\treturn (p);")
        self.edit("ft_strdup.c", "#include <stdlib.h>", "#include <stdlib.h>\n#include <string.h>")
        result = self.checks()
        self.assertEqual(self.status(result, "functions.externals"), "WARN")


class Prototypes(Base):
    def test_lstsize_must_return_unsigned(self):
        self.edit("libft.h", "unsigned int\tft_lstsize", "int\tft_lstsize")
        self.edit("ft_lstsize.c", "unsigned int\tft_lstsize", "int\tft_lstsize")
        self.assertKo(self.checks(), "header.prototypes", "ft_lstsize")

    def test_missing_declaration_is_ko(self):
        self.edit("libft.h", "int\tft_atoi(const char *nptr);\n", "")
        self.edit("ft_atoi.c", '#include "libft.h"', "")
        self.assertKo(self.checks(), "header.prototypes", "ft_atoi")

    def test_const_spelling_does_not_matter(self):
        self.edit("libft.h", "int\tft_atoi(const char *nptr);", "int\tft_atoi(char const *str);")
        self.assertEqual(self.status(self.checks(), "header.prototypes"), "OK")

    def test_list_struct_members(self):
        self.edit("libft.h", "\tvoid\t\t\t*content;", "\tint\t\t\tcontent;")
        self.assertKo(self.checks(), "header.t_list")

    def test_list_struct_has_no_extra_member(self):
        self.edit("libft.h", "\tvoid\t\t\t*content;", "\tvoid\t\t\t*content;\n\tint\t\t\tsize;")
        self.assertKo(self.checks(), "header.t_list")

    def test_header_guard_is_required(self):
        self.edit("libft.h", "#ifndef LIBFT_H\n# define LIBFT_H\n", "")
        self.edit("libft.h", "#endif\n", "")
        self.assertKo(self.checks(), "header.guard")

    def test_restrict_is_forbidden(self):
        self.edit("libft.h", "size_t\tft_strlcpy(char *dst,", "size_t\tft_strlcpy(char *restrict dst,")
        self.edit("ft_strlcpy.c", "ft_strlcpy(char *dst,", "ft_strlcpy(char *restrict dst,")
        self.assertKo(self.checks(), "header.restrict", "libft.h")

    def test_restrict_in_a_comment_is_fine(self):
        self.edit("ft_strlcpy.c", "(void)dst;", "/* pas de restrict ici */\n\t(void)dst;")
        self.assertEqual(self.status(self.checks(), "header.restrict"), "OK")


class Readme(Base):
    def test_missing_readme(self):
        os.remove(str(self.lib / "README.md"))
        self.assertKo(self.checks(), "readme.exists")

    def test_first_line_must_be_italic(self):
        self.edit("README.md", "*This project has been created as part of the 42 curriculum by login1.*",
                  "This project has been created as part of the 42 curriculum by login1.")
        self.assertKo(self.checks(), "readme.first_line")

    def test_first_line_accepts_underscores_and_several_logins(self):
        self.edit("README.md", "*This project has been created as part of the 42 curriculum by login1.*",
                  "_This project has been created as part of the 42 curriculum by login1, login2._")
        self.assertEqual(self.status(self.checks(), "readme.first_line"), "OK")

    def test_placeholder_login_is_ko(self):
        self.edit("README.md", "by login1.", "by <login1>.")
        self.assertKo(self.checks(), "readme.first_line")

    def test_required_sections(self):
        self.edit("README.md", "# Instructions", "# Usage")
        self.assertKo(self.checks(), "readme.sections", "Instructions")

    def test_section_names_in_french_are_accepted(self):
        self.edit("README.md", "# Resources", "## Ressources")
        self.assertEqual(self.status(self.checks(), "readme.sections"), "OK")

    def test_ai_usage_missing_is_a_warning(self):
        self.edit("README.md", "AI was used to review the documentation.", "")
        self.assertEqual(self.status(self.checks(), "readme.ai"), "WARN")

    def test_library_description_missing_is_a_warning(self):
        self.edit("README.md", "# Library\nIt contains ft_split, ft_itoa and ft_lstnew among others.\n", "")
        self.edit("README.md", "such as ft_strlen and ft_memcpy", "")
        self.assertEqual(self.status(self.checks(), "readme.library"), "WARN")


class Norminette(Base):
    def fake_norminette(self, directory, code, output="ok\n"):
        directory.mkdir(parents=True, exist_ok=True)
        path = directory / "norminette"
        path.write_text("#!/bin/sh\nprintf '%s'\nexit %d\n" % (output, code))
        path.chmod(path.stat().st_mode | stat.S_IEXEC)
        return str(path)

    def test_uses_the_real_exit_code_not_the_words(self):
        cmd = self.fake_norminette(self.tmp / "ko_error_path", 0,
                                   "/home/koala/error/ft_atoi.c: OK!\\n")
        check = conformity.check_norminette(self.lib, command=cmd)
        self.assertEqual(check.status, "OK")

    def test_norm_error_is_ko_with_its_output(self):
        cmd = self.fake_norminette(self.tmp / "n", 1, "ft_atoi.c: Error!\\n")
        check = conformity.check_norminette(self.lib, command=cmd)
        self.assertEqual(check.status, "KO")
        self.assertIn("ft_atoi.c", check.detail)

    def test_colors_of_norminette_do_not_reach_the_report(self):
        cmd = self.fake_norminette(self.tmp / "c", 1, "Error: \\033[94mline too long\\033[0m\\n")
        check = conformity.check_norminette(self.lib, command=cmd)
        self.assertEqual(check.status, "KO")
        self.assertIn("line too long", check.detail)
        self.assertNotIn("\033", check.detail)

    def test_missing_norminette_is_skip(self):
        check = conformity.check_norminette(self.lib, command="")
        self.assertEqual(check.status, "SKIP")


class Runner(Base):
    def test_missing_function_gets_a_stub_and_only_its_tests_fail(self):
        self.edit("Makefile", " ft_lstsize.c", "")
        result = self.checks()
        self.assertTrue(result.archive_ok)
        exe = build.link_runner(self.ws, result.defined, [FAKES / "fake_missing.c"],
                                cache=self.tmp / "cache")
        out = subprocess.run([str(exe)], stdout=subprocess.PIPE,
                             universal_newlines=True, timeout=30).stdout
        lines = dict(line.split("\t")[1:2] + [line.split("\t")[0]]
                     for line in out.splitlines())
        self.assertEqual(lines["missing/calls_absent_function"], "MISSING")
        self.assertEqual(lines["missing/calls_present_function"], "PASS")

    def test_objects_are_reused_between_runs(self):
        result = self.checks()
        cache = self.tmp / "cache"
        build.link_runner(self.ws, result.defined, [FAKES / "fake_missing.c"], cache=cache)
        objects = list(cache.rglob("fake_missing*.o"))
        self.assertEqual(len(objects), 1)
        mtime = objects[0].stat().st_mtime_ns
        build.link_runner(self.ws, result.defined, [FAKES / "fake_missing.c"], cache=cache)
        self.assertEqual(objects[0].stat().st_mtime_ns, mtime)


class Subject(unittest.TestCase):
    def test_table_has_the_43_functions(self):
        self.assertEqual(len(subject.FUNCTIONS), 43)
        parts = [f.part for f in subject.FUNCTIONS]
        self.assertEqual((parts.count(1), parts.count(2), parts.count(3)), (23, 11, 9))

    def test_externals_follow_the_subject(self):
        table = {f.name: f.externals for f in subject.FUNCTIONS}
        self.assertEqual(table["ft_strlen"], ())
        self.assertEqual(table["ft_calloc"], ("malloc",))
        self.assertEqual(table["ft_split"], ("malloc", "free"))
        self.assertEqual(table["ft_putnbr_fd"], ("write",))
        self.assertEqual(table["ft_lstclear"], ("free",))
        self.assertEqual(table["ft_lstmap"], ("malloc", "free"))
        self.assertEqual(table["ft_lstiter"], ())


if __name__ == "__main__":
    unittest.main()
