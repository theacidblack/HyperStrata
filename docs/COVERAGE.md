# HyperStrata coverage: weights, GPU, and what is measured

This is the public quant/GPU coverage statement. Plain words, every number
carries its rig or source, and anything not yet measured is labeled
`PENDING` or `IN PROGRESS`. KV knob formats and block arithmetic live in
`docs/KV_CACHE.md` and are not repeated here; GLM figures live in
`docs/GLM.md` and are not repeated here.

Sources for every number below: the header probe of all 44 weight files
(2026-10-17, every file LOADS), `docs/KV_CACHE.md` (KV knob facts),
`CHANGELOG.md` (release history), `UPSTREAM_DIVERGENCE.md` (DIV-002).
No other numbers are claimed.

## 1. Project promise

HyperStrata is the inference engine for exactly one hardware tier: **8 GB VRAM
+ 32 GB RAM** (RTX 3070 Ti Laptop class and 8 GB desktop/laptop peers), per the
master plan. Poly/Strata chase 10–12 GB; we own the tier below by running
**q4–q6 quants fast** where they tell you to drop to q2–q3. Anything outside
this tier is not our target, not our test matrix, not our docs.

Reference rig (everything is tied to this box, per `dev/README.md`): NVIDIA RTX
3070 Ti Laptop 8 GB VRAM (Ampere sm_86) + 32 GB DDR5, Windows 10/11 x86-64,
ctx 131072, temp 0.6, thinking cap 4096, 256 MiB VRAM reserve.

## 2. Model scope

Family first: Unsloth Qwen3.6-35B-A3B GGUFs, two pages (per master plan §2):

| Repo | Coverage | Rationale |
| --- | --- | --- |
| `unsloth/Qwen3.6-35B-A3B-MTP-GGUF` | UD-Q4_K_M (live, DONE) → UD-Q5_K_M → full ladder IQ1…UD-Q6_K | MTP head in VRAM caps weights at Q5 for live runs |
| `unsloth/Qwen3.6-35B-A3B-GGUF` (no MTP head) | up to UD-Q6_K | no draft head = budget freed for bigger weights |

MTP-stops-at-Q5 rule and why: the MTP draft head costs VRAM, so MTP coverage
stops at Q5; without MTP the weight budget stretches to Q6 (per master plan §2).

MTP delta (per master plan Phase 1): 20 draft tensors
(`7×F32 + 8×Q8_0 + 2–3×recipe + 2×BF16`), all wired (`load_drafter`,
`block_kv`, draft-KV knob cover them). Inventory confirms the delta as
753 tensors per MTP-page file vs 733 per base-page file
(source: `ud_inventory.json`).

"Support" per quant means (per master plan §2): (a) loader accepts the file
(tensor names, incl. MTP head tensors), (b) a dequant/compute path exists,
(c) an oracle test pins it (dequant vs ggml reference), (d) a VRAM budget row
says it fits. Live speed runs are NOT required per quant.

UD quants are mostly mixes of the K-family, so Q2_K→Q6_K coverage buys most of
the ladder; the IQ family (IQ1_*…IQ4_*) is a separate code path and comes second
(per master plan §2).

## 3. Per-file verdicts (UD ladder)

Header probe (`ud_probe.py` + `ud_inventory.json`, per master plan Phase 1):
every probed file `LOADS` — all 753 (MTP page) / 733 (base page) tensors per
file use already-supported type ids, `missing: []` on every entry.
Decode coverage: DONE, zero new kernels needed (source: master plan Phase 1).

The UD ladder is 40 files: 19 UD files on the MTP page + 21 UD files on the
base page (source: `ud_inventory.json` key count). Four extra stock files
(2× `MXFP4_MOE`, 2× `Q8_0`, one per page, 753 / 733 tensors, `LOADS`) are not
UD ladder files and are listed separately below.

Expert-path column is CURRENT STATUS from the read-only 6-subagent swarm
(source: master plan Phase 1), not a promise:

- Q2_K / Q3_K: grouped expert kernels MISSING, single-MMVQ only — PENDING.
- Q4_1: CPU fallback; stashed wiring fixes it — PENDING (no UD file in the
  inventory lists Q4_1 tensors today, so decode is unaffected).
- Q6_K grouped needs `STRATA_Q6K_EXPERTS=ON` — CONFIRMED ON in our build
  (source: master plan Phase 1).
- MXFP4: single only; grouped = stretch, after Q2_K/Q3_K — PENDING.
- Asymmetric pairs (GU grouped / D single or vice versa): Q5_1, Q4_K, IQ4_NL,
  IQ2_XXS/XS, IQ3_XXS/S, IQ2_S, IQ1_M. Correct, slower than symmetric —
  IN PROGRESS.
- Prefill CURRENT STATUS: poly-MMQ covers all but Q2_0 (42 tensors) /
  IQ1_M (29 tensors), which fall back to chunked native (GPU, fine); prefill
  `moe_mmq` path lacks K-quants → FP16 fallback (source: master plan Phase 1).

KV-oracle column: q4_0 is proven only for the live file (q4-001, §5). The full
228-cell cross-product oracle (19 weights × 3 KV × target/draft ×
decode/batch; integer tiers exact, IQ tiers epsilon; synthetic, no model file,
per `kv_weight_attend` design) is IN PROGRESS (source: master plan Phase 1).

### 3a. MTP page — 19 UD files (`unsloth/Qwen3.6-35B-A3B-MTP-GGUF`, 753 tensors each, all `LOADS`)

| File | Decode | Expert fast path (current status) | KV oracle |
| --- | --- | --- | --- |
| `UD-IQ1_M.gguf` | LOADS | IQ asymmetric pair (GU grouped / D single or vice versa), correct slower — IN PROGRESS; prefill IQ1_M falls back to chunked native where hit | 228-cell matrix IN PROGRESS |
| `UD-IQ2_M.gguf` | LOADS | IQ asymmetric pair, correct slower — IN PROGRESS | 228-cell matrix IN PROGRESS |
| `UD-IQ2_XXS.gguf` | LOADS | IQ2_XXS asymmetric pair, correct slower — IN PROGRESS | 228-cell matrix IN PROGRESS |
| `UD-IQ3_S.gguf` | LOADS | IQ3_S asymmetric pair (IQ2_S / IQ3_S family), correct slower — IN PROGRESS | 228-cell matrix IN PROGRESS |
| `UD-IQ3_XXS.gguf` | LOADS | IQ3_XXS asymmetric pair, correct slower — IN PROGRESS | 228-cell matrix IN PROGRESS |
| `UD-IQ4_NL.gguf` | LOADS | IQ4_NL asymmetric pair, correct slower — IN PROGRESS | 228-cell matrix IN PROGRESS |
| `UD-IQ4_XS.gguf` | LOADS | IQ asymmetric pair, correct slower — IN PROGRESS | 228-cell matrix IN PROGRESS |
| `UD-Q2_K_XL.gguf` | LOADS | Q2_K/Q3_K grouped MISSING, single-MMVQ only — PENDING | 228-cell matrix IN PROGRESS |
| `UD-Q3_K_M.gguf` | LOADS | Q2_K/Q3_K grouped MISSING, single-MMVQ only — PENDING | 228-cell matrix IN PROGRESS |
| `UD-Q3_K_XL.gguf` | LOADS | Q2_K/Q3_K grouped MISSING, single-MMVQ only — PENDING | 228-cell matrix IN PROGRESS |
| `UD-Q4_K_M.gguf` | LOADS | Q4_K asymmetric pair, correct slower — IN PROGRESS; **live file, q4_0 KV proven (q4-001) — DONE** | q4_0 DONE (q4-001); rest of matrix IN PROGRESS |
| `UD-Q4_K_S.gguf` | LOADS | Q4_K asymmetric pair, correct slower — IN PROGRESS | 228-cell matrix IN PROGRESS |
| `UD-Q4_K_XL.gguf` | LOADS | Q4_K asymmetric pair, correct slower — IN PROGRESS | 228-cell matrix IN PROGRESS |
| `UD-Q5_K_M.gguf` | LOADS | Q5_K grouped present; Q5_1 asymmetric where hit — IN PROGRESS; live MTP run PENDING (§5) | 228-cell matrix IN PROGRESS |
| `UD-Q5_K_S.gguf` | LOADS | Q5_K grouped present; Q5_1 asymmetric where hit — IN PROGRESS | 228-cell matrix IN PROGRESS |
| `UD-Q5_K_XL.gguf` | LOADS | Q5_K grouped present; Q5_1 asymmetric where hit — IN PROGRESS | 228-cell matrix IN PROGRESS |
| `UD-Q6_K.gguf` | LOADS | Q6_K grouped via `STRATA_Q6K_EXPERTS=ON` (confirmed ON) — CURRENT STATUS | 228-cell matrix IN PROGRESS |
| `UD-Q6_K_XL.gguf` | LOADS | Q6_K grouped via flag ON — CURRENT STATUS | 228-cell matrix IN PROGRESS |
| `UD-Q8_K_XL.gguf` | LOADS | Q8_0 path present — CURRENT STATUS (oracle-only, no live run planned) | 228-cell matrix IN PROGRESS |

### 3b. Base page — 21 UD files (`unsloth/Qwen3.6-35B-A3B-GGUF`, 733 tensors each, all `LOADS`)

| File | Decode | Expert fast path (current status) | KV oracle |
| --- | --- | --- | --- |
| `UD-IQ1_M.gguf` | LOADS | IQ asymmetric pair, correct slower — IN PROGRESS; prefill IQ1_M falls back to chunked native where hit | 228-cell matrix IN PROGRESS |
| `UD-IQ2_M.gguf` | LOADS | IQ asymmetric pair, correct slower — IN PROGRESS | 228-cell matrix IN PROGRESS |
| `UD-IQ2_XXS.gguf` | LOADS | IQ2_XXS asymmetric pair, correct slower — IN PROGRESS | 228-cell matrix IN PROGRESS |
| `UD-IQ3_S.gguf` | LOADS | IQ3_S asymmetric pair, correct slower — IN PROGRESS | 228-cell matrix IN PROGRESS |
| `UD-IQ3_XXS.gguf` | LOADS | IQ3_XXS asymmetric pair, correct slower — IN PROGRESS | 228-cell matrix IN PROGRESS |
| `UD-IQ4_NL.gguf` | LOADS | IQ4_NL asymmetric pair, correct slower — IN PROGRESS | 228-cell matrix IN PROGRESS |
| `UD-IQ4_NL_XL.gguf` | LOADS | IQ4_NL asymmetric pair, correct slower — IN PROGRESS | 228-cell matrix IN PROGRESS |
| `UD-IQ4_XS.gguf` | LOADS | IQ asymmetric pair, correct slower — IN PROGRESS | 228-cell matrix IN PROGRESS |
| `UD-Q2_K_XL.gguf` | LOADS | Q2_K/Q3_K grouped MISSING, single-MMVQ only — PENDING | 228-cell matrix IN PROGRESS |
| `UD-Q3_K_M.gguf` | LOADS | Q2_K/Q3_K grouped MISSING, single-MMVQ only — PENDING | 228-cell matrix IN PROGRESS |
| `UD-Q3_K_S.gguf` | LOADS | Q2_K/Q3_K grouped MISSING, single-MMVQ only — PENDING | 228-cell matrix IN PROGRESS |
| `UD-Q3_K_XL.gguf` | LOADS | Q2_K/Q3_K grouped MISSING, single-MMVQ only — PENDING | 228-cell matrix IN PROGRESS |
| `UD-Q4_K_M.gguf` | LOADS | Q4_K asymmetric pair, correct slower — IN PROGRESS | 228-cell matrix IN PROGRESS |
| `UD-Q4_K_S.gguf` | LOADS | Q4_K asymmetric pair, correct slower — IN PROGRESS | 228-cell matrix IN PROGRESS |
| `UD-Q4_K_XL.gguf` | LOADS | Q4_K asymmetric pair, correct slower — IN PROGRESS | 228-cell matrix IN PROGRESS |
| `UD-Q5_K_M.gguf` | LOADS | Q5_K grouped present; Q5_1 asymmetric where hit — IN PROGRESS | 228-cell matrix IN PROGRESS |
| `UD-Q5_K_S.gguf` | LOADS | Q5_K grouped present; Q5_1 asymmetric where hit — IN PROGRESS | 228-cell matrix IN PROGRESS |
| `UD-Q5_K_XL.gguf` | LOADS | Q5_K grouped present; Q5_1 asymmetric where hit — IN PROGRESS | 228-cell matrix IN PROGRESS |
| `UD-Q6_K.gguf` | LOADS | Q6_K grouped via flag ON — CURRENT STATUS; engine live run PENDING (§5) | 228-cell matrix IN PROGRESS |
| `UD-Q6_K_XL.gguf` | LOADS | Q6_K grouped via flag ON — CURRENT STATUS | 228-cell matrix IN PROGRESS |
| `UD-Q8_K_XL.gguf` | LOADS | Q8_0 path present — CURRENT STATUS (oracle-only, no live run planned) | 228-cell matrix IN PROGRESS |

Non-UD stock files (for completeness, source: `ud_inventory.json`): MTP-page
`MXFP4_MOE.gguf` (753 tensors, LOADS; MXFP4 single only — grouped PENDING),
MTP-page `Q8_0.gguf` (753 tensors, LOADS), base-page `MXFP4_MOE.gguf`
(733 tensors, LOADS; MXFP4 single only — grouped PENDING), base-page
`Q8_0.gguf` (733 tensors, LOADS).

Done when (per master plan Phase 1): every UD file's experts take a GPU kernel
path (grouped where it matters, single-MMVQ minimum) AND the 228-cell oracle
is green. That gate is IN PROGRESS.

## 4. GPU coverage

NVIDIA RTX only, Turing (sm_75) through Blackwell: functional everywhere via
CUDA PTX forward-compat, optimized for sm_86 (our rig), per the master plan
GPU lock (§1b, locked 2026-10-17).

- All integer quants (Q2_K…Q6_K, IQ family) are DP4A/integer math —
  arch-independent, run on every RTX card (source: master plan §1b).
- sm_86 is the optimized target: the reference rig is the RTX 3070 Ti Laptop
  (source: `dev/README.md` hardware table).
- Blackwell note: NVFP4/MXFP4 microscaling (2× vs FP8 in hardware) is already
  in the tree (MXFP4 MMVQ decode, NVFP4 expert path, Blackwell-gated MMQ) —
  Blackwell owners get the premium path nearly free (source: master plan §1b).
- Pre-RTX/GTX: best-effort PTX fallback, not our test matrix (source: master
  plan §1b).

Phase 3 cuts AMD/HIP, Intel, and Metal paths (source: master plan Phase 3).
See §6 for what is explicitly out of scope.

## 5. Baselines

Status labels are part of the claim. Same rig for every comparison or it does
not count (per master plan testing doctrine): RTX 3070 Ti Laptop 8 GB +
32 GB DDR5, ctx 131072, temp 0.6, thinking cap 4096.

| Run | Status | Numbers (with rig/source) |
| --- | --- | --- |
| q4-001: UD-Q4_K_M live, `--kv q4_0 --draft-kv q4_0` | DONE | 90.9 max / 46.3 low / ~55.6 avg tok/s over 48 turns; prefill 17,277 tokens in ~13 s (~1.33K tok/s); thinking cap hit once and wrapped; zero errors (source: `docs/KV_CACHE.md` + `dev/README.md`, reference rig above). Kernel unit tests: key and value readers vs ggml oracle, 112,010 values, PASS (source: `docs/KV_CACHE.md`). |
| UD-Q6_K @131072 ctx, llama.cpp | BASELINE (not our run) | ≈30 tok/s on the owner rig (source: master plan Phase 2). Never tested on HyperStrata. Engine target: beat 30 tok/s, same file, same ctx, q4_0 KV. This is the number to kill. |
| UD-Q5_K_M with MTP (same harness as q4-001: fixed config, 48-ish turns, max/low/avg, prefill, zero-error bar) | PENDING | No engine run yet (source: master plan Phase 2). |
| UD-Q6_K without MTP, engine run (q4_0 KV) | PENDING | No engine run yet (source: master plan Phase 2). |

Read the README table straight (source: `dev/README.md`): fp16 KV did
91.9 max / 55.4 low / ~61.5 avg tok/s (41 turns), q8_0 KV did 86.5 max /
56.3 low / ~61.4 avg tok/s (48 turns), q4_0 KV did the q4-001 row above.
Quantization buys no decode speed here; maxima are single-turn peaks within
noise and averages come from different agent trajectories. What q4_0 buys is
room — see `docs/KV_CACHE.md` for the layer math.

Known correction queued (source: master plan Phase 1): `docs/KV_CACHE.md`
overcounts ≈3.2× (says 2.95 GiB @131K; only 10 QSA + 1 draft layers hold KV
→ measured 0.91 GiB). Doc fix queued — PENDING. Whether freed VRAM converts
to resident experts at long context is PENDING (needs the frozen-fixture A/B,
per `docs/KV_CACHE.md`).

The `docs/VRAM_BUDGET.md` table (per-model, per-quant, MTP on/off: weights in
RAM, hot set in VRAM, measured tok/s on the reference rig) is PENDING
(source: master plan Phase 2). Until it lands, the table above IS NOT the
product promise — numbers only, no claims without a run.

## 6. Out of scope

Explicitly not covered, not tested, not documented beyond this list:

- Weight formats: AWQ / GPTQ / safetensors. This statement covers GGUF UD
  ladder files only (source: inventory scope + task scope).
- GPUs: AMD/HIP, Intel, Apple/Metal. Cut in Phase 3 (source: master plan
  Phase 3); this fork's measured rig is Windows + NVIDIA only
  (source: `dev/README.md` limitations).
- Pre-RTX / GTX: best-effort PTX fallback at most, never the test matrix
  (source: master plan §1b).
- Live speed runs per quant: we do NOT live-test every quant. Oracle tests
  prove correctness; live runs are reserved for the §2 live column, and the
  rest of the ladder ships `oracle-only` with community runs filling the speed
  column later (sources: master plan Phases 1–2, Phase 5 harness plan).
- GLM / vision work: separate job, after the text ladder is green
  (source: master plan Phase 6). KV-knob facts for `glm5next`/`dsa_attend`
  non-coverage stay in `docs/KV_CACHE.md` / `docs/GLM.md`.
