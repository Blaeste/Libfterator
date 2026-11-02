"""Compilation du moteur et des tests, édition de liens avec libft.a."""
import hashlib
import os
import subprocess
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

from . import subject

ROOT = Path(__file__).resolve().parent.parent
ENGINE = ROOT / "engine"
TESTS = ROOT / "tests"
WRAP = "-Wl,--wrap=malloc,--wrap=free"
BASE_FLAGS = ("-g", "-O0", "-w", "-fno-builtin", "-fno-omit-frame-pointer")


class BuildError(Exception):
    pass


def default_cc():
    return os.environ.get("CC", "cc")


def test_sources():
    return sorted(TESTS.rglob("*.c"))


def _headers_stamp():
    return max(p.stat().st_mtime_ns for p in ENGINE.glob("*.h"))


def _object_path(source, cache):
    rel = hashlib.sha1(str(source).encode("utf-8")).hexdigest()[:10]
    return cache / ("%s-%s.o" % (Path(source).stem, rel))


def _compile_one(args):
    source, obj, cc, flags = args
    tmp = obj.with_suffix(".%d.tmp.o" % os.getpid())
    proc = subprocess.run([cc] + list(flags) + ["-I", str(ENGINE), "-c", str(source),
                                                "-o", str(tmp)],
                          stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                          universal_newlines=True)
    if proc.returncode != 0:
        raise BuildError("compilation de %s :\n%s" % (Path(source).name, proc.stdout))
    os.replace(str(tmp), str(obj))
    return obj


def compile_objects(sources, cache, cc, flags=BASE_FLAGS):
    key = hashlib.sha1(" ".join((cc,) + tuple(flags)).encode("utf-8")).hexdigest()[:8]
    cache = Path(cache) / ("%s-%s" % (Path(cc).name, key))
    cache.mkdir(parents=True, exist_ok=True)
    headers = _headers_stamp()
    objects = []
    todo = []
    for source in sources:
        obj = _object_path(source, cache)
        objects.append(obj)
        stamp = max(Path(source).stat().st_mtime_ns, headers)
        if not obj.exists() or obj.stat().st_mtime_ns < stamp:
            todo.append((source, obj, cc, flags))
    if todo:
        with ThreadPoolExecutor(max_workers=os.cpu_count() or 2) as pool:
            list(pool.map(_compile_one, todo))
    return objects


def _write_stubs(ws, defined):
    """Une fonction absente ne doit pas empêcher les autres tests de tourner."""
    missing = [n for n in subject.NAMES if n not in defined]
    path = ws.root / "lft_stubs.c"
    lines = ["void\tlft_missing(const char *name) __attribute__((noreturn));\n"]
    for name in missing:
        lines.append('void\t%s(void)\n{\n\tlft_missing("%s");\n}\n' % (name, name))
    path.write_text("\n".join(lines))
    return path


def link_runner(ws, defined, tests=None, cache=None, cc=None, flags=BASE_FLAGS,
                link_flags=(), archive=None):
    cc = cc or default_cc()
    cache = cache or (ROOT / "build" / "obj")
    tests = test_sources() if tests is None else tests
    sources = sorted(ENGINE.glob("*.c")) + list(tests)
    objects = compile_objects(sources, cache, cc, flags)
    stubs = _write_stubs(ws, defined)
    exe = ws.root / "runner"
    cmd = [cc] + [str(o) for o in objects] + [str(stubs), str(archive or ws.archive),
                                              WRAP] + list(link_flags) + ["-o", str(exe)]
    proc = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                          universal_newlines=True)
    if proc.returncode != 0:
        raise BuildError("édition de liens avec libft.a :\n%s" % proc.stdout)
    return exe
