---
layout: default
title: Something went wrong?
---

← [Prev](BENCHMARKS.md) · [Index](INDEX.md) · [Next](../--help.md) →

# Something went wrong?

*Needs: symptom or error message · Gives: fixes for common install/start failures · Next: --help.md*
*Measure 3/6*

The common problems and what to do. Back to the [README](../README.md). The full table of
error messages, with the older engine fixes, is in the [details](DETAILS.md#troubleshooting).

> [!WARNING]
> 8 GB VRAM + 32 GB RAM is tight: long contexts can page to swap or fail. Close the server when done to free RAM.

## While installing or starting

**My PC froze, or got very slow, the first time HyperStrata started.**
That's normal while it starts, most of all the first time. HyperStrata loads 35-55 GB into your RAM, locks part of it for
the graphics card, and works out how much of the model fits on your GPU. The mouse can freeze for a few minutes.
**Wait, and don't close the window.** The next starts are much faster. Still frozen after 10 minutes? Restart the
PC, close other programs (browsers use a lot of RAM) and try again. If it keeps happening, pick a smaller size (UD-Q3_K_M
or UD-Q2_K_XL).

**It stopped while downloading or installing.**
Run `START-HERE.bat` (Linux: `./setup.sh`) again. It continues where it stopped.

**It says the NVIDIA driver is too old.**
Update it (NVIDIA App or [nvidia.com/drivers](https://www.nvidia.com/drivers)), restart the PC, and run
`START-HERE.bat` again.

**It says port 8080 is already in use.**
HyperStrata is already running. Look for its window. Or another program uses the port: `START-HERE.bat --port 8081`.
Only an address that is really taken says "already in use". Any other reason (Windows keeps the port reserved, or
the config's `"host"` is not an address of this PC) is printed as what the OS said, with what to change (#769).

**"Windows blocked the HyperStrata engine (or the image encoder) ... Smart App Control".**
On a clean Windows 11, Smart App Control refuses programs that are not signed, and HyperStrata's `hyper.exe` and
`hyper-vision.exe` are not (#735). Turn it off (Windows Security > App & browser control > Smart App Control
settings; it cannot be turned back on without a reset of Windows), or, if only the image encoder is blocked, run
`START-HERE.bat --setup --vision no`.

**A new engine misbehaves after an update.**
An update keeps the engine it replaced in `engine\.previous` (one generation, about 210 MiB).
`python setup.py --rollback-engine` puts it back (and keeps the newer one there: run it again to go forward) (#670).

**UPDATE.bat / update.sh say "git pull did not succeed", or "has no commit in common" (#1276).**
The repository's history was cleaned up on 2026-10-06, so a clone made before that cannot be updated with `git pull`.
The 0.1.40.2 scripts handle it: when no tracked file is edited they keep your old commits in the branch
`pre-cleanup-backup` and move the clone to the new history; otherwise they print the two commands for you. Your models,
settings and engine are untracked files and are never touched. An older UPDATE.bat / update.sh (before 0.1.40.2) cannot
do this itself: run these once in the HyperStrata folder, then use UPDATE.bat as usual (add `git stash` first if `git status`
lists edited files):

    git fetch origin
    git branch pre-cleanup-backup
    git checkout -B main origin/main

**Python or the build tools could not be installed.**
Install what it names (links are printed), then run it again. Everything already done is kept.

**Linux: the engine does not compile (`unsupported GNU version`, or `exception specification is incompatible` for
`cospi`/`sinpi`/`rsqrt`).** Two known mismatches between the CUDA toolkit and a new Linux (#601):
- gcc newer than 14 (Ubuntu 26.04's default 15): CUDA 12.x and 13.0 refuse it. Install g++-14 and run
  `CXX=g++-14 ./setup.sh` (CC, CXX and CUDAHOSTCXX name the build's compilers; setup passes them to CMake itself,
  so an already-configured build folder picks them up too, #1645).
- glibc 2.43 with CUDA 12.9: the toolkit's math headers clash with glibc's, already in CMake's first test compile.
  Use CUDA 12.8 or 13.x instead. Setup takes the newest toolkit it finds; `STRATA_NVCC=/usr/local/cuda-12.8/bin/nvcc
  ./setup.sh` makes it use that one (only that one).

**The first start takes minutes.**
It is reading 34-55 GB into RAM; the second start is faster while the files are in the OS cache. Started from Task
Scheduler, it can be 24x slower: see [Running it at startup](DETAILS.md#running-it-at-startup-task-scheduler).
On Linux the engine now asks the kernel to read the model files ahead (a cold start went from ~920 s to 70 s on one
PC; REPORTED, no log on file); `STRATA_READ_AHEAD=0` turns that off. On Linux with transparent huge pages on `always`, the arena no longer asks
for `MADV_HUGEPAGE` on top (a fragmented machine spent minutes compacting memory, #771); `STRATA_NO_ARENA_THP=1` skips
that request on any setting.

**Setup says the engine does not match GitHub's checksum.**
Setup checks the downloaded ready-made engine against the SHA-256 GitHub publishes. On a mismatch the file is corrupt or was changed on the way, so setup deletes it and stops; run setup again to download it afresh. If GitHub gives no checksum (offline, rate limited, your own mirror), setup warns and installs anyway. `STRATA_SKIP_SHA256=1` skips the check, only if you insist.

## While it answers

**It's very slow and the disk light keeps blinking.**
Your PC is out of free RAM. Close other programs, or pick a smaller size (UD-Q3_K_M or UD-Q2_K_XL).

**An answer stopped with "the engine stopped unexpectedly".**
Usually not enough RAM (on Linux the system then stops the engine). Just send your message again: HyperStrata starts the
engine by itself. If it keeps happening, close other programs or pick a smaller size.

**It says the prompt exceeds the context.**
The conversation is longer than the context you chose. Start a new chat, or run `SETUP.bat` and pick more
context. To let the server drop the oldest turns instead, add `"drop_oldest": true` to `strata-<model>.json`
and restart; it then leaves out the oldest question-and-answer turns until the prompt leaves room for the answer.

**It is slower than the tables.**
The monitor plugged into the graphics card and other GPU programs take VRAM from the expert cache; RAM running below
its rated speed (enable EXPO/XMP in the BIOS) slows the CPU half.

**An earlier reply that came back empty is gone from the chat history (0.1.40, #843).**
The server leaves out finished assistant turns that have no text, so the next reply does not copy the empty ones.
`STRATA_KEEP_EMPTY_TURNS=1` in the server's environment renders them as before.

**A Turing card (RTX 20) reads prompts above ~90K tokens differently (0.1.40, #743).**
The prompt's top-k selection takes a wider kernel there, with the same ids. `STRATA_TOPK_STREAM=0` restores the old one.

**Decode is several times slower with `--expert-cache N` than with `auto` (#781, #831).**
An explicit N is a byte budget that is not checked against the free VRAM once the slots are written. On a card it
fills (the log says `0 MiB of VRAM free with everything loaded - LOW`), Windows pages the GPU's memory and decode drops
from 100 to 14 tok/s with only a few hundred slots too many (REPORTED, no log on file). Use `--expert-cache auto`, or a smaller N.

**A low-RAM PC (about 32 GB) stalls with `--resident-budget-gib N` (#649).**
The budget is pinned (locked) in RAM, which leaves the OS and the CPU workers little room. `STRATA_RESIDENT_PIN=0` keeps
it pageable; a smaller budget works too.

**Decode got slower in 0.1.40 on a 32 GB PC with `--resident-budget-gib N`, and the drive is read hard (#1194, #1116, #1085).**
The start line "the file tier reads unbuffered" means the experts outside the RAM copy are read straight from the drive.
0.1.40 chose that whenever the free RAM could not hold all of them; but the same few come back token after token, and
a cache a twentieth of their size serves most of the repeats. 0.1.40.2 asks for that much (at least 1.5 GiB beside the
4 GiB headroom) and reads through the cache otherwise, which measured 7% to 118% faster decode with a third to a sixth of
the drive traffic on a 20 to 32 GB box (REPORTED, no log on file). On 0.1.40 / 0.1.40.1: `STRATA_UNBUFFERED_LOAD=0` does the same, `=1` forces the
unbuffered reads.

**Linux: the start is killed while the engine allocates the page-locked expert copy (#1250).**
Page-locked pages come from the driver in one go and cannot wait for the kernel to give back file cache, so with the
model files' cache in the way (little really free RAM, much "available") the host could run out. 0.1.40.2 gives back the
cached pages of the model files it mapped before it page-locks the copy; when the really free RAM still does not cover it,
it takes the pages in 1 GiB steps while the RAM available (cgroup limits included) stays above the headroom, page-locks
what it has, and keeps the rest resident but pageable with a warning ("only N of M GiB could be page-locked"): the answers
are the same, only the copies of the pageable part to the GPU go through the CPU. `STRATA_PIN_GUARD=0` turns this off,
`STRATA_PIN_RESERVE_GIB=N` sets the RAM that must stay free (default: `STRATA_RESIDENT_HEADROOM_GIB`, 4 GiB).

**The Windows display driver resets, then the PC blue-screens (0x141, then 0x116 in `nvlddmkm`) during an answer (#961).**
This is the NVIDIA driver, not HyperStrata: the report shows the same wedge with other CUDA programs on that driver (610.88,
RTX 4080 SUPER). Try another driver (a Studio one, or an older one) and a lower power limit; if it only happens with
`--spec 4`, `--spec 0` avoids the verify window while you wait for a driver fix.

**An RTX 50 card (a source build with CUDA 13.2) answers with nonsense, or reads prompts wrongly (#892, #968).**
CUDA 13.2.0 and 13.2.1's compiler (nvcc 13.2.51) miscompile some of the engine's kernels for sm_120: on our RTX 5070 the IQ2_S and IQ3_S
products are wrong (relative error 0.5 to 1.0 in the tests) with 13.2 and right with 13.0. The ready-made engine is built
with 13.0. If you compile it yourself, use CUDA 13.0, 13.1 or 13.2.2 (13.2.2, nvcc build 13.2.86, fixes it; an older one can sit next to a newer: `STRATA_NVCC=<path to its nvcc>`);
setup warns when it finds 13.2.0 or 13.2.1 for such a card, and takes an older 13.x when one is installed.

**Pictures are refused, or slow.**
"this server was started without the vision encoder": the model was set up for text only - run setup again with
`--vision gpu` (or `--vision cpu`). Pictures that take several seconds (about 3 s at 300 image tokens on 8 cores, more with more tokens) are read by the encoder on the CPU; `--vision gpu`
(NVIDIA, ~1.4 GB of VRAM) makes it 0.1-0.5 s (REPORTED, no log on file).

## AMD cards

**"No AMD GPU found (the amdgpu driver's KFD topology is empty)" on Linux.**
The kernel's amdgpu driver is not loaded for the card. Integrated Radeon GPUs are listed as not supported; setup
lists every card it found and whether HyperStrata can use it.

**The engine stops at start with the card's name, its architecture and the build's list.**
The engine was compiled for another card (for example after moving the HyperStrata folder to another PC). Run
`./setup.sh --setup --backend hip`: it compiles the engine for this card's architecture.

**On Windows the engine exits with `0xC0000005` in `amdhip64_7.dll` before it prints anything (#654).**
Two things in your environment can cause it, and the server now repairs both before it starts the engine (it says so in
the log): `HIP_PATH`, `HIP_DEVICE_LIB_PATH` or `LLVM_PATH` naming a ROCm folder that no longer exists (a deleted build),
and a `TEMP`/`TMP` folder the engine cannot create files in (AMD's runtime compiles its first kernels through temporary
files). If you start `hyper.exe` by hand, fix them yourself: `set HIP_PATH=`, and point `TEMP` and `TMP` at a normal
folder such as `C:\Temp`.

**Windows AMD: the driver resets (VIDEO_ENGINE_TIMEOUT_DETECTED / screen flicker / the engine dies mid-answer).**
Windows gives a GPU about 2 seconds to answer; when a graphics card does not, it resets the driver (a TDR): the screen
flickers, the engine dies, and the Event Viewer shows `VIDEO_ENGINE_TIMEOUT_DETECTED` (#613, with #579 and #541 on
Linux). PolyStrata 0.1.39 and newer say "the GPU stopped responding" in the log. It was seen with KV streaming on
(`--kv-resident 32768`, which setup turns on from 64K context) while reading a long prompt on a gfx1201 card. Try these
in order, one at a time, and tell us what changed:
1. **Send us the evidence.** The last ~80 lines of `strata-<model>.log` in the HyperStrata folder (the server window shows
   the same lines), your AMD driver version (AMD Software > System), your card and Windows version, the HyperStrata
   version, and the exact steps (context size, how long the prompt was, which app sent it). If Windows wrote a
   dump, the newest `.dmp` in `C:\Windows\LiveKernelReports\WATCHDOG` (or `C:\Windows\Minidump`) helps; it holds no
   chats, only the driver state.
2. **Find the step that hangs:** add `"STRATA_PF_STEP_SYNC": "1"` to the `"env"` block of `strata-<model>.json`
   and restart. The prompt path then waits for the GPU after each step and logs any step over 250 ms, so the log names
   the step. It is slower; use it to diagnose, then take it out.
3. **`"STRATA_KV_HOST_DMA": "1"`** in the same `"env"` block: no GPU kernel writes the RAM copy of the K/V during a
   prompt, it is copied by DMA instead. Same answers.
4. **No KV streaming:** run setup again and pick 32K context (or remove `--kv-resident` and its number from
   `"args"` in the JSON, with a context that fits the VRAM).
5. **A smaller prompt chunk:** change `--prefill` in `"args"` (for example `--prefill 512`): each GPU launch then does
   less work between two checks by Windows. Prompts are read more slowly.
6. **Optional, your decision: give Windows more time (`TdrDelay`).** The default is 2 seconds. In the registry,
   `HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\GraphicsDrivers`, a DWORD `TdrDelay` of `10` (seconds)
   lets a long GPU job finish before Windows resets the driver; restart the PC. It hides the symptom and does not fix
   a GPU that has really hung, and a hung GPU then freezes the screen for those 10 seconds. HyperStrata changes no Windows
   setting itself; to undo it, delete the value.
7. **Driver:** use the newest AMD Software release for your card (Adrenalin, not a Windows Update driver) and restart
   after installing. If it started after a driver update, tell us both versions; going back one version is a fair
   test.

**Large pinned host allocations fail on ROCm although RAM is free.**
The mapped expert mode avoids the full pinned arena (AMD/HIP notes, now archived).

**"verify: timed out at layer N" or "no progress for 60 s" on AMD (Linux), mostly with `--mmap-experts`.**
Two reports fixed it with `"GPU_PINNED_MIN_XFER_SIZE": "1048576"` (RX 6800) or `"HSA_USERPTR_FOR_PAGED_MEM": "0"` (two
R9700) in the `env` block of the server JSON, then a restart. Experimental, one machine each; on a systemd unit use
`MemoryMax`, not `MemoryHigh`. The AMD/HIP notes (now archived) cover these Linux verify-timeout workarounds.

## FAQ

Answers to what people asked about HyperStrata.

### Why not contribute this to Strata instead of a separate repo?

Someone asked Strata for GLM and DeepSeek in [issue 310](https://github.com/Niko1221/Strata/issues/310), and the maintainer closed it: "Other MoE architectures (DeepSeek, GLM) aren't planned: the kernels and the pack format are built around this model." So the other models got an engine of their own, next to Strata's.

The repo keeps Strata's git history and merges its releases, and Strata's own engine is in it unchanged. There are now open pull requests in Strata for GLM 5.3 Flash (#1315) and DeepSeek V4 Flash (#1518). Whether upstream takes them is upstream's decision; nothing is promised.

### Is the second engine related to Strata at all?

Yes. It is built on Strata's own code: its quantized matvec kernels, its GGUF reader, its draft policy and suffix drafter, its conversation memory and the llama.cpp MMQ kernels Strata compiles. The list is in `CMakeLists.txt` (the `hyper` target). What is new is the model code for the five architectures and the part that decides which experts sit in VRAM and which run on the CPU.

### How do I know the output is right?

It reads the same GGUF files llama.cpp reads, and its output is compared with llama.cpp's on the same file: greedy decoding, 64 tokens after five chat prompts, token by token. On the laptop, Qwen3.6-35B-A3B UD-Q3_K_M matched all 64 tokens on all five prompts. The other sizes and models, with the few places where they part ways and why, are in [docs/BACKENDS.md](BACKENDS.md#how-each-backend-was-checked). Guessing tokens ahead does not change the answer: with every expert on the CPU, the tokens are the same with and without guesses. Those checks left out no experts. Setup starts Qwen3.6 with `--tail-skip 6`, which skips a few low-ranked experts that are not in VRAM and moves its answers a little; `STRATA_ROUTE_TAIL_SKIP=0` turns that off ([measured](BACKENDS.md#what-was-tuned)).

Every speed in HyperStrata docs/BENCHMARKS.md says which machine, file, config and provenance; upstream README numbers are not ours.

### How fast will it be on my card?

Upstream README/community numbers are from their rigs (RTX 4070 laptop, RTX 5090 workstation, etc. — NOT the reference rig ([BENCHMARKS.md](BENCHMARKS.md))); reported, not verified for this fork. Current fork numbers: docs/BENCHMARKS.md only. What people measured on their own cards is in the [the benchmark page](BENCHMARKS.md#how-to-report-a-run). Yours can go there too; the page says what to send.

### How can I compare it with llama.cpp myself?

With `bench/bench_vs_llama.py`, the script Strata used for its own comparison. It runs HyperStrata and llama.cpp's `llama-server` in turns on the same GGUF and the same prompts and prints the medians of both ([how to run it](BENCHMARKS.md#run-the-benchmark)). In upstream runs on 2026-10-11 the upstream report was 2.7-3.5x prompt / 1.5-1.8x decode — reported, not re-verified after engine changes; do not rank current builds.

### Does it run on Radeon / AMD?

Qwen3.8-Flash-Next, upstream yes (untested in this fork): on a Radeon with 12 GB or more it runs through Strata's AMD backend. The five added models run on AMD under Linux since 0.1.42-poly.4, as a beta: setup compiles their engine for the card on your PC (there is no ready-made AMD engine, so not on Windows). A contributor report on an RX 7600 8 GB (Linux, #11) gives Qwen3.6 74 / 53 and Ornith 40 / 43 tokens/s, reads 481/444 — reported, NOT a HyperStrata run; AMD is untested in this fork. Gemma 4, Tiel Coder, LFM2.5 and GLM have not been tried on an AMD card, and setup says so when you pick them. See the AMD/HIP notes (now archived) for Hyper on AMD (beta).

### Will it run on a 6 GB card, or 4 GB?

It has only run on 8 GB cards and up. With a context of 32K, Qwen3.6 takes about 5 GB of VRAM beside its experts, Ornith about 4 GB, Gemma 4 about 6 GB, Tiel Coder about 5 GB and LFM2.5 about 4 GB. The attention caches are part of that and take their share as a conversation grows, so a short chat leaves more room. LFM2.5's caches grow the least, 20 KB a token. Untested in this fork: the supported tier here is 8 GB VRAM + 32 GB RAM. On 4 GB, none of them leaves room for experts.

### What about older cards like a GTX 1050?

Upstream, the new engine needs an RTX 20 series card or newer — untested in this fork except the reference rig ([BENCHMARKS.md](BENCHMARKS.md)). GTX 10 series cards are not supported by it. Strata has an experimental build for older cards, but that is for Qwen3.8-Flash-Next only.

### Can you add Bonsai, LFM2.5, Ling or DeepSeek V4 Flash?

This engine helps MoE models that don't fit in VRAM. A dense model like Bonsai uses all its weights for every token, so there are no experts to split, and the 1-bit Bonsai models already fit on a small card. LFM2.5-8B-A1B fitting whole on an 8 GB card, so plain llama.cpp is fine there, is PENDING — untested here, no provenance. LFM2.5-24B-A2B, the larger one, is an upstream-only setup option (`--family lfm25`), untested here. Bigger MoEs are the good fit. DeepSeek V4 Flash is an upstream-only setup option (`--family deepseek`), untested here. Ling-2.6-flash (104B, 7.4B active) is on the list.

### Can I run a fine-tune, or another model with the same architecture?

People have, on their own PCs. On Reddit, one ran HauhauCS's uncensored Qwen3.6-35B-A3B at Q4_K_M, which was not tried here, and another Tiel Coder 35B-A3B, a quantization of Ornith 1.5 that setup now installs (`--family tiel`). Their numbers are in the [the benchmark page](BENCHMARKS.md#how-to-report-a-run). The engine reads the architecture and the weights from the GGUF, so a fine-tune that keeps the architecture of Qwen3.6, Ornith 1.5 or Gemma 4 may run — untested here, best-effort only. Setup only knows the files it offers. To give it another, put the file in a folder under the name of the size closest to it, `Qwen3.6-35B-A3B-UD-Q4_K_M.gguf` for a Q4_K_M of Qwen3.6, and run setup with `--family qwen36 --model UD-Q4_K_M --gguf-dir <that folder>`.

### Can I use any GLM 5.3 Flash GGUF?

Upstream-only path, untested in this fork. The installer offers three files, and those are the ones that were run upstream: neuralll's 3.0bit (114 GB), Unsloth's UD-IQ2_XXS (102 GB) and patrickbdevaney's REAP50-Q3_K_M (79 GB, half of the routed experts removed). Other GGUFs from the same converters should load, but they haven't been run. If you already have one, upstream setup takes `./setup.sh --family glm --gguf-dir /folder/with/the/file` (upstream-only, untested here).

GLM needs a lot of RAM: beside a 32 GB card, 96 GB for the 3.0bit file, 82 GB for UD-IQ2_XXS and 60 GB for REAP50. So far it has run on Linux only.

### Why Q3 and Q2 by default on the smaller models? Isn't that a big quality loss?

A 4-bit size for the fork's house path is `--model UD-Q4_K_M` (other families' 4-bit files are upstream-only, untested here). The installer picks UD-Q3_K_M because on an 8 GB card more of its experts fit and its file is lighter on 32 GB of RAM. Upstream laptop report: Q3 94-100 vs Q4 84-85 tokens/s short-chat — that rig only; PENDING on the reference rig. It only takes Q2 when the RAM is too small for Q3. No same-day same-file A/B on file — no general claim. Take Q4 if you have the memory.

### What about NVFP4?

HyperStrata reads GGUF weight types only, so no NVFP4 for now. For GLM, NVFP4 would make the model around 204 GB, which doesn't fit in RAM on a normal PC; the 2- and 3-bit GGUFs (79 to 114 GB) are what lets the experts stay in RAM.

### Does it work on Windows?

The four smaller models (Qwen3.6, Ornith 1.5, Gemma 4, Tiel Coder) and Qwen3.8-Flash-Next do. The fork measures Qwen3.6-35B-A3B only, on the reference rig ([BENCHMARKS.md](BENCHMARKS.md)); the other families are upstream-only, untested here. Setup downloads the new engine ready-made from the latest release, built for RTX 20 to RTX 50 cards (upstream build coverage; tested/supported in this fork: reference rig ([BENCHMARKS.md](BENCHMARKS.md)) only). On a processor without AVX2, or when the engine's code changed after that release, it compiles the engine instead with Visual Studio's build tools and the CUDA toolkit, and offers to install them. GLM 5.3 Flash has only run on Linux.

### Can I use it with Claude Code, Codex or other apps?

It serves OpenAI-style and Anthropic-style APIs on localhost, with tool calls. Claude Code: `ANTHROPIC_BASE_URL=http://127.0.0.1:8080`. Codex CLI uses the Responses API at `/v1/responses` ([setup](DETAILS.md#the-responses-api-and-codex-cli)).

For Claude Code, also add `"anthropic_thinking": "on_request"` and `"fit_max_tokens": true` to `strata-<model>.json` and restart. Claude Code first asks for a session title, and without the first setting the model thinks that over instead, and runs out of the 512 tokens the request allows before it has a title: 5 s a request with Qwen3.6 on an 8 GB laptop ([more](DETAILS.md#using-it)). The second one it needs with the 32K context setup suggests: Claude Code asks for 32,000 tokens of room for the answer in every request, and the server refuses a request when the prompt leaves less than that. With the setting the server gives it the room that is left. With `"warm_start": true` as well, the server keeps Claude Code's system prompt and tool list and reads them when it starts, so the first request after a restart reads only the rest: in one upstream test 3.5 s vs 11.1 s — that run only, not a general speedup.

### Can the model see what it thought in earlier turns?

With Qwen3.6, Ornith and Tiel Coder, if you ask for it. Their template leaves the thinking of earlier answers out of the prompt, so a model that thought of two numbers and said one makes the other up when asked for it later. Add `"preserve_thinking": true` to `strata-<model>.json` and restart, or send `"chat_template_kwargs": {"preserve_thinking": true}` with a request, as with llama.cpp. The server also remembers the thinking of the answers it wrote, so an app that sends an answer back without it (the chat page, Open WebUI) still gets it, as long as the answer's text comes back unchanged.

### Can several people use one server at once?

Yes. By default the server answers one request at a time and the others wait their turn. With `"parallel": N` in `strata-<model>.json` (or `--parallel N` when you run setup) it answers up to N together, with every model. On a card that holds most of the model's experts that writes more tokens a second in all: upstream batching runs (RTX 5090 / 8 GB laptop rigs): 428 vs 312-333 and 105 vs 79 — reported, NOT HyperStrata reference-rig runs. How it works, and how to check it on your PC: [Batching in DETAILS.md](DETAILS.md#batching).

### Who maintains it?

Me, and it's MIT licensed. Strata's engine inside it comes from Strata's releases, and the added engine is all in `src/poly`.

## Still stuck?

Look in the [full troubleshooting table](DETAILS.md#troubleshooting).

← [Prev](BENCHMARKS.md) · [Index](INDEX.md) · [Next](../--help.md) →
