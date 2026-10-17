# HyperStrata

My personal playground to learn engines and chase speed on cheap cards.

## What this is

This is a small experiment, run in the open.
I pull an engine apart to learn how it works.
The goal is fast answers on one cheap card.

## Models we cover

- `unsloth/Qwen3.6-35B-A3B-MTP-GGUF` `UD-Q4_K_M.gguf` — main file
- `unsloth/Qwen3.6-35B-A3B-MTP-GGUF` `UD-Q5_K_M.gguf` — second file
- `unsloth/Qwen3.6-35B-A3B-GGUF` `UD-Q6_K.gguf` — base file, now and then
- K2-Horizon 36B-A4B `IFM Q4_K_M` — next target, PENDING port, no run
- Vision helper `mmproj-Q8_0` — per-file runs PENDING, rest PENDING, see docs/MODELS.md.

## What we tested

- Pi 4-phase game build for load. Raw speed only, not game quality.
- House rig only: RTX 3070 Ti Laptop 8GB + 32GB RAM.
- Other agents untested. Other RTX cards untested.

## v1.0.0 runs

- Q4_K_M MTP: 95.7 / 40.0 / 51.3 tok/s, 42 turns, 31160 tok in 607s, prefill 1.5K cold, PASS. Log on file, LEDGER q4-002.
- Q5_K_M MTP: 70.9 / 29.8 / 41.1 tok/s, 61 turns, 29615 tok in 721s, prefill 1.1K cold, PASS. Log on file, LEDGER q5-002.
- Rig: ctx 131072, temp 0.6, cap 4096, reserve 256, 65C, 0.775V, 6 last cores.
- Full table in docs/BENCHMARKS.md. Logs on file.
- Q6_K base REPORTED PENDING, no log. Vision 19 turns NO VERDICT PENDING.

## Roadmap

- v1.0.1: K2 MoVA 36B A4B support plus a speed and bug sweep.
- I check other cards when I can. No promises.

## Install

- No ready engine here. You bring the weights.
- Ask any CLI agent to follow docs/HOW-TO-INSTALL.md, or do it by hand.
- Then build with BUILD_HYPER.bat.
- Site: https://theacidblack.github.io/HyperStrata/
- Install: https://theacidblack.github.io/HyperStrata/docs/HOW-TO-INSTALL.html

## Reports

- Public issues off. Talk in Dev discussions or owner Threads.
- Bring rig + server log. Fake numbers close the report.

## License

Fork of PolyStrata 0.1.44.1 (51dc2427), which comes from Strata.
- https://github.com/Niko1221/Strata
- https://github.com/VecSzn/PolyStrata
- https://github.com/theacidblack/HyperStrata
Upstream never approved, supported, or endorsed this. MIT.
Code fits all RTX by design, tested only on the house rig above.
