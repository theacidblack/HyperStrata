#!/usr/bin/env python3
"""Write this project's name, HyperStrata, where the text of a PolyStrata tree says PolyStrata.

HyperStrata is a fork of PolyStrata (https://github.com/VecSzn/PolyStrata), which builds on
Strata (https://github.com/Niko1221/Strata). Run this after taking changes from PolyStrata:

    python tools/hyperstrata_names.py            # rewrite the files
    python tools/hyperstrata_names.py --check    # list what it would change, change nothing

It replaces the word PolyStrata in the files a user reads or runs (documents, setup, the server, the web app, the
tools and their tests) and nothing else; a second run changes nothing. It leaves alone the engines' sources and
build files, the benchmark records, the paper, docs/FAQ.md (it is about several projects), UPSTREAM_DIVERGENCE.md
(the derivation registry), tools/polystrata_names.py (history; superseded, not rewritten), and the places where
PolyStrata means the project this one is built on (KEEP). To name that project in new text, link it,
[PolyStrata](https://github.com/VecSzn/PolyStrata), or write "the PolyStrata project". Functional names
(PolyStrata-data, PolyStrata-main, hyper.exe, STRATA_* flags, `strata-<model>.json` stems, lowercase
`hyper.py`) are never matched: only the capital word PolyStrata is renamed.
"""
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SKIP = ("src/", "include/", "tests/", "cmake/", "third_party/", "bench/", "sycl/src/", "sycl/tools/", "tools/vision/",
        "docs/paper/", "docs/COMMUNITY_BENCHMARKS.md", "docs/FAQ.md", "UPSTREAM_DIVERGENCE.md",
        "tools/polystrata_names.py", "tools/hyperstrata_names.py")
KINDS = (".md", ".py", ".html", ".js", ".css", ".bat", ".sh", ".yml", ".yaml")
NAMES = ("Dockerfile",)
KEEP = "|".join((
    r"\[PolyStrata\]\(https://github\.com/VecSzn/PolyStrata", # a Markdown link to it
    r"VecSzn/PolyStrata\">PolyStrata",                        # an HTML link to it
    r"VecSzn/PolyStrata",                                     # their repository, in an address or by name
    r"<you>/PolyStrata",                                      # a fork-URL template in a comment
    r"[Tt]he\s+PolyStrata project|PolyStrata maintainers",
    r"PolyStrata-Paper",                                      # mirror of the paper-file guard
    r"PolyStrata-data",                                       # the model-files folder (a path, Wave B owns it)
    r"PolyStrata-main",                                       # a zip-unpacked folder (a path)
))
WORD = re.compile(f"{KEEP}|(?<![A-Za-z0-9_])(PolyStrata)(?![A-Za-z0-9_])")


def rename(text: str) -> str:
    return WORD.sub(lambda m: "HyperStrata" if m.group(1) else m.group(0), text)


def files() -> list:
    out = subprocess.run(["git", "ls-files"], cwd=ROOT, capture_output=True, text=True, check=True).stdout
    return [f for f in out.splitlines()
            if not f.startswith(SKIP) and (f.endswith(KINDS) or f in NAMES)]


def main() -> int:
    check = "--check" in sys.argv[1:]
    changed = 0
    for f in files():
        path = ROOT / f
        try:
            with open(path, encoding="utf-8", newline="") as fh:
                old = fh.read()
        except (OSError, UnicodeDecodeError) as e:
            print(f"skipped {f}: {e}", file=sys.stderr)
            continue
        new = rename(old)
        if new == old:
            continue
        changed += 1
        print(f)
        if not check:
            with open(path, "w", encoding="utf-8", newline="") as fh:
                fh.write(new)
    print(f"{changed} files " + ("would change" if check else "changed"), file=sys.stderr)
    return 1 if check and changed else 0


def demo() -> None:
    same = ("https://github.com/VecSzn/PolyStrata/releases", "[PolyStrata](https://github.com/VecSzn/PolyStrata) by",
            "VecSzn/PolyStrata", "PolyStrata-data", "Downloads\\PolyStrata-main\\PolyStrata-main",
            "https://github.com/<you>/PolyStrata/releases/latest/download/",
            "polystrata", "POLYSTRATA", "HyperStrata", "strata-poly.exe",
            "The PolyStrata project's author", "the\nPolyStrata project's 0.1.44",
            '<a href="https://github.com/VecSzn/PolyStrata">PolyStrata</a>')
    for s in same:
        assert rename(s) == s, s
    assert rename("PolyStrata's folder, PolyStrata-data and PolyStrata.") == \
        "HyperStrata's folder, PolyStrata-data and HyperStrata."
    assert rename(rename("run PolyStrata")) == "run HyperStrata"


if __name__ == "__main__":
    demo()
    sys.exit(main())
