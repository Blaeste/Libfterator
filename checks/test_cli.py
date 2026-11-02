"""Tests de bout en bout du lanceur, y compris les modes valgrind et ASan."""
import json
import os
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from fixture import make_fixture  # noqa: E402

ROOT = Path(__file__).resolve().parent.parent
CLI = str(ROOT / "libfterator")

GOOD_STRDUP = """char\t*p;
\tsize_t\tn;
\tsize_t\ti;

\tn = 0;
\twhile (s[n])
\t\tn++;
\tp = malloc(n + 1);
\tif (!p)
\t\treturn (0);
\ti = 0;
\twhile (i <= n)
\t{
\t\tp[i] = s[i];
\t\ti++;
\t}
\treturn (p);"""


class Cli(unittest.TestCase):
    def setUp(self):
        self.tmp = Path(tempfile.mkdtemp(prefix="lft-cli-"))
        self.lib = make_fixture(self.tmp / "libft")
        self.json = self.tmp / "out.json"

    def tearDown(self):
        shutil.rmtree(str(self.tmp), ignore_errors=True)

    def edit(self, name, old, new):
        path = self.lib / name
        text = path.read_text()
        self.assertIn(old, text)
        path.write_text(text.replace(old, new))

    def run_cli(self, *args):
        return subprocess.run([CLI, str(self.lib), "--no-color", "--no-norm", "--json",
                               str(self.json)] + list(args), stdout=subprocess.PIPE,
                              stderr=subprocess.STDOUT, universal_newlines=True,
                              errors="replace", timeout=300)

    def results(self):
        data = json.loads(self.json.read_text(encoding="utf-8"))
        return {t["name"]: t for t in data["tests"]}

    def strdup_body(self, body):
        self.edit("ft_strdup.c", "(void)s;\n\treturn (malloc(1));", body)

    def assertNoTraceback(self, proc):
        self.assertNotIn("Traceback", proc.stdout)


class Basics(Cli):
    def test_targeted_run_only_runs_the_pattern_and_skips_conformity(self):
        proc = self.run_cli("--run", "strlen/")
        names = self.results()
        self.assertTrue(names)
        self.assertTrue(all(n.startswith("strlen/") for n in names))
        self.assertNotIn("CONFORMITÉ", proc.stdout)

    def test_exit_code_is_one_on_failure(self):
        self.assertEqual(self.run_cli("--run", "strlen/basic").returncode, 1)

    def test_exit_code_is_zero_on_success(self):
        self.assertEqual(self.run_cli("--run", "bzero/size_zero").returncode, 0)

    def test_exit_code_is_two_for_a_missing_directory(self):
        proc = subprocess.run([CLI, str(self.tmp / "absent")], stdout=subprocess.PIPE,
                              stderr=subprocess.STDOUT, universal_newlines=True)
        self.assertEqual(proc.returncode, 2)
        self.assertNoTraceback(proc)

    def test_invalid_timeout_is_refused(self):
        proc = subprocess.run([CLI, str(self.lib), "--timeout", "0"], stdout=subprocess.PIPE,
                              stderr=subprocess.STDOUT, universal_newlines=True)
        self.assertEqual(proc.returncode, 2)

    def test_list_prints_every_test_name(self):
        proc = subprocess.run([CLI, str(self.lib), "--list"], stdout=subprocess.PIPE,
                              universal_newlines=True, timeout=120)
        names = proc.stdout.split()
        self.assertIn("split/malloc_failure_frees_everything", names)
        self.assertIn("info/strlen_null", names)
        self.assertGreater(len(names), 500)

    def test_verbose_shows_each_test(self):
        proc = self.run_cli("--run", "bzero/size_zero", "--verbose")
        self.assertIn("size_zero", proc.stdout)

    def test_info_section_is_never_counted(self):
        proc = self.run_cli("--run", "info/strlen_null")
        self.assertEqual(proc.returncode, 0)
        self.assertIn("non comptés", proc.stdout)

    def test_a_log_file_is_written(self):
        proc = self.run_cli("--run", "bzero/size_zero")
        self.assertIn("Log complet", proc.stdout)
        path = proc.stdout.split("Log complet : ")[1].split()[0]
        self.assertTrue(Path(path).is_file())
        self.assertNotIn("\033[", Path(path).read_text(encoding="utf-8"))

    def test_seed_changes_the_random_inputs_reproducibly(self):
        first = self.run_cli("--run", "memcpy/fuzz", "--seed", "7")
        again = self.run_cli("--run", "memcpy/fuzz", "--seed", "7")
        self.assertEqual(first.returncode, again.returncode)


class Robustness(Cli):
    def test_compile_error_in_the_libft_gives_a_clear_message(self):
        self.edit("ft_atoi.c", "return (0);", "return (0)")
        proc = self.run_cli()
        self.assertEqual(proc.returncode, 1)
        self.assertNoTraceback(proc)
        self.assertIn("ne peuvent pas tourner", proc.stdout)
        self.assertIn("ft_atoi.c", proc.stdout)

    def test_compile_error_in_a_targeted_run_shows_the_compiler_output(self):
        self.edit("ft_atoi.c", "return (0);", "return (0)")
        proc = self.run_cli("--run", "atoi/")
        self.assertEqual(proc.returncode, 1)
        self.assertNoTraceback(proc)
        self.assertIn("ft_atoi.c", proc.stdout)

    def test_undefined_symbol_in_the_libft_gives_a_clear_message(self):
        self.edit("ft_atoi.c", "(void)nptr;", "(void)nptr;\n\textern int\tno_such_function(void);\n\tno_such_function();")
        proc = self.run_cli("--run", "atoi/")
        self.assertEqual(proc.returncode, 1)
        self.assertNoTraceback(proc)
        self.assertIn("no_such_function", proc.stdout)

    def test_missing_function_only_affects_its_tests(self):
        self.edit("Makefile", " ft_lstsize.c", "")
        self.run_cli("--run", "lstsize/basic")
        self.assertEqual(self.results()["lstsize/basic"]["status"], "MISSING")

    def test_no_makefile_is_reported(self):
        os.remove(str(self.lib / "Makefile"))
        proc = self.run_cli()
        self.assertEqual(proc.returncode, 1)
        self.assertNoTraceback(proc)

    def test_path_with_spaces(self):
        spaced = self.tmp / "dossier avec espaces"
        shutil.move(str(self.lib), str(spaced))
        self.lib = spaced
        proc = self.run_cli("--run", "bzero/size_zero")
        self.assertEqual(proc.returncode, 0, proc.stdout)

    def test_tester_cloned_inside_the_libft_is_ignored(self):
        (self.lib / "libfterator_v2").mkdir()
        (self.lib / "libfterator_v2" / "ft_ignored.c").write_text("not c at all\n")
        proc = self.run_cli("--run", "bzero/size_zero")
        self.assertEqual(proc.returncode, 0, proc.stdout)


@unittest.skipUnless(shutil.which("valgrind"), "valgrind absent")
class Valgrind(Cli):
    def test_clean_function_passes(self):
        self.strdup_body(GOOD_STRDUP)
        proc = self.run_cli("--valgrind", "--run", "strdup/basic")
        self.assertEqual(self.results()["strdup/basic"]["status"], "PASS", proc.stdout)

    def test_leak_is_seen_by_valgrind_itself(self):
        self.strdup_body(GOOD_STRDUP.replace("n = 0;", "(void)malloc(3);\n\tn = 0;", 1))
        proc = self.run_cli("--valgrind", "--run", "strdup/basic")
        test = self.results()["strdup/basic"]
        self.assertEqual(test["status"], "VALGRIND", proc.stdout)
        self.assertIn("definitely lost", test["detail"])

    def test_invalid_write_is_seen_by_valgrind(self):
        self.strdup_body("char\t*p;\n\n\t(void)s;\n\tp = malloc(1);\n\tp[1] = 0;\n\treturn (p);")
        proc = self.run_cli("--valgrind", "--run", "strdup/empty_string")
        test = self.results()["strdup/empty_string"]
        self.assertEqual(test["status"], "VALGRIND", proc.stdout)
        self.assertIn("Invalid write", test["detail"])


class Sanitizers(Cli):
    def test_clean_function_passes(self):
        self.strdup_body(GOOD_STRDUP)
        proc = self.run_cli("--asan", "--run", "strdup/basic")
        self.assertEqual(self.results()["strdup/basic"]["status"], "PASS", proc.stdout)

    def test_heap_overflow_is_reported(self):
        self.strdup_body("char\t*p;\n\n\t(void)s;\n\tp = malloc(1);\n\tp[1] = 0;\n\treturn (p);")
        proc = self.run_cli("--asan", "--run", "strdup/empty_string")
        test = self.results()["strdup/empty_string"]
        self.assertEqual(test["status"], "CRASH", proc.stdout)
        self.assertIn("heap-buffer-overflow", test["detail"])

    def test_undefined_behavior_is_reported(self):
        self.edit("ft_atoi.c", "(void)nptr;\n\treturn (0);",
                  "volatile int\tn;\n\n\t(void)nptr;\n\tn = 2147483647;\n\tn = n + 1;\n\treturn (n);")
        proc = self.run_cli("--asan", "--run", "atoi/zero")
        test = self.results()["atoi/zero"]
        self.assertEqual(test["status"], "CRASH", proc.stdout)
        self.assertIn("runtime error", test["detail"])

    def test_malloc_failures_still_work_under_asan(self):
        self.strdup_body(GOOD_STRDUP)
        proc = self.run_cli("--asan", "--run", "strdup/malloc_failure")
        self.assertEqual(self.results()["strdup/malloc_failure_returns_null"]["status"], "PASS",
                         proc.stdout)


if __name__ == "__main__":
    unittest.main()
