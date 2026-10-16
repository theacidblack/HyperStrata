<h1 align="center">HyperStrata</h1>

<p align="center"><b>An agentic playground for inference speed</b><br>
Qwen3.6-35B-A3B on one RTX 3070 Ti Laptop GPU + 32 GB RAM · fork of PolyStrata</p>

## 1. Introduction

HyperStrata is a personal experimental fork of
[PolyStrata](https://github.com/VecSzn/PolyStrata) (built on
[Strata](https://github.com/Niko1221/Strata)). It exists for one question: how
far can an existing inference engine be pushed for **one model on one fixed PC**?

It is a purely agentic project: AI agents write the code, run the benchmarks,
and iterate — and I am learning inference engines by watching and steering them.
All cooking happens in the private `HyperStrata-Dev` repo. This public repo
holds only `main` and gets periodic pushes whenever something is found that
makes inference faster. I try all my "what ifs" here, just for the love of the game.

## 2. Verified results

Two same-day baselines, identical config except KV format — ctx 131072,
temperature 0.6, thinking cap 4096, 256 MiB VRAM reserve, Pi agent building a
playable browser RTS to completion (2026-10-16, recipe in section 3):

| Metric | fp16 KV (engine default) | q8_0 KV |
| --- | --- | --- |
| Generation | 91.9 max / 55.4 low / ~61.5 avg tok/s (41 turns) | 86.5 max / 56.3 low / ~61.4 avg tok/s (48 turns) |
| Prompt intake, fresh reads | ~1.4K tok/s (17.3K tokens / ~12 s) | ~1.4K tok/s (17.0K tokens / ~12 s) |
| Thinking cap 4096 | hit once, wrapped cleanly, run finished | never hit |

Averages are identical — no measurable KV effect on sustained decode here. The
fp16 max is ~6% higher, but those are single-turn peaks on two different agent
trajectories, so that gap is variance, not a claim. Speed falls as context
grows — that curve is still the record to beat. No loops, no crashes, no
cancellations. Failures get listed here too, not buried. Full per-request
breakdown lives in the private dev repo's test ledger.

## 3. How a baseline is made

Every number in section 2 comes from the same recipe:

**Engine.** Built once with the repo installer (`START-HERE.bat --yes
--family qwen36 --model UD-Q4_K_M --context 131072 --gguf-dir
<folder-with-the-gguf> --no-start`), reusing a local GGUF file. The generated
`run-*.bat` starts `strata-poly` on `127.0.0.1:8080` (OpenAI- and
Anthropic-compatible APIs).

**Launcher.** At launch, a temporary config is derived from the untouched source
config, injecting `"sampling": {"temperature": 0.6}` and
`"reasoning_budget_tokens": 4096`, plus `--vram-reserve-mib 256`. The default
run uses engine-default fp16 KV (no `--kv` flag); the comparison run adds only
`--kv q8_0`. KV format is the only variable between runs.

**Test.** Same day, same machine, same model file. A Pi agent session builds a
small browser game start to finish with zero mid-run interaction. All metrics —
gen max/low/avg, prefill, TTFT where present, totals, VRAM peaks — come from
the server log only, never estimated or invented. The 4096 thinking cap exists
because an uncapped run once looped dead at ~35% context on a ~16k thinking
sink.

## 4. Why this experiment exists

On an 8 GB laptop GPU, 35B-MoE weights live in RAM and stream to the card.
Throughput then depends on quant, KV format, context, and OS memory behavior —
measurable only on this exact machine. General advice doesn't transfer; numbers
do. This repo collects those numbers and the small changes they justify.

## 5. Target hardware (verified)

| Part | Value |
| --- | --- |
| GPU | NVIDIA RTX 3070 Ti Laptop, 8 GB VRAM (Ampere SM86) |
| RAM | 32 GB (2×16 GB) DDR5-4800 |
| OS | Windows 10/11, x86-64 |

Assume neither the full 8 GB nor the full 32 GB is usable. Every result is tied
to this box.

## 6. Target model and configuration

**Qwen3.6-35B-A3B**, Unsloth **`UD-Q4_K_M`** GGUF (~22.6 GB, draft block
included), via `setup.py --family qwen36`. Tested context **131072**, KV fp16
(engine default) with q8_0 compared same-day. Weights are reused locally (`--gguf-dir`), never committed;
check the model card before redistributing any.

## 7. Method and non-goals

The loop: hypothesis → try different flags or agent-written code changes →
benchmark against a same-day unmodified baseline → keep it if faster, revert and
record it if it regresses. Precise compatibility wins (exact GPU, exact RAM,
exact model) over portable ones. Crazy ideas welcome; unmeasured ones don't ship.

Not goals: general engine work, other GPUs, rewrites for their own sake,
universal-gain claims. A change that helps this laptop but hurts the general
case stays a local experiment, documented as such.

## 8. Installation

```
START-HERE.bat --yes --family qwen36 --model UD-Q4_K_M --context 131072 --gguf-dir <folder-with-the-gguf> --no-start
```

Then run the generated `run-*.bat`, open `http://127.0.0.1:8080`. Details:
`docs/INSTALL.md`, `docs/AI_SETUP.md`. `UD-Q4_K_M` wants ~27 GB RAM beside 8 GB
VRAM — tight on 32 GB; `UD-Q2_K_XL` if setup warns.

## 9. Limitations

No fork experiments published yet. Windows + NVIDIA only here; Linux/AMD/multi-GPU
paths are upstream's, untested in this fork. Long contexts on 32 GB may fail —
physics, not a bug.

## 10. Divergence tracking

Substantial fork-vs-upstream differences live in
[`UPSTREAM_DIVERGENCE.md`](UPSTREAM_DIVERGENCE.md) (reason, evidence, removal
criteria, validation). Read it before assuming any file differs on purpose.

## 11. Licensing and attribution

MIT, copyright (c) 2026 Niko1221 and the Strata contributors — [`LICENSE`](LICENSE)
retained verbatim, never replaced. `third_party/ggml` and the font/projection
assets keep their own licenses. Agent-written additions claim no upstream
ownership. No legal review performed — open an issue if anything looks wrong.

## 12. Disclaimer

Independent personal work, built with AI agents. Not affiliated with Strata or
PolyStrata; no endorsement implied. Performance statements cover only the
measured hardware, model, and configuration.
