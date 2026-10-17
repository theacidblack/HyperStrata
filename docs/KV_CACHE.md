# The KV cache formats (`strata-poly --kv`, `--draft-kv`)

`strata-poly` can store its attention K and V caches in the ggml **Q8_0** or **Q4_0** formats instead of
fp16. There are two independent knobs: `--kv fmt` for the target model and `--draft-kv fmt` for the block
that guesses ahead, each `fp16 | q8_0 | q4_0`, mixable (e.g. `--kv q8_0 --draft-kv q4_0`). A knob left out
is `fp16`. The default, both knobs fp16, is unchanged and byte-identical to the build before these options
existed.

q8_0 is measured: [Q8_KV.md](Q8_KV.md) (single-knob engine). q4_0 has no benchmark behind it: below, every
figure that would need a run is marked **unmeasured** (PENDING). The kernel bytes have a unit test
(`poly_kv_q4_test`); speed and answers do not have runs yet.

## The three formats

A cache row holds one token's value for each KV head and each head dimension (`n_kv * head_dim` values).
Stored as fp16 that is `2 * n_kv * head_dim` bytes a token for K and, again, for V.

- **fp16** (default, unchanged): 2 bytes a value, as above.
- **q8_0**: cut into 32-value blocks: for every block of 32 values, one fp16 **scale** (the block's
  largest magnitude over 127) and 32 **int8 codes**. Attention dequantizes a block with its scale on the
  fly while it computes the scores and the outputs. Measured: [Q8_KV.md](Q8_KV.md).
- **q4_0** (unmeasured): cut into 32-value blocks, transcribed from ggml's `quantize_row_q4_0_ref`. The
  block's **scale** is `d = max / -8`, where `max` is the value with the largest magnitude in the block,
  sign kept — so `d` may be negative, and is stored as is (fp16). Each value's **code** is
  `min(15, trunc(x / d + 8.5))`: C truncation toward zero with an upper clamp only and no lower clamp —
  an asymmetric 16-step grid anchored at the block's signed extreme. Values `j` and `j + 16` (`j` < 16)
  share byte `j` as its low and high nibbles (16 packed bytes a block). Reading back is
  `(nibble - 8) * d` with the stored scale as is, so a negative `d` reads correctly. Only this per-block
  arithmetic follows ggml: the planes are the engine's own — all quants in one plane, all fp16 scales in
  another — for the fused readers, not ggml's interleaved `{d, qs[16]}` blocks.

Block arithmetic (definitional, no run needed): a q4_0 block is 16 packed bytes plus one fp16 scale, 18
bytes per 32 values — **9/32 of the fp16 row and 9/17 of the q8_0 row**. The engine's printed q4_0 row
size, the GiB totals, and the expert-cache effect: unmeasured (PENDING).

## The two knobs

`--kv` sets the target model's layers; `--draft-kv` sets the guessing block's layer, independently. In
`strata-*.json` they ride as flat pairs, e.g. `["--kv", "q8_0", "--draft-kv", "q4_0"]` appended to `args`;
a knob omitted is `fp16`. Unknown values exit 2.

| target (`--kv`) | draft (`--draft-kv`) | answers / speed |
| --- | --- | --- |
| fp16 | fp16 | the default: unchanged, byte-identical |
| fp16 | q8_0 | unmeasured (PENDING) |
| fp16 | q4_0 | unmeasured (PENDING) |
| q8_0 | fp16 | unmeasured (PENDING) |
| q8_0 | q8_0 | unmeasured as a combination (PENDING): the single-knob `--kv q8_0` parity below is the nearest measured result, not this combination |
| q8_0 | q4_0 | unmeasured (PENDING) |
| q4_0 | fp16 | unmeasured (PENDING) |
| q4_0 | q8_0 | unmeasured (PENDING) |
| q4_0 | q4_0 | unmeasured (PENDING) |

## Parity with fp16 (q8_0, single-knob — carried from Q8_KV.md; q4_0: unmeasured)

With greedy sampling (`temperature: 0`, which the engine reads as its own greedy), `--no-draft` so the
speculative path cannot change a token (`timings.draft_n` was 0 on every request), and `max_tokens: 64`, the
fp16 and Q8_0 arms were compared over five prompts each: the four short factual ones and one open-ended
sentence about the colour blue.

| model | exact matches | mean character agreement | longest common prefix |
| --- | ---: | ---: | --- |
| Tiel Coder 35B-A3B | 5/5 | 1.0000 | full reply (137 chars) |
| Ornith 1.5 35B-A3B | 5/5 | 1.0000 | full reply (129 chars) |
| Occamy 1.0 | 5/5 | 1.0000 | full reply (173 chars) |
| **all three** | **15/15** | **1.0000** | — |

Every Q8_0 answer equals the fp16 one character for character. The short factual prompts (`KV OK`, `42`,
`Paris`, `2, 3, 5, 7, 11`) also equal the pre-change fp16 baseline byte for byte on all three models
(12/12), which is how the default path's byte-identity was checked; the open-ended fifth prompt is not
bit-stable at greedy on Ornith and Occamy even before this change (speculative drafts on Ornith, `--tail-skip`
and the adaptive expert cache on Occamy), so it is not used for that check. The engine/API does not return
tokens, so agreement is measured on the reply text (longest common prefix and per-prompt agreement).

No q4_0 arm has been compared against fp16: its parity is unmeasured (PENDING).

## What q4_0 saves

Per row, by block arithmetic: 9/32 of fp16, 9/17 of q8_0. Per-token and at-context GiB totals: unmeasured
(PENDING). With `--kv-whole`, which reserves the whole context's KV at load, freed VRAM goes to experts as
with q8_0; by how much under q4_0: unmeasured (PENDING).

## What q4_0 costs

Unmeasured (PENDING): no prompt tok/s, decode tok/s, first-token latency, or ms/round run exists for any
q4_0 or mixed-knob combination on any machine.

## Gates, and what was left alone

- qwen35moe-only, per knob: `--kv <val> is only supported for qwen35moe models in v1; this is a '<arch>'
  model` (the same with `--draft-kv`), not run.
- CUDA-only, per knob: `--kv <val> is not supported on this backend (CUDA only)` (the same with
  `--draft-kv`), and the engine exits, rather than doing something different.
- Like q8_0, q4_0 works in 32-value blocks: a shape whose `n_kv * head_dim` is not a multiple of 32 gets
  a clear error rather than a wrong answer.
- The `strata` engine, and its own `--kv int8`. Nothing in it changed.
- `glm5next` / GLM 5.3 Flash (`dsa_attend`): its attention is not the one these caches quantize, so
  neither knob is offered for it.
- **Gemma 4 is out of v1**, per knob as with q8_0.
- The default path stays byte-identical to the last release.

## Where it lives

- `src/poly/` — the q4_0 cache pointers, allocation, save/load, the row-size print, and the attention
  path that reads quants+scales (`qwen35.*`, `kernels.cu`, `kernels.hpp`), the `--kv` / `--draft-kv`
  parsing and gates (`main.cpp`, `engine.cpp`), and the kernel unit test (`kv_q4_test.cpp`).
- `setup.py` — the `--kv` / `--draft-kv` writer, reader, and keep-paths.
- Docs — this file for the formats and knobs; the q8_0 measurements stay in [Q8_KV.md](Q8_KV.md), which
  is frozen and measurement-bound.
