
← [Prev](HOW-TO-INSTALL.md) · [Index](INDEX.md) · [Next](KV_CACHE.md) →

# Which model fits 8 GB VRAM + 32 GB RAM (Qwen3.6-35B-A3B Unsloth UD quants)

*Start 4/4*

Reference rig and fixed config for every number on this page: see [BENCHMARKS.md](BENCHMARKS.md).

## House files

v1.0.0 gates: UD-Q4_K_M MTP and UD-Q5_K_M MTP on the reference rig, each with max/low/avg decode + prefill, log + verdict. Q5 on `hyper` MEASURED 2026-10-17 (q5-002, both gates closed). File sizes below are task-ground figures, PENDING dir-listing check per `rig.md` (per-file sizes NOT yet recorded on disk).

| File | Download | Setup RAM beside 8 GB card | Status |
| --- | ---: | ---: | --- |
| `unsloth/Qwen3.6-35B-A3B-MTP-GGUF` · `UD-Q4_K_M.gguf` (753 tensors, MTP draft head in file) | 22.6 GB (PENDING check; was 22.7, wrong) | PENDING | live runs q4-002 PASS + q4v-001 PARTIAL in [BENCHMARKS.md](BENCHMARKS.md); q4-001 REPORTED only, no log |
| `unsloth/Qwen3.6-35B-A3B-MTP-GGUF` · `UD-Q5_K_M.gguf` (753 tensors, MTP draft head in file) | 25.2 GB (PENDING check) | PENDING | q5-002 PASS 2026-10-17 on `hyper` (70.9 max / 29.8 low / 41.1 avg, 61 turns) + q5-001 on old engine; numbers in [BENCHMARKS.md](BENCHMARKS.md) |
| `unsloth/Qwen3.6-35B-A3B-GGUF` · `UD-Q6_K.gguf` (733 tensors, base page, `--no-draft`, no MTP) | 27.3 GB (PENDING check) | PENDING | q6-001 REPORTED 2026-10-18 on old engine (55.8 max / 13.5 low / ~35.5 avg, 159 turns, Vision on); console PENDING, `hyper` re-run PENDING |
| K2-Horizon 36B-A4B `IFM Q4_K_M` (arch `k2-horizon`, no MTP) | 20.83 GB (PENDING check) | PENDING | on disk, intake only; zero `k2-horizon` refs in engine, no run |
| vision encoder `mmproj-Q8_0` (CPU encoder) | 0.6 GB (PENDING check) | PENDING | used in q6-001 + q4v-001 Vision runs; per-quant runs PENDING |

## Ladder rungs and VRAM

VRAM holds dense weights + resident expert cache + KV/state + 256 MiB reserve. File size is disk, not VRAM.

KV sizes: see [KV_CACHE.md](KV_CACHE.md) — q4_0 is the fit knob. Expert-cache shares on house-file runs: see [BENCHMARKS.md](BENCHMARKS.md).

Setup rungs. Only files with live speed rows in [BENCHMARKS.md](BENCHMARKS.md) count as run here; the rest of the ladder is PENDING, not LOADS. Per-rung resident-expert VRAM table PENDING:

| File | Download | Setup RAM beside 8 GB card | Status |
| --- | ---: | ---: | --- |
| `UD-Q2_K_XL.gguf` | 12.6 GB (PENDING check) | PENDING | PENDING; grouped kernels PENDING ([Coverage](#coverage)) |
| `UD-Q3_K_M.gguf` | 17.1 GB (PENDING check) | PENDING | PENDING; grouped kernels PENDING ([Coverage](#coverage)) |
| `UD-Q4_K_M.gguf` (house MTP) | 22.6 GB (PENDING check) | PENDING | live: q4-002 PASS, q4v-001 PARTIAL, fp16-001 + q8-001 on same file family; see [BENCHMARKS.md](BENCHMARKS.md) |
| `UD-Q5_K_M.gguf` (house MTP) | 25.2 GB (PENDING check) | PENDING | live: q5-002 PASS on `hyper`, q5-001 on old engine ([Coverage](#coverage)) |
| `UD-Q6_K.gguf` (base page, `--no-draft`) | 27.3 GB (PENDING check) | PENDING | live: q6-001 REPORTED on old engine, console PENDING, `hyper` re-run PENDING; MTP-page Q6 files exist upstream but have no house live run |
| K2 Q4_K_M (`k2-horizon`) + `mmproj-Q8_0` | 20.83 GB + 0.6 GB (PENDING check) | PENDING | no run; zero engine refs for `k2-horizon` |

Full ladder: 19 UD files on the MTP page + 21 on the base page. Probe LOADS claim WITHDRAWN — treat every non-house file as PENDING. Per-file decode/expert/KV-oracle verdicts in [Coverage](#coverage). Measured gen/prefill rates only in [BENCHMARKS.md](BENCHMARKS.md); averages across rows are different runs, not rankings.

Older upstream tables removed — see git history.

## Coverage

What loads, what runs, what stays PENDING. Architecture `qwen35moe`; the engine reads GGUF files in place. Numbers live in [BENCHMARKS.md](BENCHMARKS.md); KV sizes in [KV_CACHE.md](KV_CACHE.md).

### Weight files

A header probe of all 44 weight files ran 2026-10-17, but its LOADS verdict is WITHDRAWN — treat it as PENDING. Only house files with live speed rows in [BENCHMARKS.md](BENCHMARKS.md) count as run. Decode is complete only for those house runs; the rest of the ladder still needs the expert fast paths and the KV oracle (below).

- 40 UD ladder files: 19 on the MTP page, 21 on the base page.
- 4 stock files, not UD ladder files: `MXFP4_MOE` and `Q8_0`, one of each per page, all PENDING (probe WITHDRAWN).
- The MTP draft head adds 20 tensors over the base page (753 vs 733 per file): 7×F32, 8×Q8_0, 2–3×recipe, 2×BF16, all wired.

The MTP draft head costs VRAM, so house MTP runs stop at Q5 (q4-002 + q5-002); MTP-page Q6 files exist upstream but have no house live run. Without the draft head the weight budget stretches to Q6 — base-page `--no-draft` Q6 live run q6-001 REPORTED (console PENDING).

"Support" per quant means: a live speed run in [BENCHMARKS.md](BENCHMARKS.md) (lines 34-45) on this rig pins it. The untested ladder ships oracle-only; a loader probe alone is not support. A live speed run per quant is required.

#### MTP page — 19 UD files (`unsloth/Qwen3.6-35B-A3B-MTP-GGUF`, 753 tensors each, status PENDING unless a live run exists)

| File | Expert fast path | KV oracle |
| --- | --- | --- |
| `UD-IQ1_M.gguf` | IQ asymmetric pair; correct, slower than symmetric — PENDING 228-cell oracle; prefill falls back to chunked native where hit | PENDING |
| `UD-IQ2_M.gguf` | IQ asymmetric pair; correct, slower than symmetric — PENDING 228-cell oracle | PENDING |
| `UD-IQ2_XXS.gguf` | IQ2_XXS asymmetric pair — PENDING 228-cell oracle | PENDING |
| `UD-IQ3_S.gguf` | IQ3_S asymmetric pair — PENDING 228-cell oracle | PENDING |
| `UD-IQ3_XXS.gguf` | IQ3_XXS asymmetric pair — PENDING 228-cell oracle | PENDING |
| `UD-IQ4_NL.gguf` | IQ4_NL asymmetric pair — PENDING 228-cell oracle | PENDING |
| `UD-IQ4_XS.gguf` | IQ asymmetric pair — PENDING 228-cell oracle | PENDING |
| `UD-Q2_K_XL.gguf` | Q2_K/Q3_K grouped kernels missing, single-MMVQ only — PENDING | PENDING |
| `UD-Q3_K_M.gguf` | Q2_K/Q3_K grouped kernels missing, single-MMVQ only — PENDING | PENDING |
| `UD-Q3_K_XL.gguf` | Q2_K/Q3_K grouped kernels missing, single-MMVQ only — PENDING | PENDING |
| `UD-Q4_K_M.gguf` | Q4_K asymmetric pair — PENDING 228-cell oracle; q4_0 KV pinned by `poly_kv_q4_test` (112,010 values) + q4-002 PASS live run (q4-001 REPORTED only, superseded) | q4_0 DONE on this file (q4-002 live + 112,010-value pin); rest PENDING |
| `UD-Q4_K_S.gguf` | Q4_K asymmetric pair — PENDING 228-cell oracle | PENDING |
| `UD-Q4_K_XL.gguf` | Q4_K asymmetric pair — PENDING 228-cell oracle | PENDING |
| `UD-Q5_K_M.gguf` | Q5_K grouped present; Q5_1 asymmetric where hit — IN PROGRESS; live files q5-001 (previous engine) + q5-002 PASS 2026-10-17 on `hyper` | PENDING |
| `UD-Q5_K_S.gguf` | Q5_K grouped present; Q5_1 asymmetric where hit — IN PROGRESS | PENDING |
| `UD-Q5_K_XL.gguf` | Q5_K grouped present; Q5_1 asymmetric where hit — IN PROGRESS | PENDING |
| `UD-Q6_K.gguf` | Q6_K grouped via `STRATA_Q6K_EXPERTS=ON` (confirmed ON) | PENDING (no MTP-page live run; only base-page q6-001 REPORTED) |
| `UD-Q6_K_XL.gguf` | Q6_K grouped via `STRATA_Q6K_EXPERTS=ON` (confirmed ON) | PENDING |
| `UD-Q8_K_XL.gguf` | Q8_0 path present (oracle-only, no live run planned) | PENDING |

#### Base page — 21 UD files (`unsloth/Qwen3.6-35B-A3B-GGUF`, 733 tensors each, status PENDING unless a live run exists)

| File | Expert fast path | KV oracle |
| --- | --- | --- |
| `UD-IQ1_M.gguf` | IQ asymmetric pair; correct, slower than symmetric — PENDING 228-cell oracle; prefill falls back to chunked native where hit | PENDING |
| `UD-IQ2_M.gguf` | IQ asymmetric pair — PENDING 228-cell oracle | PENDING |
| `UD-IQ2_XXS.gguf` | IQ2_XXS asymmetric pair — PENDING 228-cell oracle | PENDING |
| `UD-IQ3_S.gguf` | IQ3_S asymmetric pair — PENDING 228-cell oracle | PENDING |
| `UD-IQ3_XXS.gguf` | IQ3_XXS asymmetric pair — PENDING 228-cell oracle | PENDING |
| `UD-IQ4_NL.gguf` | IQ4_NL asymmetric pair — PENDING 228-cell oracle | PENDING |
| `UD-IQ4_NL_XL.gguf` | IQ4_NL asymmetric pair — PENDING 228-cell oracle | PENDING |
| `UD-IQ4_XS.gguf` | IQ asymmetric pair — PENDING 228-cell oracle | PENDING |
| `UD-Q2_K_XL.gguf` | Q2_K/Q3_K grouped missing, single-MMVQ only — PENDING | PENDING |
| `UD-Q3_K_M.gguf` | Q2_K/Q3_K grouped missing, single-MMVQ only — PENDING | PENDING |
| `UD-Q3_K_S.gguf` | Q2_K/Q3_K grouped missing, single-MMVQ only — PENDING | PENDING |
| `UD-Q3_K_XL.gguf` | Q2_K/Q3_K grouped missing, single-MMVQ only — PENDING | PENDING |
| `UD-Q4_K_M.gguf` | Q4_K asymmetric pair — PENDING 228-cell oracle | PENDING |
| `UD-Q4_K_S.gguf` | Q4_K asymmetric pair — PENDING 228-cell oracle | PENDING |
| `UD-Q4_K_XL.gguf` | Q4_K asymmetric pair — PENDING 228-cell oracle | PENDING |
| `UD-Q5_K_M.gguf` | Q5_K grouped present; Q5_1 asymmetric where hit — IN PROGRESS | PENDING |
| `UD-Q5_K_S.gguf` | Q5_K grouped present; Q5_1 asymmetric where hit — IN PROGRESS | PENDING |
| `UD-Q5_K_XL.gguf` | Q5_K grouped present; Q5_1 asymmetric where hit — IN PROGRESS | PENDING |
| `UD-Q6_K.gguf` | Q6_K grouped via `STRATA_Q6K_EXPERTS=ON` (confirmed ON); base-page `--no-draft` live run q6-001 REPORTED on old engine (console PENDING), `hyper` re-run PENDING | PENDING |
| `UD-Q6_K_XL.gguf` | Q6_K grouped via `STRATA_Q6K_EXPERTS=ON` (confirmed ON) | PENDING |
| `UD-Q8_K_XL.gguf` | Q8_0 path present (oracle-only, no live run planned) | PENDING |

Stock files, not UD ladder files: MTP page `MXFP4_MOE.gguf` (753 tensors, PENDING; MXFP4 single only — grouped PENDING) and `Q8_0.gguf` (753 tensors, PENDING); base page `MXFP4_MOE.gguf` (733 tensors, PENDING; MXFP4 single only — grouped PENDING) and `Q8_0.gguf` (733 tensors, PENDING).

### Expert fast paths

- Q2_K / Q3_K: grouped expert kernels missing; single-MMVQ only — PENDING.
- Q4_1: CPU fallback; a stashed wiring change fixes it — PENDING. No UD file lists Q4_1 tensors today, so decode is unaffected.
- Q6_K: grouped needs `STRATA_Q6K_EXPERTS=ON` — confirmed ON in this build.
- MXFP4: single only; grouped PENDING.
- Asymmetric pairs (grouped for gate/up, single for down, or the reverse): Q5_1, Q4_K, IQ4_NL, IQ2_XXS/XS, IQ3_XXS/S, IQ2_S, IQ1_M. Correct; relative speed unmeasured — PENDING.
- Prefill: covers all but Q2_0 (42 tensors) and IQ1_M (29 tensors), which fall back to chunked native on the GPU; the prefill `moe_mmq` path lacks K-quants and falls back to FP16.

### KV oracle

q4_0 is pinned only for UD-Q4_K_M: `poly_kv_q4_test`, 112,010 values (key and value readers against a ggml oracle, see [KV_CACHE.md](KV_CACHE.md)), plus live runs q4-002 PASS and q5-002 PASS (q4-001 REPORTED only, superseded — no log). Every other weight×KV cell is PENDING. The full 228-cell cross-product oracle — 19 weights × 3 KV × target/draft × decode/batch; integer tiers exact, IQ tiers epsilon; synthetic, no model file — is PENDING (NOT CAPTURED; frozen-fixture A/B NOT CAPTURED per LEDGER Phase-0). Whether freed VRAM converts to resident experts at long context is PENDING (needs the frozen A/B; see [KV_CACHE.md](KV_CACHE.md)).

### GPU coverage

Measured target: sm_86 (reference rig, see [BENCHMARKS.md](BENCHMARKS.md)). Turing to Blackwell via CUDA PTX is upstream design intent, untested in this fork. Integer quants use DP4A/integer math by design (upstream); verified on sm_86 only. NVFP4/MXFP4 paths exist in-tree; no Blackwell run on this tree — PENDING. Pre-RTX/GTX is best-effort PTX at most, never the test matrix.

### Runs

- q4-001: UD-Q4_K_M, `--kv q4_0 --draft-kv q4_0` — REPORTED only (no log), superseded by q4-002; kept for history, not a finding; numbers in [BENCHMARKS.md](BENCHMARKS.md).
- q4-002: UD-Q4_K_M MTP run on `hyper` — PASS 2026-10-17, 95.7 max / 40.0 low / 51.3 avg tok/s (42 turns, 31,160 tokens / 607 s; console log measured, server log only); numbers in [BENCHMARKS.md](BENCHMARKS.md). Provenance: LEDGER q4-002 row + log on file.
- q5-001: UD-Q5_K_M with MTP — MEASURED 2026-10-18 on the previous engine; `hyper` re-run DONE as q5-002 below.
- q5-002: UD-Q5_K_M with MTP — PASS 2026-10-17 on `hyper` (70.9 max / 29.8 low / 41.1 avg tok/s, 61 turns, 29,615 tok / 721 s; console log measured, server log only); numbers in [BENCHMARKS.md](BENCHMARKS.md). Provenance: LEDGER q5-002 row + log on file.
- q6-001: UD-Q6_K base (`--no-draft`, no MTP) + Vision — REPORTED 2026-10-18 on the previous engine (55.8 max / 13.5 low / ~35.5 avg, 159 turns); no verdict on the build; full console PENDING, no log on file; re-run on `hyper` PENDING. Provenance: LEDGER q6-001 row.
- Vision: functional on Q6_K base (q6-001 REPORTED) and Q4_K_M MTP (q4v-001 PARTIAL, 19 turns in paste) through the mmproj-Q8_0 CPU encoder. Shared-encoder inference does NOT prove other quants — per-quant vision runs PENDING (low priority).
- Reference: about 30 tok/s (llama.cpp, same file and context) is not our run; never presented as an engine result.
- A per-model VRAM budget table (weights in RAM, hot set in VRAM, measured tok/s) is PENDING.

### Out of scope

- Weight formats AWQ / GPTQ / safetensors. This page covers GGUF UD-ladder files only.
- GPUs: AMD/HIP, Intel, Apple/Metal. The measured rig is Windows + NVIDIA only.
- Pre-RTX/GTX cards.
- Live speed runs per quant: the untested ladder ships oracle-only; later community numbers are unverified extras, not promised.
- GLM / vision work: a separate job.

## Adding or switching models

One model in memory at a time; switch unloads the engine first ([BACKENDS.md](BACKENDS.md) registry/switch). Files another model shares are not downloaded again. With more than one installed, the start script asks which to start.

← [Prev](HOW-TO-INSTALL.md) · [Index](INDEX.md) · [Next](KV_CACHE.md) →
