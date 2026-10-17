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
> v1.0.0 gates: UD-Q4_K_M MTP and UD-Q5_K_M MTP on the reference rig, each with max/min/avg decode + prefill, log + verdict. Q5 on `hyper` is PENDING (owner running it).

## What you need in hand (both methods)

- Your specs: GPU model, VRAM, RAM, Windows version. Read them with
  `nvidia-smi` (prints GPU + driver) — named in `BUILD_HYPER.bat`.
- Floor: Windows 10/11 x86-64, 32 GB RAM, ~80 GB free disk (NVMe ideal),
  AVX2 CPU, NVIDIA driver 580 or newer. Measured rig only; rest untested
  (see `docs/BENCHMARKS.md`, Wanted hardware).
- An install folder (a drive with room for tens of GB of model files).
- Your model situation: either the folder holding your `.gguf` file(s), or
  "download them for me" (~70 GB typical; UD-IQ4_XS ~94 GB, UD-Q4_K_XL
  ~111 GB).

> [!TIP]
> House preset uses `--kv q4_0 --draft-kv q4_0`: q4_0 is the fit knob ([KV_CACHE.md](KV_CACHE.md)).

## Method 1 — an agent installs it (recommended)

Open the source folder, run any CLI coding agent in it, paste this (fill the
brackets):

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
   `CUDA_ARCH` to your card, double-click (`BUILD_HYPER.bat:23-27`):
   `75` = RTX 20 · `86` = RTX 30 (reference RTX 3070 Ti Laptop) ·
   `89` = RTX 40 · `120` = RTX 50. Unknown card or several cards:
    `75;86;89;120` (slow build, runs anywhere). Other cards than the sm_86 reference are untested here
     (goal, not promise; see `docs/BENCHMARKS.md`, Wanted hardware). First build needs internet
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

## Details

Folded detail from the old INSTALL/AI_SETUP pages. Skim or skip; the two
methods above are complete without it.

### Ground rules (both methods)

- Keep the default `--host 127.0.0.1`. Only for access from other devices,
  use `--host 0.0.0.0` **together with** `--api-key <a long random secret>`,
  and give the user the key. Never put a tunnel or port forward in front of
  a server without a key.
- Setup changes nothing beyond the install folder, its data folder, and a
  user-account Python when missing. Ask before touching drivers.
- The download is tens of GB — confirm metered connections first. Setup is
  long-running; run it in the background with a long timeout. It is
  resumable: rerunning the same command continues where it stopped.

### Check the PC

Fastest: `START-HERE.bat --check` (prints GPU/RAM/disk verdict; installs
nothing but Python + `.venv`). By hand:

| What | Windows (PowerShell) | Linux |
| --- | --- | --- |
| OS | `[Environment]::OSVersion` | `cat /etc/os-release` |
| NVIDIA GPU, VRAM, driver | `nvidia-smi --query-gpu=index,name,memory.total,driver_version --format=csv` | the same |
| AMD GPU | `Get-CimInstance Win32_VideoController \| Select-Object Name, AdapterRAM` | `lspci \| grep -i -E 'vga\|display'`; VRAM: `cat /sys/class/drm/card*/device/mem_info_vram_total` (bytes) |
| RAM | `(Get-CimInstance Win32_ComputerSystem).TotalPhysicalMemory / 1GB` | `free -g` |
| Free disk | `Get-PSDrive -PSProvider FileSystem` | `df -h .` |

If the PC misses the floor (What you need, above), say which part and stop.
Model pick by RAM: link, don't guess — `docs/MODELS.md` sizes,
`docs/MODELS.md` §§1–3 verdicts. With `--yes` and no `--model`, setup
picks for the RAM itself.

### While it downloads, tell the user

- ~70 GB from Hugging Face; at 100 Mbit/s roughly 1.5–2 hours. Stoppable,
  resumes where it left off.
- Then it prepares the model for the PC (a few minutes) and, where the
  engine compiles, compiles it.
- **While the model starts, the PC can be slow or stop responding for 1–3
  minutes** (longest the first time): 35–55 GB loads into RAM and part is
  locked for the graphics card. Normal — wait, don't close the window.
- Nothing leaves the PC: the model runs locally.

<details>
<summary>Setup-script path (START-HERE / setup.py) instead of BUILD_HYPER</summary>

Same end state as Method 2, driven by questions instead of hand edits:

```
START-HERE.bat --check            only check the PC
START-HERE.bat --yes --family qwen36 --model UD-Q4_K_M --context 131072 --gguf-dir <folder-with-the-gguf> --no-start
```

- Without `--gguf-dir`, setup downloads instead. `--data-dir PATH` moves
  the 70–120 GB of model files. `--port N` (default 8080),
  `--gpu N` / `--gpus 0,1` (default: most VRAM),
  `--backend cuda|hip` (auto on single-kind PCs).
- `--context N` default by VRAM: 32768 under 14 GB, 65536 under 20 GB,
  else 131072. `--vision yes|no|gpu|cpu` (`--yes` leaves images off).
- `--host H --api-key K` only together. `--setup` installs another model
  or changes settings. Full list: `START-HERE.bat --help`.
- `--no-start` is for agents: the server runs in the foreground and would
  block the shell. Install with it, start separately (Verify below).
- Alternative engine path: setup prebuilt in `engine-hyper/` (`BUILD.json`
  stamp; `setup.py` `build_engine_poly` / `get_poly_prebuilt`).
- No ready-made engine fits the card: setup offers to install build tools
  and compile (20–40 minutes once; `--yes` accepts).
- First build/install needs internet once (pinned ggml fetch / model
  download); later runs reuse downloads setup finished. Build time varies
  by PC — 20–40 minutes is one past measurement, not promised.
- On NVIDIA, setup uses CUDA libraries from pip (~0.4 GB). Linux: setup
  uses `sudo apt` for Python/venv when missing (and `build-essential` +
  `git` on AMD) — for an agent run, ask the user to run the `sudo` line,
  then rerun setup.
- AMD on Linux: setup installs ROCm into `.venv` (~10 GB, no sudo) unless
  system ROCm 7 exists, and compiles the engine for the card (10–20
  minutes, once). AMD on Windows: ready-made AMD engine (~550 MB, ROCm
  included), nothing compiled; before the model download setup runs
  `engine\strata-device.exe --list-devices` — a missing card there is a
  driver problem (update AMD Software).
</details>

### Windows notes

1. Download the zip (clone, or
   `https://github.com/theacidblack/HyperStrata/archive/refs/heads/main.zip`)
   and unzip it on a drive with room.
2. Double-click **`START-HERE.bat`**. Answer a few questions — or Enter each
   time for the recommended choice (the questions: Setup's questions below).
3. It downloads (~70 GB) and starts; the browser opens at
   `http://127.0.0.1:8080`.

**Next time**, double-click `START-HERE.bat` again: it starts right away (on
the reference rig ~30–90 s to load; other PCs vary, not promised). Close its
window to stop (up to a minute to unload). `SETUP.bat` (same as
`START-HERE.bat --setup`) installs another model or changes settings.
Starting at logon from Task Scheduler with wrong settings starts 24x slower:
see `DETAILS.md` (Running it at startup).

### Linux notes

```bash
./setup.sh
```

Same questions, same automatic install (uses `sudo apt` for Python and, only
if it must compile, for build tools), same start: `http://127.0.0.1:8080`.
Later runs of `./setup.sh` (or `./run-<model>.sh`) start directly;
`./setup.sh --setup` installs another model or changes settings. Other
distributions, WSL, compiling: `DETAILS.md` (Linux). Nix/NixOS:
`nix develop -c ./setup.sh`. Linux/AMD paths are upstream's, untested here.

### AMD cards, several cards

- AMD/HIP: cut in this fork (`docs/MODELS.md`, Out of scope). Upstream docs only.
- Multi-GPU: upstream path, untested in this fork. PENDING.

<details>
<summary>Docker (Linux; upstream docs, untested here)</summary>

Same idea in a container (NVIDIA cards).

1. Host: Docker with the NVIDIA Container Toolkit and driver **580 or
   newer** (CUDA 13.0).
2. Build (compiles the engine into the image, so the container never
   compiles): `docker build -t hyperstrata .`
   (`--build-arg CUDA_ARCHITECTURES=89` for one card only, faster; default
   covers RTX 30/40/50 + A-series; other cards need a rebuild with their
   arch; add `--build-arg BUILD_VISION=0` to skip the image encoder).
3. Run (first start downloads the ~70 GB model, then starts; later starts
   serve directly):
   `docker run --rm --gpus all -p 8080:8080 --ulimit memlock=-1 -v hyperstrata-data:/data hyperstrata`
   (an old `strata-data` volume keeps working with `-v strata-data:/data`).
   Setup choices are env vars: `-e MODEL=IQ2_XS -e FAMILY=qwen -e CONTEXT=32768 -e VISION=no`
   (`-e VISION=cpu` keeps the encoder on CPU; `-e KV=int8|q4_0|k8v4` picks KV
   precision). Server listens on `0.0.0.0:8080` — set `-e API_KEY=<secret>`
   before exposing the port. Health: image `HEALTHCHECK` on `/health`;
   `GET /v1/status` says what runs.
   Model files on hand: `-v /models/UD-Q4_K_XL:/gguf -e GGUF_DIR=/gguf
   -e FAMILY=unsloth -e MODEL=UD-Q4_K_XL` (mount writable first start —
   setup keeps a `<shard>.done` SHA-256 stamp per shard; `:ro` is fine
   after). Two cards: `--gpus all` takes both unless pinned (`-e GPU=0`,
   `-e GPUS=0,2 -e LAYER_SPLIT=18`). A container memory cap needs
   `-e LOW_RAM=on` (setup measures host RAM, not the cap).
   Stopping: the engine releases tens of GB of page-locked RAM on SIGTERM —
   allow 60 s (`--stop-timeout 60`, `docker stop -t 60`, or
   `stop_grace_period: 60s` in Compose), else `docker stop` kills it (137).
</details>

<details>
<summary>Older CPUs (experimental, upstream reports, untested here)</summary>

The ready-made engine needs AVX2 (Intel Haswell 2013, AMD Zen 2017+). On an
older CPU (AVX only: Sandy/Ivy Bridge, Xeon E5 v1/v2, Bulldozer; or SSE4.2
only: Nehalem/Westmere, Xeon X5600) setup says so, warns it is experimental
and slow, and compiles the engine on the PC (10–20 minutes, once; Windows
needs VS Build Tools + CUDA Toolkit, offered). By hand:
`cmake ... -DSTRATA_ISA_FLOOR=avx` (or `none`). Only i-quant packs run in
that build (not the AVX-512 Q2_0 pack); more VRAM helps most since the GPU
part is unchanged. Real-hardware reports welcome.
</details>

### Updating

**`UPDATE.bat`** (Linux: `./update.sh`) updates without starting the model.
In a `git clone` it runs `git pull`, then refreshes what `START-HERE.bat`
does before a start: the engine (a new ready-made one when needed; on Linux
a compiled engine recompiles when its source changed), Python packages, and
each installed model's settings and draft subset. Model files are untouched
and no question is asked. Close the model's window first (a running engine
cannot be replaced); start later with `START-HERE.bat` as usual. In a zip
copy it says to download the new zip — it cannot fetch files itself. By
hand: unzip the new version anywhere (or `git pull`), run `START-HERE.bat`
(`./setup.sh`); model files live in a `HyperStrata-data` folder **next to**
the install folder, so a new copy finds and reuses them (a `PolyStrata-data`
folder from before the rename is reused in place; nothing big downloads
again). On Linux after `git pull`, setup recompiles when engine source
changed (a minute or two); on compile failure it says so and starts the old
engine.

<details>
<summary>Where things are stored</summary>

- **Chats: only in the browser.** Chat tab, settings, typed API key live in
  browser local storage (`strata.*` keys, incl. `strata.theme` — frozen,
  never renamed). Another browser or private window starts empty; clearing
  site data deletes them. Pictures are not kept, only their names.
- **How the model starts:** `strata-<model>.json` in the install folder
  (context, GPUs, host, API key, ...), written by setup; next to it
  `run-<model>.bat` / `.sh`, log `strata-<model>.log`, and with "Use for
  other apps too", `strata-<model>.shared-settings.json`. Rerunning setup
  for the same model rewrites setup's keys (`exe`, `args`, `port`, `gpu`,
  `host`, `api_key`, `vision`, ...) and keeps yours (`sampling`,
  `mcp_servers`, `mcp`, `cors_origins`, ...); the earlier file is kept as
  `.bak`. Hand-added engine options in `"args"` are not carried over —
  setup names them, you re-add them.
- **Model files** (`models/`, `packs/`, `mtp/`, 70–120 GB): in
  **`HyperStrata-data` next to the install folder** (a `PolyStrata-data`
  folder is reused in place), or wherever `--data-dir` put them. Recorded
  in `%APPDATA%\HyperStrata\settings.json` (Windows) or
  `~/.config/hyperstrata/settings.json` (Linux).
- **The program itself:** `.venv/`, `engine-hyper/`, `third_party/` in the
  install folder.
- **No github.com access:** a source build downloads llama.cpp's source
  (ggml, gguf-py) from GitHub at `LLAMA_CPP_COMMIT` in `setup.py`. Put that
  commit's tree in `third_party/llama.cpp` yourself (mirror or another PC)
  and setup uses it instead.
</details>

### Setup's questions

- **Which model and size?** See `docs/MODELS.md` (sizes) with verdicts in
   `docs/MODELS.md` §§1–3. This fork expects UD-Q4_K_M with MTP or UD-Q5_K_M with MTP (guarantee
   rule, top).
- **How much context?** Tokens kept in mind at once (setup suggests one for
  the card). 384K/512K (experimental) extend past the trained 262K by rope
  scaling — setup turns it on itself (`--rope-scaling`/`--rope-scale`
  override). With `--yes`, recommended answers throughout.
- **Images?** The encoder (0.9 GB) runs on GPU (0.1–0.5 s per picture, ~1.4
  GB VRAM kept free) or CPU (~3–13 s, nothing on GPU); AMD on CPU for now.
  Details: `DETAILS.md` (images/vision).
- **Experimental speed projection?** Off unless asked — read `DETAILS.md`
  first. Changes answers; original model only.

### Tuning for your PC

`START-HERE.bat --calibrate` (`./setup.sh --calibrate`) measures a few
engine settings on the PC and keeps the fastest — kept only when measured
>3% faster here (one upstream PC reported ~15–30 min and +7%; time and gain
not promised elsewhere). Remembered per PC and model, so updates keep it.
The PC is busy during the run (model loads more than once; can stop
responding a minute or two per restart). When it finishes it **starts the
model** — do not start it again. NVIDIA cards for now. What it measures:
`DETAILS.md` (double-click START-HERE.bat).

<details>
<summary>All setup options (no questions)</summary>

```
START-HERE.bat --setup                          install another model, or change context / images
SETUP.bat                                       the same (double-click it)
START-HERE.bat --model IQ2_XS --context 32768 --vision yes --yes     no questions
START-HERE.bat --gguf-dir D:\models\IQ2_XS       use GGUF files you already have
START-HERE.bat --inspect D:\models\some.gguf     what a GGUF really holds and whether it runs (no download)
START-HERE.bat --data-dir E:\HyperStrata-data         keep the model files somewhere else
START-HERE.bat --port 8081                      another port
START-HERE.bat --gpu 1                          another GPU (setup picks the one with the most VRAM)
START-HERE.bat --gpus 0,2                       several GPUs sharing the model
START-HERE.bat --vram-reserve-mib 2048          leave 2 GB of VRAM free for other programs (remembered)
START-HERE.bat --no-browser                     do not open the chat page when ready (remembered; --browser undoes it)
START-HERE.bat --setup --backend hip            the AMD engine on a PC that also has an NVIDIA card
START-HERE.bat --setup --host 0.0.0.0 --api-key <secret>     reachable from other devices, with a key
START-HERE.bat --calibrate                      tune the engine for this PC (about 15-30 minutes), then start
START-HERE.bat --check                          only check this PC
```

(On Linux the same options go to `./setup.sh`; `START-HERE.bat --help`
lists them all. Server settings — games sharing the GPU, MCP tools, CORS,
API keys, the API itself — are in `DETAILS.md` (using it).)

- **Leaving VRAM for other programs:** the expert cache fills free VRAM
  minus `--vram-reserve-mib` (default 700). For a game, 3D program, or
  second model: `--vram-reserve-mib 2048` (into the model's
  `strata-<model>.json`; restarts). hand edit: add `"--vram-reserve-mib",
  "2048"` to `"args"`. Cache shrinks, answers can slow. VRAM another
  program already holds at start is left alone anyway; the reserve is room
  for what it takes later.
- **AMD card driving a Linux desktop:** with the default reserve the card
  fills, and desktop demand moves GPU memory into system RAM the model
  already fills — session ends or compositor fails. Two reports fixed it
  with `--vram-reserve-mib 3072` (~2.3 GB fewer experts in VRAM, a few %
  of speed). Setup and server window say so on such a PC.
- **A card under 8 GB:** when the default reserve leaves too little room,
  the engine lowers the reserve (down to 300 MiB) until it fits and warns
  if the card then ends nearly full. If the start stops with "no VRAM is
  left for the expert cache", that line says how much is short; an 8K
  context and `--draft-vocab en` at setup free the most.
- **Hand-downloaded or mirror files:** setup's step 5 prints the folder it
  expects (`HyperStrata-data\models\<SIZE>\`); put files there with
  original names, or point with `--gguf-dir`. `--inspect` reads only
  headers (file, folder, URL, `ms:`/`hf:`) — real bits per weight, storage
  types, and whether it is an installed file under any name or not one
  this engine reads (names say little: e.g. Unsloth `UD-IQ3_XXS` routed
  experts 3.22 bits/weight, `UD-Q4_K_XL` 5.10, original `Q2_0` 2.25,
  `IQ3_S` 3.33). Mirror: set `HF_ENDPOINT` first
  (`set HF_ENDPOINT=https://hf-mirror.com` /
  `export HF_ENDPOINT=https://hf-mirror.com`) — same pinned revisions and
  checks. Mainland China: `--source modelscope` (`STRATA_SOURCE`), same
  names/files, SHA-256 checked per file as published; measured 2026-10-05
  on one PC there: 11–14 MB/s, Q2_0/IQ3_S hashes matched.
</details>

### Start the server (setup path)

Setup prints the start script it wrote (`start script: run-<model>.bat`;
family tag + size, lower case, e.g. `run-iq2_xs`, `run-coder-iq1_m`):

```
Windows (PowerShell):  Start-Process -FilePath ".\run-iq2_xs.bat"            (opens its own window)
Linux:                 nohup ./run-iq2_xs.sh > strata-server.out 2>&1 &
```

Or `START-HERE.bat` / `./setup.sh` without flags starts the installed model
(asks which when several; `--yes` takes the first). The browser opens at
`http://127.0.0.1:8080` when ready. Closing its window (or stopping the
process) stops the model.

## Verify (agent contract: exact checks)

Run in order. The server answers once the model is loaded (30–90 s on later
starts, minutes the first time — poll, don't restart).

1. **Engine smoke.** `hyper.exe --help` prints `usage:` (exit code 2 is
   normal — the text match is the check).
2. **Health.** `curl http://127.0.0.1:8080/health` (no curl: open the URL
   in the browser) — `loaded: true` when ready. The server window prints
   `ready: http://127.0.0.1:8080/v1` when ready; engine log is
   `strata-<model>.log` in the install folder.
3. **Model list.** `curl http://127.0.0.1:8080/v1/models` — copy the
   `"id"` exactly. That id (not the `.gguf` filename) is what clients send.
4. **One chat.**
   ```
   curl http://127.0.0.1:8080/v1/chat/completions -H "Content-Type: application/json" -d "{\"model\": \"<id from step 3>\", \"messages\": [{\"role\": \"user\", \"content\": \"Say hello in five words.\"}], \"max_tokens\": 64, \"reasoning_effort\": \"none\"}"
   ```
   With a key set, add `-H "Authorization: Bearer <key>"`. Browser-first
   equivalent: open the chat page, type `Say hello in five words.`
5. **Live state (when slow or wrong):** `GET /status`, `GET /slots`,
   `GET /metrics`. Via management MCP: `strata_logs` with
   `source: server|engine|setup`.

| Symptom | What to do |
| --- | --- |
| `the NVIDIA driver is too old` | Update the driver (NVIDIA App or nvidia.com/drivers), restart, rerun setup. |
| Download/install stopped | Rerun the same setup command; it continues. |
| `port 8080 is already in use` | Already running (check `/health`), or use `--port 8081`. |
| Slow, disk busy, or "the engine stopped unexpectedly" | The 35–55 GB load leaves little room: close programs, or set up a smaller size (`--setup --model Q2_0` or IQ2_XS). |
| `ExpertCache: cudaMalloc(...) failed` with free VRAM (Windows) | Page file off or tiny: set a fixed size (same initial and maximum, e.g. 65536 MB), restart; a "System managed" page file may not grow in time. |
| `prompt ... exceeds the context` | Rerun setup with `--setup --context <bigger>` (needs the RAM/VRAM for the KV cache). |
| No AMD GPU found (Linux) | amdgpu lists no GPU; check card and driver. Integrated GPUs are not supported. |
| Python or build tools could not be installed | Install what the message names (links printed), then rerun. |

More: `docs/TROUBLESHOOTING.md` and `DETAILS.md` (troubleshooting).

## Connect your agent

Two directions — don't mix them:

- **Serving (this section):** the agent talks to the model through the HTTP
  API below. The agent brings its own tools; the model calls them, the
  agent runs them, the agent sends results back.
- **Managing (the other direction):** your own assistant installs/starts
  this software through `tools/strata_mcp.py` (on-disk tool name, not the engine) — that is
  `docs/DETAILS.md` (MCP assistant control), not this section.

### Addresses to paste where

| What the client asks for | What to paste |
| --- | --- |
| OpenAI-compatible base URL | `http://127.0.0.1:8080/v1` |
| OpenAI Chat Completions (full path) | `POST http://127.0.0.1:8080/v1/chat/completions` |
| Anthropic base URL | `http://127.0.0.1:8080` |
| Anthropic Messages (full path) | `POST http://127.0.0.1:8080/v1/messages` |
| Responses API (Codex CLI, stateless) | `POST http://127.0.0.1:8080/v1/responses` |
| Browser (Chat, Monitor, About) | `http://127.0.0.1:8080` |
| Health / model list | `GET http://127.0.0.1:8080/health`, `GET http://127.0.0.1:8080/v1/models` |

### API key

- Default install: **no key.** Any value works — literally `none`.
- A key exists only if setup ran with `--api-key <secret>`, the JSON has an
  `"api_key"` key, or `STRATA_API_KEY` is set (`--help.md` §2). Several
  keys work comma-separated. The client sends it as `Authorization: Bearer
  <key>` or `x-api-key: <key>`; with a key set, `/v1/models`, `/props`,
  `/slots` need it too.
- Only expose beyond this PC (`--host 0.0.0.0`) **together with** a key.
  Read the key from `strata-<model>.json` — never invent one; none there
  means the client uses `none`. `strata_connect_info`
  (`tools/strata_mcp.py`, both on-disk tool names, not the engine) prints the same URLs plus key state.

### Model id (not the filename)

The `.gguf` filename (e.g. `Qwen3.6-35B-A3B-UD-Q4_K_M.gguf`, in the
launcher's `"args"` `--model`) is **not** what the client sends. The API id
is `"model_name"` from `strata-<model>.json`, falling back to the config
stem (`serve/models.py:106`): `curl .../v1/models`, copy `"id"` exactly,
paste into the client's model field. `"aliases"` in the config are extra
accepted names — a wrong name usually still answers, but paste the real id
so logs and `/metrics` make sense.

### Client configs

**Any OpenAI-compatible app:** base URL `http://127.0.0.1:8080/v1`, key
`none` (or configured key), model = id from above.

```python
from openai import OpenAI
client = OpenAI(base_url="http://127.0.0.1:8080/v1", api_key="none")  # or your api_key
r = client.chat.completions.create(model="<id from /v1/models>", messages=[{"role": "user", "content": "Hello!"}])
print(r.choices[0].message.content)
```

**OpenCode** (`opencode.jsonc`): provider `strata` (`@ai-sdk/openai-compatible`,
`baseURL http://127.0.0.1:8080/v1`, `apiKey none`), model key = id above,
`limit.context` = setup context (`max_context` in `/health`),
`limit.output` well under it, `reasoningEffort` variants per Task
(`DETAILS.md` (using it) for field names).

**Claude Code:** `ANTHROPIC_BASE_URL=http://127.0.0.1:8080`,
`ANTHROPIC_MODEL` = a Claude name it knows (refused otherwise; the server
ignores the name), any `ANTHROPIC_AUTH_TOKEN` (or configured key). Add
`"fit_max_tokens": true` to the JSON: Claude Code asks 32,000 answer tokens
per request, refused when the prompt leaves less room.

**Codex CLI / Responses apps:** `http://127.0.0.1:8080/v1/responses`
(stateless; starting `config.toml`: `DETAILS.md` (Responses API/Codex),
or `strata_connect_info`). **Anthropic apps:** `.../v1/messages`.
**Cursor / Continue / Open WebUI / aider:** same OpenAI base URL + key + id.

### Pictures through the agent

Needs `"vision"` in the JSON (setup `--vision yes|no|gpu|cpu`); `/health`
`"images": false` means text only — rerun `--setup --vision yes` (`cpu` on
AMD / tight VRAM). Shapes: OpenAI `image_url` part (`data:` URL, `http(s):`
URL, or local path), Anthropic `image` block, Responses `input_image`
(`DETAILS.md` (images/vision)). JPEG/PNG/BMP/GIF direct; WebP/TIFF/AVIF via
Pillow. A 640×480 photo ≈ 300 context tokens (GPU default up to 1,024, CPU
300 — `"max_tokens"` in `"vision"`). Repeat sends re-encode once; agents
screenshotting every turn pay every turn — downscale first. Picture failures:
check `/health` images flag, server window vision-encoder lines, engine log.
Image *generation* is opt-in and out of scope here.

### Thinking and context

- The model thinks first (OpenAI: streamed `reasoning_content`; Anthropic:
  `thinking` blocks). Per-request: OpenAI `"reasoning_effort":
  "none"|"low"|"medium"|"high"`; Anthropic `"output_config" {"effort": ...}`,
  `"thinking" {"type": "disabled"}` or `{"type": "enabled",
  "budget_tokens": N}` (under 2K low, under 8K medium, more high). Default:
  **high**. Levels are trained instructions, not a hard cap.
- Agent rule: `none` (or `low`) for tool/fix steps, `medium`/`high` for
  planning — thinking costs output tokens and time on every tool step.
  Opt-in cap: `"reasoning_budget_tokens": N` (request or JSON; request wins,
  `0` = none; reference install `4096`). JSON `"reasoning_effort"` sets the
  default; web-chat shared settings and the request win over it. Opt-in
  extras only on symptom: `"effort_position": "end"`, `"warm_start": true`,
  `"tool_call_recovery": true`, `"reasoning_close_retry": true`
  (`DETAILS.md` (using it)). One agent: leave `"parallel"` at one
  (`docs/DETAILS.md`, Batching).
- Context (setup 8K–262K; reference `131072`; `/health` `max_context` and
  `/v1/models` report it): over-long requests are **refused, never cut**
  (400) — including `max_tokens` past the edge. Fix: `"fit_max_tokens":
  true`, smaller `max_tokens`, compact history, or `--setup --context
  <bigger>`. Map OpenCode `limit.context`/`limit.output` to it. History
  resend is cheap while the prefix matches (server checkpoints + reuses the
  shared start); editing early messages, changing effort without
  `"effort_position": "end"`, or overflow re-reads everything.

### Tool-call loop

All three endpoints take tools; the **agent** runs the loop: send messages
+ tools → model replies a call (streamed name-first) → agent executes
locally → send result back as `tool`/`tool_result` → repeat until text.
Server-side `"mcp_servers"` tools are the **chat page's only** — API agents
keep their own tools (exception: `"strata_mcp": true` opts one
`/v1/chat/completions` request into server-run MCP tools; don't configure
both or calls run twice). Per-answer caps: `max_rounds` 8, `timeout_s` 60 s,
`max_result_chars` 20,000 (`serve/mcp.py:37`).

### When the agent output looks wrong

Top-down, one change, retry the same prompt: (1) read the server window
(`ready:` line; per-request refusal lines) + engine log + `/status`
`/slots` `/metrics`; (2) looping/rambling/no tool call: cap thinking first
(`low`/`none`, or budget N; `reasoning_close_retry` for replies ending
inside thinking; `reasoning_loop_recovery stop|recover` for repeats;
`repeat_stop_tokens` 256 default for token-stuck); (3) nonsense/flaky args:
lower `sampling` `temperature` (reference `0.6`; `0` greedy); (4)
mis-shaped calls: `"tool_call_recovery": true`, else fix the tool schema
agent-side; (5) context 400s: fit flag / smaller `max_tokens` / compact /
bigger `--context`; (6) slow loop: `none`/`low` tool steps, no `parallel`,
pictures only when needed, check Monitor fill + last prefill/decode rates.

<details>
<summary>Playwright + vision loop (agentic browser test, chat page only)</summary>

No Playwright wiring ships here — generic MCP shape (`DETAILS.md`
(tools-from-MCP-servers)): `"mcp_servers": {"playwright": {"command":
"<cmd>", "args": [...]}}`, `"mcp": {"timeout_s": 60,
"max_result_chars": 20000, "max_rounds": 8}` (stdio; or `url` + `headers`
for Streamable HTTP). Install a Playwright MCP per its docs, paste its
command/args, restart; the server window lists offered tools, Monitor lists
them, Sampling drawer toggles them (default on). Screenshots arrive as image
parts, so this needs `"vision"` + `/health` `"images": true`. Long pages
come back cut — ask for clipped/downscaled screenshots when layout is
enough. End-to-end timing on the reference rig: PENDING, none claimed.
</details>

## Next steps

- Sizes and what loads: `docs/MODELS.md`, verdicts `docs/MODELS.md` §§1–3.
- Every setting, the API, measured numbers: `docs/DETAILS.md`.
- Assistant-run install/start/stop (management MCP): `docs/DETAILS.md` (MCP assistant control).
- Several requests at once: `docs/DETAILS.md` (Batching).
- Several cards: upstream path, untested in this fork.
- Something failed: `docs/TROUBLESHOOTING.md`.

## PENDING (onboarding only, no dates)

Closed by this page: shipped-script step (was "no build script"),
`--help.md` as flag truth, VS/CUDA/arch table, smoke = `usage:` text,
3 + 3 edits with MTP-vs-base and vision rules, port/browserless flags,
agent prompt + verdict wording, verify sequence with browser-first hello.

Still PENDING:

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

← [Prev](../README.md) · [Index](INDEX.md) · [Next](MODELS.md) →
