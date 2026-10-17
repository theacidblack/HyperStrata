<h1 align="center">HyperStrata</h1>

<p align="center">Local LLM inference experiment: Qwen3.6-35B-A3B Mixture-of-Experts (MoE) GGUF on one 8 GB VRAM card + 32 GB RAM, Windows + CUDA.</p>

<p align="center">Personal experiment on one rig. What ran, what is PENDING, and the logs behind each row.</p>

What I run: Qwen3.6-35B-A3B (Unsloth UD quants, q4–q6) on one 8 GB card + 32 GB RAM.
Where it came from: fork of PolyStrata.

Why Hyper: Hyper = High Performance (Hi-Per → Hyper): tok/s on cheap cards. Hyper + Strata (the OG engine name) = HyperStrata: speed per VRAM dollar.

My personal experience with 8 GB VRAM and 32 GB RAM covers many models on the 35B-A3B architecture, and here I want to bring only what runs well on such specs.

## What this is

1. One rig: RTX 3070 Ti Laptop 8 GB + 32 GB RAM, Windows x86-64, CUDA. No other hardware measured here.
2. Model scope: Qwen3.6-35B-A3B Unsloth UD quants q4–q6. Curation default is NO — only what I run is covered, nothing else implied.
3. Engine change recorded here: one knob — `--kv q4_0` with an independent `--draft-kv q4_0` on MTP configs (DIV-002). Weight quant format, sampler defaults, server API shape: unchanged.
4. What this is not: general model support, cross-platform support, or a performance claim.
5. Current stage: interactive learning of repo structure and covering my needs first. No optimization work yet, no new models yet — map first, then cut.

## Architecture

```text
GGUF weights (Unsloth UD q4–q6)
  -> loader (per-expert cache, VRAM reserve 256 MiB)
  -> attention: q4_0 KV + draft KV (DIV-002)
  -> sampler (temp 0.6, thinking cap 4096)
  -> serve/server.py (OpenAI-compatible, 127.0.0.1:8080)
```

- `--kv q4_0` applies to KV activation rows, orthogonal to weight quant by design.
- `--draft-kv q4_0` applies to MTP draft layers only. Q6 configs here use `--no-draft`.
- Vision encoder runs on CPU in the Vision configs here (`"gpu": false`, 0 VRAM).
- Reference decode context for the runs below: ctx 131072, temp 0.6, thinking cap 4096, `--vram-reserve-mib 256`.

## q4_0 KV cache on 8 GB VRAM (plain words)

Weights are what the model knows. KV cache is what it remembers about YOUR conversation. On 8 GB VRAM the remembering is what OOMs this rig, not the knowing. This fork adds a q4_0 KV cache plus an independent `--draft-kv` knob for the draft block, so the remembered part takes less memory.

Full-context sizes by block arithmetic: fp16 ≈2.75 / q8_0 ≈1.46 / q4_0 ≈0.77 GiB. On the q4-001 rig the reservation measured ≈0.91 GiB — the 0.91-vs-0.77 delta is unexplained, see `docs/KV_CACHE.md`. Fixed pair in the live q4/q5 runs below: q4_0 KV + q4_0 draft KV.

## Runs on RTX 3070 Ti Laptop 8 GB (tok/s)

Same model, same machine, same config — only the KV-cache format changes. The workload is an agent building a playable browser game start to finish, so the numbers are messy real-world decode, not a clean benchmark loop. Full rows: `docs/BENCHMARKS.md`.

| run-id | model / quant | max (tok/s) | low (tok/s) | avg (tok/s) | prefill | provenance |
| --- | --- | --- | --- | --- | --- | --- |
| fp16-001 | UD-Q4_K_M, fp16 KV | 91.9 | 55.4 | ~61.5 | ~1.4K tok/s (17.3K tokens / ~12 s) | 41 turns, 35,603 tok/579s, `docs/BENCHMARKS.md` |
| q8-001 | UD-Q4_K_M, q8_0 KV | 86.5 | 56.3 | ~61.4 | ~1.4K tok/s (17.0K / ~12 s) | 48 turns, 26,388 tok/430s, `docs/BENCHMARKS.md` |
| q4-001 | UD-Q4_K_M, q4_0 KV + q4_0 draft | 90.9 | 46.3 | ~55.6 | ~1.33K tok/s (17.3K / ~13 s) | 48 turns, REPORTED not log-verified, `docs/BENCHMARKS.md` |
| q5-001 | UD-Q5_K_M, q4_0 KV + q4_0 draft | 63.4 | 31.0 | ~42.7 | 17,591 tokens in ~15 s (~1.17K tok/s) | 69 turns, 40,734 tok/954s, expert cache 16.4–64.3%, game verdict pending user, `docs/BENCHMARKS.md` |
| q6-001 | UD-Q6_K base (no MTP, `--no-draft`) + Vision (mmproj-Q8_0, CPU encoder) | 55.8 | 13.5 | ~35.5 | 17,664 tok in ~20 s (~0.88K tok/s) initial; late prefills 100+s (swap pressure, owner-diagnosed) | 159 turns, KV flags not stated, expert cache 13.5–62.2%, max ctx 106,510/131,072 no OOM; MEASURED from chat paste 2026-10-18, console archive PENDING; game SHIP/punch-list NOT stated; llama.cpp ~30 tok/s ref is NOT our run, `docs/BENCHMARKS.md` |
| q4v-001 | UD-Q4_K_M MTP + Vision (mmproj-Q8_0, CPU encoder) | 92.4 | 47.2 | — (partial) | 14,548 tok in ~11 s (~1.3K tok/s) | 19 turns pasted 2026-10-18, game unfinished, transcript pending, NO verdict, `docs/BENCHMARKS.md` |
| *PENDING Cyber-Tiel* | *game verdict* | *—* | *—* | *—* | *—* | *SHIP/punch-list pending user* |

Max is one turn; avg covers the whole run. REPORTED rows are not log-verified and outrank nothing. MEASURED metrics come from the server log — except q6-001, measured from chat paste (console archive PENDING). Nothing here is estimated; no dates promised on anything PENDING.

## How I run it

Two ways. Both start with the source code on your PC.

1. Agent (recommended, no effort): open a CLI agent in the source folder and
   ask it to build the engine for your specs and install a model. It checks
   your GPU/VRAM/RAM, asks what it needs, and predicts the edge cases. Full
   script for it: `docs/HOW-TO-INSTALL.md` Method 1.
2. Manual: read `docs/HOW-TO-INSTALL.md` Method 2, then open the files it
   points at. Start from `examples/model.example.json` +
   `examples/run-model.example.bat` — 3 paths to fill in each header.
   Then run the generated `run-*.bat` and open `http://127.0.0.1:8080`.

```
START-HERE.bat --yes --family qwen36 --model UD-Q4_K_M --context 131072 --gguf-dir <folder-with-the-gguf> --no-start
```

`UD-Q4_K_M` is tight on 32 GB beside 8 GB VRAM; use `UD-Q2_K_XL` if setup warns. Specs outside `docs/COVERAGE.md` — or outside what upstream once covered — get no guarantee: nothing out there was tested by me, and this repo changed enough that anything outside my scope can be broken. That's defined, not discovered.

Build and test procedure used here (Windows PowerShell 5.1, MSVC build env for compiles):

```powershell
cmd /c '"<BuildTools>\VC\Auxiliary\Build\vcvars64.bat" >nul && ninja -j 8'
# workdir: build-poly (MSVC env required, ~340 targets)

ctest
# workdir: build-poly (122 tests, ~4 min)

python serve/server.py --config <model>.json
# server listens on 127.0.0.1 only; no 0.0.0.0 without explicit --api-key setup
```

Temp lives in JSON `sampling`, not engine flags. Vision needs the `"vision"` section + `hyper-vision.exe`. The engine binary does not serve HTTP by itself; `serve/server.py` sits in front and the JSON carries engine args.

## Docs

- `docs/COVERAGE.md` — per-model truth: what is covered and what is PENDING.
- `docs/BENCHMARKS.md` — runs table with provenance per row.
- `docs/KV_CACHE.md` — KV accounting: 0.77 arithmetic / 0.91 measured, delta unexplained.
- `docs/MODELS.md` — which weight file each config expects.
- `docs/INSTALL.md` — how setup runs on this rig and what it warns about.
- `UPSTREAM_DIVERGENCE.md` — per-divergence reason, evidence, and removal criteria.

## Status

Ran on this rig: q4_0 KV cache + independent `--draft-kv` knob ([`CHANGELOG.md`](CHANGELOG.md), DIV-002); oracle arms on post-f72a15e9 rebuild (112,010 + 91,530 values, 5/5 pairs, 6/6 rows); ctest 121/122 green (`ple_parity` fails by design without the 20 GB Q2_0 fixture); the five measured runs + one partial in the table above.

PENDING, no dates: per-quant VRAM budget table; q6 console archive; q4v full run + transcript; q5/q6 game verdicts; q4-001 raw log (stays REPORTED until then); grouped expert kernels for Q2_K/Q3_K; cross-product oracle matrix; frozen-fixture A/B — the q4_0 work is experimental until that lands. Long contexts on 32 GB may fail — physics, not a bug.

Limits: Windows + NVIDIA + CUDA on the reference rig only; no support promised, reports read and acknowledged, no fix timeline; loader-verified is not speed-tested.

## Hardware: RTX 3070 Ti Laptop 8 GB VRAM + 32 GB RAM, Windows 11, CUDA

| Part | Value |
| --- | --- |
| GPU | NVIDIA RTX 3070 Ti Laptop, 8 GB VRAM (Ampere SM86) |
| RAM | 32 GB (2×16 GB) DDR5-4800 |
| OS | Windows 10/11, x86-64 |

One person's machine. Numbers above are from this box only; nothing here says what other cards do. Assume neither the full 8 GB nor the full 32 GB is usable.

## Where things come from

- Engine, server, web app, installer, docs: [Strata](https://github.com/Niko1221/Strata) via [PolyStrata](https://github.com/VecSzn/PolyStrata). Chain: Strata → PolyStrata → HyperStrata. Upstream did the engineering; agents experiment on top here. No approval, support, or endorsement by them is implied. Upstream sync is frozen at `0.1.44.1`, no further syncs — see [`UPSTREAM_DIVERGENCE.md`](UPSTREAM_DIVERGENCE.md).
- Model weights: Unsloth **`UD-Q4_K_M`** GGUF of **Qwen3.6-35B-A3B**, stored locally and reused — never committed anywhere. This repo ships no weights: get the file yourself from Hugging Face. What the installer downloads, and any model-install code inherited from PolyStrata/Strata, is outside what this fork has audited — fetch and run weights at your own risk, under their licenses. Check the model card before redistributing.
- Test workload: the agent builds its own small browser game from locally available libraries. No benchmark data ships with this repo.

## License and disclaimer

MIT, copyright (c) 2026 Niko1221 and the Strata contributors — [`LICENSE`](LICENSE) retained verbatim, never replaced. Agent-written additions claim no upstream ownership. Independent personal work, built with AI agents; not affiliated with Strata or PolyStrata, no endorsement implied. Performance statements cover only the measured hardware, model, and configuration above.

## Community

Reports welcome. Include what you ran, your hardware, the exact config and flags, and the full server log — not an excerpt. Negative results are welcome, not buried. No fix timeline is promised, and model requests fall under the curation default-NO above.

## A note on tone

If something here sounds edgy — sorry. This repo is maintained ~100% by AI
agents, and they can be impolite and write weird stuff. I'm working on keeping
it clean, but things slip through: contradictions, stale docs, leftover edge.
The repo is big, I'm one person, and this isn't a mainstream engine for public
use — it's a playground for experimenting and learning how engines work, how to
maintain them, and how to optimize them.
