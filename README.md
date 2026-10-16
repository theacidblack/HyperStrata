<h1 align="center">HyperStrata</h1>

<p align="center"><b>An agentic playground for inference speed</b><br>
Qwen3.6-35B-A3B on one RTX 3070 Ti Laptop GPU + 32 GB RAM · fork of PolyStrata</p>

> Translated READMEs (`.de/.es/.fr/.ja/.pt-BR/.zh-CN`) still describe upstream
> PolyStrata and lag behind. English here is authoritative.

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

## 2. Why this experiment exists

On an 8 GB laptop GPU, 35B-MoE weights live in RAM and stream to the card.
Throughput then depends on quant, KV format, context, and OS memory behavior —
measurable only on this exact machine. General advice doesn't transfer; numbers
do. This repo collects those numbers and the small changes they justify.

## 3. Upstream and acknowledgments

- [Strata](https://github.com/Niko1221/Strata) — original engine, server, web app.
- [PolyStrata](https://github.com/VecSzn/PolyStrata) — the `strata-poly` GGUF
  engine this fork derives from, plus its docs and benchmarks.

Upstream did the engineering; agents experiment on top of it here. No approval,
support, or endorsement by them is implied. Upstream improvements are merged in
(see section 10), never silently dropped.

## 4. Target hardware (verified)

| Part | Value |
| --- | --- |
| GPU | NVIDIA RTX 3070 Ti Laptop, 8 GB VRAM (Ampere SM86) |
| RAM | 32 GB (2×16 GB) DDR5-4800 |
| OS | Windows 10/11, x86-64 |

Assume neither the full 8 GB nor the full 32 GB is usable. Every result is tied
to this box.

## 5. Target model and configuration

**Qwen3.6-35B-A3B**, Unsloth **`UD-Q4_K_M`** GGUF (~22.6 GB, draft block
included), via `setup.py --family qwen36`. Tested context **131072**, KV fp16
(engine default). Weights are reused locally (`--gguf-dir`), never committed;
check the model card before redistributing any.

## 6. Method and non-goals

The loop: hypothesis → try different flags or agent-written code changes →
benchmark against a same-day unmodified baseline → keep it if faster, revert and
record it if it regresses. Precise compatibility wins (exact GPU, exact RAM,
exact model) over portable ones. Crazy ideas welcome; unmeasured ones don't ship.

Not goals: general engine work, other GPUs, rewrites for their own sake,
universal-gain claims. A change that helps this laptop but hurts the general
case stays a local experiment, documented as such.

## 7. Verified results

No fork measurements published yet — this section fills in when the first
battery completes, with model, quant, context, workload, and baseline per number.
Upstream's tables in their docs are theirs: cited, not claimed. Failures get
listed too, not buried.

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
