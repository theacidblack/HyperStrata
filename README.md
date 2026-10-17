[Index](docs/INDEX.md) · [Next](docs/HOW-TO-INSTALL.md) →

# HyperStrata

Local AI chat on your own PC. One NVIDIA card, 8 GB VRAM, 32 GB RAM. Nothing leaves your machine.

Tested with two model files: Qwen3.6-35B-A3B in Q4_K_M and Q5_K_M (MTP versions). Both measured here, logs on file. Bigger sizes and vision run too, but no release depends on them.

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

- `hyper` engine for Qwen3.6-35B-A3B MoE (architecture `qwen35moe`), Windows + CUDA, source in `src/poly/`.
- q4_0 KV cache with an independent `--draft-kv` knob on MTP files (`fp16 | q8_0 | q4_0`); q4_0 is the fit knob for 8 GB cards — sizes in [docs/KV_CACHE.md](docs/KV_CACHE.md).
- House quants measured here, both gated per release: UD-Q4_K_M MTP + UD-Q5_K_M MTP.
- OpenAI- and Anthropic-compatible server plus a web app (`serve/`).
- Pictures: vision tested and working (build notes in CHANGELOG Known issues).
- Every published number carries a log and the locked bench spec; stock machines read lower.

## Speed right now

Raw speed under optimal settings from a load-generating game build — never a capability score. The build simulates agentic workflow to generate load; what is recorded is completion + speed + errors.

- Engine binary: `hyper` (`src/poly/`, arch `qwen35moe`), built 2026-10-17.
- Model file: `UD-Q4_K_M.gguf` (MTP page).
- Flags: `--kv q4_0 --draft-kv q4_0`.
- Run: q4-002, 2026-10-17, 42 turns.
- Decode: 95.7 max / 40.0 low / 51.3 avg tok/s (31,160 tokens / 607 s).
- Prefill: 1.5K tok/s cold (18,557 tokens in ~12 s); warm re-reads to 66.0K on cache hits.
- Status: LOGGED, zero errors, consistent with q4-001 within noise.
- Q5 run: q5-002, 2026-10-17, 61 turns.
- Q5 decode: 70.9 max / 29.8 low / 41.1 avg tok/s (29,615 tokens / 721 s).
- Q5 prefill: ~1.1K tok/s cold (18,617 tokens in ~17 s); warm re-reads excluded (cache hits).
- Q5 status: LOGGED, zero errors; slower decode than Q4 as expected from the bigger quant; different session, band not ranking.
- Bench CPU: locked 65 C all day via UXTU (below full potential, never throttles).
- Bench GPU: undervolt 0.775 V, +1000 mem, ≈512 GB/s on the 256-bit bus (150 W card); hotspot ≈70 C under heavy load, no overheat.
- Bench cooling/cores: NVMe stays cool, no throttle; engine `--threads 6` + Process Lasso mask on the 6 last physical cores, no SMT.
- Bench config: ctx 131072, temp 0.6, thinking cap 4096, 256 MiB VRAM reserve; locked so runs reproduce, no throttle in any logged run.
- Full table and rig: [docs/BENCHMARKS.md](docs/BENCHMARKS.md).
- REPORTED outranks nothing. No number counts until it carries a log. Llama ~30 tok/s is not our run — never measured on this tree.

## Install

Follow [docs/HOW-TO-INSTALL.md](docs/HOW-TO-INSTALL.md). It checks the PC against the model by VRAM and RAM, runs setup, starts the server, and ends with one chat. The agent method is tested with the Pi agent only (others untested, no guarantee); the manual method is on the same page. Weights are Unsloth Qwen3.6-35B-A3B GGUFs, user-supplied, never committed.

## Usage

1. Start the server. It serves on `http://127.0.0.1:8080`.
2. Open that address for the web app, or point an OpenAI- or Anthropic-compatible client at it (base URL `http://127.0.0.1:8080/v1`, key `none`, model = the `"id"` from `/v1/models`).

House presets live in `examples/`: `qwen36-q4_k_m.json` with `run-qwen36-q4_k_m.bat` (text), `-vision` variants, and a Q6_K base with `--no-draft` (no MTP head). They are examples to copy and edit, not the canonical install path.

## Docs

- [docs/INDEX.md](docs/INDEX.md) — full doc map; start here.
- [docs/HOW-TO-INSTALL.md](docs/HOW-TO-INSTALL.md) — install, start and verify the server.
- [docs/MODELS.md](docs/MODELS.md) — which weight files load, and what they cost in VRAM and RAM.
- [docs/KV_CACHE.md](docs/KV_CACHE.md) — `--kv` and `--draft-kv` formats and their sizes.
- [docs/BACKENDS.md](docs/BACKENDS.md) — how the engine is put together per model family.
- [docs/DETAILS.md](docs/DETAILS.md) — every measured number, the API and all settings.
- [docs/BENCHMARKS.md](docs/BENCHMARKS.md) — which runs were measured, on what rig.
- [docs/TROUBLESHOOTING.md](docs/TROUBLESHOOTING.md) — what to check when setup or inference fails.
- [--help.md](--help.md) — flags reference, checked against `hyper.exe --help`.
- [CHANGELOG.md](CHANGELOG.md) — release history.
- [UPSTREAM_DIVERGENCE.md](UPSTREAM_DIVERGENCE.md) — what this fork changes relative to upstream.
- [THIRD_PARTY.md](THIRD_PARTY.md) — third-party pieces and their licenses.

## Reports and security

Pull requests are not accepted. The owner implements what covers many users, on the one reference rig, so every number stays measured here.

Bug and bench reports from people running it are welcome: include the rig, the exact config and flags, and the full server log (not an excerpt). A report from hardware the owner does not have is unverified here. Fake numbers close the report. No fix timeline is promised.

Report a security issue privately through the repository Security tab, Report a vulnerability: [direct link](https://github.com/theacidblack/HyperStrata/security/advisories/new). Include what you found, how to reproduce (request, config keys, version), and what an attacker gains. Owner triage, no backports, no promised timeline.

## Repo map

- `README.md` — entry, status, runs, install pointers
- `BUILD_HYPER.bat` / `SETUP.bat` / `chat.py` / `hyper.py` — build, setup, chat client
- `src/` + `include/` + `CMakeLists.txt` — `hyper` engine sources
- `serve/` — HTTP server, OpenAI/Anthropic API, vision helper (`server.py`)
- `tools/` — bench, inspect, pack, tokenizer, MCP helpers (`hyper_*.py`)
- `docs/` — full manual, start at docs/INDEX.md
- `examples/` — house Q4_K_M / Q5 / Q6 + vision `.json` configs and `.bat` launchers
- `bench/` + `tests/` — micro-benches, `results/`, unit tests
- `CHANGELOG.md` / `UPSTREAM_DIVERGENCE.md` / `--help.md` — history, upstream diff, flags truth
- Local only, never committed: `engine-hyper/` + `build-*/` (build outputs); `*.gguf` + user-local weights. Exception: one experimental GGUF is tracked under `data/experimental-speed-projection/` (untrack decision open — see CHANGELOG Known issues).

## Where it comes from

Engine, server, installer, docs: [Strata](https://github.com/Niko1221/Strata) via [PolyStrata](https://github.com/VecSzn/PolyStrata). Chain: Strata → PolyStrata → HyperStrata. Upstream did the engineering; experiments happen here. No approval or endorsement implied. Upstream sync frozen at `0.1.44.1`. Divergences: [`UPSTREAM_DIVERGENCE.md`](UPSTREAM_DIVERGENCE.md). Third-party pieces: [`THIRD_PARTY.md`](THIRD_PARTY.md).

## Where it started

- First house-model run: q4-001, 2026-10-17, 48 turns.
- Engine binary: `hyper` (`src/poly/`, arch `qwen35moe`).
- Model file: `UD-Q4_K_M.gguf` (MTP page).
- Flags: `--kv q4_0 --draft-kv q4_0`.
- Decode: 90.9 max / 46.3 low / ~55.6 avg tok/s.
- Prefill: 17,277 tokens in about 13 s (~1.33K tok/s).
- Status: REPORTED, raw server log NOT on file, not log-verified.
- This is the historical baseline every later release compares against.
- Full table and rig: [docs/BENCHMARKS.md](docs/BENCHMARKS.md).

## License

MIT, copyright (c) 2026 Niko1221 and the Strata contributors — [`LICENSE`](LICENSE) kept verbatim. Agent-written additions claim no upstream ownership. Not affiliated, no endorsement. Numbers cover only the measured hardware, model, and config above.

[Index](docs/INDEX.md) · [Next](docs/HOW-TO-INSTALL.md) →
