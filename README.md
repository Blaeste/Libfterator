# Libfterator v2

Testeur pour la libft de 42, à jour du **sujet v19.3**.

```bash
git clone -b v2 https://github.com/Blaeste/libfterator.git
cd libfterator
./libfterator /chemin/vers/ta/libft
```

Rien à installer si `cc`, `make` et Python 3 sont présents. Ton projet n'est jamais modifié : tout se passe sur une copie.

Le testeur compile une seule fois puis lance **499 tests comptés** et 40 cas d'information. Un run complet avec tous les contrôles du sujet prend environ 5 secondes.

## Ce qui est vérifié

**Le sujet** (`CONFORMITÉ`) :

| Groupe | Contrôles |
|---|---|
| Fichiers | `Makefile`, `libft.h` et `ft_*.c` à la racine, aucun fichier inutile, chaque `ft_*.c` compilé |
| README | première ligne en italique avec les logins, sections Description, Instructions, Resources, usage de l'IA, description de la bibliothèque |
| Header | prototypes exacts (dont `unsigned int ft_lstsize`), `t_list`, garde d'inclusion, pas de `restrict` |
| Makefile | `NAME`, règles, `cc`, `-Wall -Wextra -Werror`, `ar`, pas de `libtool` ni `-std=c99`, pas de relink, effets de `clean`, `fclean`, `re` |
| Fonctions | les 43 présentes avec `make` seul, helpers `static`, pas de globales, fonctions externes autorisées fonction par fonction |
| Norme | norminette, si elle est installée |

Chaque contrôle donne `OK` ou `KO`. `WARN` (point à regarder) et `SKIP` (contrôle impossible) ne comptent pas dans le score.

**Les fonctions** (parties 1, 2 et 3). Chaque test contrôle automatiquement :

- **les fuites**, sur tous les tests et sans valgrind ;
- **les débordements** à l'octet près, grâce à une page protégée juste après chaque bloc : un octet de trop provoque un arrêt immédiat ;
- **le terminateur oublié** : la mémoire de `malloc` est remplie d'un motif non nul, donc un `\0` manquant se voit à tous les coups ;
- **les double free, les `free` invalides et les accès après un `free`** ;
- **les paramètres `const`** : les chaînes d'entrée sont en lecture seule dans les tests concernés.

Les fonctions qui allouent sont aussi testées quand **chaque `malloc` échoue à tour de rôle** : le retour doit être `NULL`, sans fuite ni crash.

## Lire le résultat

```
ft_strlcat .......................  13/15  KO
  OOB   dst_not_terminated_within_size
        lecture 1 octet(s) après la fin d'un buffer de 5 octet(s)
```

| Statut | Signification |
|---|---|
| `FAIL` | mauvais résultat, avec la valeur obtenue et la valeur attendue |
| `CRASH` | le test a planté, par exemple un segfault |
| `TIMEOUT` | le test a dépassé la limite (boucle infinie probable) |
| `LEAK` | de la mémoire n'a pas été libérée |
| `MEMERR` | double free, `free` invalide ou écriture avant le début d'un bloc |
| `OOB` | lecture ou écriture hors limites, ou écriture dans un paramètre `const` |
| `MISSING` | fonction absente de `libft.a` |
| `VALGRIND` | erreur vue par valgrind (avec `--valgrind`) |
| `INTERNAL` | le testeur lui-même n'a pas pu continuer, par exemple sans mémoire système |

Pour rejouer un test, ou l'ouvrir dans gdb :

```bash
./libfterator ../libft --run strlcat/dst_not_terminated
./libfterator ../libft --gdb strlcat/dst_not_terminated_within_size
```

Sous gdb, tape `run`. L'exécution s'arrête sur l'assertion qui échoue.

## Cas indéfinis

Le sujet ne définit pas `ft_strlen(NULL)`, `ft_substr(NULL, 0, 5)` ni un `ft_atoi` qui dépasse un `int`. Ces cas apparaissent dans une section à part qui montre ce que fait ta libft, **sans compter dans le score** : une libft protégée et une libft qui ne l'est pas obtiennent la même note.

```
CAS INDÉFINIS, à titre d'information (non comptés)
  strlen_null ...................... SIGSEGV
  substr_null ...................... retourne NULL
```

## Options

| Option | Effet |
|---|---|
| `--run MOTIF` | seulement les tests dont le nom contient `MOTIF`, sans les contrôles du sujet |
| `--list` | liste les tests |
| `--verbose` | affiche chaque test |
| `--no-color` | sans couleurs |
| `--timeout S` | limite par test en secondes, 5 par défaut |
| `--seed N` | graine des tests aléatoires, 42 par défaut |
| `--no-conformity` | saute les contrôles du sujet |
| `--no-norm` | saute la norminette |
| `--valgrind` | relance chaque test sous valgrind (environ une minute sur 12 cœurs) |
| `--asan` | recompile la libft avec AddressSanitizer et UBSan |
| `--gdb NOM` | ouvre un test dans gdb |
| `--json FICHIER` | écrit les résultats en JSON |

Le code de sortie vaut `0` si tout passe, `1` s'il y a un KO et `2` en cas de mauvaise utilisation. Chaque run écrit un rapport dans `out/`.

## Pré-requis

- Linux ou WSL : l'interception de `malloc` repose sur `ld --wrap`, absent de macOS
- `cc`, `make`, `ar`, `nm`, Python 3 (testé avec 3.10 et 3.13), clang ou gcc (testé avec clang 12 et gcc 10)
- En option : norminette, valgrind, gdb

## Ajouter un test

Les tests sont de vrais fichiers C dans `tests/`, et tout nouveau `.c` de ce dossier est compilé automatiquement. Le moteur est détaillé dans `engine/lft_test.h`.

```c
TEST(strlcpy, truncate)
{
	char	*dest = lft_guard_buf(6);	// un octet de trop provoque un OOB

	ASSERT_EQ(ft_strlcpy(dest, "Hello World", 6), 11);
	ASSERT_STR(dest, "Hello");
}
```

Le premier argument de `TEST` est le nom de la fonction sans `ft_`. Pour tester les échecs de `malloc` :

```c
TEST(strdup, malloc_failure_returns_null)
{
	MALLOC_FAIL_LOOP
	{
		char	*r = ft_strdup("hello");

		if (lft_mf_injected())
			ASSERT_NULL(r);
		else
			free(r);
	}
}
```

Pour lancer les tests du testeur lui-même :

```bash
python3 -m unittest discover -s checks
```

La correspondance entre les 329 tests de la v1 et ceux de la v2 est dans [docs/PORTAGE.md](docs/PORTAGE.md).
