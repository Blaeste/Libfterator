"""Point d'entrée : ./libfterator /chemin/vers/libft [options]."""
import argparse
import os
import shutil
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

from . import build, conformity, project, report

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
    args = parser.parse_args(argv)
    if args.timeout <= 0:
        parser.error("--timeout doit être positif")
    return args


def run_tests(exe, args, extra=(), env=None):
    """Lance le runner et retourne les résultats ligne par ligne."""
    cmd = [str(exe), "--timeout", str(args.timeout), "--seed", str(args.seed)] + list(extra)
    if args.run:
        cmd += ["--run", args.run]
    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, universal_newlines=True,
                            errors="replace", env=env)
    results = []
    for line in proc.stdout:
        parsed = report.parse_line(line)
        if parsed:
            results.append(parsed)
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
           "--error-exitcode=42", str(exe), "--real-malloc", "--no-fork", "--only", name]
    try:
        proc = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                              universal_newlines=True, errors="replace", timeout=timeout)
    except subprocess.TimeoutExpired:
        return report.TestResult("TIMEOUT", name, 0, "valgrind : plus de %d s" % timeout)
    if proc.returncode == 42:
        lines = [line.split("== ", 1)[-1] for line in proc.stdout.splitlines()
                 if line.startswith("==")]
        return report.TestResult("VALGRIND", name, 0, "\n".join(lines[:8]))
    for line in proc.stdout.splitlines():
        parsed = report.parse_line(line)
        if parsed and parsed.name == name:
            return parsed
    return report.TestResult("CRASH", name, 0, "fin anormale sous valgrind (code %d)"
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
            raise build.BuildError("compilation ASan de %s :\n%s" % (source.name, proc.stdout))
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
    rep = report.Reporter(color=False if args.no_color else None, verbose=args.verbose)
    source = Path(args.libft)
    if not source.is_dir():
        rep.error("Dossier introuvable : %s" % source)
        return EXIT_USAGE
    source = source.resolve()
    cc = build.default_cc()
    for tool in ("make", "ar", "nm", cc):
        if shutil.which(tool) is None:
            rep.error("Outil requis introuvable : %s" % tool)
            return EXIT_USAGE
    if args.valgrind and shutil.which("valgrind") is None:
        rep.error("valgrind n'est pas installé")
        return EXIT_USAGE
    if args.gdb and shutil.which("gdb") is None:
        rep.error("gdb n'est pas installé")
        return EXIT_USAGE

    ws = project.Workspace(source, ROOT / "build" / "work", tester_root=ROOT)
    try:
        return _run(args, rep, ws, cc)
    finally:
        ws.cleanup()


def _run(args, rep, ws, cc):
    targeted = bool(args.run or args.list or args.gdb)
    with_conformity = not (args.no_conformity or targeted)
    if not args.list:
        rep.banner(ws.source)
    if with_conformity:
        conf = conformity.run_all(ws, norminette=not args.no_norm, cc=cc)
        rep.conformity(conf)
    else:
        conf = conformity.build_only(ws)
    if not conf.archive_ok:
        rep.error("Impossible de construire libft.a : les tests ne peuvent pas tourner.",
                  conf.build_output[-1500:] if not with_conformity else "")
        _finish(args, rep, conf if with_conformity else None, [])
        return EXIT_KO
    if conf.used_bonus_rule:
        rep.echo("%sLes fonctions absentes de make ont été prises avec make bonus pour "
                 "pouvoir les tester.%s" % (rep.c.yellow, rep.c.reset))
        rep.echo()

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
        rep.error("Le testeur n'a pas pu être construit avec cette libft.", str(err))
        _finish(args, rep, conf if with_conformity else None, [])
        return EXIT_KO

    if args.list:
        for name in list_tests(exe):
            if not args.run or args.run in name:
                print(name)
        return EXIT_OK
    if args.gdb:
        return subprocess.call(["gdb", "-q", "-ex", "break lft_fail", "--args", str(exe),
                                "--no-fork", "--only", args.gdb])

    results = run_valgrind(exe, args) if args.valgrind else run_tests(exe, args, extra, env)
    prefix = str(ws.lib) + "/"
    for result in results:
        result.detail = result.detail.replace(prefix, "")
    rep.tests(results)
    _finish(args, rep, conf if with_conformity else None, results)
    bad_conf = with_conformity and any(c.status == "KO" for c in conf.checks)
    return EXIT_KO if bad_conf or any(r.is_bad for r in results) else EXIT_OK


def _finish(args, rep, conf, results):
    rep.summary(conf, results)
    log = report.write_log(ROOT / "out", rep)
    print("%sLog complet : %s%s" % (rep.c.dim, log, rep.c.reset), file=rep.out)
    if args.json:
        report.to_json(args.json, conf, results, args.seed)
