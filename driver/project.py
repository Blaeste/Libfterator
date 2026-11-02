"""Copie de travail du projet : le dossier d'origine n'est jamais modifié."""
import os
import shutil
import subprocess
import tempfile
from collections import namedtuple
from pathlib import Path

MakeResult = namedtuple("MakeResult", "code output")

_CLEAN_ENV = ("MAKEFLAGS", "MFLAGS", "MAKELEVEL", "MAKEOVERRIDES", "CC", "CFLAGS",
              "AR", "ARFLAGS", "LDFLAGS", "NAME")


def _ignore_factory(tester_root):
    def ignore(directory, names):
        ignored = []
        for name in names:
            path = Path(directory) / name
            if name == ".git" or (tester_root and path.resolve() == tester_root):
                ignored.append(name)
        return ignored
    return ignore


class Workspace:
    def __init__(self, source, work_root, tester_root=None):
        self.source = Path(source).resolve()
        work_root = Path(work_root)
        work_root.mkdir(parents=True, exist_ok=True)
        self.root = Path(tempfile.mkdtemp(prefix="run-", dir=str(work_root)))
        self.lib = self.root / "libft"
        shutil.copytree(str(self.source), str(self.lib), symlinks=True,
                        ignore=_ignore_factory(tester_root))
        # Les restes d'une ancienne compilation fausseraient tous les contrôles.
        for pattern in ("*.o", "*.a"):
            for path in self.lib.rglob(pattern):
                path.unlink()
        self.env = {k: v for k, v in os.environ.items() if k not in _CLEAN_ENV}
        self.env["LC_ALL"] = "C"

    @property
    def archive(self):
        return self.lib / "libft.a"

    def make(self, *args, timeout=180):
        try:
            proc = subprocess.run(["make", "-C", str(self.lib)] + list(args),
                                  stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                  env=self.env, timeout=timeout)
        except subprocess.TimeoutExpired:
            return MakeResult(124, "make %s : plus de %d s" % (" ".join(args), timeout))
        except OSError as err:
            return MakeResult(127, "make introuvable : %s" % err)
        return MakeResult(proc.returncode, proc.stdout.decode("utf-8", "replace"))

    def objects(self):
        return sorted(self.lib.rglob("*.o"))

    def cleanup(self):
        shutil.rmtree(str(self.root), ignore_errors=True)
