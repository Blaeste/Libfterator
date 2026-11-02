"""Contrôles statiques du sujet : fichiers, Makefile, symboles, header, README, norme."""
import os
import re
import shutil
import subprocess
import time
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

from . import subject, symbols

OK = "OK"
KO = "KO"
WARN = "WARN"
SKIP = "SKIP"

_COMPILER_SYMBOLS = re.compile(
    r"^(_GLOBAL_OFFSET_TABLE_|__stack_chk_fail|__stack_chk_guard|_Unwind_Resume"
    r"|__x86\.get_pc_thunk\..*|__asan_.*|__ubsan_.*|__lsan_.*|__sanitizer_.*|_DYNAMIC)$")
# Un compilateur optimisant peut remplacer une boucle par l'un de ces appels.
_BUILTIN_REPLACEMENTS = {"memset", "memcpy", "memmove", "memcmp", "strlen", "bzero", "strchr"}
_COMPILER_WORD = re.compile(
    r"(?:^|[\s;&|@(])((?:\S*/)?(?:cc|gcc|clang|tcc|g\+\+|c\+\+)(?:-\d+(?:\.\d+)*)?)\s")
_AR_WORD = re.compile(r"(?:^|[\s;&|@(])(?:\S*/)?ar\s+[-a-zA-Z]*[rqs]")
_REQUIRED_FLAGS = ("-Wall", "-Wextra", "-Werror")
_ITALIC_LINE = re.compile(
    r"^(\*|_)This project has been created as part of the 42 curriculum by "
    r"(?P<logins>[^<>*_]+?)\.?\1\.?\s*$")
_AI_WORDS = re.compile(
    r"\b(AI|IA|LLM|ChatGPT|GPT|Claude|Copilot|Gemini)\b|artificial intelligence"
    r"|intelligence artificielle", re.IGNORECASE)
_ANSI = re.compile(r"\033\[[0-9;]*m")
_LIBRARY_HEADING = re.compile(r"librar|librair|biblioth|function|fonction", re.IGNORECASE)


class Check:
    def __init__(self, key, group, label, status, detail=""):
        self.key = key
        self.group = group
        self.label = label
        self.status = status
        self.detail = detail

    def __repr__(self):
        return "Check(%s, %s, %r)" % (self.key, self.status, self.detail)


class Result:
    def __init__(self):
        self.checks = []
        self.archive_ok = False
        self.defined = set()
        self.used_bonus_rule = False
        self.optimized = False
        self.build_output = ""

    def add(self, *args):
        self.checks.append(Check(*args))
        return self.checks[-1]


def _tail(text, lines=12):
    return "\n".join(text.strip().splitlines()[-lines:])


# ------------------------------- Fichiers -------------------------------

def check_files(ws, result):
    group = "Fichiers"
    src = ws.source
    problems = []
    extra = []
    bonus = []
    for name in ("Makefile", "libft.h"):
        if not (src / name).is_file():
            problems.append("%s absent de la racine" % name)
    for path in sorted(src.iterdir()):
        name = path.name
        if path.is_dir():
            if name == ".git":
                continue
            sources = [p for p in path.rglob("*") if p.suffix in (".c", ".h")]
            if sources:
                problems.append("%s/ contient des sources (tout doit être à la racine)" % name)
            elif not name.startswith("."):
                extra.append(name + "/")
        elif name.endswith(".c") and not name.startswith("ft_"):
            problems.append("%s : seuls les ft_*.c sont demandés" % name)
        elif name.endswith(".h") and name != "libft.h":
            problems.append("%s : seul libft.h est demandé" % name)
        elif name.endswith(("_bonus.c", "_bonus.h")):
            bonus.append(name)
        elif not (name in ("Makefile", "libft.h", "README.md", ".gitignore")
                  or name.endswith((".c", ".o", ".a"))):
            extra.append(name)
    if bonus:
        extra.append("suffixe _bonus inutile, les listes sont obligatoires : "
                     + ", ".join(bonus))
    result.add("files.layout", group, "Makefile, libft.h et ft_*.c à la racine",
               KO if problems else OK, "\n".join(problems))
    result.add("files.extra", group, "Aucun fichier non demandé",
               WARN if extra else OK, "\n".join(extra))


def check_unused_sources(ws, result):
    members = symbols.members(ws.archive, ws.env)
    if members is None:
        return
    built = {Path(m).stem for m in members}
    unused = sorted(p.name for p in ws.source.glob("ft_*.c") if p.stem not in built)
    result.add("files.unused", "Fichiers", "Chaque ft_*.c est compilé dans libft.a",
               KO if unused else OK, ", ".join(unused))


# ------------------------------- Makefile -------------------------------

def _makefile_code(ws):
    path = ws.lib / "Makefile"
    if not path.is_file():
        return ""
    lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
    return "\n".join(line for line in lines if not line.lstrip().startswith("#"))


def _archive_stamp(ws):
    def stamp(path):
        return path.stat().st_mtime_ns if path.exists() else None
    return stamp(ws.archive), {p.name: stamp(p) for p in ws.objects()}


def _check_rules(ws, code, result):
    missing = []
    if not re.search(r"^\s*NAME\s*[:?+]?=\s*libft\.a\s*$", code, re.MULTILINE):
        missing.append("NAME = libft.a")
    for rule in ("all", "libft.a", "clean", "fclean", "re"):
        if ws.make("-n", rule).code != 0:
            missing.append("$(NAME)" if rule == "libft.a" else rule)
    result.add("make.rules", "Makefile", "NAME et règles $(NAME), all, clean, fclean, re",
               KO if missing else OK, "manquant : " + ", ".join(missing) if missing else "")


def _check_dry_run(ws, code, result):
    group = "Makefile"
    dry = ws.make("-n", "all").output
    compile_lines = [line for line in dry.splitlines() if re.search(r"\s-c(\s|$)", line)]
    compilers = set()
    missing_flags = set()
    for line in compile_lines:
        match = _COMPILER_WORD.search(line)
        if match:
            compilers.add(os.path.basename(match.group(1)))
        for flag in _REQUIRED_FLAGS:
            if not re.search(r"(^|\s)%s(\s|$)" % re.escape(flag), line):
                missing_flags.add(flag)
        if re.search(r"(^|\s)-O([1-9sfgz]|fast)\b", line):
            result.optimized = True
    if not compile_lines:
        result.add("make.cc", group, "Compilation avec cc", SKIP,
                   "aucune ligne de compilation lisible avec make -n")
        result.add("make.flags", group, "Flags -Wall -Wextra -Werror", SKIP,
                   "aucune ligne de compilation lisible avec make -n")
    else:
        wrong = sorted(c for c in compilers if c != "cc")
        result.add("make.cc", group, "Compilation avec cc", KO if wrong else OK,
                   "le sujet impose cc, le Makefile utilise " + ", ".join(wrong) if wrong else "")
        result.add("make.flags", group, "Flags -Wall -Wextra -Werror",
                   KO if missing_flags else OK,
                   "absent : " + ", ".join(sorted(missing_flags)) if missing_flags else "")
    forbidden = [word for word in ("-std=c99", "libtool") if word in dry or word in code]
    result.add("make.forbidden", group, "Pas de libtool ni de -std=c99",
               KO if forbidden else OK, ", ".join(forbidden))
    has_ar = any(_AR_WORD.search(line) for line in dry.splitlines())
    result.add("make.ar", group, "Archive créée avec ar", OK if has_ar else SKIP,
               "" if has_ar else "aucun appel à ar visible avec make -n")


def _check_relink(ws, result):
    before = _archive_stamp(ws)
    ws.make()
    ws.make("all")
    after = _archive_stamp(ws)
    relinked = before != after
    result.add("make.relink", "Makefile", "Pas de relink inutile", KO if relinked else OK,
               "un second make reconstruit des fichiers alors que rien n'a changé"
               if relinked else "")


def _check_rebuild(ws, result):
    sources = sorted(ws.lib.glob("ft_*.c"))
    if not sources:
        return
    before = _archive_stamp(ws)[0]
    time.sleep(0.02)
    os.utime(str(sources[0]))
    ws.make()
    rebuilt = _archive_stamp(ws)[0] != before
    result.add("make.rebuild", "Makefile", "Modifier un .c reconstruit libft.a",
               OK if rebuilt else KO,
               "" if rebuilt else "%s modifié, libft.a inchangé" % sources[0].name)


def _check_re(ws, result):
    problem = ""
    before = _archive_stamp(ws)[0]
    time.sleep(0.02)
    if ws.make("re").code != 0:
        problem = "make re échoue"
    elif not ws.archive.exists():
        problem = "libft.a absent après make re"
    elif _archive_stamp(ws)[0] == before:
        problem = "make re n'a pas reconstruit libft.a"
    result.add("make.re", "Makefile", "re reconstruit tout", KO if problem else OK, problem)


def _check_clean(ws, result):
    problems = []
    if not ws.objects():
        ws.make()
    if ws.make("clean").code != 0:
        problems.append("make clean échoue")
    left = [p.name for p in ws.objects()]
    if left:
        problems.append("%d fichier(s) .o restant(s), ex. %s" % (len(left), left[0]))
    if not ws.archive.exists():
        problems.append("libft.a ne doit pas être supprimé par clean")
    if ws.make("clean").code != 0:
        problems.append("un second make clean échoue")
    result.add("make.clean", "Makefile", "clean supprime les .o et garde libft.a",
               KO if problems else OK, "\n".join(problems))


def _check_fclean(ws, result):
    """Vérifié sur une archive de côté et un faux .o, pour éviter un build complet."""
    problems = []
    saved = ws.root / "libft.a.saved"
    if ws.archive.exists():
        shutil.copy2(str(ws.archive), str(saved))
    else:
        ws.make()
    sources = sorted(ws.lib.glob("ft_*.c"))
    if sources:
        sources[0].with_suffix(".o").write_bytes(b"")
    if ws.make("fclean").code != 0:
        problems.append("make fclean échoue")
    if ws.archive.exists():
        problems.append("libft.a toujours présent")
    if ws.objects():
        problems.append("fichiers .o toujours présents")
    if ws.make("fclean").code != 0:
        problems.append("un second make fclean échoue")
    result.add("make.fclean", "Makefile", "fclean supprime libft.a et les .o",
               KO if problems else OK, "\n".join(problems))
    for path in ws.objects():
        path.unlink()
    if saved.exists():
        os.replace(str(saved), str(ws.archive))


def check_makefile(ws, result):
    group = "Makefile"
    code = _makefile_code(ws)
    if not code:
        result.add("make.rules", group, "Makefile présent", KO, "Makefile absent")
        return
    _check_rules(ws, code, result)
    _check_dry_run(ws, code, result)
    build = ws.make()
    result.build_output = build.output
    if build.code != 0 or not ws.archive.is_file():
        reason = _tail(build.output) if build.code != 0 else "libft.a absent après make"
        result.add("make.build", group, "make construit libft.a à la racine", KO, reason)
        return
    if symbols.members(ws.archive, ws.env) is None:
        result.add("make.build", group, "make construit libft.a à la racine", KO,
                   "libft.a n'est pas une archive")
        return
    result.add("make.build", group, "make construit libft.a à la racine", OK)
    _check_relink(ws, result)
    _check_rebuild(ws, result)
    _check_re(ws, result)
    _check_clean(ws, result)
    _check_fclean(ws, result)
    if not ws.archive.is_file():
        ws.make("fclean")
        ws.make()
    result.archive_ok = ws.archive.is_file()


def quick_build(ws, result):
    """Construit libft.a sans les contrôles du Makefile (--run, --no-conformity)."""
    build = ws.make()
    result.build_output = build.output
    result.archive_ok = build.code == 0 and ws.archive.is_file()
    if not result.archive_ok:
        reason = _tail(build.output) if build.code != 0 else "libft.a absent après make"
        result.add("make.build", "Makefile", "make construit libft.a à la racine", KO, reason)


# ------------------------------- Symboles -------------------------------

def _externals_table(syms):
    per_member = {}
    for sym in syms:
        entry = per_member.setdefault(sym.member, {"defines": set(), "externals": set(),
                                                   "calls": set()})
        if sym.kind in ("T", "W"):
            entry["defines"].add(sym.name)
        elif sym.kind == "U" and not _COMPILER_SYMBOLS.match(sym.name):
            if sym.name.startswith("ft_"):
                entry["calls"].add(sym.name)
            else:
                entry["externals"].add(sym.name)
    return per_member


def _allowed_for(defines):
    known = [subject.BY_NAME[d] for d in defines if d in subject.BY_NAME]
    if not known:
        return {"malloc", "free", "write"}
    allowed = set()
    for function in known:
        allowed.update(function.externals)
    return allowed


def check_symbols(ws, result):
    group = "Fonctions"
    syms = symbols.read_symbols(ws.archive, ws.env)
    result.defined = symbols.defined_functions(syms)
    missing = [n for n in subject.NAMES if n not in result.defined]
    if missing and ws.make("-n", "bonus").code == 0:
        if ws.make("bonus").code == 0:
            result.used_bonus_rule = True
            syms = symbols.read_symbols(ws.archive, ws.env)
            result.defined = symbols.defined_functions(syms)
    detail = ""
    if missing:
        detail = "absentes de make : " + ", ".join(missing)
        if result.used_bonus_rule:
            detail += "\nle sujet v19.3 les rend obligatoires, make bonus ne compte plus"
    result.add("functions.required", group, "Les 43 fonctions sont dans libft.a avec make",
               KO if missing else OK, detail)

    helpers = sorted(result.defined - set(subject.NAMES))
    result.add("functions.static", group, "Helpers déclarés static", KO if helpers else OK,
               "non static : " + ", ".join(helpers) if helpers else "")

    global_vars = sorted({s.name for s in syms if s.kind in "DBCGSRdbgsr"
                          and not s.name.startswith((".", "__"))
                          and "." not in s.name})
    local_static = sorted({s.name for s in syms if s.kind in "dbgsr"
                           and not s.name.startswith((".", "__")) and "." in s.name})
    if global_vars:
        result.add("functions.globals", group, "Pas de variable globale", KO,
                   ", ".join(global_vars))
    elif local_static:
        result.add("functions.globals", group, "Pas de variable globale", WARN,
                   "variables static locales : " + ", ".join(local_static))
    else:
        result.add("functions.globals", group, "Pas de variable globale", OK)

    table = _externals_table(syms)
    ko_lines = []
    warn_lines = []
    for member in sorted(table):
        entry = table[member]
        forbidden = entry["externals"] - _allowed_for(entry["defines"])
        real = sorted(forbidden - _BUILTIN_REPLACEMENTS) if result.optimized else sorted(forbidden)
        soft = sorted(forbidden & _BUILTIN_REPLACEMENTS) if result.optimized else []
        names = ", ".join(sorted(entry["defines"])) or member
        if real:
            ko_lines.append("%s utilise %s" % (names, ", ".join(real)))
        if soft:
            warn_lines.append("%s appelle %s (remplacement possible par -O)" %
                              (names, ", ".join(soft)))
    status = KO if ko_lines else (WARN if warn_lines else OK)
    result.add("functions.externals", group, "Fonctions externes autorisées par le sujet",
               status, "\n".join(ko_lines + warn_lines))

    result.add(*_indirect_check(table))


def _indirect_check(table):
    owners = {}
    for entry in table.values():
        for name in entry["defines"]:
            owners[name] = entry
    lines = []
    for function in subject.FUNCTIONS:
        entry = owners.get(function.name)
        if entry is None:
            continue
        reached = set()
        seen = set()
        todo = list(entry["calls"])
        while todo:
            name = todo.pop()
            if name in seen or name not in owners:
                continue
            seen.add(name)
            reached.update(owners[name]["externals"])
            todo.extend(owners[name]["calls"])
        indirect = reached - set(function.externals) - entry["externals"]
        if indirect:
            lines.append("%s utilise %s via %s" % (function.name, ", ".join(sorted(indirect)),
                                                   ", ".join(sorted(seen))))
    return ("functions.indirect", "Fonctions", "Pas de fonction externe indirecte",
            WARN if lines else OK, "\n".join(lines))


# -------------------------------- Header --------------------------------

def _compile_snippet(code, lib, cc):
    proc = subprocess.run([cc, "-x", "c", "-fsyntax-only", "-Wall", "-Wextra", "-Werror",
                           "-I", str(lib), "-"], input=code, stdout=subprocess.PIPE,
                          stderr=subprocess.STDOUT, universal_newlines=True)
    return proc.returncode == 0, proc.stdout


def _check_prototype(args):
    function, lib, cc = args
    code = ('#include <stddef.h>\n#include "libft.h"\n'
            "_Static_assert(__builtin_types_compatible_p(__typeof__(%s), %s), \"x\");\n"
            % (function.name, function.ctype))
    good, output = _compile_snippet(code, lib, cc)
    if good:
        return None
    if re.search(r"undeclared|implicit", output):
        return "%s : non déclarée dans libft.h" % function.name
    return "%s : le prototype du sujet est %s" % (function.name, function.ctype)


def check_header(ws, result, cc="cc"):
    group = "Header"
    if not (ws.lib / "libft.h").is_file():
        result.add("header.prototypes", group, "Prototypes conformes au sujet", KO,
                   "libft.h absent")
        return
    with ThreadPoolExecutor(max_workers=os.cpu_count() or 2) as pool:
        lines = [line for line in pool.map(_check_prototype,
                                           [(f, ws.lib, cc) for f in subject.FUNCTIONS]) if line]
    result.add("header.prototypes", group, "Prototypes conformes au sujet",
               KO if lines else OK, "\n".join(lines))

    struct = ('#include <stddef.h>\n#include "libft.h"\n'
              "_Static_assert(__builtin_types_compatible_p("
              "__typeof__(((t_list *)0)->content), void *), \"content\");\n"
              "_Static_assert(__builtin_types_compatible_p("
              "__typeof__(((t_list *)0)->next), struct s_list *), \"next\");\n"
              "_Static_assert(sizeof(t_list) == 2 * sizeof(void *), \"taille\");\n")
    good, _ = _compile_snippet(struct, ws.lib, cc)
    result.add("header.t_list", group, "Structure t_list du sujet", OK if good else KO,
               "" if good else "attendu : void *content et struct s_list *next, rien d'autre")

    good, _ = _compile_snippet('#include <stddef.h>\n#include "libft.h"\n#include "libft.h"\n',
                               ws.lib, cc)
    result.add("header.guard", group, "Header protégé contre la double inclusion",
               OK if good else KO, "" if good else "inclure libft.h deux fois ne compile pas")

    alone, output = _compile_snippet('#include "libft.h"\n', ws.lib, cc)
    result.add("header.standalone", group, "libft.h compile seul", OK if alone else WARN,
               "" if alone else _tail(output, 4))

    used = []
    for path in sorted(ws.source.glob("*")):
        if path.suffix in (".c", ".h") and path.is_file():
            text = path.read_text(encoding="utf-8", errors="replace")
            text = re.sub(r"/\*.*?\*/", "", text, flags=re.DOTALL)
            text = re.sub(r"//[^\n]*", "", text)
            if re.search(r"\b(restrict|__restrict)\b", text):
                used.append(path.name)
    result.add("header.restrict", group, "Pas de restrict dans les prototypes",
               KO if used else OK, ", ".join(used))


# -------------------------------- README --------------------------------

def _readme_sections(text):
    """Retourne [(titre, contenu)] pour les titres # et les titres soulignés."""
    sections = []
    title = None
    body = []
    lines = text.splitlines()
    for i, line in enumerate(lines):
        match = re.match(r"^#{1,6}\s*(.+?)\s*#*\s*$", line)
        underlined = (i + 1 < len(lines) and line.strip()
                      and re.match(r"^(=+|-+)\s*$", lines[i + 1]))
        if match or underlined:
            if title is not None:
                sections.append((title, "\n".join(body)))
            title = match.group(1) if match else line.strip()
            body = []
        else:
            body.append(line)
    if title is not None:
        sections.append((title, "\n".join(body)))
    return sections


def check_readme(ws, result):
    group = "README"
    path = ws.source / "README.md"
    if not path.is_file():
        result.add("readme.exists", group, "README.md à la racine", KO, "README.md absent")
        return
    result.add("readme.exists", group, "README.md à la racine", OK)
    text = path.read_text(encoding="utf-8", errors="replace")
    first = text.splitlines()[0] if text.strip() else ""
    good = bool(_ITALIC_LINE.match(first))
    result.add("readme.first_line", group, "Première ligne en italique avec les logins",
               OK if good else KO,
               "" if good else "attendu : *This project has been created as part of the "
               "42 curriculum by <login>.*")
    sections = _readme_sections(text)
    titles = [title for title, _ in sections]
    wanted = (("Description", r"description"), ("Instructions", r"instructions?"),
              ("Resources", r"res+ources?"))
    absent = [name for name, pattern in wanted
              if not any(re.search(pattern, t, re.IGNORECASE) for t in titles)]
    result.add("readme.sections", group, "Sections Description, Instructions, Resources",
               KO if absent else OK, "absent : " + ", ".join(absent) if absent else "")
    resources = "\n".join(body for title, body in sections
                          if re.search(r"res+ources?", title, re.IGNORECASE))
    mentions_ai = bool(_AI_WORDS.search(resources))
    result.add("readme.ai", group, "Usage de l'IA décrit dans Resources",
               OK if mentions_ai else WARN,
               "" if mentions_ai else "le sujet demande pour quelles tâches l'IA a servi")
    library = (any(_LIBRARY_HEADING.search(t) for t in titles)
               or len(re.findall(r"\bft_\w+", text)) >= 3)
    result.add("readme.library", group, "Description détaillée de la bibliothèque",
               OK if library else WARN,
               "" if library else "le sujet demande une description détaillée de la bibliothèque")


# ------------------------------- Norminette ------------------------------

def check_norminette(lib, command=None, timeout=180):
    label = "Norminette sur ft_*.c et libft.h"
    if command is None:
        command = shutil.which("norminette") or ""
    if not command:
        return Check("norm", "Norme", label, SKIP, "norminette introuvable")
    lib = Path(lib)
    files = sorted(p.name for p in lib.iterdir() if p.suffix in (".c", ".h") and p.is_file())
    if not files:
        return Check("norm", "Norme", label, SKIP, "aucun fichier à vérifier")
    try:
        proc = subprocess.run([command] + files, cwd=str(lib), stdout=subprocess.PIPE,
                              stderr=subprocess.STDOUT, timeout=timeout)
    except (OSError, subprocess.TimeoutExpired) as err:
        return Check("norm", "Norme", label, SKIP, "lancement impossible : %s" % err)
    output = _ANSI.sub("", proc.stdout.decode("utf-8", "replace"))
    if proc.returncode == 0:
        return Check("norm", "Norme", label, OK)
    errors = [line for line in output.splitlines()
              if line.strip() and not line.endswith(": OK!")
              and not line.startswith("Setting locale")]
    return Check("norm", "Norme", label, KO, _tail("\n".join(errors), 15))


# --------------------------------- Tout ----------------------------------

def run_all(ws, norminette=True, cc="cc"):
    result = Result()
    norm_future = None
    pool = None
    if norminette:
        pool = ThreadPoolExecutor(max_workers=1)
        norm_future = pool.submit(check_norminette, ws.source)
    check_files(ws, result)
    check_readme(ws, result)
    check_header(ws, result, cc)
    check_makefile(ws, result)
    if result.archive_ok:
        check_symbols(ws, result)
        check_unused_sources(ws, result)
    if norm_future is not None:
        result.checks.append(norm_future.result())
        pool.shutdown()
    order = ("Fichiers", "README", "Header", "Makefile", "Fonctions", "Norme")
    result.checks.sort(key=lambda check: order.index(check.group))
    return result


def build_only(ws):
    """Construit libft.a sans contrôle, pour un lancement ciblé."""
    result = Result()
    quick_build(ws, result)
    if result.archive_ok:
        try:
            syms = symbols.read_symbols(ws.archive, ws.env)
        except symbols.NmError as err:
            result.add("make.build", "Makefile", "libft.a lisible", KO, str(err))
            result.archive_ok = False
            return result
        result.defined = symbols.defined_functions(syms)
        missing = [n for n in subject.NAMES if n not in result.defined]
        if missing and ws.make("-n", "bonus").code == 0 and ws.make("bonus").code == 0:
            result.used_bonus_rule = True
            result.defined = symbols.defined_functions(
                symbols.read_symbols(ws.archive, ws.env))
    return result
