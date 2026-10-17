← [Prev](COVERAGE.md) · [Index](INDEX.md) · [Next](Q8_KV.md) →

*Needs: coverage scope known, defaults running · Gives: KV format knobs, VRAM savings, status · Next: Q8_KV.md*
*Run 2/13*

# The KV cache formats (`hyper --kv`, `--draft-kv`)
q4_0 KV cache cuts Qwen3.6-35B-A3B KV memory to 9/32 of fp16 per row (288 vs 1024 bytes) — what fits on 8 GB VRAM.

`hyper` can store its attention K and V caches in the ggml **Q8_0** or **Q4_0** formats instead of
fp16. There are two independent knobs: `--kv fmt` for the target model and `--draft-kv fmt` for the block
that guesses ahead, each `fp16 | q8_0 | q4_0`, mixable (e.g. `--kv q8_0 --draft-kv q4_0`). A knob left out
is `fp16`. The default (both knobs fp16) is byte-identical to pre-q4_0 build per 12/12 short-prompt check (poly_kv_q8_test + fp16/Q8_0 reply comparison); re-run after any engine edit.

q8_0 is measured: [Q8_KV.md](Q8_KV.md) (single-knob engine). q4_0 is REPORTED once (no server log on file — NOT measured): a live agent run
under `--kv q4_0 --draft-kv q4_0` (q4-001: 90.9 max / 46.3 low / ~55.6 avg tok/s over 48 turns, prefill
17,277 tokens in ~13 s — all REPORTED, unverified; thinking-cap and zero-error claims UNVERIFIED, no log), plus kernel unit tests
(`poly_kv_q4_test`: key and value readers against the ggml oracle, 112,010 values) and a mixed-knob
engine test (`poly_draft_race_test`: PASS, 5/5 pairs, rows round-trip 6/6). A frozen-fixture A/B against
fp16/q8_0 is still pending — figures that need one stay marked **PENDING**.

## The three formats

A cache row holds one token's value for each KV head and each head dimension (`n_kv * head_dim` values).
Stored as fp16 that is `2 * n_kv * head_dim` bytes a token for K and, again, for V.

- **fp16** (default, unchanged): 2 bytes a value, as above.
- **q8_0**: cut into 32-value blocks: for every block of 32 values, one fp16 **scale** (the block's
  largest magnitude over 127) and 32 **int8 codes**. Attention dequantizes a block with its scale on the
  fly while it computes the scores and the outputs. Measured: [Q8_KV.md](Q8_KV.md).
- **q4_0**: cut into 32-value blocks, transcribed from ggml's `quantize_row_q4_0_ref`. The
  block's **scale** is `d = max / -8`, where `max` is the value with the largest magnitude in the block,
  sign kept — so `d` may be negative, and is stored as is (fp16). Each value's **code** is
  `min(15, trunc(x / d + 8.5))`: C truncation toward zero with an upper clamp only and no lower clamp —
  an asymmetric 16-step grid anchored at the block's signed extreme. Values `j` and `j + 16` (`j` < 16)
  share byte `j` as its low and high nibbles (16 packed bytes a block). Reading back is
  `(nibble - 8) * d` with the stored scale as is, so a negative `d` reads correctly. Only this per-block
  arithmetic follows ggml: the planes are the engine's own — all quants in one plane, all fp16 scales in
  another — for the fused readers, not ggml's interleaved `{d, qs[16]}` blocks.

Block arithmetic (definitional, no run needed): a q4_0 block is 16 packed bytes plus one fp16 scale, 18
bytes per 32 values — **9/32 of the fp16 row and 9/17 of the q8_0 row**. On Qwen3.6 (`n_kv * head_dim` =
512: 2 KV heads × key length 256) that is 288 bytes a K or V row, printed by the engine at load
(`q4_0 KV cache: 288 bytes a row`, confirmed in q4-001), vs 1024 fp16 and 544 q8_0. Per layer (K+V):
2048 / 1088 / 576 bytes a token. Only 10 QSA target layers plus the draft layer hold KV (11 rows a
token, same geometry); GDN layers keep recurrent state, not KV rows. At full 131072 context that is
11 × per-layer bytes × 131072: fp16 ≈ 2.75 GiB, q8_0 ≈ 1.46 GiB, q4_0 ≈ 0.77 GiB by row
arithmetic (measured q4_0 reservation ≈ 0.91 GiB) — 71.9% saved vs
fp16, 47.1% vs q8_0. Whether freed VRAM converts to resident experts at long context is PENDING (needs
the frozen A/B).

## The two knobs

`--kv` sets the target model's layers; `--draft-kv` sets the guessing block's layer, independently. In
`strata-*.json` they ride as flat pairs, e.g. `["--kv", "q8_0", "--draft-kv", "q4_0"]` appended to `args`;
a knob omitted is `fp16`. Unknown values exit 2.

| target (`--kv`) | draft (`--draft-kv`) | answers / speed |
| --- | --- | --- |
| fp16 | fp16 | The default (both knobs fp16) is byte-identical to pre-q4_0 build per 12/12 short-prompt check (poly_kv_q8_test + fp16/Q8_0 reply comparison); re-run after any engine edit. |
| fp16 | q8_0 | mixed-knob engine test only (draft_race, acceptance +6.25pp vs control, rows exact); no live run |
| fp16 | q4_0 | unmeasured (PENDING) |
| q8_0 | fp16 | mixed-knob engine test only (acceptance +12.50pp vs control, rows exact); no live run |
| q8_0 | q8_0 | unmeasured as a combination (PENDING): the single-knob `--kv q8_0` parity below is the nearest measured result, not this combination |
| q8_0 | q4_0 | mixed-knob engine test only (acceptance +12.50pp vs control, rows exact); no live run |
| q4_0 | fp16 | unmeasured (PENDING) |
| q4_0 | q8_0 | mixed-knob engine test only (acceptance +2.08pp vs control, rows exact); no live run |
| q4_0 | q4_0 | REPORTED once live (q4-001, REPORTED not log-verified, section above) + engine test (acceptance +2.08pp, rows exact) |

## Parity with fp16 (q8_0, single-knob — carried from Q8_KV.md; q4_0: engine tests + one live run)

With greedy sampling (`temperature: 0`, which the engine reads as its own greedy), `--no-draft` so the
speculative path cannot change a token (`timings.draft_n` was 0 on every request), and `max_tokens: 64`, the
fp16 and Q8_0 arms were compared over five prompts each: the four short factual ones and one open-ended
sentence about the colour blue.

| model | exact matches | mean character agreement | longest common prefix |
| --- | ---: | ---: | --- |
| Tiel Coder 35B-A3B | 5/5 | 1.0000 | full reply (137 chars) |
| Ornith 1.5 35B-A3B | 5/5 | 1.0000 | full reply (129 chars) |
| Occamy 1.0 | 5/5 | 1.0000 | full reply (173 chars) |

Every Q8_0 answer equals the fp16 one character for character. The short factual prompts (`KV OK`, `42`,
`Paris`, `2, 3, 5, 7, 11`) also equal the pre-change fp16 baseline byte for byte on all three models
(12/12), which is how the default path's byte-identity was checked; the open-ended fifth prompt is not
bit-stable at greedy on Ornith and Occamy even before this change (speculative drafts on Ornith, `--tail-skip`
and the adaptive expert cache on Occamy), so it is not used for that check. The engine/API does not return
tokens, so agreement is measured on the reply text (longest common prefix and per-prompt agreement).

No frozen-prompt q4_0 arm has been compared against fp16. What exists: the kernel unit tests (key and
value readers vs the ggml oracle, 112,010 values, PASS), the mixed-knob engine test (acceptance within
+2.08…+12.50pp of the fp16 control on all five pairs, never a drop; save/load rows bit-exact 6/6), and
one full live agent run with zero errors. A failed exact prefix/tool match on a draft-only pair during
testing was investigated and found legitimate: a speculative walk diverges under any draft quantization
by construction (different guesses accept different runs), so those gates report but do not fail.

## What q4_0 saves

Per row, by block arithmetic: 9/32 of fp16, 9/17 of q8_0 — on Qwen3.6, 288 / 1024 / 544 bytes a K or V
row. At-context totals from the layer count (11 KV-holding layers: 10 QSA + draft; GDN keeps
recurrent state, not KV rows): fp16 ≈ 2.75 GiB, q8_0 ≈ 1.46 GiB, q4_0 ≈ 0.77 GiB by row arithmetic
(measured ≈ 0.91 GiB) at full 131072 context. At-run-context GiB figures WITHDRAWN 2026-10-18 — PENDING:
no log on file, run context unverified, and the stated ≈1.8 / 0.9 / 0.5 GiB at ~78K conflicts with
11-layer arithmetic (≈1.6 / 0.9 / 0.5). Recompute only from the log once it lands. With `--kv-whole`, which reserves the whole context's KV at load, freed VRAM goes to experts as
with q8_0; measured conversion under q4_0 at long context: PENDING (needs the frozen A/B).

## What q4_0 costs

One REPORTED data point (no log on file — UNVERIFIED), no controlled comparison: q4-001 under both knobs
reportedly did 90.9 max / 46.3 low / ~55.6 avg tok/s (48 turns, ~36.7K tokens in ~660 s) with prefill
17,277 tokens in ~13 s (≈1.33K tok/s) — no speed claim either way; "in line with fp16/q8_0 scatter" is
UNVERIFIED without the log, not a finding. Per-pair ms/round from the engine test: 54–89 ms/round across knobs (mildly
slower on quantized drafts, slowness alone is not gated). Frozen-fixture decode numbers: PENDING.

## Gates, and what was left alone

- qwen35moe-only, per knob: `--kv <val> is only supported for qwen35moe models in v1; this is a '<arch>'
  model` (the same with `--draft-kv`), not run.
- CUDA-only, per knob: `--kv <val> is not supported on this backend (CUDA only)` (the same with
  `--draft-kv`), and the engine exits, rather than doing something different.
- Like q8_0, q4_0 works in 32-value blocks: a shape whose `n_kv * head_dim` or `head_dim` is not a
  multiple of 32 gets a clear error naming the offending knob(s) rather than a wrong answer.
- The `strata` engine, and its own `--kv int8`. Nothing in it changed.
- `glm5next` / GLM 5.3 Flash (`dsa_attend`): its attention is not the one these caches quantize, so
  neither knob is offered for it.
- **Gemma 4 is out of v1**, per knob as with q8_0.
- The default (both knobs fp16) is byte-identical to pre-q4_0 build per 12/12 short-prompt check (poly_kv_q8_test + fp16/Q8_0 reply comparison); re-run after any engine edit.

## Where it lives

- `src/poly/` — the q4_0 cache pointers, allocation, save/load, the row-size print, and the attention
  path that reads quants+scales (`qwen35.*`, `kernels.cu`, `kernels.hpp`), the `--kv` / `--draft-kv`
  parsing and gates (`main.cpp`, `engine.cpp`), the kernel unit test (`kv_q4_test.cpp`), and the
  mixed-knob engine test (`draft_race_test.cpp`).
- `setup.py` — the `--kv` / `--draft-kv` writer, reader, and keep-paths.
- Docs — this file for the formats and knobs; the q8_0 measurements stay in [Q8_KV.md](Q8_KV.md), which
   is frozen and measurement-bound.

← [Prev](COVERAGE.md) · [Index](INDEX.md) · [Next](Q8_KV.md) →
