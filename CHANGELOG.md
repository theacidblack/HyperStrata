← [Prev](--help.md) · [Index](docs/INDEX.md) · [Next](CONTRIBUTING.md) →

# Changelog — HyperStrata
Keywords: Qwen3.6-35B-A3B MoE GGUF, Unsloth UD quantization, q4_0 KV cache, 8 GB VRAM Windows CUDA local inference.

HyperStrata runs Qwen3.6-35B-A3B (MoE) on one hardware tier: 8 GB VRAM + 32 GB RAM PCs.

The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).
Fork changes only; upstream history is the git log. Each entry ships in the
private dev repo first and reaches the public `main` only on releases.

Version scheme is `v<upstream>.<serial>`: the first numbers name the upstream
release this state includes (e.g. `0.1.44.1`), the last number is our own
release counter. The serial never resets, so it can grow past 1000 across
upstream syncs (`v0.1.44.1.1`, …, `v0.1.44.1.87`, …). A higher serial is always
newer, whatever the base. First public release will be `v1.0.0` (flag-plant, contains q4_0).
Exception: `v0.1.45` below predates the scheme and stays PARKED, do not use as base —
read it as "aligned to upstream `0.1.44.1`, serial 0".

## [Unreleased]

> [!NOTE]
> Lab notebook: every number carries provenance. REPORTED outranks nothing.

### Changed

- Project identity moves from PolyStrata-fork wording to Hyper-identity
  (Wave A1). Fork history and upstream URLs preserved. No functional change
  in this wave.

### Fixed (honesty/claim audit wave, 2026-10-18 — no engine change)

- README q6-001 row was stale ("no run yet") — now MEASURED (55.8/13.5/~35.5,
  159 turns, chat-paste provenance, console archive PENDING); llama ref kept
  labeled NOT-our-run. README states current stage: interactive learning of
  repo structure, needs first — no optimization, no new models yet.
- q4-001 retagged REPORTED (not measured) in `docs/KV_CACHE.md`,
  `docs/BENCHMARKS.md`, and here; cap-hit/zero-error claims UNVERIFIED
  (no log on file); at-run-context GiB figures (≈1.8/1.9/≈6.7/≈3.6)
  withdrawn to PENDING — full-context 2.75/1.46/0.77 GiB arithmetic verifies.
- `docs/BENCHMARKS.md` sections repaired (measured rows to Results,
  Not-yet-run holds Q4-MTP+Vision / Cyber-Tiel / other-Vision PENDING).
- `docs/COVERAGE.md` sterile refs rewritten to committed paths or tagged
  workspace-only/unverifiable.
- Inherited upstream promises narrowed to house stance (8 GB VRAM + 32 GB
  RAM tested/supported; wider RTX = goal not promise): `docs/AI_SETUP.md`,
  `docs/MODELS.md`, `docs/BACKENDS.md`, `docs/FAQ.md`,
  `docs/COMMUNITY_BENCHMARKS.md` (llama numbers now qualified
  reported-NOT-re-verified), `tools/make_poly_release.py` docstring.
- Consistency: v1.0.0 documented as first-public flag-plant; `v0.1.45.87`
  example rebased off non-PARKED `v0.1.44.1.N`; issue templates follow
  issues-DISABLED stance; README upstream line states sync frozen at
  `0.1.44.1`; `cd PolyStrata` → `cd HyperStrata` in MAC/STRIX_HALO docs.

### Verified

- UD full-ladder decode present in code for all 44 files (both pages):
  Header probe 2026-10-17 (workspace-only, artifacts not in repo) found no unsupported type id in the 44 probed files. Decode-kernel need PENDING the 228-cell oracle + live runs; all non-q4-001 files code-complete-unmeasured.
  Hardware runs after our changes: q4-001 (UD-Q4_K_M + q4_0 KV) only — REPORTED, no log, PENDING verification.
  All other files: loader accepts file per 2026-10-17 probe; decode/fast-path pinned only by PENDING 228-cell oracle + PENDING live runs.
- MTP delta is wired: 20 draft tensors per MTP file
  (`7×F32 + 8×Q8_0 + 2–3×recipe + 2×BF16`) covered by `load_drafter`,
  `block_kv`, and the draft-KV knob.

### In progress

- Fast-path gaps (no known wrong-answer failure via oracle arms on post-f72a15e9 rebuild; slower where noted, PENDING re-test after any engine edit):
  - Grouped expert kernels missing for Q2_K / Q3_K (single-MMVQ only).
  - Q4_1 wiring to land (decode + fixture + parity row).
  - Cross-product oracle test in build: 228 cells
    (19 weights × 3 KV × target/draft × decode/batch; integer tiers exact,
    IQ tiers epsilon; synthetic, no model file). Status: PENDING.
- Asymmetric expert pairs: no wrong-answer failure observed via oracle arms; slower than symmetric — PENDING 228-cell oracle.
  (Q5_1, Q4_K, IQ4_NL, IQ2_XXS/XS, IQ3_XXS/S, IQ2_S, IQ1_M). PENDING fast paths.

### Docs

- `docs/KV_CACHE.md` figures corrected 2026-10-18: full-context arithmetic
  fp16 ≈2.75 / q8_0 ≈1.46 / q4_0 ≈0.77 GiB kept; rig reservation ≈0.91 GiB
  measured, 0.91-vs-0.77 delta unexplained. At-run-context GiB figures
  withdrawn to PENDING (no q4-001 log on file).

### Planned

- Vision Phase 6 (after the text ladder is green): 9-step scoped job covering
  the KV-quant × vision matrix for Qwen3.6 (`mmproj-F16.gguf`); GLM/DeepSeek
  stay text-only by design. Status: planned, no numbers yet.

## [v0.1.44.1.1] — 2026-10-17 (canonical q4_0 tag)

Same engine, tests, numbers, and docs as `v0.1.45` below, plus the version
scheme itself (`v<upstream>.<serial>`, serial never resets).

### Added

- q4_0 KV cache for the `strata-poly` / qwen35moe engine, plus an independent
  `--draft-kv` knob for the draft block. Each knob takes `fp16 | q8_0 | q4_0`,
  mixable (e.g. `--kv q8_0 --draft-kv q4_0`); a knob left out is `fp16`;
     the default (both omitted) is byte-identical to before per poly_kv_q8_test 12/12 short-prompt check + review trace post-f72a15e9; re-run poly_kv_q8_test + poly_attend_test after any engine edit. Upstream strata-poly
  has neither; see `UPSTREAM_DIVERGENCE.md` (DIV-002) and `docs/KV_CACHE.md`.
- `docs/KV_CACHE.md`: formats, knobs, gates, measured numbers.
- `examples/qwen36-q4_0/`: placeholder config + launcher showing the q4_0 run
  shape with no hardcoded paths, and a weights note.
- Web app About page: per-engine-neutral `Q4_0` label; the Draft KV row hides
  when the knob is fp16.

### Measured

- `poly_kv_q4_test`: PASS (112,010 values — key and value readers vs the ggml
  oracle, byte layout, ring, attend/attend_batch).
- `poly_kv_q8_test`: PASS (91,530 values). `poly_attend_test`: all passed.
- `poly_draft_race_test`: PASS (7 repeated runs match, 5/5 knob pairs,
  save/load rows bit-exact 6/6, no acceptance drop over 5pp on any pair) on post-f72a15e9 rebuild sm_86. Full suite not green: ple_parity needs 20 GB Q2_0 fixture by design.

### Reported (not log-verified)

- q4-001 (live, `--kv q4_0 --draft-kv q4_0`): 90.9 max / 46.3 low / ~55.6 avg
  tok/s over 48 turns; prefill 17,277 tokens in ~13 s — all REPORTED, no log
  on file. Thinking-cap and zero-error claims UNVERIFIED. Reference rig:
  RTX 3070 Ti Laptop 8 GB + 32 GB DDR5, ctx 131072, temp 0.6, thinking cap
  4096. No speed claim — capacity result (288 B/row: 9/32 of fp16, 9/17 of
  q8_0; at-run-context GiB figures PENDING recomputation, no log — the
  ≈1.9 / ≈6.7 / ≈3.6 GiB figures are withdrawn 2026-10-18).
  Full-context A/B against fp16/q8_0: PENDING.

### Fixed (audit findings, pre-release)

- Web app mislabeled poly's q4_0 as the main engine's Hadamard variant.
- Geometry guard covered `head_dim` (scales plane assumed it already).
- Q4 key reader had no oracle arm; draft_race gated an exact prefix/tool match
  that legitimate draft quantization cannot pass (reported, not gated, now).

## [v0.1.45] — 2026-10-17 (superseded pre-scheme first push)

First public push, aligned to upstream `0.1.44.1`; superseded by `v0.1.44.1.1`
above, which is the canonical q4_0 tag.

## Versus upstream `0.1.44.1`

### What we changed (fork-only)

- q4_0 KV cache for qwen35moe (ggml-true `quantize_row_q4_0_ref`
  transcription, per-block formats in alloc/save/load/attend).
- Independent `--draft-kv` knob (`setup.py` writer/reader/carry included).
- Web app label/UX: neutral `Q4_0` label, Draft KV row hidden when fp16.
- Guards: per-knob qwen35moe-only and CUDA-only gates, block-multiple
  shape errors naming the offending knob.
- Example pack: `examples/qwen36-q4_0/` run shape.
- Docs: `docs/KV_CACHE.md` (new), DIV-002 in `UPSTREAM_DIVERGENCE.md`.

### What we picked from upstream (unchanged)

- Everything else at upstream `main` `f72a15e9` (batch slots to 8, Qwen3.6
  arch file, MXFP4 for DeepSeek-V4 only, docs). Default solo path
  byte-identical per poly_kv_q8_test 12/12 short-prompt check + review trace post-f72a15e9; re-run poly_kv_q8_test + poly_attend_test after any engine edit; `--kv` / prompt-batch handling unchanged.
- Frozen baseline kept at upstream commit 51dc2427 (baseline lives in git history); no further syncs,
  clean break per the 8 GB plan.

← [Prev](--help.md) · [Index](docs/INDEX.md) · [Next](CONTRIBUTING.md) →
