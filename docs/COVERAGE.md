← [Prev](MODELS.md) · [Index](INDEX.md) · [Next](KV_CACHE.md) →

*Needs: finished Start path, model picked · Gives: what loads, what runs, what stays pending · Next: KV_CACHE.md*
*Run 1/13*

# Qwen3.6-35B-A3B (Unsloth UD GGUF) coverage on 8 GB VRAM + 32 GB RAM

This is the public quant/GPU coverage statement. Plain words, every number
carries its rig or source, and anything not yet measured is labeled
`PENDING` or `IN PROGRESS`. KV knob formats and block arithmetic live in
`docs/KV_CACHE.md` and are not repeated here; GLM figures live in
`docs/GLM.md` and are not repeated here.

Sources for every number below: the header probe of all 44 weight files
(2026-10-17, every file LOADS), `docs/KV_CACHE.md` (KV knob facts),
`CHANGELOG.md` (release history), `UPSTREAM_DIVERGENCE.md` (DIV-002).
No other numbers are claimed.

Provenance note 2026-10-18: every `master plan` citation below is a
workspace-only working doc — NOT committed in this repo, reader-unverifiable.
`ud_inventory.json` / `ud_probe.py` are likewise uncommitted probe artifacts.
Claims resting only on those sources are workspace-only, not independently
verifiable by the reader; committed sources are `README.md`,
`docs/BENCHMARKS.md`, `docs/KV_CACHE.md`, `CHANGELOG.md`,
`UPSTREAM_DIVERGENCE.md`.

## 1. Project scope (intent, no date promised)

My personal experience with 8 GB VRAM and 32 GB RAM covers many models on
the 35B-A3B architecture, and here I want to bring only what runs well on
such specs.

- What works: the runs in `docs/BENCHMARKS.md`, measured on the reference
  rig below.
- What's not in scope: other tiers and hardware — untested here, no support
  implied.
- What's planned: the PENDING rows in `docs/BENCHMARKS.md` — intent only, no
  timeline promised.

Reference rig (everything is tied to this box, per `README.md`): NVIDIA RTX
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
(source: `ud_inventory.json` — workspace-only, uncommitted).

"Support" per quant means (per master plan §2): (a) loader accepts the file
(tensor names, incl. MTP head tensors), (b) a dequant/compute path exists,
(c) an oracle test pins it (dequant vs ggml reference), (d) a VRAM budget row
says it fits. Live speed runs are NOT required per quant.

UD quants are mostly mixes of the K-family, so Q2_K→Q6_K coverage buys most of
the ladder; the IQ family (IQ1_*…IQ4_*) is a separate code path and comes second
(per master plan §2).

## 3. Per-file verdicts (UD ladder)

Header probe (`ud_probe.py` + `ud_inventory.json`, both workspace-only uncommitted, per master plan Phase 1 — workspace-only):
every probed file `LOADS` — all 753 (MTP page) / 733 (base page) tensors per
file use already-supported type ids, `missing: []` on every entry.
Decode status: 2026-10-17 probe found no unsupported type id; kernel need PENDING 228-cell oracle + live runs.

The UD ladder is 40 files: 19 UD files on the MTP page + 21 UD files on the
base page (source: `ud_inventory.json` key count — workspace-only, uncommitted). Four extra stock files
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
  IQ2_XXS/XS, IQ3_XXS/S, IQ2_S, IQ1_M. Correct; relative speed unmeasured — PENDING frozen-fixture A/B, no speed claim.
- Prefill CURRENT STATUS: poly-MMQ covers all but Q2_0 (42 tensors) /
  IQ1_M (29 tensors), which fall back to chunked native (GPU, fine); prefill
  `moe_mmq` path lacks K-quants → FP16 fallback (source: master plan Phase 1).

KV-oracle column: q4_0 pinned only for UD-Q4_K_M by poly_kv_q4_test 112,010 values + q4-001 REPORTED live run; all other cells PENDING. The full
228-cell cross-product oracle (19 weights × 3 KV × target/draft ×
decode/batch; integer tiers exact, IQ tiers epsilon; synthetic, no model file,
per `kv_weight_attend` design) is IN PROGRESS (source: master plan Phase 1).

### 3a. MTP page — 19 UD files (`unsloth/Qwen3.6-35B-A3B-MTP-GGUF`, 753 tensors each, all `LOADS`)

| File | Decode | Expert fast path (current status) | KV oracle |
| --- | --- | --- | --- |
| `UD-IQ1_M.gguf` | LOADS | IQ asymmetric pair (GU grouped / D single or vice versa), no wrong-answer failure observed via oracle arms; slower than symmetric — PENDING 228-cell oracle.; prefill IQ1_M falls back to chunked native where hit | 228-cell matrix IN PROGRESS |
| `UD-IQ2_M.gguf` | LOADS | IQ asymmetric pair, no wrong-answer failure observed via oracle arms; slower than symmetric — PENDING 228-cell oracle. | 228-cell matrix IN PROGRESS |
| `UD-IQ2_XXS.gguf` | LOADS | IQ2_XXS asymmetric pair, no wrong-answer failure observed via oracle arms; slower than symmetric — PENDING 228-cell oracle. | 228-cell matrix IN PROGRESS |
| `UD-IQ3_S.gguf` | LOADS | IQ3_S asymmetric pair (IQ2_S / IQ3_S family), no wrong-answer failure observed via oracle arms; slower than symmetric — PENDING 228-cell oracle. | 228-cell matrix IN PROGRESS |
| `UD-IQ3_XXS.gguf` | LOADS | IQ3_XXS asymmetric pair, no wrong-answer failure observed via oracle arms; slower than symmetric — PENDING 228-cell oracle. | 228-cell matrix IN PROGRESS |
| `UD-IQ4_NL.gguf` | LOADS | IQ4_NL asymmetric pair, no wrong-answer failure observed via oracle arms; slower than symmetric — PENDING 228-cell oracle. | 228-cell matrix IN PROGRESS |
| `UD-IQ4_XS.gguf` | LOADS | IQ asymmetric pair, no wrong-answer failure observed via oracle arms; slower than symmetric — PENDING 228-cell oracle. | 228-cell matrix IN PROGRESS |
| `UD-Q2_K_XL.gguf` | LOADS | Q2_K/Q3_K grouped MISSING, single-MMVQ only — PENDING | 228-cell matrix IN PROGRESS |
| `UD-Q3_K_M.gguf` | LOADS | Q2_K/Q3_K grouped MISSING, single-MMVQ only — PENDING | 228-cell matrix IN PROGRESS |
| `UD-Q3_K_XL.gguf` | LOADS | Q2_K/Q3_K grouped MISSING, single-MMVQ only — PENDING | 228-cell matrix IN PROGRESS |
| `UD-Q4_K_M.gguf` | LOADS | Q4_K asymmetric pair, no wrong-answer failure observed via oracle arms; slower than symmetric — PENDING 228-cell oracle.; live file: q4_0 KV pinned by poly_kv_q4_test + q4-001 REPORTED run — PENDING frozen-fixture A/B + log verification. | q4_0 DONE (q4-001); rest of matrix IN PROGRESS |
| `UD-Q4_K_S.gguf` | LOADS | Q4_K asymmetric pair, no wrong-answer failure observed via oracle arms; slower than symmetric — PENDING 228-cell oracle. | 228-cell matrix IN PROGRESS |
| `UD-Q4_K_XL.gguf` | LOADS | Q4_K asymmetric pair, no wrong-answer failure observed via oracle arms; slower than symmetric — PENDING 228-cell oracle. | 228-cell matrix IN PROGRESS |
| `UD-Q5_K_M.gguf` | LOADS | Q5_K grouped present; Q5_1 asymmetric where hit — IN PROGRESS; live MTP run q5-001 MEASURED 2026-10-18, previous engine; hyper re-run PENDING (max 63.4 / avg ~42.7, q4_0 + q4_0 draft, 69 turns; §5) | 228-cell matrix IN PROGRESS |
| `UD-Q5_K_S.gguf` | LOADS | Q5_K grouped present; Q5_1 asymmetric where hit — IN PROGRESS | 228-cell matrix IN PROGRESS |
| `UD-Q5_K_XL.gguf` | LOADS | Q5_K grouped present; Q5_1 asymmetric where hit — IN PROGRESS | 228-cell matrix IN PROGRESS |
| `UD-Q6_K.gguf` | LOADS | Q6_K grouped via `STRATA_Q6K_EXPERTS=ON` (confirmed ON) — CURRENT STATUS | 228-cell matrix IN PROGRESS |
| `UD-Q6_K_XL.gguf` | LOADS | Q6_K grouped via flag ON — CURRENT STATUS | 228-cell matrix IN PROGRESS |
| `UD-Q8_K_XL.gguf` | LOADS | Q8_0 path present — CURRENT STATUS (oracle-only, no live run planned) | 228-cell matrix IN PROGRESS |

### 3b. Base page — 21 UD files (`unsloth/Qwen3.6-35B-A3B-GGUF`, 733 tensors each, all `LOADS`)

| File | Decode | Expert fast path (current status) | KV oracle |
| --- | --- | --- | --- |
| `UD-IQ1_M.gguf` | LOADS | IQ asymmetric pair, no wrong-answer failure observed via oracle arms; slower than symmetric — PENDING 228-cell oracle.; prefill IQ1_M falls back to chunked native where hit | 228-cell matrix IN PROGRESS |
| `UD-IQ2_M.gguf` | LOADS | IQ asymmetric pair, no wrong-answer failure observed via oracle arms; slower than symmetric — PENDING 228-cell oracle. | 228-cell matrix IN PROGRESS |
| `UD-IQ2_XXS.gguf` | LOADS | IQ2_XXS asymmetric pair, no wrong-answer failure observed via oracle arms; slower than symmetric — PENDING 228-cell oracle. | 228-cell matrix IN PROGRESS |
| `UD-IQ3_S.gguf` | LOADS | IQ3_S asymmetric pair, no wrong-answer failure observed via oracle arms; slower than symmetric — PENDING 228-cell oracle. | 228-cell matrix IN PROGRESS |
| `UD-IQ3_XXS.gguf` | LOADS | IQ3_XXS asymmetric pair, no wrong-answer failure observed via oracle arms; slower than symmetric — PENDING 228-cell oracle. | 228-cell matrix IN PROGRESS |
| `UD-IQ4_NL.gguf` | LOADS | IQ4_NL asymmetric pair, no wrong-answer failure observed via oracle arms; slower than symmetric — PENDING 228-cell oracle. | 228-cell matrix IN PROGRESS |
| `UD-IQ4_NL_XL.gguf` | LOADS | IQ4_NL asymmetric pair, no wrong-answer failure observed via oracle arms; slower than symmetric — PENDING 228-cell oracle. | 228-cell matrix IN PROGRESS |
| `UD-IQ4_XS.gguf` | LOADS | IQ asymmetric pair, no wrong-answer failure observed via oracle arms; slower than symmetric — PENDING 228-cell oracle. | 228-cell matrix IN PROGRESS |
| `UD-Q2_K_XL.gguf` | LOADS | Q2_K/Q3_K grouped MISSING, single-MMVQ only — PENDING | 228-cell matrix IN PROGRESS |
| `UD-Q3_K_M.gguf` | LOADS | Q2_K/Q3_K grouped MISSING, single-MMVQ only — PENDING | 228-cell matrix IN PROGRESS |
| `UD-Q3_K_S.gguf` | LOADS | Q2_K/Q3_K grouped MISSING, single-MMVQ only — PENDING | 228-cell matrix IN PROGRESS |
| `UD-Q3_K_XL.gguf` | LOADS | Q2_K/Q3_K grouped MISSING, single-MMVQ only — PENDING | 228-cell matrix IN PROGRESS |
| `UD-Q4_K_M.gguf` | LOADS | Q4_K asymmetric pair, no wrong-answer failure observed via oracle arms; slower than symmetric — PENDING 228-cell oracle. | 228-cell matrix IN PROGRESS |
| `UD-Q4_K_S.gguf` | LOADS | Q4_K asymmetric pair, no wrong-answer failure observed via oracle arms; slower than symmetric — PENDING 228-cell oracle. | 228-cell matrix IN PROGRESS |
| `UD-Q4_K_XL.gguf` | LOADS | Q4_K asymmetric pair, no wrong-answer failure observed via oracle arms; slower than symmetric — PENDING 228-cell oracle. | 228-cell matrix IN PROGRESS |
| `UD-Q5_K_M.gguf` | LOADS | Q5_K grouped present; Q5_1 asymmetric where hit — IN PROGRESS | 228-cell matrix IN PROGRESS |
| `UD-Q5_K_S.gguf` | LOADS | Q5_K grouped present; Q5_1 asymmetric where hit — IN PROGRESS | 228-cell matrix IN PROGRESS |
| `UD-Q5_K_XL.gguf` | LOADS | Q5_K grouped present; Q5_1 asymmetric where hit — IN PROGRESS | 228-cell matrix IN PROGRESS |
| `UD-Q6_K.gguf` | LOADS | Q6_K grouped via flag ON — CURRENT STATUS; engine live run MEASURED 2026-10-18, previous engine; hyper re-run PENDING (§5) | 228-cell matrix IN PROGRESS |
| `UD-Q6_K_XL.gguf` | LOADS | Q6_K grouped via flag ON — CURRENT STATUS | 228-cell matrix IN PROGRESS |
| `UD-Q8_K_XL.gguf` | LOADS | Q8_0 path present — CURRENT STATUS (oracle-only, no live run planned) | 228-cell matrix IN PROGRESS |

Non-UD stock files (for completeness, source: `ud_inventory.json` — workspace-only, uncommitted): MTP-page
`MXFP4_MOE.gguf` (753 tensors, LOADS; MXFP4 single only — grouped PENDING),
MTP-page `Q8_0.gguf` (753 tensors, LOADS), base-page `MXFP4_MOE.gguf`
(733 tensors, LOADS; MXFP4 single only — grouped PENDING), base-page
`Q8_0.gguf` (733 tensors, LOADS).

Done when (per master plan Phase 1): every UD file's experts take a GPU kernel
path (grouped where it matters, single-MMVQ minimum) AND the 228-cell oracle
is green. That gate is IN PROGRESS.

## 4. GPU coverage

Measured target: sm_86 (RTX 3070 Ti Laptop, this tree). Turing–Blackwell via CUDA PTX is upstream design intent, untested in this fork — see TEST_REQUESTS.

- Integer quants use DP4A/integer math by design (upstream); verified on sm_86 only. Other RTX cards: untested in this fork, see TEST_REQUESTS.
- sm_86 is the optimized target: the reference rig is the RTX 3070 Ti Laptop
  (source: `README.md` hardware table).
- NVFP4/MXFP4 paths exist in-tree (MXFP4 MMVQ, NVFP4 expert path). No Blackwell run on THIS tree — PENDING, see TEST_REQUESTS.
- Pre-RTX/GTX: no run on THIS tree, not in test matrix. Untested, see TEST_REQUESTS — no support implied.

Phase 3 cuts AMD/HIP, Intel, and Metal paths (source: master plan Phase 3).
See §6 for what is explicitly out of scope.

## 5. Baselines

Status labels are part of the claim. Same rig for every comparison or it does
not count (per master plan testing doctrine): RTX 3070 Ti Laptop 8 GB +
32 GB DDR5, ctx 131072, temp 0.6, thinking cap 4096.

| Run | Status | Numbers (with rig/source) |
| --- | --- | --- |
| q4-001: UD-Q4_K_M live, `--kv q4_0 --draft-kv q4_0` | REPORTED (NOT log-verified) | 90.9 max / 46.3 low / ~55.6 avg tok/s over 48 turns; prefill 17,277 tokens in ~13 s (~1.33K tok/s); thinking cap hit once and wrapped; zero errors (source: `docs/KV_CACHE.md` + `README.md`, reference rig above). Kernel unit tests: key and value readers vs ggml oracle, 112,010 values, PASS (source: `docs/KV_CACHE.md`). |
| UD-Q6_K @131072 ctx, llama.cpp | BASELINE (not our run) | ≈30 tok/s on the owner rig (source: master plan Phase 2). Never tested on HyperStrata. Reference: ~30 tok/s (llama.cpp, same file/ctx, not our run), q4_0 KV. |
| UD-Q5_K_M with MTP, q5-001 (same harness as q4-001: fixed config, max/low/avg, prefill, zero-error bar) | MEASURED 2026-10-18, previous engine; hyper re-run PENDING | 63.4 max / 31.0 low / ~42.7 avg tok/s over 69 turns, `--kv q4_0 --draft-kv q4_0`; prefill 17,591 tokens in ~15 s (~1.17K tok/s); expert cache 16.4–64.3%; zero errors, zero cancel=True; game verdict (SHIP/punch-list) pending user (source: server log measured 2026-10-18). |
| UD-Q6_K without MTP, engine run q6-001 (same harness: fixed config, max/low/avg, prefill, expert cache) | MEASURED 2026-10-18, previous engine; hyper re-run PENDING | 55.8 max / 13.5 low / ~35.5 avg tok/s (mean of 159 done-line rates; wall secs include prefill on big-context turns) over 159 turns, `--no-draft`, NO MTP, + Vision server (mmproj-Q8_0, CPU encoder, `images on`); KV flags not stated in source; prefill 17,664 tokens in ~20 s (~0.88K tok/s) initial, late-run prefills 100+s (swap pressure, owner-diagnosed); expert cache 13.5–62.2%; max ctx 106,510/131,072, no OOM; no-Playwright game-build run; owner verdict (quoted): "Q6 worth using without Vision; usable with Vision too; lowest dip at swap page." SHIP/punch-list NOT stated (source: chat paste measured 2026-10-18; full console archive PENDING, not on file). |

Read the README table straight (source: `README.md`): fp16 KV did
91.9 max / 55.4 low / ~61.5 avg tok/s (41 turns), q8_0 KV did 86.5 max /
56.3 low / ~61.4 avg tok/s (48 turns), q4_0 KV did the q4-001 row above. LOGGED, log on file.
Quantization buys no decode speed here; maxima are single-turn peaks within
noise and averages come from different agent trajectories. What q4_0 buys is
room — see `docs/KV_CACHE.md` for the layer math.

Vision status 2026-10-18: functional on Q6_K base (q6-001 screenshot Q&A)
+ Q4_K_M MTP (screenshot Q&A) via mmproj-Q8_0 CPU encoder. Cross-quant
function INFERRED (encoder path independent of weight quant), per-quant
vision runs PENDING low priority.

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
  (source: `README.md` limitations).
- Pre-RTX / GTX: best-effort PTX fallback at most, never the test matrix
  (source: master plan §1b).
- Live speed runs per quant: we do NOT live-test every quant. Oracle tests pin the tested cells on the tested revision; untested cells PENDING; live runs reserved for §2 live column, and the
  rest of the ladder ships `oracle-only` ships oracle-only; later community numbers, if any, are unverified extras, not promised. (sources: master plan Phases 1–2, Phase 5 harness plan).
- GLM / vision work: separate job, after the text ladder is green
  (source: master plan Phase 6). KV-knob facts for `glm5next`/`dsa_attend`
  non-coverage stay in `docs/KV_CACHE.md` / `docs/GLM.md`. Update
  2026-10-18 (q6-001 MEASURED, owner-reported chat paste, run ended):
  vision functionality confirmed on Q6_K base config
  (Qwen3.6-35B-A3B-UD-Q6_K, --no-draft, no MTP) + Vision (mmproj-Q8_0, CPU
  encoder), owner-verified output, WITH vision server on (`images on`),
  no-Playwright run — numbers recorded: 55.8 max / 13.5 low / ~35.5 avg
  decode tok/s over 159 turns, prefill 17,664 tok in ~20 s (~0.88K tok/s),
  expert cache 13.5–62.2%, max ctx 106,510/131,072 no OOM; full console
  archive PENDING (not on file). Q4-MTP +
   Vision test queued next. No verdict beyond the owner's quoted words (§5).

← [Prev](MODELS.md) · [Index](INDEX.md) · [Next](KV_CACHE.md) →
