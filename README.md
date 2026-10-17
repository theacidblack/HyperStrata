<h1 align="center">HyperStrata</h1>

<p align="center">Qwen3.6-35B-A3B (MoE) GGUF on one 8 GB VRAM card + 32 GB RAM, Windows + CUDA. Personal experiment, one rig.</p>

What I run: Unsloth UD quants q4–q6, house model Q4_K_M MTP. Fork of PolyStrata.

My experience with 8 GB VRAM and 32 GB RAM covers a lot of the 35B-A3B space. I keep what runs well on these specs. Nothing else is covered, nothing else promised.

## Status

Interactive learning of the repo, covering my needs first. No optimization yet, no new models yet.

## What changed here

One knob: `--kv q4_0` with an independent `--draft-kv q4_0` on MTP configs. KV cache is what the model remembers about your conversation — on 8 GB that's what OOMs, not the weights. Full-context sizes: fp16 ≈2.75 / q8_0 ≈1.46 / q4_0 ≈0.77 GiB; measured reservation ≈0.91 GiB, delta unexplained ([`docs/KV_CACHE.md`](docs/KV_CACHE.md)).

What it doesn't change: weight quants, sampler defaults, server API shape.

## Runs (same box, ctx 131072, temp 0.6, thinking cap 4096, 256 MiB reserve)

Agent builds a browser game start to finish — messy real decode, not a clean loop. Full rows: [`docs/BENCHMARKS.md`](docs/BENCHMARKS.md).

| run | model / quant | max | low | avg | provenance |
| --- | --- | --- | --- | --- | --- |
| fp16-001 | UD-Q4_K_M (MTP), fp16 KV | 91.9 | 55.4 | ~61.5 | 41 turns, log on file |
| q8-001 | UD-Q4_K_M (MTP), q8_0 KV | 86.5 | 56.3 | ~61.4 | 48 turns, log on file |
| q4-001 | UD-Q4_K_M (MTP), q4_0 KV + draft | 90.9 | 46.3 | ~55.6 | 48 turns, REPORTED, no log |
| q5-001 | UD-Q5_K_M (MTP), q4_0 KV + draft | 63.4 | 31.0 | ~42.7 | 69 turns, log on file, verdict pending user |
| q6-001 | UD-Q6_K (base, no MTP), KV n/s + Vision | 55.8 | 13.5 | ~35.5 | 159 turns, chat paste, archive PENDING, verdict NOT stated |
| q4v-001 | UD-Q4_K_M (MTP), q4_0 KV + draft + Vision | 92.4 | 47.2 | — (partial) | 19 turns pasted, unfinished, NO verdict |

Max is one turn, avg covers the run. REPORTED outranks nothing. No dates on anything PENDING.

PENDING: q4v full run + transcript, q5/q6 verdicts, q4 log, per-quant VRAM table, Q2_K/Q3_K fast paths, frozen A/B (q4_0 experimental until then). ctest 121/122 (`ple_parity` fails by design, needs a 20 GB fixture). Long contexts on 32 GB may fail.

## Install

1. Agent (recommended): open your preferred CLI agent in the source folder, ask it to build the engine for your specs and install a model. It checks your specs and asks what it needs. Script for it: [`docs/HOW-TO-INSTALL.md`](docs/HOW-TO-INSTALL.md).
2. Manual: same page, Method 2. Presets: [`examples/`](examples/).

Specs outside [`docs/COVERAGE.md`](docs/COVERAGE.md) get no guarantee — untested by me, and this repo changed enough to break anything outside my scope.

## Rig

RTX 3070 Ti Laptop 8 GB (Ampere sm_86) + 32 GB DDR5, Windows 10/11 x86-64. Numbers above are from this box only. Assume neither the full 8 GB nor the full 32 GB is usable.

## Where it comes from

Engine, server, installer, docs: [Strata](https://github.com/Niko1221/Strata) via [PolyStrata](https://github.com/VecSzn/PolyStrata). Chain: Strata → PolyStrata → HyperStrata. Upstream did the engineering; agents experiment here. No approval or endorsement implied. Upstream sync frozen at `0.1.44.1`. Details per divergence: [`UPSTREAM_DIVERGENCE.md`](UPSTREAM_DIVERGENCE.md).

Weights: Unsloth Qwen3.6-35B-A3B GGUFs, stored locally, never committed. Ships no weights — get them from Hugging Face. Installer-downloaded code is outside what this fork audited; fetch at your own risk, check the model card.

## License

MIT, copyright (c) 2026 Niko1221 and the Strata contributors — [`LICENSE`](LICENSE) kept verbatim. Agent-written additions claim no upstream ownership. Not affiliated, no endorsement. Performance statements cover only the measured hardware, model, and config above.

## Reports

Bug reports welcome: what you ran, hardware, exact config and flags, full server log — not an excerpt. No fix timeline. No PRs right now. Model requests fall under curation default-NO.
