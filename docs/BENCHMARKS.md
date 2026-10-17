← [Prev](DETAILS.md) · [Index](INDEX.md) · [Next](TROUBLESHOOTING.md) →

# Qwen3.6-35B-A3B benchmarks on RTX 3070 Ti Laptop 8 GB VRAM: q4_0 KV cache runs

*Needs: a configured model · Gives: measured runs, rig and verdicts · Next: TROUBLESHOOTING.md*

The RTS bench builds a mini-RTS browser game (Three.js + vanilla JS/CSS/HTML) in four phases — scaffold + scene, units + orders, combat + objective, playability pass — ending in a SHIP READY verdict or a punch list. Fixed config for every run below: ctx 131072, temp 0.6, thinking cap 4096, 256 MiB VRAM reserve. KV format is the only variable. Rig for all runs: RTX 3070 Ti Laptop 8 GB VRAM + 32 GB DDR5.

## Results

Canonical table — README, [docs/MODELS.md ## Coverage](MODELS.md#coverage) and CHANGELOG link here, never retype numbers.

| Run | Date | Model file | KV | gen tok/s max/low/avg | Prefill | VRAM / experts | Verdict | Provenance |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| fp16-001 | 2026-10-16 | `UD-Q4_K_M.gguf` (MTP page) | fp16 (default, no `--kv` flag) | 91.9 / 55.4 / ~61.5 (41 turns) | ~1.4K tok/s (17.3K in ~12 s) | expert cache 15.7–67.3% | SHIP READY, all 4 phases DONE | Server log + transcript measured, recorded in run ledger |
| q8-001 | 2026-10-16 | `UD-Q4_K_M.gguf` (MTP page) | q8_0 (`--kv q8_0`) | 86.5 / 56.3 / ~61.4 (48 turns) | ~1.4K tok/s (17.0K in ~12 s) | expert cache 16.9–69.7% | SHIP READY, all 4 phases DONE | Server log + transcript measured, recorded in run ledger; one condensation resume mid-run, finished via resume rule |
| q4-001 | 2026-10-17 | `UD-Q4_K_M.gguf` (MTP page) | q4_0 + q4_0 draft (`--kv q4_0 --draft-kv q4_0`) | 90.9 / 46.3 / ~55.6 (48 turns) | 17,277 tok in ~13 s (~1.33K tok/s) | not recorded | REPORTED, zero errors REPORTED — NOT SHIP READY; raw server log NOT on file — treat as reported, not independently verified. Verdict PENDING log. |
| q5-001 | 2026-10-18 | `UD-Q5_K_M.gguf` (MTP page) | q4_0 + q4_0 draft (`--kv q4_0 --draft-kv q4_0`) | 63.4 / 31.0 / ~42.7 (69 turns) | 17,591 tok in ~15 s (~1.17K tok/s) | expert cache 16.4–64.3% | game verdict pending user, zero errors | server log measured 2026-10-18; game verdict (SHIP/punch-list) pending user |
| q6-001 | 2026-10-18 | `UD-Q6_K.gguf` (base page, no MTP, `--no-draft`) + Vision server (mmproj-Q8_0, CPU encoder, `images on`) | KV flags not stated in source | 55.8 / 13.5 / ~35.5 (159 turns, mean of done-line rates; wall secs include prefill on big-context turns) | 17,664 tok in ~20 s (~0.88K tok/s) initial; late-run prefills 100+s (swap pressure, owner-diagnosed) | expert cache 13.5–62.2%; max ctx 106,510/131,072 | Owner verdict (quoted): "Q6 worth using without Vision; usable with Vision too; lowest dip at swap page." SHIP/punch-list NOT stated | Chat paste measured 2026-10-18, no-Playwright game-build run; full console archive PENDING; reference ~30 tok/s (llama.cpp, same file/ctx) is NOT our run |
| q4v-001 | — | `UD-Q4_K_M.gguf` (MTP page) + Vision | not stated | peak 92.4 / 47.2 / — (19 turns pasted, partial) | not recorded | not recorded | NO verdict, unfinished — PENDING full run | README pasted turns, no log on file |

<details><summary>Run notes</summary>

Notes: fp16-001 hit the thinking cap once (4096, wrapped cleanly). q8-001 never hit the cap. q4-001 cap-hit claim UNVERIFIED (no log). q5-001 ran 69 turns (40,734 tokens / 954 s), context past 69K prompts, no condensation, zero cancel=True. Averages come from different agent trajectories, so cross-row avg/max deltas are noise, not rankings.

</details>

## Not yet run

| Model | Status | What the run must record |
| --- | --- | --- |
| Q4-MTP + Vision | PENDING | Same bench, fixed config; max/low/avg gen, prefill, VRAM/experts, SHIP/blocked verdict |
| Cyber-Tiel | PENDING | Same bench, fixed config; max/low/avg gen, prefill, VRAM/experts, SHIP/blocked verdict |
| All other Vision variants | INFERRED working, per-quant runs PENDING (low priority) | Vision functional confirmed 2026-10-18 on Q6_K base (screenshot Q&A, q6-001) + Q4_K_M MTP (screenshot Q&A); same mmproj-Q8_0 CPU-encoder path independent of weight quant, so cross-quant function is inferred not measured |

## How to report a run

Share a measurement from your own hardware so others can judge how the engine might run on a similar machine. One model, card, or context length is enough to contribute. A report is only useful when it can be checked, so:

- Attach your rig and the server log. Report the GPU and its memory, the CPU, the RAM and its speed, the OS, the driver and CUDA (or ROCm) version, the engine build or commit, the model file, and the exact command.
- Compare against a run of the same build done the same day. Numbers from a build and a card that were not measured together cannot be ranked.
- State what you did not test. Untested behavior stays untested; do not present it as working.
- Fake numbers close the report. A report from hardware the owner does not have is unverified here, so keep the log and do not guess.

The quickest report comes from the built-in benchmark (`BENCH.bat` / `./bench.sh`, or `tools/hyper_bench.py`), which writes a report in the format below. See [Run the benchmark](#run-the-benchmark).

### What to record

Include enough information for someone else to repeat your run:

- **Hardware:** GPU model, VRAM, selected GPUs for a multi-GPU run, CPU, installed
  RAM, and storage type. Include PCIe link speed and width, GPU power limits, and
  other workloads when known or relevant.
- **Software:** OS, engine commit and version, driver and CUDA version
  (or ROCm for HIP), and release binary or source build. Include changed build options.
- **Model:** exact repository and revision when available, quantization, GGUF
  filenames, and vision encoder if enabled. Identify custom packs, expert
  profiles, or draft vocabularies, with hashes or reproducible preparation steps.
- **Settings:** launch command and relevant run configuration, context limit, KV
  type and streaming window, expert cache, low-RAM mode, prefill size, CPU workers,
  MTP, reasoning effort, sampling, and vision. State whether calibration or
  experimental speed projection was enabled; attach the resulting settings.
- **Workload:** benchmark command or script, shareable prompts or instructions to
  build them, actual prompt and generated token counts, output cap, repetitions,
  and cache state for each run.

Use GB or GiB consistently and name the unit. Mark unavailable values explicitly
instead of guessing. Remove credentials and private content from configurations,
prompts, and logs before publishing them.

### How to measure

Start with a configuration that already works on your machine. You do not need to
download every model or test the largest context. A useful initial speed report
tests one short prompt and one longer prompt that fits, with a fixed output cap
(for example, 256 tokens). Record the actual output length if the model ends early.

For each configuration, try at least three measured runs and publish all of them
with a median and range. If you only have one run, say so. Explain any warm-up and
whether model loading is included in the timing.

The engine reuses conversation prefixes and adapts its expert cache. Separate prompts
that are fully processed from follow-ups that reuse tokens, and report the number
of reused and freshly read tokens from the engine log. Describe how you reset or
retained state between runs. A warmed expert cache and a reused prompt prefix are
different conditions; record both when possible.

Keep these measurements separate:

- **Prompt throughput:** freshly processed prompt tokens per second, from the
  engine's timing output. Keep the corresponding token count and duration.
- **Decode throughput:** generated tokens per second, from the engine's timing
  output. Count reasoning tokens too, or explain any different convention.
- **Time to first token:** seconds from sending the request to its first generated
  token. For a streaming API measurement, ignore keep-alives and empty deltas and
  state whether the first token is reasoning or answer text.
- **Total latency:** elapsed seconds for the whole request. State whether measured
  at the engine or client, and whether loading, vision encoding, or queueing is included.
- **Memory:** observed RAM and VRAM usage, including whether the value is a startup
  snapshot or a peak during inference. Report paging or failed requests, with the server-log excerpt.

Attach the engine timing lines and your measurement script if you used one. Avoid
calculating decode throughput by dividing generated tokens by total request time,
which also includes prompt processing. Cancelled or failed requests belong in the
report as failures, outside the successful throughput summary.

When comparing versions or settings, run both on the same machine and workload
and list every changed setting. Preserve the per-run results: output text and
draft acceptance can change speed even when the input is identical.

### Run the benchmark

The simplest report is one command after setup. Windows: double-click `BENCH.bat`. Linux: `./bench.sh`. Or
`python tools/hyper_bench.py` from the project folder. Setup also offers it at the end of an interactive install
(default no).

It starts the model with the settings you already have (the newest `strata-*.json`; `--config FILE` picks another one),
or uses a server that is already running on that port (`--url http://127.0.0.1:8080` for another address). It then
sends a fixed suite through the HTTP API, three runs each, and reports medians with ranges. The default takes about
3 to 10 minutes once the model has loaded:

| Test | What it sends | What it reports |
| --- | --- | --- |
| chat | three short questions, up to 100 new tokens | decode tok/s, time to first token |
| 4K and 16K prompt | a fresh word-list prompt of about 4,096 and 16,384 tokens (skipped when your context is smaller) | prompt tok/s, time to first token |
| agent session | a system prompt and four tools, then five turns that each add about 1,500 words of a project file and continue from the model's own earlier answers | tokens the conversation cache reused, tokens read again, time to first token per turn |
| `--clients N` | N requests at the same moment (the server needs `--batch` to run them together) | total tok/s, p50 and p95 first-token time and latency |
| `--long` | the prompt test at about 32,768 tokens | prompt tok/s, time to first token |

Speeds are the engine's own timings (`prompt_per_second`, `predicted_per_second` in the answer); first-token time and
latency are measured by the client. Thinking is off for the suite so the answers are short and the runs comparable
(`--think` leaves it on). Every prompt carries a unique tag, so the fresh-prompt tests never get help from the cache.
Close other programs first: a browser or a game changes the numbers.

The result goes to `bench/results/<date>-community-<gpu>-<model>/` (or `--out DIR`):

- `README.md` in the format below, with the GPU, VRAM, CPU, RAM, OS, driver, engine and version, model and
  quantization, context and your settings filled in from the machine and the server
- `results.json` with every request (tokens, timings, errors) and the machine facts
- `engine-log-excerpt.txt` with the engine's start-up lines (when the benchmark started the server)

Home folders, user and host names, local network addresses and API keys are replaced by placeholders before anything is
written. Read the report once before you share it: the benchmark cannot know what else you consider private.

To share, keep your folder with your log; no review service promised here. Add what the tool does not know: background load, power limits, storage.

To compare with an earlier run on the same machine (before and after an update or a setting change):

```bash
python tools/hyper_bench.py --compare bench/results/2026-10-09-community-rtx-3060-0141
python tools/hyper_bench.py --compare OLD_FOLDER --compare-only NEW_FOLDER   # two saved runs, nothing is started
```

The tool is a speed check, not a quality test. For recall and other checks, see [Optional correctness
checks](#optional-correctness-checks); for A/B tests of two builds use `tools/ab_engine.py`.

`python hyper.py benchmark --model <name> --prompt-tokens 24000`
gives the two speeds the README uses.

### Optional correctness checks

Speed measurements alone do not establish answer quality. Include a small check
relevant to your workload, such as a completed coding task with test results, a
tool call followed by its result, or an image question with an expected answer.

For long-context recall, the repository includes
[`tools/needle_bench.py`](../tools/needle_bench.py). With a running local server,
from the repository root:

```bash
python tools/needle_bench.py --help
python tools/needle_bench.py --url http://127.0.0.1:8080 --lengths 32k,128k --depths 10,50,90 --out needles.json
```

Choose lengths your configured context can hold. The script supports `--api-key`
for an authenticated server; keep that key out of published commands. Save the
reported actual prompt lengths, misses, errors, and skipped cases as well as
successes. A needle test measures recall on those inputs, not overall model quality.

### Report template

Copy this into your report's `README.md`, replace the placeholders, and remove
sections that do not apply. Use `not measured` for missing measurements; the table
contains no example performance numbers.

````markdown
# Community benchmark on GPU name

Measured on DATE by HANDLE. State what was tested and the main limitation.

## Hardware and software

- GPU and VRAM; CPU; installed RAM; storage; PCIe link if known:
- OS; driver; CUDA or ROCm:
- Engine commit; engine version; release binary or source build:
- Background workloads and any power limits:

## Model and configuration

- Model repository and revision; quantization; GGUF filenames:
- Vision encoder; custom packs or profiles:
- Context; KV type and streaming; cache; prefill; low-RAM mode:
- MTP; reasoning; sampling; calibration; experimental speed projection:

```text
Exact launch command and relevant configuration, with credentials removed.
```

## Method

Link prompts and scripts. Describe output cap, repetitions, warm-up, loading,
prompt reuse, expert-cache state, timing boundaries, and memory measurement.

## Results

| Configuration | Actual prompt tokens | Reused tokens | Generated tokens | Runs | Prompt tok/s median and range | Decode tok/s median and range | TTFT seconds median and range |
| --- | ---: | ---: | ---: | ---: | --- | --- | --- |
| Fill in measured values | | | | | | | |

Link per-run data and logs. Include total latency and memory observations when
measured, with units and timing boundaries. List failures and skipped cases.

## Correctness and limitations

Describe checks, expected answers, observed results, and untested behavior.
````

## Wanted hardware

The engine has been built and measured on a few machines. The wanted reports below are PENDING: no support is implied until the engine is measured on that hardware. Write a report as described in [How to report a run](#how-to-report-a-run): the build, the card and PCIe width, the CPU and RAM speed, the model file, the exact command, three runs per size, and what you did not test.

This list is edited by the owner when needed; items carry no response promise. An item stays until a report answers it.

Reports from 8 GB VRAM + 32 GB RAM jump the queue — that is the rig this is built for.

Better rigs are welcome to report, but they sit outside what this is for, so they get less attention.

6 GB VRAM + 32 GB RAM is welcome too — reports accepted, best-effort, nothing promised.

| Wanted | Why | Example of an earlier answer |
|---|---|---|
| Two NVIDIA cards of different sizes (a large and a small one), in both card orders, with `--layer-split auto` | There is no unequal pair here to check the split planner; the card order and the small card's free memory changed the split | #880, #1238 |
| Three or four NVIDIA GPUs | No multi-GPU NVIDIA rig of that size here | #880 |
| Pascal (sm_61: Tesla P40, P100, GTX 10 series) | Older cards run on the fp32 path and can behave differently from newer ones | #875, #876 |
| A Linux host with 32 GB of RAM or less | The expert file tier and the resident mode depend on how much of the model fits in RAM | #1194 |
| A long soak (an hour or more of back-to-back requests) on a split or resident setup | Shows crashes, drift and heat that a short run does not | #848 |

## Docs honesty

Every number above carries its source in the Provenance column. Numbers without a server log on file are marked reported, not measured. PENDING means code-done-unmeasured: the loader accepts the file, no live run has happened. No lemma or projected numbers appear here.

Test suite: PENDING. An earlier `ctest 121/122` figure (source: `README.md`) cannot be reproduced from this tree: `tests/CMakeLists.txt` is absent, so no current result is recorded here.

← [Prev](DETAILS.md) · [Index](INDEX.md) · [Next](TROUBLESHOOTING.md) →
