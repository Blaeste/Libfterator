"""Point d'entrée : ./libfterator /chemin/vers/libft [options]."""
import argparse
import os
import shutil
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

from . import __version__, build, conformity, project, report, stream
from .lang import lang, set_lang, tr

ROOT = Path(__file__).resolve().parent.parent
EXIT_OK = 0
EXIT_KO = 1
EXIT_USAGE = 2


def parse_args(argv):
    parser = argparse.ArgumentParser(
        prog="libfterator",
        description="Testeur de libft pour le sujet v19.3. Le projet n'est jamais "
                    "modifié : tout se passe sur une copie.")
    parser.add_argument("libft", help="dossier de la libft")
    parser.add_argument("--version", action="version", version="libfterator " + __version__)
    parser.add_argument("--run", metavar="MOTIF",
                        help="uniquement les tests dont le nom contient MOTIF")
    parser.add_argument("--list", action="store_true", help="liste les tests sans les lancer")
    parser.add_argument("--verbose", action="store_true", help="affiche chaque test")
    parser.add_argument("--no-color", action="store_true", help="désactive les couleurs")
    parser.add_argument("--timeout", type=int, default=5, metavar="S",
                        help="limite par test en secondes (défaut : 5)")
    parser.add_argument("--seed", type=int, default=42, metavar="N",
                        help="graine des tests aléatoires (défaut : 42)")
    parser.add_argument("--no-conformity", action="store_true",
                        help="saute les contrôles du sujet")
    parser.add_argument("--no-norm", action="store_true", help="saute la norminette")
    parser.add_argument("--valgrind", action="store_true",
                        help="relance chaque test sous valgrind")
    parser.add_argument("--asan", action="store_true",
                        help="recompile la libft avec AddressSanitizer et UBSan")
    parser.add_argument("--gdb", metavar="NOM", help="ouvre un test (suite/nom) dans gdb")
    parser.add_argument("--json", metavar="FICHIER", help="écrit les résultats en JSON")
    parser.add_argument("--lang", choices=("fr", "en"), default="fr",
                        help="langue des messages (défaut : fr)")
    parser.add_argument("--stream", action="store_true",
                        help="une ligne JSON par événement sur la sortie, au fil du run")
    args = parser.parse_args(argv)
    if args.timeout <= 0:
        parser.error("--timeout doit être positif")
    return args


def run_tests(exe, args, extra=(), env=None, on_result=None):
    """Lance le runner et retourne les résultats ligne par ligne (on_result : à chacun, au fil du run)."""
    cmd = [str(exe), "--timeout", str(args.timeout), "--seed", str(args.seed),
           "--lang", lang()] + list(extra)
    if args.run:
        cmd += ["--run", args.run]
    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, universal_newlines=True,
                            errors="replace", env=env)
    results = []
    for line in proc.stdout:
        parsed = report.parse_line(line)
        if parsed:
            results.append(parsed)
            if on_result is not None:
                on_result(parsed)
    proc.wait()
    return results


def list_tests(exe):
    out = subprocess.run([str(exe), "--list"], stdout=subprocess.PIPE,
                         universal_newlines=True).stdout
    return out.split()


def _valgrind_one(job):
    exe, name, timeout = job
    cmd = ["valgrind", "--quiet", "--leak-check=full",
           "--errors-for-leak-kinds=definite,indirect,possible",
           "--error-exitcode=42", str(exe), "--real-malloc", "--no-fork", "--lang", lang(),
           "--only", name]
    try:
        proc = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                              universal_newlines=True, errors="replace", timeout=timeout)
    except subprocess.TimeoutExpired:
        return report.TestResult("TIMEOUT", name, 0, tr("valgrind : plus de %d s",
                                                        "valgrind: more than %d s") % timeout)
    if proc.returncode == 42:
        lines = [line.split("== ", 1)[-1] for line in proc.stdout.splitlines()
                 if line.startswith("==")]
        return report.TestResult("VALGRIND", name, 0, "\n".join(lines[:8]))
    for line in proc.stdout.splitlines():
        parsed = report.parse_line(line)
        if parsed and parsed.name == name:
            return parsed
    return report.TestResult("CRASH", name, 0, tr("fin anormale sous valgrind (code %d)",
                                                  "abnormal end under valgrind (code %d)")
                             % proc.returncode)


def run_valgrind(exe, args):
    names = list_tests(exe)
    if args.run:
        names = [n for n in names if args.run in n]
    names = [n for n in names if not n.startswith("info/")]
    jobs = [(exe, name, max(args.timeout * 12, 60)) for name in names]
    with ThreadPoolExecutor(max_workers=os.cpu_count() or 2) as pool:
        return list(pool.map(_valgrind_one, jobs))


def build_asan_archive(ws, cc):
    flags = ("-g", "-O0", "-w", "-fsanitize=address,undefined",
             "-fno-sanitize-recover=undefined", "-fno-omit-frame-pointer")
    sources = sorted(ws.lib.glob("ft_*.c"))
    objs = []
    for source in sources:
        obj = ws.root / (source.stem + ".asan.o")
        proc = subprocess.run([cc] + list(flags) + ["-I", str(ws.lib), "-c", str(source),
                                                    "-o", str(obj)],
                              stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                              universal_newlines=True)
        if proc.returncode != 0:
            raise build.BuildError(tr("compilation ASan de %s :\n%s",
                                      "ASan compile of %s:\n%s") % (source.name, proc.stdout))
        objs.append(str(obj))
    archive = ws.root / "libft_asan.a"
    subprocess.run(["ar", "rcs", str(archive)] + objs, check=True)
    return archive


def asan_env():
    env = dict(os.environ)
    env["ASAN_OPTIONS"] = ("detect_leaks=0:exitcode=98:allocator_may_return_null=1"
                           ":handle_segv=0:handle_sigbus=0")
    env["UBSAN_OPTIONS"] = "print_stacktrace=0:exitcode=98"
    return env


def main(argv=None):
    args = parse_args(sys.argv[1:] if argv is None else argv)
    set_lang(args.lang)
    quiet = None
    if args.stream:
        # La sortie est aux événements : le rapport ne va que dans out/.
        stream.enable(sys.stdout)
        quiet = open(os.devnull, "w")
        stream.emit("start", tester="libfterator", version=__version__, lang=lang())
    rep = report.Reporter(stream=quiet, color=False if args.no_color else None, verbose=args.verbose)
    source = Path(args.libft)
    if not source.is_dir():
        return _usage_error(args, rep, tr("Dossier introuvable : %s",
                                          "Directory not found: %s") % source)
    source = source.resolve()
    cc = build.default_cc()
    for tool in ("make", "ar", "nm", cc):
        if shutil.which(tool) is None:
            return _usage_error(args, rep, tr("Outil requis introuvable : %s",
                                              "Required tool not found: %s") % tool)
    if args.valgrind and shutil.which("valgrind") is None:
        return _usage_error(args, rep, tr("valgrind n'est pas installé",
                                          "valgrind is not installed"))
    if args.gdb and shutil.which("gdb") is None:
        return _usage_error(args, rep, tr("gdb n'est pas installé", "gdb is not installed"))

    ws = project.Workspace(source, ROOT / "build" / "work", tester_root=ROOT)
    try:
        return _run(args, rep, ws, cc)
    finally:
        ws.cleanup()


def _usage_error(args, rep, title):
    """Une erreur avant tout test : dite, et gardée dans le JSON s'il est demandé."""
    rep.error(title)
    stream.emit("error", title=title, detail="")
    if args.json:
        report.to_json(args.json, None, [], args.seed, lang(),
                       error={"title": title, "detail": ""})
    return EXIT_USAGE


def _run(args, rep, ws, cc):
    targeted = bool(args.run or args.list or args.gdb)
    with_conformity = not (args.no_conformity or targeted)
    if not args.list:
        rep.banner(ws.source)
    if with_conformity:
        stream.emit("step", step="subject")
        conf = conformity.run_all(ws, norminette=not args.no_norm, cc=cc,
                                  on_check=stream.check if stream.enabled() else None)
        rep.conformity(conf)
    else:
        conf = conformity.build_only(ws)
    if not conf.archive_ok:
        title = tr("Impossible de construire libft.a : les tests ne peuvent pas tourner.",
                   "Cannot build libft.a: the tests cannot run.")
        rep.error(title, conf.build_output[-1500:] if not with_conformity else "")
        stream.emit("error", title=title, detail=conf.build_output[-1500:])
        _finish(args, rep, conf if with_conformity else None, [],
                error={"title": title, "detail": conf.build_output[-1500:]})
        return EXIT_KO
    notes = []
    if conf.used_bonus_rule:
        notes.append(tr("Les fonctions absentes de make ont été prises avec make bonus pour "
                        "pouvoir les tester.",
                        "The functions missing with make were taken with make bonus, to test "
                        "them."))
        rep.echo("%s%s%s" % (rep.c.yellow, notes[-1], rep.c.reset))
        rep.echo()

    stream.emit("step", step="build")
    try:
        archive = None
        link_flags = ()
        env = None
        extra = []
        if args.asan:
            archive = build_asan_archive(ws, cc)
            link_flags = ("-fsanitize=address,undefined",)
            env = asan_env()
            extra = ["--real-malloc"]
        exe = build.link_runner(ws, conf.defined, cc=cc, link_flags=link_flags, archive=archive)
    except build.BuildError as err:
        title = tr("Le testeur n'a pas pu être construit avec cette libft.",
                   "The tester could not be built with this libft.")
        rep.error(title, str(err))
        stream.emit("error", title=title, detail=str(err))
        _finish(args, rep, conf if with_conformity else None, [],
                error={"title": title, "detail": str(err)}, notes=notes)
        return EXIT_KO

    if args.list:
        for name in list_tests(exe):
            if not args.run or args.run in name:
                print(name)
        return EXIT_OK
    if args.gdb:
        return subprocess.call(["gdb", "-q", "-ex", "break lft_fail", "--args", str(exe),
                                "--no-fork", "--lang", lang(), "--only", args.gdb])

    prefix = str(ws.lib) + "/"
    on_result = None
    if stream.enabled():
        names = [n for n in list_tests(exe) if not args.run or args.run in n]
        if args.valgrind:
            names = [n for n in names if not n.startswith("info/")]
        stream.emit("plan", tests=len(names))
        on_result = lambda result: stream.test(result, prefix)  # noqa: E731
    stream.emit("step", step="tests")
    if args.valgrind:
        results = run_valgrind(exe, args)
        for result in results if on_result else ():
            on_result(result)
    else:
        results = run_tests(exe, args, extra, env, on_result)
    for result in results:
        result.detail = result.detail.replace(prefix, "")
    rep.tests(results)
    _finish(args, rep, conf if with_conformity else None, results, notes=notes)
    bad_conf = with_conformity and any(c.status == "KO" for c in conf.checks)
    return EXIT_KO if bad_conf or any(r.is_bad for r in results) else EXIT_OK


def _finish(args, rep, conf, results, error=None, notes=()):
    rep.summary(conf, results)
    log = report.write_log(ROOT / "out", rep)
    print(tr("%sLog complet : %s%s",
             "%sFull log: %s%s") % (rep.c.dim, log, rep.c.reset), file=rep.out)
    if args.json:
        report.to_json(args.json, conf, results, args.seed, lang(), log=log, error=error,
                       notes=notes)
