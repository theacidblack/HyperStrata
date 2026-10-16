<h1 align="center">HyperStrata</h1>

<p align="center"><b>A personal experiment in hardware-specific inference optimization</b><br>
Qwen3.6-35B-A3B on one RTX 3070 Ti Laptop GPU + 32 GB RAM · fork of PolyStrata</p>

> The translated READMEs ([简体中文](README.zh-CN.md) · [日本語](README.ja.md) ·
> [Deutsch](README.de.md) · [Français](README.fr.md) · [Español](README.es.md) ·
> [Português](README.pt-BR.md)) still describe upstream PolyStrata and lag behind
> this file. English here is authoritative for this fork.

## 1. Introduction

HyperStrata is a personal experimental fork of
[PolyStrata](https://github.com/VecSzn/PolyStrata) (itself built on
[Strata](https://github.com/Niko1221/Strata)). It exists to answer one question:
how far can an existing inference engine be pushed for **one model on one fixed
PC** — a laptop with an 8 GB card and 32 GB of RAM?

It is a learning project: hardware tuning, benchmarking, upstream
synchronization, and release maintenance, done in the open. It is not a competing
engine, not a product, and not a claim of authorship over upstream work.

## 2. Why this experiment exists

Running 35B-parameter mixture-of-experts models on an 8 GB laptop GPU means most
of the weights live in system RAM and stream to the card on demand. Throughput
then depends on quantization choice, KV-cache format, context length, and OS
memory behavior — all measurable, all specific to the machine. General advice
does not transfer; only numbers from this exact configuration count. This repo
collects those numbers and the small changes they justify.

## 3. Upstream projects and acknowledgments

- [Strata](https://github.com/Niko1221/Strata) by Niko1221 — the original engine,
  server, and web app. The Qwen engine, installer, server, and web UI in this
  repo are Strata's work, unchanged.
- [PolyStrata](https://github.com/VecSzn/PolyStrata) — the fork this repo derives
  from: the `strata-poly` engine that runs GLM/Qwen3.6/Ornith/Gemma/Tiel/MiniCPM-V
  and DeepSeek/LFM models from GGUF files, plus its docs and benchmarks.

The upstream authors did the engineering; this fork runs experiments on top of
it. Nothing here implies their approval, support, or endorsement. Upstream
improvements are merged back in (see section 10), never silently dropped.

## 4. Target hardware (verified on this machine)

| Part | Value | How verified |
| --- | --- | --- |
| GPU | NVIDIA GeForce RTX 3070 Ti Laptop GPU, 8 GB VRAM (Ampere SM86) | `nvidia-smi` |
| System RAM | 32 GB (2×16 GB), 4800 MT/s | `Win32_PhysicalMemory` (2 sticks, configured 4800) |
| OS | Windows 10/11, x86-64 | test machine |

Do not assume all 8 GB of VRAM or all 32 GB of RAM are usable — the OS, drivers,
and background load take their share. Every result below is tied to this box.

## 5. Target model and tested configuration

- Model: **Qwen3.6-35B-A3B** (MoE), Unsloth quant **`UD-Q4_K_M`**, single GGUF
  file of ~22.6 GB. The file already contains the draft (MTP) block, so
  speculative decoding needs no extra download.
- Runtime: `strata-poly` engine via `setup.py --family qwen36 --model UD-Q4_K_M`.
- Tested context: **131072** tokens (inside the trained 262144, no RoPE rescaling).
- KV cache: `q4_0` (half the memory of default `int8`) for long contexts on 8 GB.
- The GGUF is reused from a local copy (`--gguf-dir`); weights are never committed
  to this repo. Check the model card's license before redistributing any weights.

## 6. Objectives and non-goals

Objectives: maximal practical tokens/s (prompt intake and generation) for the
above model/config on the above hardware; predictable VRAM/RAM peaks; reliable
long-context runs; honest numbers with same-day unmodified baselines.

Non-goals: general-purpose engine improvements, other GPUs, other models beyond
smoke tests, code rewrites for their own sake, and any claim of universal gains.
A change that helps this laptop but hurts the general case stays a local
experiment, documented as such.

## 7. Current implementation and verified results

Status: the fork's history is currently identical to upstream — no engine changes
yet, so there are **no fork-specific measurements to report**. The numbers in
upstream's docs (including the speed tables below in the original README text)
are upstream's, measured on their machines — cited, not claimed.

What works here today is upstream's stack, installed by its installer and run
with the configuration in section 5. First local benchmarks will appear here
with model, quant, context, workload, and baseline once they exist. Failed
experiments will be listed too, not buried.

## 8. Installation and reproduction

Standard path (unchanged upstream installer):

```
Windows:  START-HERE.bat --yes --family qwen36 --model UD-Q4_K_M --context 131072 --kv q4_0 --gguf-dir <folder-with-the-gguf> --no-start
Linux:    ./setup.sh     --yes --family qwen36 --model UD-Q4_K_M --context 131072 --kv q4_0 --gguf-dir <folder-with-the-gguf> --no-start
```

`--gguf-dir` reuses a file you already have instead of downloading (~22.6 GB).
Setup writes a `run-*.bat`/`run-*.sh` starter plus `strata-*.json` config (both
git-ignored, machine-local). Then start the generated script and open
`http://127.0.0.1:8080`. Full installer docs: `docs/INSTALL.md`, `docs/AI_SETUP.md`.

Note: `UD-Q4_K_M` wants roughly 27 GB of RAM beside an 8 GB card — tight on
32 GB. If setup warns, use the smaller `UD-Q2_K_XL` instead.

## 9. Known limitations and failed experiments

- No fork experiments have run yet, so there is no failure list — this section
  stays honest rather than full.
- Tight memory headroom (above): long contexts + large quant may fail on 32 GB;
  that is expected physics, not a bug.
- Tested on Windows + NVIDIA only. Linux, AMD, and multi-GPU paths are upstream's
  and untested here.
- Translated READMEs lag behind this file (see top note).

## 10. Upstream divergence tracking

Every substantial fork-vs-upstream difference is recorded in
[`UPSTREAM_DIVERGENCE.md`](UPSTREAM_DIVERGENCE.md) with reason, evidence,
upstream status, removal criteria, and validation steps. The sync procedure
(merge upstream, adapt patches, re-measure) is kept with it. Read it before
assuming any file here differs from upstream on purpose.

## 11. Licensing and third-party attribution

- Engine and docs: **MIT License**, copyright (c) 2026 Niko1221 and the Strata
  contributors — see [`LICENSE`](LICENSE), retained verbatim from upstream as the
  license requires. No new license file replaces it.
- `third_party/ggml` keeps its own license (see `third_party/ggml/LICENSE`); the
  font and the experimental speed-projection vector keep theirs (per upstream).
- Our original contributions (this README, future `DIV-*` patches) fall under the
  same MIT terms via the fork; they claim no ownership over upstream code.
- Model weights are downloaded at setup time, not shipped. The GGUF quant used
  here comes from the Unsloth quant family referenced in upstream's
  `docs/BACKENDS.md`; consult the model card for weight licensing before
  redistributing anything. No legal review has been performed — open an issue if
  anything here looks wrong.

## 12. Disclaimer

HyperStrata is independent personal experimentation. It does not compete with,
impersonate, or represent Strata or PolyStrata, and no endorsement by their
authors is implied or claimed. All performance statements are limited to the
hardware, model, and configuration they were measured on.
