← [Prev](LOGIT_BIAS.md) · [Index](INDEX.md) · [Next](COMMUNITY_BENCHMARKS.md) →

*Needs: reference rig, finished bench runs · Gives: measured gen/prefill rates and SHIP verdicts · Next: COMMUNITY_BENCHMARKS.md*
*Run 13/13*

# Qwen3.6-35B-A3B benchmarks on RTX 3070 Ti Laptop 8 GB VRAM: q4_0 KV cache runs

The RTS bench builds a mini-RTS browser game (Three.js + vanilla JS/CSS/HTML) in four phases — scaffold + scene, units + orders, combat + objective, playability pass — ending in a SHIP READY verdict or a punch list. Fixed config for every run below: ctx 131072, temp 0.6, thinking cap 4096, 256 MiB VRAM reserve. KV format is the only variable. Rig for all runs: RTX 3070 Ti Laptop 8 GB VRAM + 32 GB DDR5.

## Results

Canonical table — README, COVERAGE and CHANGELOG link here, never retype numbers.

| Run | Date | Model file | KV | gen tok/s max/low/avg | Prefill | VRAM / experts | Verdict | Provenance |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| fp16-001 | 2026-10-16 | `UD-Q4_K_M.gguf` (MTP page) | fp16 (default, no `--kv` flag) | 91.9 / 55.4 / ~61.5 (41 turns) | ~1.4K tok/s (17.3K in ~12 s) | expert cache 15.7–67.3% | SHIP READY, all 4 phases DONE | Server log + transcript measured, recorded in run ledger |
| q8-001 | 2026-10-16 | `UD-Q4_K_M.gguf` (MTP page) | q8_0 (`--kv q8_0`) | 86.5 / 56.3 / ~61.4 (48 turns) | ~1.4K tok/s (17.0K in ~12 s) | expert cache 16.9–69.7% | SHIP READY, all 4 phases DONE | Server log + transcript measured, recorded in run ledger; one condensation resume mid-run, finished via resume rule |
| q4-001 | 2026-10-17 | `UD-Q4_K_M.gguf` (MTP page) | q4_0 + q4_0 draft (`--kv q4_0 --draft-kv q4_0`) | 90.9 / 46.3 / ~55.6 (48 turns) | 17,277 tok in ~13 s (~1.33K tok/s) | not recorded | REPORTED, zero errors REPORTED — NOT SHIP READY; raw server log NOT on file — treat as reported, not independently verified. Verdict PENDING log. |
| q5-001 | 2026-10-18 | `UD-Q5_K_M.gguf` (MTP page) | q4_0 + q4_0 draft (`--kv q4_0 --draft-kv q4_0`) | 63.4 / 31.0 / ~42.7 (69 turns) | 17,591 tok in ~15 s (~1.17K tok/s) | expert cache 16.4–64.3% | game verdict pending user, zero errors | server log measured 2026-10-18; game verdict (SHIP/punch-list) pending user |
| q6-001 | 2026-10-18 | `UD-Q6_K.gguf` (base page, no MTP, `--no-draft`) + Vision server (mmproj-Q8_0, CPU encoder, `images on`) | KV flags not stated in source | 55.8 / 13.5 / ~35.5 (159 turns, mean of done-line rates; wall secs include prefill on big-context turns) | 17,664 tok in ~20 s (~0.88K tok/s) initial; late-run prefills 100+s (swap pressure, owner-diagnosed) | expert cache 13.5–62.2%; max ctx 106,510/131,072, no OOM | Owner verdict (quoted): "Q6 worth using without Vision; usable with Vision too; lowest dip at swap page." SHIP/punch-list NOT stated | Chat paste measured 2026-10-18, no-Playwright game-build run; full console archive PENDING; reference ~30 tok/s (llama.cpp, same file/ctx) is NOT our run |

<details><summary>Run notes</summary>

Notes: fp16-001 hit the thinking cap once (4096, wrapped cleanly). q8-001 never hit the cap. q4-001 cap-hit claim UNVERIFIED (no log). q5-001 ran 69 turns (40,734 tokens / 954 s), context past 69K prompts, no condensation, zero cancel=True. Averages come from different agent trajectories, so cross-row avg/max deltas are noise, not rankings.

</details>

## Not yet run

| Model | Status | What the run must record |
| --- | --- | --- |
| Q4-MTP + Vision | PENDING | Same bench, fixed config; max/low/avg gen, prefill, VRAM/experts, SHIP/blocked verdict |
| Cyber-Tiel | PENDING | Same bench, fixed config; max/low/avg gen, prefill, VRAM/experts, SHIP/blocked verdict |
| All other Vision variants | INFERRED working, per-quant runs PENDING (low priority) | Vision functional confirmed 2026-10-18 on Q6_K base (screenshot Q&A, q6-001) + Q4_K_M MTP (screenshot Q&A); same mmproj-Q8_0 CPU-encoder path independent of weight quant, so cross-quant function is inferred not measured |

## Docs honesty

Every number above carries its source in the Provenance column. Numbers without a server log on file are marked reported, not measured. PENDING means code-done-unmeasured: the loader accepts the file, no live run has happened. No lemma or projected numbers appear here.

← [Prev](LOGIT_BIAS.md) · [Index](INDEX.md) · [Next](COMMUNITY_BENCHMARKS.md) →
