# Changelog — HyperStrata

Fork changes only; upstream history is the git log. Each entry ships in the
private dev repo first and reaches the public `main` only on releases.

## Unreleased (toward v0.1.45 — first public change)

### Added
- q4_0 KV cache for the `strata-poly` / qwen35moe engine plus an independent
  `--draft-kv` knob for the draft block (each `fp16|q8_0|q4_0`, mixable;
  omitted is fp16; default byte-identical). Upstream strata-poly has neither.
  See `UPSTREAM_DIVERGENCE.md` (DIV-002) and `docs/KV_CACHE.md`.
- `docs/KV_CACHE.md`: formats, knobs, gates, measured numbers.
- `examples/qwen36-q4_0/`: placeholder config + launcher showing the q4_0 run
  shape with no hardcoded paths, and a weights note.
- Web app About page: per-engine-neutral `Q4_0` label; the Draft KV row hides
  when the knob is fp16.

### Measured
- q4-001 (live, `--kv q4_0 --draft-kv q4_0`): 90.9 max / 46.3 low / ~55.6 avg
  tok/s, 48 turns; prefill 17,277 tokens in ~13 s; thinking cap hit once and
  wrapped; zero errors. No speed claim — capacity result (288 B/row: 28.1% of
  fp16, 52.9% of q8_0; ≈1.9 GiB at ~78K context vs ≈6.7 fp16 / ≈3.6 q8_0).
- `poly_kv_q4_test`: PASS (112,010 values — key and value readers vs the ggml
  oracle, byte layout, ring, attend/attend_batch).
- `poly_kv_q8_test`: PASS (91,530 values). `poly_attend_test`: all passed.
- `poly_draft_race_test`: PASS (7 repeated runs match, 5/5 knob pairs,
  save/load rows bit-exact 6/6, no acceptance drop over 5pp on any pair).

### Merged
- Upstream `main` at `f72a15e9` (batch slots to 8, Qwen3.6 arch file, MXFP4
  for DeepSeek-V4 only, docs). Default solo path byte-identical; `--kv` /
  prompt-batch handling unchanged. Only divergence: DIV-001 (docs) + DIV-002.

### Fixed (audit findings, pre-release)
- Web app mislabeled poly's q4_0 as the main engine's Hadamard variant.
- Geometry guard covered `head_dim` (scales plane assumed it already).
- Q4 key reader had no oracle arm; draft_race gated exact prefix/tool match
  that legitimate draft quantization cannot pass (reported, not gated, now).
