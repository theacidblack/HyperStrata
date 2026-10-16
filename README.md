<h1 align="center">HyperStrata</h1>

<p align="center"><b>A personal experiment in hardware-specific inference optimization</b><br>
Qwen3.6-35B-A3B on one RTX 3070 Ti Laptop GPU + 32 GB RAM · fork of PolyStrata</p>

> Translated READMEs (`.de/.es/.fr/.ja/.pt-BR/.zh-CN`) still describe upstream
> PolyStrata and lag behind. English here is authoritative.

## 1. Introduction

HyperStrata is a personal experimental fork of
[PolyStrata](https://github.com/VecSzn/PolyStrata) (built on
[Strata](https://github.com/Niko1221/Strata)). One question: how far can an
existing inference engine be pushed for **one model on one fixed PC**? A learning
project, done in the open — not a competing engine, product, or authorship claim.

**How this project is run:** all cooking happens in the private
`HyperStrata-Dev` repo (`dev` → `main` for prep and testing). This public repo
holds only `main`, updated on releases — a new version lands here only with
measured results, and only when a record falls.

## 2. Why this experiment exists

On an 8 GB laptop GPU, 35B-MoE weights live in RAM and stream to the card.
Throughput then depends on quant, KV format, context, and OS memory behavior —
measurable only on this exact machine. General advice doesn't transfer; numbers
do. This repo collects those numbers and the small changes they justify.

## 3. Upstream and acknowledgments

- [Strata](https://github.com/Niko1221/Strata) — original engine, server, web app.
- [PolyStrata](https://github.com/VecSzn/PolyStrata) — the `strata-poly` GGUF
  engine this fork derives from, plus its docs and benchmarks.

Upstream did the engineering; this fork experiments on top. No approval,
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

## 6. Objectives and non-goals

Max practical tok/s for the above on the above; predictable VRAM/RAM peaks;
reliable long-context runs; honest same-day baselines. Not goals: general engine
work, other GPUs, rewrites for their own sake, universal-gain claims. A change
that helps this laptop but hurts the general case stays a local experiment.

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
assets keep their own licenses. Our additions claim no upstream ownership. No
legal review performed — open an issue if anything looks wrong.

## 12. Disclaimer

Independent personal work. Not affiliated with Strata or PolyStrata; no
endorsement implied. Performance statements cover only the measured hardware,
model, and configuration.
