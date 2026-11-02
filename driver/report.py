"""Affichage terminal et fichiers de log."""
import datetime
import json
import re
import sys
from pathlib import Path

from . import subject

COUNTED_BAD = ("FAIL", "CRASH", "TIMEOUT", "LEAK", "MEMERR", "OOB", "MISSING",
               "INTERNAL", "VALGRIND")

_PART_OF_SUITE = {f.name[3:]: f.part for f in subject.FUNCTIONS}

PART_TITLES = {1: "PARTIE 1, fonctions de la libc", 2: "PARTIE 2, fonctions supplémentaires",
               3: "PARTIE 3, listes chaînées"}


class Colors:
    def __init__(self, enabled):
        codes = {"red": "31", "green": "32", "yellow": "33", "cyan": "36", "gray": "90",
                 "bold": "1", "dim": "2", "reset": "0"}
        for name, code in codes.items():
            setattr(self, name, "\033[%sm" % code if enabled else "")


class TestResult:
    __slots__ = ("status", "name", "ms", "detail")

    def __init__(self, status, name, ms, detail):
        self.status = status
        self.name = name
        self.ms = ms
        self.detail = detail

    @property
    def suite(self):
        return self.name.split("/", 1)[0]

    @property
    def is_bad(self):
        return self.status in COUNTED_BAD


def parse_line(line):
    parts = line.rstrip("\n").split("\t", 3)
    if len(parts) != 4:
        return None
    try:
        ms = int(parts[2])
    except ValueError:
        return None
    detail = parts[3].replace("\\n", "\n").replace("\\t", "\t")
    return TestResult(parts[0], parts[1], ms, detail)


class Reporter:
    def __init__(self, stream=None, color=None, verbose=False):
        self.out = stream or sys.stdout
        enabled = self.out.isatty() if color is None else color
        self.c = Colors(enabled)
        self.verbose = verbose
        self.log_lines = []

    def echo(self, text=""):
        print(text, file=self.out)
        self.log_lines.append(text)

    # -------------------------- En-têtes -------------------------

    def banner(self, path):
        c = self.c
        self.echo("%s%sLibfterator v2%s · sujet libft v19.3" % (c.bold, c.cyan, c.reset))
        self.echo("%sProjet : %s%s" % (c.dim, path, c.reset))
        self.echo()

    def section(self, title):
        c = self.c
        self.echo("%s%s%s%s" % (c.bold, c.cyan, title, c.reset))

    def _status_word(self, status):
        c = self.c
        colors = {"OK": c.green, "PASS": c.green, "KO": c.red, "FAIL": c.red, "CRASH": c.red,
                  "LEAK": c.red, "MEMERR": c.red, "OOB": c.red, "TIMEOUT": c.red,
                  "MISSING": c.red, "INTERNAL": c.red, "VALGRIND": c.red,
                  "WARN": c.yellow, "SKIP": c.gray, "INFO": c.cyan}
        return "%s%-5s%s" % (colors.get(status, ""), status, c.reset)

    # ------------------------- Conformité ------------------------

    def conformity(self, result):
        self.section("CONFORMITÉ AU SUJET")
        group = None
        for check in result.checks:
            if check.group != group:
                group = check.group
                self.echo("  %s%s%s" % (self.c.bold, group, self.c.reset))
            self.echo("    %s %s" % (self._status_word(check.status), check.label))
            if check.detail and (check.status != "OK"):
                for line in check.detail.splitlines():
                    self.echo("          %s%s%s" % (self.c.dim, line, self.c.reset))
        self.echo()

    # --------------------------- Tests ---------------------------

    def function_line(self, suite, results):
        bad = [r for r in results if r.is_bad]
        total = len(results)
        word = "ft_" + suite + " "
        dots = "." * max(2, 34 - len(word))
        passed = total - len(bad)
        if bad:
            score = "%s%3d/%-3d KO%s" % (self.c.red, passed, total, self.c.reset)
        else:
            score = "%s%3d/%-3d ok%s" % (self.c.green, passed, total, self.c.reset)
        self.echo("  %s%s%s%s %s" % (word, self.c.dim, dots, self.c.reset, score))
        if bad and all(r.status == "MISSING" for r in results):
            self.echo("    %s %s" % (self._status_word("MISSING"), results[0].detail))
            return
        for result in results:
            if result.is_bad or self.verbose:
                self.test_line(result)

    def test_line(self, result):
        self.echo("    %s %s" % (self._status_word(result.status),
                                 result.name.split("/", 1)[1]))
        if result.detail and result.status != "PASS":
            for line in result.detail.splitlines()[:6]:
                self.echo("          %s%s%s" % (self.c.dim, line, self.c.reset))

    def tests(self, results):
        by_suite = []
        for result in results:
            if not by_suite or by_suite[-1][0] != result.suite:
                by_suite.append((result.suite, []))
            by_suite[-1][1].append(result)
        part = None
        for suite, group in by_suite:
            if suite == "info":
                continue
            current = _PART_OF_SUITE.get(suite, 0)
            if current != part:
                if part is not None:
                    self.echo()
                part = current
                self.section(PART_TITLES.get(current, "AUTRES TESTS"))
            self.function_line(suite, group)
        if part is not None:
            self.echo()
        info = [r for r in results if r.suite == "info"]
        if info:
            self.section("CAS INDÉFINIS, à titre d'information (non comptés)")
            for result in info:
                word = result.name.split("/", 1)[1] + " "
                dots = "." * max(2, 34 - len(word))
                self.echo("  %s%s%s%s %s" % (word, self.c.dim, dots, self.c.reset,
                                             result.detail.splitlines()[0]
                                             if result.detail else ""))
            self.echo()

    # --------------------------- Résumé --------------------------

    def summary(self, conformity, results):
        c = self.c
        self.section("RÉSUMÉ")
        ko = warn = counted = 0
        if conformity is not None:
            ko = sum(1 for x in conformity.checks if x.status == "KO")
            warn = sum(1 for x in conformity.checks if x.status == "WARN")
            counted = sum(1 for x in conformity.checks if x.status in ("OK", "KO"))
            color = c.red if ko else c.green
            self.echo("  Conformité : %s%d/%d%s%s" % (
                color, counted - ko, counted, c.reset,
                "  (%d avertissement(s))" % warn if warn else ""))
        scored = [r for r in results if r.suite != "info"]
        bad = [r for r in scored if r.is_bad]
        if scored:
            color = c.red if bad else c.green
            self.echo("  Tests      : %s%d/%d%s" % (color, len(scored) - len(bad), len(scored),
                                                    c.reset))
        by_kind = {}
        for result in bad:
            by_kind[result.status] = by_kind.get(result.status, 0) + 1
        if by_kind:
            self.echo("  Détail     : " + ", ".join(
                "%d %s" % (n, kind) for kind, n in sorted(by_kind.items())))
            self.echo("  %sRejouer un test : --run suite/nom, ou --gdb suite/nom pour le "
                      "déboguer%s" % (c.dim, c.reset))
        self.echo()

    def error(self, title, detail=""):
        self.echo("%s%s%s" % (self.c.red, title, self.c.reset))
        for line in detail.strip().splitlines():
            self.echo("  " + line)


def write_log(out_dir, reporter):
    """Écrit le rapport sans couleurs et retourne son chemin."""
    out_dir = Path(out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)
    stamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S_%f")
    log = out_dir / ("libfterator_%s.log" % stamp)
    clean = re.compile(r"\033\[[0-9;]*m")
    lines = [clean.sub("", line) for line in reporter.log_lines]
    log.write_text("\n".join(lines) + "\n", encoding="utf-8")
    return log


def to_json(path, conformity, results, seed):
    data = {
        "seed": seed,
        "conformity": [{"key": c.key, "status": c.status, "detail": c.detail}
                       for c in (conformity.checks if conformity else [])],
        "tests": [{"name": r.name, "status": r.status, "detail": r.detail} for r in results],
    }
    Path(path).write_text(json.dumps(data, ensure_ascii=False, indent=1), encoding="utf-8")
