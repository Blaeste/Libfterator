"""--stream : une ligne JSON par événement sur la sortie, au fil du run.

Pour un affichage en direct dans un autre outil (42pal) : start, step
(subject, build, tests), check, plan (le nombre de tests), test, error. Le
rapport habituel ne va plus sur la sortie (il reste dans out/), et le
fichier de --json reste la référence à la fin.
"""
import json
import threading

_out = None
_lock = threading.Lock()


def enable(out):
    global _out
    _out = out


def enabled():
    return _out is not None


def emit(event, **fields):
    if _out is None:
        return
    with _lock:
        _out.write(json.dumps(dict(event=event, **fields), ensure_ascii=False) + "\n")
        _out.flush()


def check(c):
    emit("check", key=c.key, group=c.group, label=c.label, status=c.status, detail=c.detail)


def test(result, prefix=""):
    emit("test", name=result.name, status=result.status, detail=result.detail.replace(prefix, ""),
         ms=result.ms)
