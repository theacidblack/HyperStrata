# Changelog — HyperStrata

HyperStrata runs Qwen3.6-35B-A3B (MoE) on one hardware tier: 8 GB VRAM + 32 GB RAM PCs.

The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).
Fork changes only; upstream history is the git log. Each entry ships in the
private dev repo first and reaches the public `main` only on releases.

Version scheme is `v<upstream>.<serial>`: the first numbers name the upstream
release this state includes (e.g. `0.1.44.1`), the last number is our own
release counter. The serial never resets, so it can grow past 1000 across
upstream syncs (`v0.1.44.1.1`, …, `v0.1.45.87`, …). A higher serial is always
newer, whatever the base. Exception: `v0.1.45` below predates the scheme —
read it as "aligned to upstream `0.1.44.1`, serial 0".

## [Unreleased]

### Verified

- UD full-ladder decode verified: 40/40 files load. Every probed file uses
  already-supported type ids — no new decode kernels needed for the ladder.
  Method: header probe (`ud_probe.py` + `ud_inventory.json`).
- MTP delta is wired: 20 draft tensors per MTP file
  (`7×F32 + 8×Q8_0 + 2–3×recipe + 2×BF16`) covered by `load_drafter`,
  `block_kv`, and the draft-KV knob.

### In progress

- Fast-path gaps (correct today, slower than symmetric paths where noted):
  - Grouped expert kernels missing for Q2_K / Q3_K (single-MMVQ only).
  - Q4_1 wiring to land (decode + fixture + parity row).
  - Cross-product oracle test in build: 228 cells
    (19 weights × 3 KV × target/draft × decode/batch; integer tiers exact,
    IQ tiers epsilon; synthetic, no model file). Status: PENDING.
- Asymmetric expert pairs stay correct but slower than symmetric
  (Q5_1, Q4_K, IQ4_NL, IQ2_XXS/XS, IQ3_XXS/S, IQ2_S, IQ1_M). PENDING fast paths.

### Docs

- `docs/KV_CACHE.md` full-context figure is under correction (queued):
  it states q4_0 ≈ 2.95 GiB at 131072 context, but only 10 QSA + 1 draft
  layers hold KV, which measures ≈ 0.91 GiB. Corrected figure: PENDING
  the doc fix. Per-row arithmetic below is unaffected.

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
  the default (both omitted) is byte-identical to before. Upstream strata-poly
  has neither; see `UPSTREAM_DIVERGENCE.md` (DIV-002) and `docs/KV_CACHE.md`.
- `docs/KV_CACHE.md`: formats, knobs, gates, measured numbers.
- `examples/qwen36-q4_0/`: placeholder config + launcher showing the q4_0 run
  shape with no hardcoded paths, and a weights note.
- Web app About page: per-engine-neutral `Q4_0` label; the Draft KV row hides
  when the knob is fp16.

### Measured

- q4-001 (live, `--kv q4_0 --draft-kv q4_0`): 90.9 max / 46.3 low / ~55.6 avg
  tok/s over 48 turns; prefill 17,277 tokens in ~13 s; thinking cap hit once
  and wrapped; zero errors. Reference rig: RTX 3070 Ti Laptop 8 GB +
  32 GB DDR5, ctx 131072, temp 0.6, thinking cap 4096.
  No speed claim — capacity result (288 B/row: 9/32 of fp16, 9/17 of q8_0;
  ≈1.9 GiB at ~78K context vs ≈6.7 fp16 / ≈3.6 q8_0). Full-context A/B
  against fp16/q8_0: PENDING.
- `poly_kv_q4_test`: PASS (112,010 values — key and value readers vs the ggml
  oracle, byte layout, ring, attend/attend_batch).
- `poly_kv_q8_test`: PASS (91,530 values). `poly_attend_test`: all passed.
- `poly_draft_race_test`: PASS (7 repeated runs match, 5/5 knob pairs,
  save/load rows bit-exact 6/6, no acceptance drop over 5pp on any pair).

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
  byte-identical; `--kv` / prompt-batch handling unchanged.
- Frozen baseline kept at `archive/PolyStrata_original`; no further syncs,
  clean break per the 8 GB plan.
