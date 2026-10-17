← [Index](docs/INDEX.md) · [Next](docs/HOW-TO-INSTALL.md) →

# HyperStrata

Personal experiment: Qwen3.6-35B-A3B (MoE) GGUF on one 8 GB VRAM + 32 GB RAM box, Windows + CUDA.

House model: Unsloth UD-Q4_K_M with MTP — the engine-performance target. Fits 8+32, fast decode at 131072 ctx with room left for agent + browser on the same box. Release-tested on that file only; q5/q6/Vision measured occasionally, never gating.

## Status

v1.0.0 flag-plant. First runs since the first release; fp16-001 and q8-001 below are the baseline everything is judged against.

Rebuilt `hyper` engine passed a Q4_K_M MTP stability run 2026-10-17 (informal owner log, no run id; formal q4-002 PENDING). All table rows are previous-engine builds; re-runs on `hyper` PENDING before they carry over.

## What changed in v1.0.0

- One knob: `--kv q4_0` with independent `--draft-kv q4_0` on MTP configs (`fp16 | q8_0 | q4_0`; omitted is `fp16`, byte-identical to before).
- Full-context KV sizes: fp16 ≈2.75 / q8_0 ≈1.46 / q4_0 ≈0.77 GiB; measured reservation ≈0.91 GiB, delta unexplained (`docs/KV_CACHE.md`).
- Rebuilt `hyper` engine; Q4 stability 2026-10-17 informal owner log (prefill ~1400 tok/s, decode 60–90 easing ~45 at 57k ctx, zero errors); formal q4-002 PENDING.
- Honesty pass: REPORTED vs MEASURED split, every number provenanced, PENDING stays PENDING.
- Coverage truth: `docs/COVERAGE.md` states what loads, what ran, what is PENDING.
- Agent install path (`docs/HOW-TO-INSTALL.md` Method 1) + house launcher presets in `examples/`.

Full history: [`CHANGELOG.md`](CHANGELOG.md).

## Runs (same box, ctx 131072, temp 0.6, thinking cap 4096, 256 MiB reserve)

Agent builds a browser game start to finish — messy real decode, not a clean loop. Full rows: [`docs/BENCHMARKS.md`](docs/BENCHMARKS.md).

| run | model / quant | max | low | avg | provenance |
| --- | --- | --- | --- | --- | --- |
| fp16-001 | UD-Q4_K_M (MTP), fp16 KV | 91.9 | 55.4 | ~61.5 | 41 turns, log on file — baseline |
| q8-001 | UD-Q4_K_M (MTP), q8_0 KV | 86.5 | 56.3 | ~61.4 | 48 turns, log on file — baseline |
| q4-001 | UD-Q4_K_M (MTP), q4_0 KV + draft | 90.9 | 46.3 | ~55.6 | 48 turns, REPORTED, no log |
| q5-001 | UD-Q5_K_M (MTP), q4_0 KV + draft | 63.4 | 31.0 | ~42.7 | 69 turns, log on file, verdict pending |
| q6-001 | UD-Q6_K (base, no MTP) + Vision | 55.8 | 13.5 | ~35.5 | 159 turns, chat paste, archive PENDING, verdict NOT stated |
| q4v-001 | UD-Q4_K_M (MTP) + Vision | peak 92.4 | 47.2 | — (partial) | 19 turns pasted, unfinished, NO verdict |

Max is one turn, avg covers the run. REPORTED outranks nothing. Llama ~30 tok/s is not our run — never measured on this tree.

## How I run it

1. Agent (recommended): open a CLI agent in the source folder, follow [`docs/HOW-TO-INSTALL.md`](docs/HOW-TO-INSTALL.md) Method 1. It checks specs against [`docs/COVERAGE.md`](docs/COVERAGE.md) and finishes with server up + one chat.
2. Manual: same page, Method 2. Set `CUDA_ARCH` in `BUILD_HYPER.bat` to your card (`75` = RTX 20 · `86` = RTX 30 · `89` = RTX 40 · `120` = RTX 50).

House presets in [`examples/`](examples/): `qwen36-q4_k_m.json` + `run-qwen36-q4_k_m.bat` (text), `-vision` variants, Q6_K base with `--no-draft` (no MTP head).

Serve: `http://127.0.0.1:8080`. Weights: Unsloth Qwen3.6-35B-A3B GGUFs, user-supplied, never committed.

## How does it work

`strata-poly` engine (`src/poly/`), arch `qwen35moe`, from GGUF files; pictures go through the `strata-vision` helper. The KV knob trims the conversation memory that OOMs 8 GB cards; weight quants, sampler defaults, server API shape unchanged.

Start: [`docs/HOW_IT_WORKS.md`](docs/HOW_IT_WORKS.md), then [`docs/DETAILS.md`](docs/DETAILS.md) and [`docs/KV_CACHE.md`](docs/KV_CACHE.md).

## Something went wrong

Read [`docs/TROUBLESHOOTING.md`](docs/TROUBLESHOOTING.md), then [`docs/FAQ.md`](docs/FAQ.md); flags truth is [`--help.md`](--help.md). Reports need rig + exact config and flags + full server log — see Reports below.

## Next

<details><summary>Pending gaps</summary>

- Formal q4-002 run (log + transcript + verdict) on the `hyper` engine.
- Q2_K/Q3_K grouped expert fast paths; 228-cell weight×KV oracle green.
- Per-quant VRAM table; frozen-fixture A/B (q4_0 experimental until then).
- q4 log, q4v full run + transcript, q5/q6 verdicts.
- ctest 121/122 (`ple_parity` fails by design, needs 20 GB fixture). Long contexts on 32 GB may fail.

</details>

## Docs

Full map: [`docs/INDEX.md`](docs/INDEX.md).

Start: [`docs/HOW-TO-INSTALL.md`](docs/HOW-TO-INSTALL.md) · [`docs/INSTALL.md`](docs/INSTALL.md) · [`docs/AI_SETUP.md`](docs/AI_SETUP.md)

Run: [`docs/MODELS.md`](docs/MODELS.md) · [`docs/COVERAGE.md`](docs/COVERAGE.md) · [`docs/KV_CACHE.md`](docs/KV_CACHE.md) · [`docs/Q8_KV.md`](docs/Q8_KV.md) · [`docs/BACKENDS.md`](docs/BACKENDS.md) · [`docs/DETAILS.md`](docs/DETAILS.md) · [`docs/HOW_IT_WORKS.md`](docs/HOW_IT_WORKS.md) · [`docs/IMAGES.md`](docs/IMAGES.md) · [`docs/MCP_SERVER.md`](docs/MCP_SERVER.md) · [`docs/BATCHING.md`](docs/BATCHING.md) · [`docs/MULTI_GPU.md`](docs/MULTI_GPU.md) · [`docs/LLAMA_SWAP.md`](docs/LLAMA_SWAP.md) · [`docs/LOGIT_BIAS.md`](docs/LOGIT_BIAS.md)

Measure: [`docs/BENCHMARKS.md`](docs/BENCHMARKS.md) · [`docs/COMMUNITY_BENCHMARKS.md`](docs/COMMUNITY_BENCHMARKS.md) · [`docs/TEST_REQUESTS.md`](docs/TEST_REQUESTS.md) · [`docs/TROUBLESHOOTING.md`](docs/TROUBLESHOOTING.md) · [`docs/FAQ.md`](docs/FAQ.md) · [`--help.md`](--help.md)

About: [`CHANGELOG.md`](CHANGELOG.md) · [`CONTRIBUTING.md`](CONTRIBUTING.md) · [`SECURITY.md`](SECURITY.md) · [`UPSTREAM_DIVERGENCE.md`](UPSTREAM_DIVERGENCE.md) · [`LICENSE`](LICENSE)

## Where it comes from

Engine, server, installer, docs: [Strata](https://github.com/Niko1221/Strata) via [PolyStrata](https://github.com/VecSzn/PolyStrata). Chain: Strata → PolyStrata → HyperStrata. Upstream did the engineering; experiments happen here. No approval or endorsement implied. Upstream sync frozen at `0.1.44.1`. Divergences: [`UPSTREAM_DIVERGENCE.md`](UPSTREAM_DIVERGENCE.md). Third-party pieces: [`THIRD_PARTY.md`](THIRD_PARTY.md).

## Repo map

- `README.md` — entry, status, runs, install pointers
- `BUILD_HYPER.bat` / `SETUP.bat` / `chat.py` / `hyper.py` — build, setup, chat client
- `src/` + `include/` + `CMakeLists.txt` — `hyper` engine sources
- `serve/` — HTTP server, OpenAI API, vision helper (`server.py`, `images.py`)
- `tools/` — bench, inspect, pack, tokenizer, MCP helpers (`hyper_*.py`)
- `docs/` — full manual, start at docs/INDEX.md
- `examples/` — house Q4_K_M / Q5 / Q6 + vision `.json` configs and `.bat` launchers
- `bench/` + `tests/` — micro-benches, `results/`, unit tests
- `CHANGELOG.md` / `UPSTREAM_DIVERGENCE.md` / `--help.md` — history, upstream diff, flags truth
- Local only, never committed: `engine-hyper/` + `build-*/` (build outputs); `*.gguf` + `data/` (user-local weights)

## Rig

RTX 3070 Ti Laptop 8 GB (Ampere sm_86) + 32 GB DDR5, Windows 10/11 x86-64. All numbers above from this box only. Assume neither the full 8 GB nor the full 32 GB is usable.

## License

MIT, copyright (c) 2026 Niko1221 and the Strata contributors — [`LICENSE`](LICENSE) kept verbatim. Agent-written additions claim no upstream ownership. Not affiliated, no endorsement. Numbers cover only the measured hardware, model, and config above.

## Reports

Reports: [to be decided by owner]. Include rig, exact config and flags, full server log — not an excerpt. No fix timeline. No PRs right now. Model requests fall under curation default-NO.

← [Index](docs/INDEX.md) · [Next](docs/HOW-TO-INSTALL.md) →