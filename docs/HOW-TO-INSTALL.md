# How to install HyperStrata

The one install page. Two ways to the same end: a built engine, a working
launcher, the server up, one agent chatting with it.

> **Guarantee rule (both methods):** only the models on the coverage list
> (`docs/COVERAGE.md` §§1–3: Unsloth Qwen3.6-35B-A3B GGUFs, MTP page + base
> page) are expected to run. Anything else is untested here, not guaranteed —
> engine changes can break what upstream once supported.

## What you need in hand (both methods)

- Your specs: GPU model, VRAM, RAM, Windows version. Read them with
  `nvidia-smi` (prints GPU + driver) — named in `BUILD_HYPER.bat`.
- An install folder (a drive with room for tens of GB of model files).
- Your model situation: either the folder holding your `.gguf` file(s), or
  "download them for me".

## Method 1 — an agent installs it (recommended)

Open the source folder, run any CLI coding agent in it, paste this (fill the
brackets):

> Build the engine tailored for my specs and get one model chatting.
> My GPU: [model + VRAM]. RAM: [GB]. OS: [Windows 10/11].
> Install folder: [path]. Model files: [folder holding the .gguf |
> download them to (drive)]. Wanted model: [filename or size, e.g.
> UD-Q4_K_M]. Read AGENTS.md first, check coverage per docs/COVERAGE.md
> §§1–3, use BUILD_HYPER.bat or setup, make my JSON + launcher from
> examples/, verify with --help.md flags, finish with the server up and my
> agent connected per docs/AI_SETUP.md §8.

The agent's contract (hold it to this):

1. Read `AGENTS.md` first.
2. Compare what you asked for against `docs/COVERAGE.md` §§1–3 and say one
   of these two sentences, word for word: `Covered: <file> is on the
   coverage list, expected to run.` or `Not covered: <file> is off the
   coverage list, not guaranteed — engine changes can break what upstream
   once supported.` Off-list: warn-and-continue only; never promise it.
3. Build with `BUILD_HYPER.bat` (or setup) for your card — `CUDA_ARCH` must
   match your GPU (table in Method 2 step 3).
4. Make your JSON + launcher from `examples/model.example.json` +
   `examples/run-model.example.bat` (the 3 + 3 edits in Method 2 step 4).
5. Verify with `--help.md` flags: engine smoke (`hyper.exe --help` prints
   `usage:` — exit code 2 is normal), then `http://127.0.0.1:8080/health`
   says `loaded: true`, then `/v1/models`, then one short chat.
6. Finish with the server up at `http://127.0.0.1:8080` and your agent
   connected per `docs/AI_SETUP.md` §8 (base URL + key + model id below).

What the agent must hand back: built engine (`engine-hyper\hyper.exe`),
working launcher (JSON + bat), server up on `127.0.0.1:8080`, first chat
answered in the browser.

## Method 2 — manual (same stages, you type)

1. **Prereqs.** Windows 10/11 x86-64, current NVIDIA driver (yours: the only
   thing you install yourself; `START-HERE.bat` installs Python 3.12 for
   your user account when missing — `docs/INSTALL.md` Path B step 1).
   For the build: Visual Studio 2019 or 2022 with the workload `Desktop
   development with C++`, NVIDIA CUDA Toolkit 13.x (RTX 50 needs 13.0+;
   cards older than RTX 20 need CUDA 12.x plus the experimental lines),
   `cmake` + `ninja` (the install's `.venv` already has them, else
   `.venv\Scripts\python.exe -m pip install cmake ninja`) — all named in
   `BUILD_HYPER.bat` (top comments + failure branches).
2. **Get the source.** `git clone https://github.com/theacidblack/HyperStrata.git`,
   or download the zip and unzip it (`docs/INSTALL.md` Windows step).
3. **Build the engine.** Close any running server window first (building
   while the model is loaded can run the PC out of memory). Open
   `BUILD_HYPER.bat`, set `CUDA_ARCH` to your card, double-click
   (`BUILD_HYPER.bat:23-27`):
   `75` = RTX 20 · `86` = RTX 30 (reference RTX 3070 Ti Laptop) ·
   `89` = RTX 40 · `120` = RTX 50. Unknown card or several cards:
   `75;86;89;120` (slow build, runs anywhere). First build needs internet
   once (pinned ggml fetch) and takes a while; later builds reuse what
   compiled. Success = `engine-hyper\hyper.exe` exists and `hyper.exe
   --help` prints `usage:` (exit code 2 is normal — the text match is the
   check, `BUILD_HYPER.bat` step 8).
4. **Config + launcher.** Copy `examples/model.example.json` to
   `<your-model>.json` and make its 3 edits (header lines 2–7): (1) `"exe"`
   = your engine exe (`engine-hyper\hyper.exe`), absolute path; (2) `"args"`
   `"--model"` = your model `.gguf`, absolute path, keep
   `"--max-context" 131072` unless setup told you otherwise; (3) `"vision"`
   = your `.mmproj` for pictures, or delete the whole `"vision"` block for
   text-only. MTP files keep `"--kv"` + `"--draft-kv"`; base (no-MTP) files
   use `"--no-draft"` instead (`--help.md` intro). Copy
   `examples/run-model.example.bat` and make its 3 edits (lines 8–10):
   `REPO_ROOT` = install folder (holds `serve\server.py`), `MODEL_JSON` =
   your copied `.json` name, `PY` = install's `.venv\Scripts\python.exe`.
   Port taken: change `--port` / `"port"` (default `8080`, `--help.md` §2).
   No browser window: drop `--open` (`--help.md` §2).
5. **Serve.** Run your bat. It runs `python serve/server.py --engine hyper
   --config <yours>.json --port 8080 --open`
   (`examples/run-model.example.bat:24`). Chat at
   `http://127.0.0.1:8080`. Check `/health` (`loaded: true` when ready;
   no `curl`: open the URL in the browser). First start is slow (tens of GB
   load; the window says what it is doing) — wait, don't close it. Close
   the window to stop (give it up to a minute to unload). Server stays on
   `127.0.0.1` — `0.0.0.0` only with `--api-key` plus a long random key
   (`--help.md` §2).
6. **Connect your agent** (`docs/AI_SETUP.md` §8, browser first). Any
   OpenAI-compatible app: base URL `http://127.0.0.1:8080/v1`, key `none`
   (or the configured key), model = the `"id"` from `/v1/models` — not the
   `.gguf` filename. First chat: open the browser page, type `Say hello in
   five words.` Chats live in the browser only (clearing the site's data
   deletes them).

## PENDING (onboarding only, no dates)

Closed by this page: shipped-script step (was "no build script"),
`--help.md` as flag truth, VS/CUDA/arch table, smoke = `usage:` text,
3 + 3 edits with MTP-vs-base and vision rules, port/browserless flags,
agent prompt + verdict wording, verify sequence with browser-first hello.

Still PENDING — full list in `docs/ONBOARDING_GAPS.md`:

- One per-model GB/download-time table used everywhere; where files land;
  reuse-vs-redownload check.
- Per-quant VRAM fit table; the exact low-memory warning string + switch.
- Setup glossary in place: `--check` output meanings, `--yes` defaults,
  `--gguf-dir` folder shape, per-flag lines.
- Key-making step for `0.0.0.0`; first-start minutes + progress-vs-stuck
  lines; firewall allow rule.
- Pack/tokenizer/expert-profile/`lib_dirs` sources; mmproj-mismatch symptom;
  per-context memory guidance.
- Build deep cuts: one-time download size + offline alt, safe-clean command,
  JOBS-vs-RAM rule, missing-`BUILD.json` consequence, vision-helper pointer,
  antivirus note.
- Prereq deep cuts: git floor, checkout size, single Python floor, Store-stub
  fix, `.venv` failures, admin statement, network/proxy/offline limit, CPU
  floor.
- Old-GPU / no-AVX2 opt-in wording; AMD + iGPU verdicts; Linux as appendix.
- README/AI_SETUP leftovers: read order, per-flag lines, old names, failure
  rows, log name, MCP decision line.
