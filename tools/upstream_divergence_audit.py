#!/usr/bin/env python3
"""Upstream divergence audit (report only, never modifies anything).

Compares this checkout against the upstream baseline and cross-checks
UPSTREAM_DIVERGENCE.md. Output is review input, not proof of equivalence.

    python tools/upstream_divergence_audit.py [--upstream upstream/main]
"""
from __future__ import annotations

import argparse
import re
import subprocess
import sys
from pathlib import Path

DIV_RE = re.compile(r"^##\s+(DIV-\d+)\s+[—-]\s*(.+)$", re.M)
STATUS_RE = re.compile(r"^\s*-\s*\*\*Status:\*\*\s*(.+?)\s*$", re.M)


def git(*args: str, cwd: Path) -> str:
    out = subprocess.run(["git", *args], cwd=cwd, capture_output=True, text=True)
    if out.returncode != 0:
        raise SystemExit(f"git {' '.join(args)} failed: {out.stderr.strip()}")
    return out.stdout.strip()


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--upstream", default="upstream/main",
                    help="upstream tracking branch to compare against")
    args = ap.parse_args()

    root = Path(git("rev-parse", "--show-toplevel", cwd=Path.cwd()))
    branch = git("rev-parse", "--abbrev-ref", "HEAD", cwd=root)
    head = git("rev-parse", "HEAD", cwd=root)
    try:
        upstream_head = git("rev-parse", args.upstream, cwd=root)
    except SystemExit:
        print(f"unknown upstream ref {args.upstream!r} (did you `git fetch upstream`?)")
        return 2
    base = git("merge-base", "HEAD", args.upstream, cwd=root)

    fork_only = git("log", "--oneline", f"{args.upstream}..HEAD", cwd=root)
    upstream_only = git("log", "--oneline", f"HEAD..{args.upstream}", cwd=root)
    changed = git("diff", "--name-status", f"{base}", "HEAD", cwd=root)

    print(f"root:            {root}")
    print(f"branch:          {branch} @ {head[:12]}")
    print(f"upstream:        {args.upstream} @ {upstream_head[:12]}")
    print(f"merge-base:      {base[:12]}")
    print(f"fork-only:       {len(fork_only.splitlines()) if fork_only else 0} commits")
    for line in fork_only.splitlines()[:20]:
        print(f"  {line}")
    print(f"upstream-only:   {len(upstream_only.splitlines()) if upstream_only else 0} commits")
    for line in upstream_only.splitlines()[:20]:
        print(f"  {line}")

    reg = root / "UPSTREAM_DIVERGENCE.md"
    body = reg.read_text(encoding="utf-8") if reg.exists() else ""
    ids = DIV_RE.findall(body)
    statuses = dict(zip([i for i, _ in ids], STATUS_RE.findall(body))) if ids else {}
    print(f"registry:        {reg.name} {'found' if body else 'MISSING'} "
          f"({len(ids)} entries)")
    needs_decision = [i for i, s in statuses.items()
                      if s.strip().lower() in ("upstream-equivalent", "deprecated", "needs-review")]
    for i, title in ids:
        flag = "  <-- needs decision" if i in needs_decision else ""
        print(f"  {i} [{statuses.get(i, 'status unknown')}] {title}{flag}")

    if changed:
        mentioned = [p for p in changed.splitlines()
                     if any(tok in body for tok in p.split()[-1].split("/")[-1:])]
        unmentioned = [p for p in changed.splitlines() if p not in mentioned]
        print(f"changed files:   {len(changed.splitlines())} vs merge-base")
        if unmentioned:
            print("review candidates (changed, not named in registry):")
            for line in unmentioned[:30]:
                print(f"  {line}")
    else:
        print("changed files:   0 vs merge-base (at parity)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
