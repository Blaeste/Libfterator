"""Point d'entrée : ./libfterator /chemin/vers/libft [options]."""
import argparse
import os
import shutil
import signal
import subprocess
import sys
import tempfile
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

from . import __version__, build, conformity, project, report, stream
from .lang import LANGS, lang, set_lang, tr

ROOT = Path(__file__).resolve().parent.parent
EXIT_OK = 0
EXIT_KO = 1
EXIT_USAGE = 2
# Les codes de fin du runner allé au bout (engine/runner.c) : 0 quand chaque test
# passe, 1 quand l'un échoue. Un autre code (2 quand fork ou pipe échoue) ou un
# signal veut dire qu'il s'est arrêté en route.
RUNNER_FINISHED = (0, 1)


def asked_lang(argv):
    """La langue de --lang (ou --lang=), lue avant l'aide pour l'écrire dans cette langue.

    Le dernier --lang compte, comme pour argparse ; une langue inconnue donne le
    français ici, et argparse la refuse ensuite."""
    code = "fr"
    for i, arg in enumerate(argv):
        if arg == "--lang" and i + 1 < len(argv):
            code = argv[i + 1]
        elif arg.startswith("--lang="):
            code = arg.split("=", 1)[1]
    return code if code in LANGS else "fr"


def parse_args(argv):
    set_lang(asked_lang(argv))
    parser = argparse.ArgumentParser(
        prog="libfterator",
        description=tr("Testeur de libft pour le sujet v19.3. Le projet n'est jamais "
                       "modifié : tout se passe sur une copie.",
                       "A tester of the libft for the subject v19.3. The project is never "
                       "changed: everything happens on a copy."))
    parser.add_argument("libft", help=tr("dossier de la libft", "folder of the libft"))
    parser.add_argument("--version", action="version", version="libfterator " + __version__)
    parser.add_argument("--run", metavar=tr("MOTIF", "PATTERN"),
                        help=tr("uniquement les tests dont le nom contient MOTIF",
                                "only the tests whose name holds PATTERN"))
    parser.add_argument("--list", action="store_true",
                        help=tr("liste les tests sans les lancer", "lists the tests without running them"))
    parser.add_argument("--verbose", action="store_true", help=tr("affiche chaque test", "shows each test"))
    parser.add_argument("--no-color", action="store_true", help=tr("désactive les couleurs", "no colors"))
    parser.add_argument("--timeout", type=int, default=5, metavar="S",
                        help=tr("limite par test en secondes (défaut : 5)",
                                "limit per test in seconds (default: 5)"))
    parser.add_argument("--seed", type=int, default=42, metavar="N",
                        help=tr("graine des tests aléatoires (défaut : 42)",
                                "seed of the random tests (default: 42)"))
    parser.add_argument("--no-conformity", action="store_true",
                        help=tr("saute les contrôles du sujet", "skips the checks of the subject"))
    parser.add_argument("--no-norm", action="store_true",
                        help=tr("saute la norminette", "skips the norminette"))
    parser.add_argument("--valgrind", action="store_true",
                        help=tr("relance chaque test sous valgrind", "runs each test again under valgrind"))
    parser.add_argument("--asan", action="store_true",
                        help=tr("recompile la libft avec AddressSanitizer et UBSan",
                                "builds the libft again with AddressSanitizer and UBSan"))
    parser.add_argument("--gdb", metavar=tr("NOM", "NAME"),
                        help=tr("ouvre un test (suite/nom) dans gdb", "opens a test (suite/name) in gdb"))
    parser.add_argument("--json", metavar=tr("FICHIER", "FILE"),
                        help=tr("écrit les résultats en JSON", "writes the results in JSON"))
    parser.add_argument("--lang", choices=LANGS, default="fr",
                        help=tr("langue des messages (défaut : fr)", "language of the messages (default: fr)"))
    parser.add_argument("--stream", action="store_true",
                        help=tr("une ligne JSON par événement sur la sortie, au fil du run",
                                "one JSON line per event on the output, as the run goes"))
    args = parser.parse_args(argv)
    if args.timeout <= 0:
        parser.error(tr("--timeout doit être positif", "--timeout must be positive"))
    return args


def run_tests(exe, args, extra=(), env=None, on_result=None):
    """Lance le runner ; retourne (résultats, code de fin, sa sortie d'erreur).

    on_result : appelé à chaque résultat, au fil du run."""
    cmd = [str(exe), "--timeout", str(args.timeout), "--seed", str(args.seed),
           "--lang", lang()] + list(extra)
    if args.run:
        cmd += ["--run", args.run]
    # Dans un fichier et pas un pipe : le runner n'y est jamais bloqué.
    with tempfile.TemporaryFile() as errors:
        proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=errors,
                                universal_newlines=True, errors="replace", env=env)
        results = []
        for line in proc.stdout:
            parsed = report.parse_line(line)
            if parsed:
                results.append(parsed)
                if on_result is not None:
                    on_result(parsed)
        proc.wait()
        errors.seek(0)
        stderr = errors.read().decode("utf-8", "replace")
    return results, proc.returncode, stderr


def _ending(code):
    """Comment le runner a fini : tué par SIGKILL, code de sortie 2."""
    if code < 0:
        try:
            name = signal.Signals(-code).name
        except ValueError:
            name = "signal %d" % -code
        return tr("tué par %s", "killed by %s") % name
    return tr("code de sortie %d", "exit code %d") % code


def incomplete_run(got, planned, code, stderr=""):
    """L'erreur d'un run qui n'est pas allé au bout, ou None.

    Un résultat par test de la liste et un code de fin normal : le run est
    complet. Sinon les tests sans résultat ne comptent pas, et le run ne doit
    jamais passer pour un succès."""
    if code in RUNNER_FINISHED and got == planned:
        return None
    if got != planned:
        title = tr("Les tests se sont arrêtés avant la fin : %d résultat(s) pour %d test(s).",
                   "The tests stopped before the end: %d result(s) for %d test(s).") % (got, planned)
    else:
        title = tr("Le runner des tests a fini en erreur.", "The test runner ended with an error.")
    lines = [tr("Le runner des tests : %s.", "The test runner: %s.") % _ending(code)]
    lines += stderr.strip().splitlines()[-10:]
    lines.append(tr("Ce run n'est pas un succès : relance-le.", "This run is not a pass: run it again."))
    return {"title": title, "detail": "\n".join(lines)}


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
    # La liste des tests à lancer : chacun doit avoir son résultat à la fin.
    names = [n for n in list_tests(exe) if not args.run or args.run in n]
    if args.valgrind:
        names = [n for n in names if not n.startswith("info/")]
    on_result = None
    if stream.enabled():
        stream.emit("plan", tests=len(names))
        on_result = lambda result: stream.test(result, prefix)  # noqa: E731
    stream.emit("step", step="tests")
    error = None
    if args.valgrind:
        results = run_valgrind(exe, args)
        for result in results if on_result else ():
            on_result(result)
    else:
        results, code, errors = run_tests(exe, args, extra, env, on_result)
        error = incomplete_run(len(results), len(names), code, errors)
    for result in results:
        result.detail = result.detail.replace(prefix, "")
    rep.tests(results)
    if error:
        rep.error(error["title"], error["detail"])
        rep.echo()
        stream.emit("error", title=error["title"], detail=error["detail"])
    _finish(args, rep, conf if with_conformity else None, results, error=error, notes=notes)
    bad_conf = with_conformity and any(c.status == "KO" for c in conf.checks)
    return EXIT_KO if error or bad_conf or any(r.is_bad for r in results) else EXIT_OK


def _finish(args, rep, conf, results, error=None, notes=()):
    rep.summary(conf, results, incomplete=error is not None)
    log = report.write_log(ROOT / "out", rep)
    print(tr("%sLog complet : %s%s",
             "%sFull log: %s%s") % (rep.c.dim, log, rep.c.reset), file=rep.out)
    if args.json:
        report.to_json(args.json, conf, results, args.seed, lang(), log=log, error=error,
                       notes=notes)
