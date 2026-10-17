[Index](docs/INDEX.md) · [Next](docs/HOW-TO-INSTALL.md) →

# HyperStrata

Run 35-billion-parameter models on your own PC: 8 GB VRAM + 32 GB RAM, NVIDIA, Windows. You bring the weights — the engine runs them, nothing leaves your machine.

Tested: Qwen3.6-35B-A3B in Q4_K_M and Q5_K_M (MTP versions), both measured with logs on file. Q6 and vision run too, not measured here.

Site (same docs, black theme): https://theacidblack.github.io/HyperStrata/

- [Features](#features)
- [Speed right now](#speed-right-now)
- [Install](#install)
- [Usage](#usage)
- [Docs](#docs)
- [Reports and security](#reports-and-security)
- [Where it started](#where-it-started)
- [License](#license)

## Features

- `hyper` engine for Qwen3.6-35B-A3B MoE, Windows + CUDA, source in `src/poly/`.
- q4_0 KV cache plus `--draft-kv` on MTP files, to fit 8 GB cards — sizes in [docs/KV_CACHE.md](docs/KV_CACHE.md).
- OpenAI- and Anthropic-compatible server plus a web app (`serve/`).
- Pictures: vision works.
- Every number below carries a log. Stock machines read lower.

## Speed right now

Speed only, measured with a game build as load. Never a quality score.

- Run: q4-002, 2026-10-17, Q4_K_M MTP, `--kv q4_0 --draft-kv q4_0`, 42 turns.
- Decode: 95.7 max / 40.0 low / 51.3 avg tok/s (31,160 tokens / 607 s).
- Prefill: 1.5K tok/s cold (18,557 tokens in ~12 s); warm re-reads are cache hits, not speed.
- Status: clean run, zero errors.
- Run: q5-002, 2026-10-17, Q5_K_M MTP, same flags, 61 turns.
- Decode: 70.9 max / 29.8 low / 41.1 avg tok/s (29,615 tokens / 721 s).
- Prefill: ~1.1K tok/s cold (18,617 tokens in ~17 s).
- Status: clean run, zero errors; slower than Q4, as expected from the bigger file.
- Bench: 65 C CPU, 0.775 V GPU, 6 last cores no SMT, ctx 131072, temp 0.6, cap 4096, 256 MiB reserve. Locked so runs reproduce.
- Full table and rig: [docs/BENCHMARKS.md](docs/BENCHMARKS.md).
- No log, no number. Llama's ~30 tok/s is not our run.

## Install

Follow [docs/HOW-TO-INSTALL.md](docs/HOW-TO-INSTALL.md). It checks your PC, runs setup, starts the server, ends with one chat. The agent method is tested with the Pi agent only. Weights are yours, never shipped.

## Usage

1. Start the server. It serves on `http://127.0.0.1:8080`.
2. Open that address, or point any OpenAI-compatible client at `http://127.0.0.1:8080/v1` (key `none`, model = the `"id"` from `/v1/models`).

House presets in `examples/` (`qwen36-q4_k_m.json` + `run-qwen36-q4_k_m.bat`, `-vision` variants, Q6_K base). Copy and edit them.

## Docs

- [docs/INDEX.md](docs/INDEX.md) — full doc map; start here.
- [docs/HOW-TO-INSTALL.md](docs/HOW-TO-INSTALL.md) — install, start and verify the server.
- [docs/MODELS.md](docs/MODELS.md) — which weight files load, and what they cost.
- [docs/KV_CACHE.md](docs/KV_CACHE.md) — `--kv` and `--draft-kv` formats and sizes.
- [docs/BACKENDS.md](docs/BACKENDS.md) — how the engine is put together.
- [docs/DETAILS.md](docs/DETAILS.md) — every measured number, the API and all settings.
- [docs/BENCHMARKS.md](docs/BENCHMARKS.md) — which runs were measured, on what rig.
- [docs/TROUBLESHOOTING.md](docs/TROUBLESHOOTING.md) — what to check when setup or inference fails.
- [--help.md](--help.md) — flags reference, checked against `hyper.exe --help`.
- [CHANGELOG.md](CHANGELOG.md) — release history.
- [UPSTREAM_DIVERGENCE.md](UPSTREAM_DIVERGENCE.md) — what this fork changes relative to upstream.
- [THIRD_PARTY.md](THIRD_PARTY.md) — third-party pieces and licenses.

## Reports and security

Pull requests are not accepted. Bug and bench reports are welcome: include the rig, the exact flags, and the full server log. Reports from hardware we don't have are unverified. Fake numbers close the report.

Security issues: use the repository Security tab, Report a vulnerability: [direct link](https://github.com/theacidblack/HyperStrata/security/advisories/new). Say what you found and how to reproduce it.

## Repo map

- `README.md` — entry, status, runs, install pointers
- `BUILD_HYPER.bat` / `SETUP.bat` / `chat.py` / `hyper.py` — build, setup, chat client
- `src/` + `include/` + `CMakeLists.txt` — `hyper` engine sources
- `serve/` — HTTP server, OpenAI/Anthropic API, vision helper (`server.py`)
- `tools/` — bench, inspect, pack, tokenizer, MCP helpers (`hyper_*.py`)
- `docs/` — full manual, start at docs/INDEX.md
- `examples/` — house Q4_K_M / Q5 / Q6 + vision configs and launchers
- `bench/` + `tests/` — micro-benches, `results/`, unit tests
- `CHANGELOG.md` / `UPSTREAM_DIVERGENCE.md` / `--help.md` — history, upstream diff, flags truth
- Never committed: `engine-hyper/` + `build-*/` outputs, `*.gguf` weights. One experimental weight file is tracked by mistake, pending removal.

## Where it comes from

Engine, server, installer, docs: [Strata](https://github.com/Niko1221/Strata) via [PolyStrata](https://github.com/VecSzn/PolyStrata). Upstream did the engineering; experiments happen here. No approval or endorsement implied. Upstream sync frozen at `0.1.44.1`.

## Where it started

- First house-model run: q4-001, 2026-10-17, 48 turns.
- Decode: 90.9 max / 46.3 low / ~55.6 avg tok/s.
- Prefill: 17,277 tokens in about 13 s (~1.33K tok/s).
- Status: REPORTED, raw server log NOT on file, not log-verified.
- This is the baseline every later release compares against.
- Full table and rig: [docs/BENCHMARKS.md](docs/BENCHMARKS.md).

## License

MIT, copyright (c) 2026 Niko1221 and the Strata contributors — [`LICENSE`](LICENSE) kept verbatim. Not affiliated, no endorsement. Numbers cover only the measured hardware, model, and config above.

[Index](docs/INDEX.md) · [Next](docs/HOW-TO-INSTALL.md) →
