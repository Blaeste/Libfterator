"""La langue des messages : le français par défaut, l'anglais avec --lang en.

Chaque texte est écrit dans les deux langues là où il sert : tr("Fichiers", "Files").
Le moteur C fait de même avec TR() (engine/lft_test.h)."""

LANGS = ("fr", "en")
_current = "fr"


def set_lang(code):
    global _current
    _current = code if code in LANGS else "fr"


def lang():
    return _current


def tr(fr, en):
    return en if _current == "en" else fr
