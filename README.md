<h1 align="center">HyperStrata</h1>

<p align="center"><b>How fast can one model go on one fixed laptop?</b><br>
Qwen3.6-35B-A3B on a single RTX 3070 Ti (8 GB) + 32 GB RAM · fork of PolyStrata</p>

## The idea

HyperStrata is a personal experiment: take an existing inference engine and
squeeze it for exactly one model on exactly one PC. No new architecture, no
claims about other hardware — just measure, change one thing, measure again,
and keep whatever survives contact with the numbers.

The work is done by AI agents. I supervise: every number below was checked
before it shipped, and negative results get published, not buried.
Development happens in the private `HyperStrata-Dev` repo; this public repo
holds only `main` and moves when something is found that actually helps.

## What we've measured

Same model, same machine, same config — only the KV-cache format changes
(ctx 131072, temp 0.6, thinking cap 4096, 256 MiB VRAM reserve). The workload
is an agent building a playable browser RTS start to finish, so the numbers
are messy real-world decode, not a clean benchmark loop:

| | fp16 KV (engine default) | q8_0 KV | q4_0 KV, both knobs (ours) |
| --- | --- | --- | --- |
| Generation | 91.9 max / 55.4 low / ~61.5 avg tok/s (41 turns) | 86.5 max / 56.3 low / ~61.4 avg tok/s (48 turns) | 90.9 max / 46.3 low / ~55.6 avg tok/s (48 turns) |
| Prompt intake | ~1.4K tok/s (17.3K tokens / ~12 s) | ~1.4K tok/s (17.0K / ~12 s) | ~1.33K tok/s (17.3K / ~13 s) |
| Thinking cap 4096 | hit once, wrapped, finished | never hit | hit once, wrapped, finished |

Read it straight: quantization buys **no decode speed here**. The maxima are
single-turn peaks within noise, and the averages come from different agent
trajectories, so don't compare them turn-for-turn. What q4_0 buys is
**room** — 288 bytes a row (28% of fp16, 53% of q8_0), so at ~78K context the
KV cache holds ≈1.9 GiB instead of ≈6.7 (fp16) or ≈3.6 (q8_0). On an 8 GB
card, that room is the difference between experts staying in VRAM and
spilling to CPU. That last claim still needs a frozen-fixture A/B before it
counts as a result — until then q4_0 stays opt-in and the default is
untouched.

## How a number gets here

**Engine.** Built from this repo with the stock installer, reusing a locally
stored model file. The launcher starts `strata-poly` on `127.0.0.1:8080`
(OpenAI- and Anthropic-compatible).

**Launcher.** Each run derives a throwaway config from the untouched source:
temperature 0.6, thinking budget 4096, 256 MiB VRAM reserve. The runs differ
in exactly one flag — the KV format. Nothing else moves.

**Test.** Same day, same machine, same model file. An agent session builds a
small browser game with zero mid-run interaction. Every metric comes from
the server log — never estimated. The 4096 thinking cap exists because an
uncapped run once looped dead at ~35% context on a 16K thinking sink.

**Policy.** Hypothesis → change flags or code → benchmark against a same-day
unmodified baseline → keep it if faster, revert and record it if it
regresses. Nothing ships unmeasured.

## What's ours

One engine change so far: a **q4_0 KV cache for the poly engine plus an
independent `--draft-kv` knob** for the draft block (upstream strata-poly has
neither — fp16 or q8_0, one knob for everything). The quantization is a
straight transcription of ggml's `quantize_row_q4_0`, covered by a
round-trip test against the ggml oracle (101,770 values) and one full live
run with zero errors. Details in [`UPSTREAM_DIVERGENCE.md`](UPSTREAM_DIVERGENCE.md)
(DIV-002) and `docs/KV_CACHE.md`.

## Hardware (everything is tied to this box)

| Part | Value |
| --- | --- |
| GPU | NVIDIA RTX 3070 Ti Laptop, 8 GB VRAM (Ampere SM86) |
| RAM | 32 GB (2×16 GB) DDR5-4800 |
| OS | Windows 10/11, x86-64 |

Assume neither the full 8 GB nor the full 32 GB is usable. Nothing here
claims to generalize.

## Install

```
START-HERE.bat --yes --family qwen36 --model UD-Q4_K_M --context 131072 --gguf-dir <folder-with-the-gguf> --no-start
```

Then run the generated `run-*.bat` and open `http://127.0.0.1:8080`.
Details: `docs/INSTALL.md`, `docs/AI_SETUP.md`. `UD-Q4_K_M` wants ~27 GB RAM
beside 8 GB VRAM — tight on 32 GB; use `UD-Q2_K_XL` if setup warns.

## Where things come from

- Engine, server, web app, installer, docs: [Strata](https://github.com/Niko1221/Strata)
  via [PolyStrata](https://github.com/VecSzn/PolyStrata) (`strata-poly` engine).
  Upstream did the engineering; agents experiment on top here. No approval,
  support, or endorsement by them is implied. Upstream improvements are merged
  in, never silently dropped — see [`UPSTREAM_DIVERGENCE.md`](UPSTREAM_DIVERGENCE.md).
- Model weights: Unsloth **`UD-Q4_K_M`** GGUF of **Qwen3.6-35B-A3B** (~22.6 GB),
  stored locally and reused — never committed anywhere. Check the model card
  before redistributing.
- Test workload: the agent builds its own small browser game from locally
  available libraries. No benchmark data ships with this repo.

## Limitations

Our q4_0 work is unit-tested and live-run once; the frozen-fixture A/B is
still pending, so treat it as experimental. Windows + NVIDIA only here;
Linux, AMD, and multi-GPU paths are upstream's and untested in this fork.
Long contexts on 32 GB may fail — physics, not a bug.

## License and disclaimer

MIT, copyright (c) 2026 Niko1221 and the Strata contributors — [`LICENSE`](LICENSE)
retained verbatim, never replaced. Agent-written additions claim no upstream
ownership. Independent personal work, built with AI agents; not affiliated
with Strata or PolyStrata, no endorsement implied. Performance statements
cover only the measured hardware, model, and configuration above.
