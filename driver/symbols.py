"""Lecture de libft.a avec nm : symboles définis et appels externes par objet."""
import re
import subprocess
from collections import namedtuple

Symbol = namedtuple("Symbol", "member name kind")

_LINE = re.compile(r"^.*\[(?P<member>[^\]]+)\]:\s+(?P<name>\S+)\s+(?P<kind>[A-Za-z?])\b")


class NmError(Exception):
    pass


def read_symbols(archive, env=None):
    proc = subprocess.run(["nm", "-P", "-A", str(archive)], stdout=subprocess.PIPE,
                          stderr=subprocess.STDOUT, env=env)
    text = proc.stdout.decode("utf-8", "replace")
    if proc.returncode != 0:
        raise NmError(text.strip())
    symbols = []
    for line in text.splitlines():
        match = _LINE.match(line)
        if match:
            symbols.append(Symbol(match.group("member"), match.group("name"),
                                  match.group("kind")))
    return symbols


def defined_functions(symbols):
    return {s.name for s in symbols if s.kind in ("T", "W")}


def members(archive, env=None):
    proc = subprocess.run(["ar", "t", str(archive)], stdout=subprocess.PIPE,
                          stderr=subprocess.DEVNULL, env=env,
                          universal_newlines=True)
    return proc.stdout.split() if proc.returncode == 0 else None
