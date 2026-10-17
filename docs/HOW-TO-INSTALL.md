---
layout: default
title: How to install HyperStrata
---

← [Prev](../README.md) · [Index](INDEX.md) · [Next](MODELS.md) →

# How to install HyperStrata

*Needs: PC specs, install folder, model files location · Gives: built engine, working server, one agent chatting · Next: [MODELS.md](MODELS.md)*
*Start 1/4*

The one install page. Two ways to the same end: a built engine, a working
launcher, the server up, one agent chatting with it.

> **Guarantee rule (both methods):** only house-model UD-Q4_K_M with MTP and UD-Q5_K_M with MTP are expected to run. Rest of coverage list
> (`docs/MODELS.md` §§1–3) is oracle-only/occasional, never gating. Anything else is untested here, not guaranteed —
> engine changes can break what upstream once supported.
>
> v1.0.0 gates: UD-Q4_K_M MTP and UD-Q5_K_M MTP on the reference rig, each with max/min/avg decode + prefill, log + verdict. q5-002 measured 2026-10-17: both gates closed.

## What you need in hand (both methods)

- Your specs: GPU model, VRAM, RAM, Windows version. Read them with
  `nvidia-smi` (prints GPU + driver) — named in `BUILD_HYPER.bat`.
- Floor: Windows 10/11 x86-64, 32 GB RAM, ~80 GB free disk (NVMe ideal),
  AVX2 CPU, NVIDIA driver 580 or newer. Measured rig only; rest untested
  (see `docs/BENCHMARKS.md`, Wanted hardware).
- An install folder (a drive with room for tens of GB of model files).
- Your model situation: either the folder holding your `.gguf` file(s), or
  "download them for me" (~70 GB typical). If the PC misses the floor, say
  which part and stop. Model pick by RAM: `docs/MODELS.md` sizes, §§1–3 verdicts.

> [!TIP]
> House preset uses `--kv q4_0 --draft-kv q4_0`: q4_0 is the fit knob ([KV_CACHE.md](KV_CACHE.md)).

## Method 1 — an agent installs it (recommended)

Tested with the Pi agent only — other agents might work, but untested here,
no guarantee. Agent coverage comes later. Open the source folder, run the
Pi agent in it, paste this (fill the brackets):

> Build the engine tailored for my specs and get one model chatting.
> My GPU: [model + VRAM]. RAM: [GB]. OS: [Windows 10/11].
> Install folder: [path]. Model files: [folder holding the .gguf |
> download them to (drive)]. Wanted model: [filename or size, e.g.
> UD-Q4_K_M]. Read AGENTS.md first, check coverage per docs/MODELS.md
> §§1–3, use BUILD_HYPER.bat or setup, make my JSON + launcher from
> examples/, verify with --help.md flags, finish with the server up and my
> agent connected per the Connect section below.

The agent's contract (hold it to this):

1. Read `AGENTS.md` first.
2. Compare what you asked for against `docs/MODELS.md` §§1–3 and say one
   of these two sentences, word for word: `Covered: <file> is on the
   coverage list, expected to run.` or `Not covered: <file> is off the
   coverage list, not guaranteed — engine changes can break what upstream
   once supported.` Off-list: warn-and-continue only; never promise it.
3. Build with `BUILD_HYPER.bat` (or setup) for your card — `CUDA_ARCH` must
   match your GPU (table in Method 2 step 3).
4. Make your JSON + launcher from `examples/model.example.json` +
   `examples/run-model.example.bat` (the 3 + 3 edits in Method 2 step 4).
5. Verify per the Verify section below: engine smoke (`hyper.exe --help`
   prints `usage:` — exit code 2 is normal), then
   `http://127.0.0.1:8080/health` says `loaded: true`, then `/v1/models`,
   then one short chat.
6. Finish with the server up at `http://127.0.0.1:8080` and your agent
   connected per the Connect section below (base URL + key + model id).

What the agent must hand back: built engine (`engine-hyper\hyper.exe`),
working launcher (JSON + bat), server up on `127.0.0.1:8080`, first chat
answered in the browser.

## Method 2 — manual (same stages, you type)

1. **Prereqs.** Windows 10/11 x86-64, NVIDIA driver 580 or newer (yours: the
   only thing you install yourself; `START-HERE.bat` installs Python 3.12 for
   your user account when missing). For the build: Visual Studio 2019 or 2022
   with the workload `Desktop development with C++`, NVIDIA CUDA Toolkit 13.x
   (RTX 50 needs 13.0+; cards older than RTX 20 need CUDA 12.x plus the
   experimental lines), `cmake` + `ninja` (the install's `.venv` already has
   them, else `.venv\Scripts\python.exe -m pip install cmake ninja`) — all
   named in `BUILD_HYPER.bat` (top comments + failure branches).
2. **Get the source.** `git clone https://github.com/theacidblack/HyperStrata.git`,
   or download the zip and unzip it.
3. **Build the engine.** Close any running server window first (a loaded model
   holds 35–55 GB of RAM the build reuses). Open `BUILD_HYPER.bat`, set
   `CUDA_ARCH` to your card, double-click (`BUILD_HYPER.bat:28-31`):
   `75` = RTX 20 · `86` = RTX 30 (reference RTX 3070 Ti Laptop) ·
   `89` = RTX 40 · `120` = RTX 50. Unknown card or several cards:
   `75;86;89;120` (slow build, runs anywhere). Other cards than sm_86 are
   untested here (goal, not promise; see `docs/BENCHMARKS.md`). First build needs internet
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
6. **Connect your agent** (Connect section below, browser first). Any
   OpenAI-compatible app: base URL `http://127.0.0.1:8080/v1`, key `none`
   (or the configured key), model = the `"id"` from `/v1/models` — not the
   `.gguf` filename. First chat: open the browser page, type `Say hello in
   five words.` Chats live in the browser only (clearing the site's data
   deletes them).

## Setup path (START-HERE instead of hand edits)

Same end state as Method 2, driven by setup instead of hand edits:

```
START-HERE.bat --check            only check the PC
START-HERE.bat --yes --family qwen36 --model UD-Q4_K_M --context 131072 --gguf-dir <folder-with-the-gguf> --no-start
```

- Without `--gguf-dir`, setup downloads instead. `--data-dir PATH` moves
  the ~70–120 GB of model files. `--port N` (default 8080),
  `--gpu N` (default: most VRAM). `--host H --api-key K` only together.
- `--no-start` is for agents: the server runs in the foreground and would
  block the shell. Install with it, start separately (Verify below).
- Full list: `START-HERE.bat --help`.

## Updating

**`UPDATE.bat`** updates without starting the model. In a `git clone` it runs `git pull`, then refreshes the engine, Python packages, and each installed model's settings. Model files are untouched. Close the model's window first (a running engine cannot be replaced); start later with `START-HERE.bat`. In a zip copy it says to download the new zip — model files live in a `HyperStrata-data` folder **next to** the install folder, so a new copy finds and reuses them (a `PolyStrata-data` folder is reused in place). `SETUP.bat` (same as `START-HERE.bat --setup`) installs another model or changes settings.

## Where things are stored

- **Chats: only in the browser.** Chat tab, settings, typed API key live in
  browser local storage (`strata.*` keys, incl. `strata.theme` — frozen,
  never renamed). Another browser or private window starts empty; clearing
  site data deletes them. Pictures are not kept, only their names.
- **Model files** (~70–120 GB): **`HyperStrata-data` next to the install
  folder** (a `PolyStrata-data` folder is reused in place), or wherever
  `--data-dir` put them.
- **The program itself:** `.venv/`, `engine-hyper/`, `third_party/` in the
  install folder.

## Verify (agent contract: exact checks)

Run in order. The server answers once the model is loaded (30–90 s on later
starts, minutes the first time — poll, don't restart).

1. **Engine smoke.** `hyper.exe --help` prints `usage:` (exit code 2 is
   normal — the text match is the check).
2. **Health.** `curl http://127.0.0.1:8080/health` (no curl: open the URL
   in the browser) — `loaded: true` when ready; the engine log is the
   `"log"` file in your JSON.
3. **Model list.** `curl http://127.0.0.1:8080/v1/models` — copy the
   `"id"` exactly. That id (not the `.gguf` filename) is what clients send.
4. **One chat.**
   ```
   curl http://127.0.0.1:8080/v1/chat/completions -H "Content-Type: application/json" -d "{\"model\": \"<id from step 3>\", \"messages\": [{\"role\": \"user\", \"content\": \"Say hello in five words.\"}], \"max_tokens\": 64, \"reasoning_effort\": \"none\"}"
   ```
   With a key set, add `-H "Authorization: Bearer <key>"`. Browser-first
   equivalent: open the chat page, type `Say hello in five words.`
5. **Live state (when slow or wrong):** `GET /status`, `GET /slots`,
   `GET /metrics`. Via management MCP: `strata_logs`
   (`tools/hyper_mcp.py`).

| Symptom | What to do |
| --- | --- |
| `the NVIDIA driver is too old` | Update the driver (NVIDIA App or nvidia.com/drivers), restart, rerun setup. |
| `port 8080 is already in use` | Already running (check `/health`), or use `--port 8081`. |
| Slow, disk busy, or "the engine stopped unexpectedly" | The 35–55 GB load leaves little room: close programs. |
| `prompt ... exceeds the context` | Rerun setup with `--setup --context <bigger>` (needs the RAM/VRAM for the KV cache). |

More: `docs/TROUBLESHOOTING.md` and `DETAILS.md` (troubleshooting).

## Connect your agent

Two directions — don't mix them:

- **Serving (this section):** the agent talks to the model through the HTTP
  API below. The agent brings its own tools; the model calls them, the
  agent runs them, the agent sends results back.
- **Managing (the other direction):** your own assistant installs/starts
  this software through `tools/hyper_mcp.py` (on-disk tool file) — that is
  `docs/DETAILS.md` (MCP assistant control), not this section.

### Addresses to paste where

| What the client asks for | What to paste |
| --- | --- |
| OpenAI-compatible base URL | `http://127.0.0.1:8080/v1` |
| Browser (Chat, Monitor, About) | `http://127.0.0.1:8080` |
| Health / model list | `GET http://127.0.0.1:8080/health`, `GET http://127.0.0.1:8080/v1/models` |

### API key

- Default install: **no key.** Any value works — literally `none`.
- A key exists only if setup ran with `--api-key <secret>` or the JSON has
  an `"api_key"` key (`--help.md` §2). Several keys work comma-separated.
  The client sends it as `Authorization: Bearer <key>` or `x-api-key:
  <key>`; with a key set, `/v1/models` needs it too.
- Only expose beyond this PC (`--host 0.0.0.0`) **together with** a key.
  Read the key from your `<model>.json` — never invent one; no key there
  means the client uses `none`. `strata_connect_info`
  (`tools/hyper_mcp.py`) prints the same URLs plus key state.

### Model id (not the filename)

The `.gguf` filename (e.g. `Qwen3.6-35B-A3B-UD-Q4_K_M.gguf`, in the
launcher's `"args"` `--model`) is **not** what the client sends. The API id
is `"model_name"` from your `<model>.json`: `curl .../v1/models`, copy
`"id"` exactly, paste into the client's model field. `"aliases"` in the
config are extra accepted names — a wrong name usually still answers, but
paste the real id so logs and `/metrics` make sense.

### Clients

**Any OpenAI-compatible app:** base URL `http://127.0.0.1:8080/v1`, key
`none` (or configured key), model = id from above. Pictures need a
`-vision` preset (`examples/qwen36-q4_k_m-vision.json`,
`examples/qwen36-q5_k_m-vision.json`); `/health` `"images": false` means
text only.

```python
from openai import OpenAI
client = OpenAI(base_url="http://127.0.0.1:8080/v1", api_key="none")  # or your api_key
r = client.chat.completions.create(model="<id from /v1/models>", messages=[{"role": "user", "content": "Hello!"}])
print(r.choices[0].message.content)
```

## Next steps

- Sizes and what loads: `docs/MODELS.md`, verdicts `docs/MODELS.md` §§1–3.
- Every setting, the API, measured numbers: `docs/DETAILS.md`.
- Assistant-run install/start/stop (management MCP): `docs/DETAILS.md` (MCP assistant control).
- Something failed: `docs/TROUBLESHOOTING.md`.

← [Prev](../README.md) · [Index](INDEX.md) · [Next](MODELS.md) →
