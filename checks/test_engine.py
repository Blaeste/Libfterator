"""Tests du moteur C : de faux tests doivent produire les bons statuts."""
import os
import shutil
import subprocess
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
ENGINE = ROOT / "engine"
FAKES = Path(__file__).resolve().parent / "fakes"
CC = os.environ.get("CC", "cc")
WRAP = "-Wl,--wrap=malloc,--wrap=free"

_tmp = None


def setUpModule():
    global _tmp
    _tmp = Path(tempfile.mkdtemp(prefix="lft-engine-"))


def tearDownModule():
    shutil.rmtree(str(_tmp), ignore_errors=True)


def build(fake, cc=CC):
    exe = _tmp / (Path(fake).stem + "_" + cc)
    if exe.exists():
        return exe
    cmd = [cc, "-g", "-Wall", "-Wextra", "-Werror", "-I", str(ENGINE),
           str(FAKES / fake)]
    cmd += sorted(str(p) for p in ENGINE.glob("*.c"))
    cmd += [WRAP, "-o", str(exe)]
    proc = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                          universal_newlines=True)
    if proc.returncode != 0:
        raise AssertionError("compilation de %s:\n%s" % (fake, proc.stdout))
    return exe


class Result:
    def __init__(self, proc):
        self.code = proc.returncode
        self.lines = proc.stdout.splitlines()
        self.tests = {}
        for line in self.lines:
            status, name, ms, detail = line.split("\t", 3)
            int(ms)
            self.tests[name] = (status, detail)

    def status(self, name):
        return self.tests[name][0]

    def detail(self, name):
        return self.tests[name][1]


def run(fake, *args):
    proc = subprocess.run([str(build(fake))] + list(args),
                          stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                          universal_newlines=True, timeout=60)
    return Result(proc)


class Statuses(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.res = run("fake_status.c", "--timeout", "1")

    def test_passing_test_is_pass(self):
        self.assertEqual(self.res.status("fake/ok"), "PASS")

    def test_failed_assert_is_fail_and_names_the_line(self):
        self.assertEqual(self.res.status("fake/assert_fails"), "FAIL")
        self.assertIn("fake_status.c:", self.res.detail("fake/assert_fails"))
        self.assertIn("3", self.res.detail("fake/assert_fails"))

    def test_segfault_is_crash(self):
        self.assertEqual(self.res.status("fake/segfault"), "CRASH")
        self.assertIn("SIGSEGV", self.res.detail("fake/segfault"))

    def test_infinite_loop_is_timeout(self):
        self.assertEqual(self.res.status("fake/hangs"), "TIMEOUT")

    def test_exit_inside_a_test_is_not_a_pass(self):
        self.assertEqual(self.res.status("fake/exits_early"), "CRASH")

    def test_output_of_a_test_cannot_forge_a_result(self):
        self.assertEqual(self.res.status("fake/prints_garbage"), "PASS")
        self.assertNotIn("fake/forged", self.res.tests)

    def test_one_crash_does_not_stop_the_others(self):
        self.assertEqual(self.res.status("other/second_suite"), "PASS")

    def test_exit_code_is_one_when_a_test_fails(self):
        self.assertEqual(self.res.code, 1)


class Selection(unittest.TestCase):
    def test_list_prints_names_without_running(self):
        proc = subprocess.run([str(build("fake_status.c")), "--list"],
                              stdout=subprocess.PIPE, universal_newlines=True,
                              timeout=5)
        names = proc.stdout.split()
        self.assertIn("fake/hangs", names)
        self.assertIn("other/second_suite", names)
        self.assertEqual(proc.returncode, 0)

    def test_run_filters_by_substring(self):
        res = run("fake_status.c", "--run", "other/")
        self.assertEqual(list(res.tests), ["other/second_suite"])
        self.assertEqual(res.code, 0)

    def test_only_needs_the_exact_name(self):
        res = run("fake_status.c", "--only", "fake/ok")
        self.assertEqual(list(res.tests), ["fake/ok"])
        res = run("fake_status.c", "--only", "fake/o")
        self.assertEqual(res.tests, {})

    def test_no_fork_runs_in_the_process_and_reports(self):
        res = run("fake_status.c", "--no-fork", "--only", "fake/ok")
        self.assertEqual(res.status("fake/ok"), "PASS")


class Assertions(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.res = run("fake_assert.c")

    def test_all_assertions_can_pass(self):
        self.assertEqual(self.res.status("assert/all_pass"), "PASS")

    def test_str_shows_got_and_expected(self):
        detail = self.res.detail("assert/str_diff")
        self.assertEqual(self.res.status("assert/str_diff"), "FAIL")
        self.assertIn('"abc"', detail)
        self.assertIn('"abd"', detail)

    def test_str_handles_null(self):
        self.assertEqual(self.res.status("assert/str_null"), "FAIL")
        self.assertIn("NULL", self.res.detail("assert/str_null"))

    def test_unprintable_bytes_are_escaped(self):
        detail = self.res.detail("assert/str_unprintable")
        self.assertIn("\\xff", detail)
        self.assertIn("\\x0a", detail)
        self.assertIn('\\"', detail)

    def test_mem_names_the_first_differing_byte(self):
        detail = self.res.detail("assert/mem_diff")
        self.assertEqual(self.res.status("assert/mem_diff"), "FAIL")
        self.assertIn("octet 2", detail)

    def test_ptr_shows_the_offset(self):
        self.assertIn("3 octet", self.res.detail("assert/ptr_shifted"))

    def test_null_and_not_null(self):
        self.assertEqual(self.res.status("assert/null_expected"), "FAIL")
        self.assertEqual(self.res.status("assert/not_null_expected"), "FAIL")

    def test_sign_compares_signs_in_every_direction(self):
        self.assertEqual(self.res.status("assert/sign_opposite"), "FAIL")
        self.assertEqual(self.res.status("assert/sign_zero_vs_positive"), "FAIL")
        self.assertEqual(self.res.status("assert/sign_negative_vs_zero"), "FAIL")

    def test_eq_on_size_t(self):
        self.assertEqual(self.res.status("assert/eq_large_size_t"), "FAIL")


class Allocations(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.res = run("fake_alloc.c", "--timeout", "30")

    def test_balanced_test_passes(self):
        self.assertEqual(self.res.status("alloc/clean"), "PASS")

    def test_leak_is_reported_with_its_size(self):
        self.assertEqual(self.res.status("alloc/leak"), "LEAK")
        self.assertIn("1 bloc", self.res.detail("alloc/leak"))
        self.assertIn("10 octet", self.res.detail("alloc/leak"))

    def test_leak_counts_every_block(self):
        self.assertIn("2 bloc", self.res.detail("alloc/leak_two_blocks"))
        self.assertIn("15 octet", self.res.detail("alloc/leak_two_blocks"))

    def test_double_free_is_memerr(self):
        self.assertEqual(self.res.status("alloc/double_free"), "MEMERR")
        self.assertIn("double free", self.res.detail("alloc/double_free"))

    def test_invalid_free_is_memerr(self):
        self.assertEqual(self.res.status("alloc/free_inside_block"), "MEMERR")
        self.assertEqual(self.res.status("alloc/free_stack"), "MEMERR")

    def test_free_null_is_allowed(self):
        self.assertEqual(self.res.status("alloc/free_null_is_fine"), "PASS")

    def test_blocks_of_the_test_count_for_leaks(self):
        self.assertEqual(self.res.status("alloc/test_block_counts_for_leaks"), "LEAK")
        self.assertEqual(self.res.status("alloc/test_block_can_be_freed"), "PASS")

    def test_counters(self):
        self.assertEqual(self.res.status("alloc/counters"), "PASS",
                         self.res.detail("alloc/counters"))

    def test_memory_is_poisoned_to_expose_missing_terminators(self):
        self.assertEqual(self.res.status("alloc/memory_is_not_zeroed"), "PASS")

    def test_zero_size(self):
        self.assertEqual(self.res.status("alloc/zero_size_is_unique_and_freeable"), "PASS")

    def test_huge_size_fails_like_malloc(self):
        self.assertEqual(self.res.status("alloc/huge_size_returns_null"), "PASS",
                         self.res.detail("alloc/huge_size_returns_null"))

    def test_many_blocks_stay_supported(self):
        self.assertEqual(self.res.status("alloc/many_blocks"), "PASS",
                         self.res.detail("alloc/many_blocks"))
        self.assertEqual(self.res.status("alloc/many_cycles_do_not_exhaust_mappings"),
                         "PASS",
                         self.res.detail("alloc/many_cycles_do_not_exhaust_mappings"))


class Guards(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.res = run("fake_guard.c")

    def assertOob(self, name, *words):
        self.assertEqual(self.res.status(name), "OOB", self.res.detail(name))
        for word in words:
            self.assertIn(word, self.res.detail(name))

    def test_in_bounds_accesses_pass(self):
        for name in ("guard/block_in_bounds", "guard/buf_in_bounds",
                     "guard/buf_front_in_bounds", "guard/str_copy_is_correct"):
            self.assertEqual(self.res.status(name), "PASS", self.res.detail(name))

    def test_one_byte_past_a_block(self):
        self.assertOob("guard/block_write_one_past", "1 octet", "5 octet", "criture")
        self.assertOob("guard/block_read_one_past", "1 octet", "5 octet", "ecture")

    def test_write_before_a_block_is_seen_at_free(self):
        self.assertEqual(self.res.status("guard/block_write_before_start"), "MEMERR")
        self.assertIn("avant", self.res.detail("guard/block_write_before_start"))

    def test_zero_size_block_has_no_usable_byte(self):
        self.assertOob("guard/zero_size_access")

    def test_use_after_free(self):
        self.assertOob("guard/use_after_free", "libéré")

    def test_guarded_buffers(self):
        self.assertOob("guard/buf_write_one_past", "1 octet")
        self.assertOob("guard/buf_front_write_before", "avant")

    def test_guarded_strings(self):
        self.assertOob("guard/str_read_past_terminator")
        self.assertOob("guard/str_write_is_refused", "lecture seule")
        self.assertOob("guard/str_front_read_before", "avant")
        self.assertOob("guard/mem_read_past")

    def test_stack_overflow_is_a_plain_crash(self):
        self.assertEqual(self.res.status("guard/stack_overflow_is_plain_crash"), "CRASH")
        self.assertIn("SIGSEGV", self.res.detail("guard/stack_overflow_is_plain_crash"))


class MallocFailures(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.res = run("fake_mallocfail.c")

    def test_correct_function_passes_every_failure(self):
        self.assertEqual(self.res.status("mf/correct_function"), "PASS",
                         self.res.detail("mf/correct_function"))

    def test_leak_names_the_failing_malloc(self):
        name = "mf/leak_is_reported_with_the_malloc_number"
        self.assertEqual(self.res.status(name), "LEAK")
        self.assertIn("n°3", self.res.detail(name))

    def test_unchecked_malloc_is_a_crash_naming_the_malloc(self):
        name = "mf/unchecked_malloc_crashes"
        self.assertEqual(self.res.status(name), "CRASH")
        self.assertIn("n°1", self.res.detail(name))

    def test_non_null_return_after_failure_fails(self):
        name = "mf/non_null_after_failure_fails"
        self.assertEqual(self.res.status(name), "FAIL")
        self.assertIn("n°1", self.res.detail(name))

    def test_function_without_malloc_fails(self):
        name = "mf/function_without_malloc_fails"
        self.assertEqual(self.res.status(name), "FAIL")
        self.assertIn("malloc", self.res.detail(name))

    def test_blocks_of_the_test_are_never_injected(self):
        name = "mf/test_blocks_are_never_injected"
        self.assertEqual(self.res.status(name), "PASS", self.res.detail(name))


if __name__ == "__main__":
    unittest.main()
