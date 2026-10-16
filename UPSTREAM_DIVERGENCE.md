# Upstream Divergence Registry — internal

Tracks every substantial difference between this fork (`theacidblack/HyperStrata`)
and upstream (`VecSzn/PolyStrata`). Internal document: it lives on `dev` for now
and reaches `preview`/`main` only through the normal promotion pipeline.

It must always answer, per divergence: what changed, why, what upstream code it
touches, whether it is still necessary, whether upstream now does it better, what
breaks if removed, and how to validate removal. Unknowns are marked, never guessed.

## Comparison baseline (established 2026-10-16)

- Upstream: `https://github.com/VecSzn/PolyStrata`, tracked branch `upstream/main`.
- Upstream HEAD audited: `51dc2427` ("docs: a user's GLM read of 1M tokens on
  poly.5, and the cold first run", 2026-10-15).
- Our branches at audit: `origin/main`, `origin/dev`, `origin/preview` — all
  `51dc2427`, all clean worktrees.
- Method: `git merge-base origin/main upstream/main` equals both heads;
  `git log upstream/main..origin/main` and `git log origin/main..upstream/main`
  are both empty. Full parity: zero fork-only commits, zero pending upstream commits.
- Limitation: history before this fork's creation was not separately audited; the
  fork was cut directly at `51dc2427`, so there is no older hidden divergence.

## Registry

### Baseline entries

## DIV-001 — Repository identity and README

- **Category:** other (docs/identity, no engine behavior change)
- **Status:** active
- **Location:** `README.md` (English only; the six translated READMEs still describe upstream and lag — see README top note)
- **Origin:** fork design decision 2026-10-16 (identity task)
- **Reason:** the fork needs a public identity that states what it is (personal
  hardware-optimization experiment on fixed hardware) without claiming upstream
  authorship, endorsement, or universal gains.
- **Behavioral difference:** documentation only. Engine code, server, installer,
  and in-code `PolyStrata` naming are untouched; no runtime behavior differs.
- **Hardware relevance:** no code effect; documents the 8 GB VRAM / 32 GB RAM target.
- **Evidence:** `git diff upstream/main -- README.md`; `LICENSE` byte-identical to upstream.
- **Dependencies:** none
- **Upstream status:** no equivalent (upstream describes the general engine, correctly)
- **Maintenance cost:** low — English README must be kept true as experiments land;
  translations lag by design until content stabilizes.
- **Removal criteria:** never (a fork always needs its own identity statement);
  content evolves with results.
- **Validation procedure:** re-read against the identity task's definition of done;
  confirm no authorship/endorsement/universal-gain claims and no invented benchmarks.

Use the next free `DIV-00N` id for each new entry. One entry per coherent problem,
not one per edit. Trivial edits (formatting, lockfile noise, generated files) get
no entry.

### Entry template

```markdown
## DIV-XXX — <short title>

- **Category:** hardware-optimization | feature | architecture | bug-fix | compatibility | tooling | other
- **Status:** experimental | active | needs-review | upstream-equivalent | deprecated | removed
- **Location:** files, modules, config keys, symbols (paths relative to repo root)
- **Origin:** our commit/PR; upstream commit if related; otherwise "fork design decision <date>"
- **Reason:** the concrete problem this solves (one paragraph, with numbers where measurable)
- **Behavioral difference:** what we do differently from upstream (observable behavior, not just code shape)
- **Hardware relevance:** yes (8 GB VRAM / 32 GB RAM) with measured numbers | no
- **Evidence:** tests, benchmark runs (model/quant/context/workload, same-day baseline), or "unknown"
- **Dependencies:** other DIV ids relying on this, or "none known"
- **Upstream status:** no equivalent | partial equivalent (<ref>) | equivalent (<ref>) | unknown
- **Maintenance cost:** known conflicts/rebases caused, or "unknown yet"
- **Removal criteria:** conditions under which this can be deleted
- **Validation procedure:** tests + benchmarks to run before changing or removing it
```

## Classification

- **Fork-only / modified-upstream / fork-only-feature:** our registry entries.
- **Upstream-only:** upstream has it, we don't yet — track as sync work, not as a DIV entry.
- **Upstream-equivalent:** upstream now covers it — verify, then deprecate/remove ours through
  a reviewed change with regression tests, never silently.
- **Unknown:** evidence insufficient — mark and investigate, do not classify by filename similarity.
  Different code is not automatically different behavior, and identical filenames are not
  automatically identical behavior.

## Update rules

Update this file in the same PR that adds, modifies, or removes fork-specific behavior —
especially hardware tuning, memory management, scheduling, model loading, or anything
overriding upstream. Every relevant PR names affected DIV ids or states explicitly that
no registry change is needed. Status changes (`needs-review`, `upstream-equivalent`,
`deprecated`) from sync audits land here too.

## Sync audit procedure (repeatable)

Run from any worktree root (`main/`, `dev/`, or `preview/`):

```
git fetch upstream --prune
git rev-parse upstream/main
git log --oneline upstream/<last-baseline>..upstream/main   # what upstream did since
git log --oneline upstream/main..HEAD                       # what we did since (should be DIV-linked)
git merge-base HEAD upstream/main
git diff --name-status <merge-base> HEAD                    # our changed files
```

Then per changed area: check registry coverage, flag semantic conflicts even in
unchanged files (behavior/dependency check, not just textual diff), update affected
entries' upstream status + evidence, and record the new baseline commit at the top of
this file. `tools/upstream_divergence_audit.py` automates the reporting half; its
output is review input, not proof.

## Automation

`tools/upstream_divergence_audit.py` (stdlib only, git CLI) reports: baseline vs local
commits, fork-only/upstream-only counts, our changed files vs merge-base, registered
DIV ids + statuses, and changed files not mentioned in the registry (review candidates).
It never deletes code, resolves conflicts, rewrites history, or changes statuses.
