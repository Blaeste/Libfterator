"""Audit du portage : chaque test de la v1 doit exister dans la v2."""
import re
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(Path(__file__).resolve().parent))
sys.path.insert(0, str(ROOT))

from fixture import make_fixture  # noqa: E402


def current_names():
    with tempfile.TemporaryDirectory(prefix="lft-portage-") as tmp:
        lib = make_fixture(Path(tmp) / "libft")
        proc = subprocess.run([str(ROOT / "libfterator"), str(lib), "--list"],
                              stdout=subprocess.PIPE, universal_newlines=True, timeout=120)
    return set(proc.stdout.split())


def portage_rows():
    rows = []
    text = (ROOT / "docs" / "PORTAGE.md").read_text(encoding="utf-8")
    for line in text.splitlines():
        match = re.match(r"^\| `([^`]+)` \| `([^`]+)` \| (.*) \|$", line)
        if match:
            rows.append(match.groups())
    return rows


class Portage(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.names = current_names()
        cls.rows = portage_rows()

    def test_the_329_tests_of_v1_are_listed(self):
        self.assertEqual(len(self.rows), 329)
        self.assertEqual(len({row[0] for row in self.rows}), 329)

    def test_every_target_exists_in_v2(self):
        missing = [row for row in self.rows if row[1] not in self.names]
        self.assertEqual(missing, [])

    def test_v2_has_more_tests_than_v1(self):
        self.assertGreater(len(self.names), 329)


if __name__ == "__main__":
    unittest.main()
