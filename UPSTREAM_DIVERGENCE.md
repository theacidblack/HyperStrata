← [Prev](CHANGELOG.md) · [Index](docs/INDEX.md) →

# Upstream Divergence Registry

*Needs: the changelog · Gives: each fork difference with its reason, evidence, and removal criteria · Next: none (end of chain)*

Tracks every substantial difference between this fork (`theacidblack/HyperStrata`)
and upstream (`VecSzn/PolyStrata`). It ships with the repo so every change can
be checked against its reason.

It must always answer, per divergence: what changed, why, what upstream code it
touches, whether it is still necessary, whether upstream now does it better, what
breaks if removed, and how to validate removal. Unknowns are marked, never guessed.

## Comparison baseline (established 2026-10-16, re-baselined 2026-10-16)

- Upstream: `https://github.com/VecSzn/PolyStrata`, tracked branch `upstream/main`.
- OG upstream: `https://github.com/Niko1221/Strata` — chain Strata → PolyStrata → HyperStrata, no endorsement either way.
- Upstream HEAD audited: `f72a15e9` ("setup, docs: what parallel requests gain on a small card now", 2026-10-16) — 13 commits since the previous baseline, all parallel/batch serving for strata-poly + MXFP4-on-GPU + per-model joint-head support.
- Previous baseline: `51dc2427` (2026-10-15), at full parity (zero fork-only, zero upstream-only).
- Our branches at re-baseline: sync branch `upstream-sync/0.1.44.1-poly.6` merged cleanly (no conflicts; upstream touched 27 engine/setup/docs files, we touched only README/divergence docs — disjoint sets). Audit: `merge-base HEAD upstream/main` = `f72a15e9`, upstream-only = 0 commits.
- Sync verdict 2026-10-16: diff review finds upstream delta additive/opt-in (batch default 0 = solo path; MXFP4 paths cannot engage UD-Q4_K_M weights; `--kv`/`--prompt-batch`/prefill handling). Byte-identity PENDING rebuild + poly_kv_q8_test/poly_attend_test; no DIV entry affected per audit 2026-10-16; no upstream equivalent found at f72a15e9. See "Sync audit 2026-10-16" below.
- Limitation: history before this fork's creation was not separately audited; the
  fork was cut directly at `51dc2427`, so there is no older hidden divergence.

## Sync audit 2026-10-16 (`51dc2427` → `f72a15e9`)

- Upstream did: batch slots up to 8 (`--batch`/`--slots`/`--check-batch`, `"parallel": N` config, `setup.py --parallel` for poly), joint `step()` with per-model one-head rows (`qwen35.cpp` covers our Qwen3.6-35B-A3B arch), MXFP4 decode on-GPU (DeepSeek-V4-only), the poly batching section in its docs + FAQ numbers, extended `test_setup_parallel.py`.
- Single-user 8 GB impact: none — solo path behavior-identical, no default shifts, our fp16/q8_0 baselines stand (numbers belong to the pre-sync binary; re-verify after rebuild).
- Registry impact: DIV-001 unaffected (docs-identity only). No new DIV: upstream's parallel work is `Upstream-only` → sync work, not fork behavior; our q4_0 KV assumptions (`--kv` unused for poly, KV cell bytes, `poly_batch` math) hold verbatim.
- Method: `git fetch upstream --prune`; `git log --oneline 51dc2427..upstream/main` (13 commits); `tools/upstream_divergence_audit.py` confirms merge-base = `f72a15e9`, upstream-only = 0.

## Registry

### Baseline entries

## DIV-001 — Repository identity and README

- **Category:** other (docs/identity, no engine behavior change)
- **Status:** active
- **Location:** `README.md` (English only; the six translated READMEs were removed in `da46e080`, English-only by design until content stabilizes)
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
- **Identity rename note (2026-10-18, uncommitted):** user-facing docs moved to Hyper identity (9 files);
  `hyper.py`, `tools/hyper_*.py`, `hyper.exe` / `engine-hyper/` landed with old names kept as compat
  (`setup.py:748-763`). Old `polystrata.py` / `tools/strata_*.py` still on disk pending deletion review (DIV-003).

## DIV-002 — Independent q4_0 KV-cache knobs for target model and draft block

- **Category:** feature (engine + setup + docs, opt-in, default byte-identical)
- **Status:** active
- **Location:** `src/poly/engine.hpp:41,75` (`KvFmt::kQ4_0`, `Options::draft_kv_format`); `src/poly/engine.cpp` (`Engine::open()` per-knob gates); `src/poly/qwen35.cpp` / `qwen35.hpp` (`block_kv()`, per-block alloc/save/load/attend, `head_dim` guard); `src/poly/main.cpp:597-616,1057-1058,1148-1168` (`kv=`/`draft_kv=` INFO, `--draft-kv` help/parse); `src/poly/kernels.cu` (`store_q4[_at]`, q4 reader/dequant arms); `include/strata/poly/kernels.hpp` (`Kvq4`); `src/poly/kv_q4_test.cpp` (new: round-trip, layout, ring, attend key+value arms); `src/poly/draft_race_test.cpp` (mixed-knob arm); `serve/web/app.js:411-420` (neutral `Q4_0` label, Draft KV row hidden when fp16); `setup.py:767` (`POLY_KV_FORMATS`) + writer/reader/carry; `docs/KV_CACHE.md` (new), the archived GLM notes, `README.md`, `CHANGELOG.md`, `examples/qwen36-q4_0/`
- **Origin:** fork change 2026-10-16/17 (q4_0 KV plan v2; commits `f262e222`, `259a690e`, `f7a8d229`, `a95fe7cf`, `5d73df09`, `66602d44`, `99a16422`, `886f5fd0`, `3edabd82` + audit fixes on top)
- **Reason:** upstream `strata-poly` offers exactly one KV choice (`fp16`, or `q8_0` on qwen35moe) shared by target layers and the draft block. On an 8 GB card the KV cache competes with the expert cache; a q4_0 option at 9/32 of fp16 per row (ggml-true transcription of `quantize_row_q4_0_ref`) plus an independent draft knob lets small/draft traffic run cheap while keeping target precision — mixable per block, both knobs omitted = fp16 exactly as before.
- **Behavioral difference:** two new opt-in flags; per-block formats in alloc/save/load/attend; setup writes/carries them; new docs. No-flag behavior unchanged per review trace + post-f72a15e9 rebuild: `poly_kv_q8_test` PASS 91530 values + `poly_attend_test` all passed; `poly_kv_q4_test` PASS 112010 values incl. store→attend round-trip vs ggml oracle through both the key and value readers; `poly_draft_race_test` PASS with 5/5 knob pairs and bit-exact save/load rows 6/6). Full ctest not green: ple_parity needs Q2_0 fixture by design. Re-run after any engine edit.
- **Hardware relevance:** yes (8 GB VRAM) — REPORTED once live (q4-001, 2026-10-17, REPORTED not log-verified, reference rig ctx 131072 temp 0.6: 90.9 max / 46.3 low / ~55.6 avg tok/s, 48 turns — all REPORTED, no log). At-run-context GiB figures PENDING recomputation (≈1.9 / ≈6.7 / ≈3.6 GiB withdrawn 2026-10-18: not recomputable without the log). Frozen-fixture A/B pending. No Perf-record claimed.
- **Evidence:** `poly_kv_q4_test: PASS (112010 values checked)`; `poly_kv_q8_test: PASS (91530 values)`; `poly_attend_test: all passed`; `poly_draft_race_test: PASS (race 7 runs match, mixed 5/5 pairs, rows round-trip 6/6)`; full test_setup_* battery: 667 PASS (session record — rerun on release day); engine oracle arms as above; q4-001: REPORTED zero errors, raw server log NOT on file.
- **Dependencies:** none
- **Upstream status:** no equivalent in strata-poly (verified at `f72a15e9`: `KvFmt` is `{kFp16, kQ8_0}`, no `--draft-kv`). Upstream's `--kv q4_0` is the main Strata engine's (Hadamard-rotated, different engine, unrelated). Its parallel/batch work at `f72a15e9` is orthogonal and merged.
- **Maintenance cost:** rebase surface is the per-block branches in `qwen35.cpp` + kernel arms; upstream batch-slot `step()` path is format-agnostic (selection is per-block, never per-sequence). Known deferred gap (documented, not silent): cross-format blob loads rely on `row_bytes` inequality + never-cross-processes, with no enforced format tag in `load_rows` — a signature change across four upstream cards, parked for post-release.
- **Removal criteria:** retire only when upstream ships equivalent mixed target/draft KV selection; then delete the shim + note the LEDGER run that re-validated parity.
- **Validation procedure:** rebuild `hyper` from source (CMake target `hyper`, sm_86, TESTS=ON), run `poly_kv_q4_test` + `poly_kv_q8_test` + `poly_attend_test` + mixed `draft_race_test` arm; A/B harness gates before any perf claim.
- **Dual-naming note (2026-10-18, uncommitted):** `--engine` accepts `hyper` (new) and `strata` (compat) (`serve/server.py:7093`); engine log lines tagged `[hyper]` or `[strata]` (`serve/server.py:558`); `/health` `service` still `strata` (`hyper.py:39`); `HYPER_*` env added beside kept `STRATA_*` (`HYPER_ENGINE_STALL_S`, `HYPER_ENGINE_READY_S`, `HYPER_VISION_*`).

Use the next free registry id for each new entry. One entry per coherent problem,
not one per edit. Trivial edits (formatting, lockfile noise, generated files) get
no entry.

## DIV-003 — Hyper rename tooling and compatibility duplication

- **Category:** tooling (docs/identity + binary names, no engine behavior change)
- **Status:** active
- **Location:** `hyper.py` (new) + `polystrata.py` (old, still on disk); `tools/hyper_*.py` (new) + `tools/strata_*.py` (old, still on disk); `tools/polystrata_names.py` (kept as history, never rewritten); `setup.py:744-763` (`POLY_EXE`/`ENGINE_POLY_DIR` = hyper names, `OLD_*` compat); docs titles/commands/paths (rename edits, this registry included)
- **Origin:** fork rename 2026-10-18 (uncommitted)
- **Reason:** user-facing names move to Hyper identity while old installs keep working: new names land first, old names stay as compat, deletion only after review.
- **Behavioral difference:** dual naming everywhere compat matters: `--engine hyper|strata`, `[hyper]`/`[strata]` log tags, `HyperStrata-*`/`hyperstrata` settings + data paths with `PolyStrata-*`/`polystrata` read-once fallback, `hyper.exe` recognized beside `strata-poly.exe`. No-flag behavior unchanged.
- **Hardware relevance:** no
- **Evidence:** `setup.py:748-763,4601-4608,4730-4743,5897,6224,6267`; `serve/server.py:558,7093`; CMake target `hyper` (`CMakeLists.txt:1837`); both CLIs on disk (`hyper.py`, `polystrata.py`)
- **Dependencies:** DIV-001 (identity), DIV-002 (dual-named env/flags)
- **Upstream status:** no equivalent (our rename only; upstream keeps its names)
- **Maintenance cost:** CLI duplication (`hyper.py` vs `polystrata.py`) and tool duplication (`tools/hyper_*` vs `tools/strata_*`) until deletion review; `STRATA_*` functional identifiers frozen until a later rename step
- **Removal criteria:** delete old names only on explicit owner go per rename step, after rebuild + ctest + launcher revalidation; `tools/polystrata_names.py` is never deleted (frozen)
- **Validation procedure:** after any rename edit: `hyper.py --list-models` + one launcher JSON parse + engine-path resolve; full gate (rebuild + ctest + live smoke) before deleting any old name

### Entry template

```markdown
## <id> — <short title>

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

← [Prev](CHANGELOG.md) · [Index](docs/INDEX.md) →
