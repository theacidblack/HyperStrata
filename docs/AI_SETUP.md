← [Prev](INSTALL.md) · [Index](INDEX.md) · [Next](MODELS.md) →

*Needs: install method chosen, agent ready to run · Gives: agent-run PC check, setup, verified server · Next: MODELS.md*
*Start 3/4*

# Installing PolyStrata for a user (instructions for AI agents)

This page is written for an AI coding assistant (Claude Code, Cursor, Codex, GitHub Copilot, ...) that a user asked to
set up PolyStrata on their PC. This page is upstream's setup guide (Qwen3.8-first), kept as archive. This fork: Qwen3.6-35B-A3B, Windows + NVIDIA, 8 GB VRAM + 32 GB RAM tier. It serves an OpenAI- and Anthropic-compatible API on `http://127.0.0.1:8080`. The human-oriented
version of everything below is [INSTALL.md](INSTALL.md).

Work through the steps in order. Tell the user what you are doing in plain words; they may not be technical.

## 0. Ground rules

- **Never expose the server without an API key.** Keep the default `--host 127.0.0.1`. Only if the user asks for
  access from other devices, use `--host 0.0.0.0` **together with** `--api-key <a long random secret>`, and give the
  user the key. Never put a tunnel or port forward in front of a server without a key.
- Do not change the user's system beyond what setup does (setup installs Python for the user account if needed,
  and everything else inside the PolyStrata folder and its `PolyStrata-data` folder). Ask before installing drivers.
- The model download is ~70 GB (94 GB for Unsloth's UD-IQ4_XS, 111 GB for UD-Q4_K_XL). Confirm the user is fine with that before you
  start, especially on a metered connection.
- Setup is long-running (an hour or more on a slow connection). Run it in the background or with a long timeout and
  poll its output; do not kill it because it is quiet for a while. It is resumable: running the same command again
  continues where it stopped.

## 1. Check the PC

The quickest way is setup's own check, which prints the GPU(s), driver, RAM, CPU, free disk and whether this PC can run
PolyStrata, and installs nothing except Python and the `.venv` it needs to run (step 2 gets the repository first):

```
Windows:  START-HERE.bat --check
Linux:    ./setup.sh --check
```

To check by hand:

| What | Windows (PowerShell) | Linux |
| --- | --- | --- |
| OS | `[Environment]::OSVersion` | `cat /etc/os-release` |
| NVIDIA GPU, VRAM, driver | `nvidia-smi --query-gpu=index,name,memory.total,driver_version --format=csv` | the same |
| AMD GPU | `Get-CimInstance Win32_VideoController \| Select-Object Name, AdapterRAM` (AdapterRAM is capped at 4 GB; trust the model name) | `lspci \| grep -i -E 'vga\|display'`; VRAM: `cat /sys/class/drm/card*/device/mem_info_vram_total` (bytes); `rocm-smi` only if ROCm is installed |
| RAM | `(Get-CimInstance Win32_ComputerSystem).TotalPhysicalMemory / 1GB` | `free -g` |
| Free disk | `Get-PSDrive -PSProvider FileSystem` | `df -h .` |

Requirements (upstream list, inherited — untested in this fork except the reference rig below; details: [INSTALL.md](INSTALL.md#what-you-need)):

- **GPU:** NVIDIA RTX 20, 30, 40 or 50 series, or AMD Radeon RX 7900 XT / XTX, RX 7800 XT / 7700 XT, RX 9060 XT,
  RX 9070 / 9070 XT, Radeon AI PRO R9700, RX 6800 / 6900 series — upstream list, NOT verified here. Tested/supported in this fork: 8 GB VRAM NVIDIA + 32 GB RAM only (reference rig: RTX 3070 Ti Laptop; what this fork covers: [TEST_REQUESTS.md](TEST_REQUESTS.md)). 12 GB of VRAM or more (an NVIDIA card with 8 GB runs; speed PENDING on your config — expect slower than larger cards, no figure claimed). GTX 10 series and older, and integrated GPUs, are not supported; Pascal / Volta cards (P40, V100) and some
  older AMD cards have experimental paths the user opts into ([OLDER_GPUS.md](OLDER_GPUS.md)).
- **Driver:** NVIDIA 580 or newer. AMD on Linux: the kernel's amdgpu driver; on Windows: a current AMD Adrenalin
  driver. If the driver is missing or too old, tell the user to update it (NVIDIA App / nvidia.com/drivers, or AMD
  Software) and restart; do not install drivers yourself unless they ask.
- **RAM:** 32 GB or more (see step 3). **Disk:** ~80 GB free, ideally on an NVMe SSD. **CPU:** x86-64 with AVX2. Without AVX2 (Xeon E5 v1/v2 and older) setup still installs, as an experimental and slow build it compiles on the PC (10-20 minutes): tell the user that before starting ([INSTALL.md](INSTALL.md#older-cpus-experimental)).
- **OS:** Windows 10/11 or Linux (Ubuntu 22.04/24.04 and Debian 12/13 are fully automatic; Debian 13 gets the oldest CUDA 13 its NVIDIA repo has, currently 13.1).

If the PC does not meet them, say which part is missing and stop.

## 2. Get PolyStrata

```
git clone https://github.com/theacidblack/HyperStrata.git
```

Without git: download https://github.com/theacidblack/HyperStrata/archive/refs/heads/main.zip and unzip it. Pick a drive with
enough free space: the model files go to a `PolyStrata-data` folder **next to** the PolyStrata folder (or `--data-dir <path>`
on another drive).

## 3. Pick the model

By the PC's RAM — upstream tiers, untested in this fork except 32 GB on the reference rig (what this fork covers: [TEST_REQUESTS.md](TEST_REQUESTS.md)). Ask the user whether they mainly want it for code - then the Coder is the fit at 32 GB:

| RAM | `--family` / `--model` | Notes |
| --- | --- | --- |
| 32 GB | `--family coder` (size IQ1_M) | for code; with a 24 GB card Q2_0 and IQ2_XS also run (setup picks the low-RAM mode) — upstream report, untested here |
| 48 GB | `--family qwen --model IQ2_XS` | or `Q2_0` (smallest file) |
| 64 GB | `--family qwen --model IQ2_XS` (upstream pick, untested here) | IQ3_XXS / IQ3_S are larger files, upstream reports slightly better quality; speed PENDING on your rig. |
| 96 GB+ | `--family qwen --model IQ3_S` | `--family unsloth --model UD-IQ4_XS` (~4-bit, 94 GB, part of the experts read from the SSD under ~80 GB of RAM); UD-Q4_K_XL is experimental (NVIDIA only, NVMe SSD; one upstream report: 7-8.5 tokens/s on a 64 GB rig — NOT a HyperStrata run, PENDING re-measure). |

`--family swift` (Swift 1.5, a fine-tune that thinks shorter; sizes Q2_0, IQ2_XS, IQ3_XXS) is the alternative to
`qwen`. With `--yes` and no `--model`, setup picks the recommended size for the RAM itself. More: [MODELS.md](MODELS.md).

`--family glm` (GLM 5.3 Flash) is another model with its own engine, and experimental: Linux, one NVIDIA card with
32 GB, 96 GB of RAM or more, a 114 GB download, no images. Take it only when the user asks for it, and read
[GLM.md](GLM.md) first.

`--family deepseek` (DeepSeek V4 Flash) runs on that engine too, as experimental: Linux, one NVIDIA card with
32 GB, 72 GB of RAM or more by setup's estimate, a 91 GB download, no images. Take it only when the user asks for it,
and read [DEEPSEEK.md](DEEPSEEK.md) first.

`--family lfm25` (LFM2.5-24B-A2B, by Liquid AI) runs on that engine as well, experimental: Linux or Windows (setup
compiles the engine there, as for the three below), one NVIDIA card, an 11 to 25 GB download, about 16 to 31 GB of
RAM beside an 8 GB card by setup's estimate, no images. Leave `--model` out: setup takes UD-Q4_K_XL, or UD-Q3_K_XL
on a PC with too little RAM for it; QAD-Q4_0, UD-Q6_K_XL and Q8_0 are by name. Its license allows commercial use
only to companies with less than 10 million US dollars of yearly revenue. Take it only when the user asks for it,
and read [LFM.md](LFM.md) first.

`--family qwen36`, `--family ornith` and `--family gemma4` (Qwen3.6-35B-A3B, Ornith 1.5, Gemma 4 26B-A4B) are
smaller models on GLM's engine, and experimental too: Linux or Windows, one NVIDIA card, an 11 to 24 GB download,
about 15 to 28 GB of RAM beside an 8 GB card by setup's estimate, by size. On Windows setup compiles the engine
(the CUDA Toolkit and Visual Studio's 2022 compiler, which it offers to install). They read pictures with
`--vision yes` (about 1 GB more to download; the encoder runs on the CPU of a card with less than 16 GB). Each
has three sizes. Leave `--model` out: setup takes UD-Q3_K_M (APEX-I-COMPACT for Ornith), or the smallest on a PC
with too little RAM for it. The largest is by name, `--model UD-Q4_K_M` or `--model APEX-I-QUALITY`.
Take one only when the user asks for it; [BACKENDS.md](BACKENDS.md#install) has what was measured.

## 4. Run setup without questions

From the PolyStrata folder (Windows: in `cmd`, or `cmd /c START-HERE.bat ...` from PowerShell):

```
Windows:  START-HERE.bat --yes --family qwen --model IQ2_XS --no-start
Linux:    ./setup.sh --yes --family qwen --model IQ2_XS --no-start
```

The flags (all of them: `START-HERE.bat --help`):

| Flag | Meaning |
| --- | --- |
| `--yes` | take the recommended answer to every question (no prompts) |
| `--family qwen\|swift\|coder\|unsloth\|glm` | the model version |
| `--model Q2_0\|IQ2_XS\|IQ3_XXS\|IQ3_S\|IQ1_M\|UD-IQ4_XS\|UD-Q4_K_XL` | the size (the Coder is IQ1_M, Unsloth UD-IQ4_XS or UD-Q4_K_XL) |
| `--context N` | context in tokens; default by VRAM: 32768 under 14 GB, 65536 under 20 GB, else 131072 |
| `--vision yes\|no\|gpu\|cpu` | read pictures; `--yes` leaves images off. AMD cards: `cpu` |
| `--gpu N` / `--gpus 0,1` / `--gpus all` | one card, or several sharing the model (default: the card with the most VRAM) |
| `--backend cuda\|hip` | NVIDIA or AMD engine; chosen by itself on a PC with only one kind of card |
| `--data-dir PATH` | where the 70-120 GB of model files go |
| `--port N` | the server port (default 8080) |
| `--host H --api-key K` | listen beyond this PC; **only together with a key** |
| `--no-start` | install only, do not start the server |
| `--setup` | install another model or change settings of an installed one |
| `--check` | only check the PC |

`--no-start` is recommended for agents: the server runs in the foreground until its window is closed, which would
block your shell. Start it separately in step 6.

Notes:
- **Linux:** setup uses `sudo apt` (or dnf/pacman) to install Python with venv if it is missing, and on AMD may need
  `build-essential` and `git`. You cannot type the user's password: if a `sudo` step is needed, ask the user to run it
  (e.g. `sudo apt install python3-venv build-essential git`) and then rerun setup.
- **AMD on Linux:** setup installs ROCm into `.venv` (~10 GB, no sudo) unless a system ROCm 7 exists, and compiles the
  engine for the card (10-20 minutes, once).
- **AMD on Windows (new in 0.1.34):** setup downloads the ready-made AMD engine (~550 MB, ROCm included) - nothing
  is compiled and only the AMD driver is needed. Before the model download it runs `engine\strata-device.exe
  --list-devices`; if that does not list the card, the driver is the problem (tell the user to update AMD Software).
  One card per model and no images on Windows for now. It is new: ask the user to report how it runs (docs/AMD_HIP.md).
- **NVIDIA with no ready-made engine for the card:** setup offers to install build tools and compile (20-40 minutes);
  `--yes` accepts.

## 5. While it downloads, tell the user

- It downloads about 70 GB from Hugging Face; how long depends on their connection (at 100 Mbit/s roughly 1.5-2
  hours). It can be stopped and continues where it left off.
- Then it prepares the model for their PC (a few minutes) and, on AMD on Linux, compiles the engine.
- **When the model starts, the PC can be slow or stop responding for 1-3 minutes** (longest the first time): PolyStrata
  loads 35-55 GB into RAM and locks part of it for the graphics card. That is normal; they should not close the window.
- Nothing leaves their PC: the model runs locally.

## 6. Start the server

Setup prints the start script it wrote (`start script: run-<model>.bat`). The name is the family tag plus the size,
lower case: `run-iq2_xs`, `run-swift-iq2_xs`, `run-coder-iq1_m`, `run-unsloth-ud-iq4_xs`.

```
Windows (PowerShell):  Start-Process -FilePath ".\run-iq2_xs.bat"            (opens its own window)
Linux:                 nohup ./run-iq2_xs.sh > strata-server.out 2>&1 &
```

Or `START-HERE.bat` / `./setup.sh` without flags, which starts the installed model (it asks which one when several are
installed; add `--yes` to take the first). The server opens the user's browser on `http://127.0.0.1:8080` when it is
ready. Closing its window (or stopping the process) stops the model.

## 7. Verify

The HTTP server answers once the model is loaded (30-90 s on later starts, a few minutes the first time). Poll:

```
curl http://127.0.0.1:8080/health
```

It returns `{"status": "ok", "model": ..., "max_context": ..., "images": ..., "api_key": ..., "loaded": true, ...}`.
Then:

```
curl http://127.0.0.1:8080/v1/models
curl http://127.0.0.1:8080/v1/chat/completions -H "Content-Type: application/json" -d "{\"model\": \"strata\", \"messages\": [{\"role\": \"user\", \"content\": \"Say hello in five words.\"}], \"max_tokens\": 64, \"reasoning_effort\": \"none\"}"
```

With an API key, add `-H "Authorization: Bearer <key>"`. The server window prints a `ready: http://127.0.0.1:8080/v1`
line when it is ready; the engine log is `strata-<model>.log` in the PolyStrata folder.

## 8. Connect the user's apps

The goal of this step: an agent (Claude Code, OpenCode, Pi, any
OpenAI-compatible client) chats with the local model **and** calls tools.
There are two directions — do not mix them up:

- **Serving (this section):** the agent talks to the model through the
  HTTP API below. The agent brings its own tools; the model calls them,
  the agent runs them, the agent sends the results back.
- **Managing (the other direction):** your own assistant installs/starts
  HyperStrata through `tools/strata_mcp.py`. That is
  [MCP_SERVER.md](MCP_SERVER.md) and the `Alternative` section at the end
  of this page — it does not serve chats to agents.

### 8.1 Addresses to paste where

The server listens on `http://127.0.0.1:8080` (change with `--port` in
setup, or edit the run script). Endpoints
(`serve/server.py:6-7`, [DETAILS.md](DETAILS.md#using-it)):

| What the client asks for | What to paste |
| --- | --- |
| OpenAI-compatible base URL | `http://127.0.0.1:8080/v1` |
| OpenAI Chat Completions (full path) | `POST http://127.0.0.1:8080/v1/chat/completions` |
| Anthropic base URL | `http://127.0.0.1:8080` |
| Anthropic Messages (full path) | `POST http://127.0.0.1:8080/v1/messages` |
| Responses API (Codex CLI and other Responses apps, stateless) | `POST http://127.0.0.1:8080/v1/responses` |
| Browser (Chat, Monitor, About) | `http://127.0.0.1:8080` |
| Health / model list (paste in a browser or `curl`) | `GET http://127.0.0.1:8080/health`, `GET http://127.0.0.1:8080/v1/models` |

Check first: `curl http://127.0.0.1:8080/health` should answer
`{"status": "ok", "model": ..., "max_context": ..., "images": ...,
"api_key": ..., "loaded": true, ...}`. `"images": true` means this
model reads pictures; `false` means text only
(`serve/server.py:5673-5676`). `/v1/models` lists only the loaded
model, with its context limit and input modalities
([DETAILS.md](DETAILS.md#using-it)).

### 8.2 API key: when needed, where it lives, what the client sends

- Default install: **no key.** Any value works — literally `none`.
- A key exists only if setup ran with `--api-key <secret>`, or the
  model's `strata-<model>.json` has an `"api_key"` key, or the
  `STRATA_API_KEY` environment variable is set (`--help.md` §2,
  `serve/server.py:7111-7113`). Several keys work separated by commas
  (`"api_key": "key1,key2"`, or a list when a key itself contains a
  comma), as llama.cpp does (`serve/server.py:6690-6691`,
  [DETAILS.md](DETAILS.md#using-it)).
- The client sends it as its API key. Over HTTP that is `Authorization:
  Bearer <key>` or `x-api-key: <key>` (`serve/server.py:3089`). With a
  key set, metadata endpoints (`/v1/models`, `/props`, `/slots`) need
  it too.
- Only expose beyond this PC (`--host 0.0.0.0`) **together with** a key
  (ground rules at the top of this page). The key value lives in
  `strata-<model>.json` next to the run script — read it from there and
  paste it into the client. Never invent one; if the file has none,
  the client uses `none`.
- `strata_connect_info` (the `tools/strata_mcp.py` tool) prints the same
  URLs plus the key state for the running server
  (`tools/hyper_mcp.py:1655-1698`): `openai_base_url` is `{base}/v1`,
  `anthropic_base_url` is `{base}`, messages endpoint is
  `{base}/v1/messages`.

### 8.3 Model name to select (`model_name`, not the filename)

The GGUF filename (e.g. `Qwen3.6-35B-A3B-UD-Q4_K_M.gguf`, named in the
launcher's `"args"` `--model`) is **not** what the client sends. The API
id is `"model_name"` from `strata-<model>.json`, falling back to the
config file's stem (`serve/models.py:106`):

```
"id": str(cfg.get("model_name") or Path(config).stem.replace("strata-", "", 1))
```

Do this, zero guessing:

1. `curl http://127.0.0.1:8080/v1/models` (add `-H "Authorization:
   Bearer <key>"` when a key is set).
2. Copy the `"id"` exactly into the client's model field.
3. `"aliases"` in the config lists extra names the model also answers
   to; any other name is still served, as before
   ([DETAILS.md](DETAILS.md#using-it)). So a wrong name usually still
   answers — but paste the real id anyway so logs and `/metrics` make
   sense.

### 8.4 Copy-paste client configs

**Any OpenAI-compatible app or agent:** base URL
`http://127.0.0.1:8080/v1`, API key `none` (or the configured key),
model = the id from §8.3 above.

```python
from openai import OpenAI
client = OpenAI(base_url="http://127.0.0.1:8080/v1", api_key="none")  # or your api_key
r = client.chat.completions.create(model="<id from /v1/models>", messages=[{"role": "user", "content": "Hello!"}])
print(r.choices[0].message.content)
```

**OpenCode** (`opencode.jsonc` in the project, or
`~/.config/opencode/`; field names are OpenCode's —
[DETAILS.md](DETAILS.md#using-it)):

```jsonc
{
  "$schema": "https://opencode.ai/config.json",
  "provider": {
    "strata": {
      "npm": "@ai-sdk/openai-compatible",
      "name": "HyperStrata (local)",
      "options": { "baseURL": "http://127.0.0.1:8080/v1", "apiKey": "none" },
      "models": {
        "strata": {
          "name": "HyperStrata (local)",
          "limit": { "context": 131072, "output": 32768 },
          "options": { "reasoningEffort": "high" },
          "variants": {
            "low": { "reasoningEffort": "low" },
            "medium": { "reasoningEffort": "medium" },
            "none": { "reasoningEffort": "none" }
          }
        }
      }
    }
  },
  "model": "strata/strata"
}
```

Set `limit.context` to the context setup chose (the `max_context` in
`/health`, or `--max-context` in the launcher). OpenCode compacts the
conversation before it gets there. Keep `limit.output` well under it
(see §8.7). The model key (`strata` above) must match the id from §8.3,
or an alias in the config.

**Claude Code:** `ANTHROPIC_BASE_URL=http://127.0.0.1:8080`,
`ANTHROPIC_MODEL` set to a Claude model name it knows (it refuses names
it doesn't; HyperStrata ignores the name), and any
`ANTHROPIC_AUTH_TOKEN` (or the configured key)
([DETAILS.md](DETAILS.md#using-it);
`tools/hyper_mcp.py:1680-1682`). Add `"fit_max_tokens": true` to
`strata-<model>.json`: Claude Code asks for 32,000 tokens of answer in
every request, and the server refuses a request whose prompt leaves
less room than that in the context.

**Codex CLI and other Responses API apps:**
`http://127.0.0.1:8080/v1/responses` (stateless; Codex's `config.toml`:
[DETAILS.md](DETAILS.md#the-responses-api-and-codex-cli)).
`tools/strata_mcp.py`'s `strata_connect_info` prints a starting
`config.toml` (`tools/hyper_mcp.py:1683-1685`).

**Anthropic-compatible apps:** `http://127.0.0.1:8080/v1/messages`.
**Cursor / Continue / Open WebUI / aider:** same OpenAI base URL plus
key plus model id (`tools/hyper_mcp.py:1693-1696` lists them).

### 8.5 Pictures through the agent (vision)

Needs `"vision"` in `strata-<model>.json` (setup `--vision yes|no|gpu|cpu`;
`--help.md` §5). If `/health` says `"images": false`, the model is text
only — rerun setup with `--setup --vision yes` (or `cpu` on AMD / tight
VRAM). No engine flags: the server's `vision` section handles it
(`--help.md` §5; `serve/server.py:2619-2620`).

Shapes the server reads (`serve/frontend.py:171-182`,
[DETAILS.md](DETAILS.md#images-vision)):

- **OpenAI API:** an `image_url` part — a `data:` URL, an `http(s)://`
  URL, or a local file path (`serve/server.py:2358-2378`):
  `{"type": "image_url", "image_url": {"url": "data:image/jpeg;base64,..."}}`
  next to `{"type": "text", ...}` parts.
- **Anthropic API:** an `image` block with a `base64` (or `url`) source,
  as usual.
- **Responses API:** `input_image` with `image_url`, same sources
  (`serve/responses.py:77-83`).

JPEG/PNG/BMP/GIF work directly; WebP/TIFF/AVIF convert via Pillow
(`serve/server.py:13-16`). A picture becomes context tokens (a 640x480
photo ≈ 300; GPU default up to 1,024, CPU default 300 — `"max_tokens"`
in the `"vision"` section, `--vision-tokens N` at setup). The same
picture sent again on later turns (as chat apps do) is encoded once.
Agents that send a screenshot every turn pay the tokens every turn —
downscale or crop the screenshot first when the detail allows it.

Minimal OpenAI example ([DETAILS.md](DETAILS.md#images-vision)):

```python
import base64
from openai import OpenAI
client = OpenAI(base_url="http://127.0.0.1:8080/v1", api_key="none")
img = base64.b64encode(open("photo.jpg", "rb").read()).decode()
r = client.chat.completions.create(model="<id from /v1/models>", messages=[{"role": "user", "content": [
    {"type": "image_url", "image_url": {"url": f"data:image/jpeg;base64,{img}"}},
    {"type": "text", "text": "What is in this picture?"}]}])
print(r.choices[0].message.content)
```

If the agent says pictures fail: check `/health` `images` flag, then the
server window for the vision-encoder lines, then the engine log
(`strata-<model>.log`). Image *generation* (`POST
/v1/images/generations`, Qwen-Image) is a separate opt-in —
[IMAGES.md](IMAGES.md) — not this.

### 8.6 Thinking: what it costs an agent, what to set

The model thinks before it answers (streamed as `reasoning_content`,
Anthropic: `thinking` blocks). Per request
([DETAILS.md](DETAILS.md#using-it); template mapping
`serve/frontend.py:137-162`):

| API | How |
| --- | --- |
| OpenAI | `"reasoning_effort": "none" \| "low" \| "medium" \| "high"` (also `"reasoning": {"effort": ...}`) |
| Anthropic | `"output_config": {"effort": "low" \| "medium" \| "high"}`, `"thinking": {"type": "disabled"}`, or `"thinking": {"type": "enabled", "budget_tokens": N}` (under 2K = low, under 8K = medium, more = high) |

Without a setting the model uses its own default, **high**. The levels
are trained instructions, not a hard token cap: easy questions think
briefly on any level; hard ones think longest on `high`
([DETAILS.md](DETAILS.md#using-it)).

What this means for an agent loop:

- Thinking **counts as output tokens** and **takes time** on every tool
  step. An agent that thinks hard before each of 20 tool calls pays 20
  times. Rule of thumb: `none` (or `low`) for quick tool/fix steps,
  `medium`/`high` for the planning step. OpenCode `variants` above let
  the user switch per Task.
- A hard cap (opt-in): `"reasoning_budget_tokens": N` in a request caps
  the thinking at N tokens — the server ends it with a short wrap-up
  and `</think>` and answers from there. Same key in
  `strata-<model>.json` sets it for every request; a request's own value
  wins, `0` means no budget (`--help.md` §6;
  `serve/server.py:3454-3465`). Reference install uses `4096`.
- Default for requests naming none: `"reasoning_effort"` in
  `strata-<model>.json` (outside `"args"` — the engine has no such
  option). The web chat's shared settings ("Use for other apps too")
  and a request's own level win over it.
- Keep-agents-fast extras (all opt-in, off default — only when the
  symptom matches): `"effort_position": "end"` keeps the cached chat
  when only the effort changes (an agent's "think harder" switch);
  `"anthropic_thinking": "on_request"` renders no-thinking requests
  without thinking (what Claude Code's short helper calls need);
  `"warm_start": true` reuses the last agent system-prompt+tools prefix
  after a restart; `"tool_call_recovery": true` reads slightly
  mis-shaped tool calls; `"reasoning_close_retry": true` continues once
  a reply that stopped inside `<think>` ([DETAILS.md](DETAILS.md#using-it)).
- PolyStrata answers one request at a time; `"parallel": N` in the
  model's config (or setup `--parallel N`) decodes up to N together
  ([BATCHING.md](BATCHING.md)). Leave at one for a single agent.

### 8.7 Context window vs agent history

- Chosen in setup (8K–262K; the reference install uses `131072`).
  `GET /health` `max_context` and `/v1/models` context limit report it.
- Requests longer than that are **refused, never silently cut** (400).
  A request whose `max_tokens` would run past the context is refused too
  — agents that always ask for their full output cap get
  `prompt ... exceeds the context` instead: add `"fit_max_tokens": true`
  to `strata-<model>.json` to clamp it to the room left
  ([DETAILS.md](DETAILS.md#using-it)). A prompt that leaves no room at
  all is still refused.
- Map it in the client: OpenCode `limit.context` = setup context,
  `limit.output` well under it ([DETAILS.md](DETAILS.md#using-it)).
  Claude Code + `fit_max_tokens` (above) is the same fix.
- Resending the full history each turn is the normal agent pattern and
  is cheap here when the prefix matches: the server checkpoints the
  conversation (live session + checkpoints in RAM) and reuses the shared
  start (system prompt + tool list), so a new turn reads only what
  changed ([DETAILS.md](DETAILS.md#using-it)). What breaks the reuse:
  editing an early message, changing the effort without
  `"effort_position": "end"`, or a context that overflowed — then the
  whole prompt reads again and the turn is slow.

### 8.8 The tool-call loop (how a turn works)

All three endpoints take tools (OpenAI Chat, Anthropic Messages,
Responses — [DETAILS.md](DETAILS.md#using-it)). The loop is always the
same and the **agent** runs it:

1. Agent sends `messages` + `tools` (+ `tool_choice`, usually `auto`).
2. Model replies with a tool call (streamed: name first, then arguments
   piece by piece, like OpenAI/Anthropic).
3. Agent executes the tool locally, sends the result back as a
   `tool`/`tool_result` message on the next request.
4. Repeat until the model answers with text.

Server-side MCP tools (`"mcp_servers"` in `strata-<model>.json`,
[DETAILS.md](DETAILS.md#tools-from-mcp-servers)) are **only the chat
page's**: API clients "see the API exactly as before and keep their own
tools". The one exception: a `POST /v1/chat/completions` request with
`"strata_mcp": true` opts into the server running its MCP tools for
that request (then it gets `strata_mcp` tool events in the stream). An
external agent (Claude/Pi/OpenAI-compatible) normally does **not** set
this — it passes its own tools and runs them itself. Do not configure
both for the same tools or calls run twice.

Limits per answer there: up to `max_rounds` calls in a row (default 8),
each call timing out after `timeout_s` (default 60 s) into an `error:
...` result, results cut at `max_result_chars` (default 20,000)
(`serve/mcp.py:37`, [DETAILS.md](DETAILS.md#tools-from-mcp-servers)).

### 8.9 When the agent output looks wrong

Work top-down; change one thing, retry the same prompt:

1. **Look at what actually happened.** The server window prints a
   `ready: http://127.0.0.1:8080/v1` line when it is ready
   (`serve/server.py:7330`); per-request lines name refusals (context
   overflow, bad level, loop recovery). The engine log is
   `strata-<model>.log` in the PolyStrata folder. Live state:
   `GET /status`, `GET /slots`, `GET /metrics`. Via the management MCP:
   `strata_logs` with `source: server|engine|setup`.
2. **Looping / rambling / never calls the tool:** cap the thinking
   first — per-request `"reasoning_effort": "low"` (or `"none"` for a
   pure tool step), or `"reasoning_budget_tokens": N` in the config
   (`--help.md` §6). A reply that ends inside its thinking (empty
   answer, agent stops) is `"reasoning_close_retry": true`; whole-passage
   repeats are `"reasoning_loop_recovery": "stop"|"recover"`; a reply
   stuck on one token is ended by `"repeat_stop_tokens": N` (default
   256) ([DETAILS.md](DETAILS.md#using-it)).
3. **Confident nonsense / flaky tool arguments:** lower the randomness —
   `"temperature"` in `sampling` (reference install `0.6`; `0` = greedy,
   always the most likely word; `--help.md` §4,
   `serve/server.py:6882-6885`). The Sampling drawer edits the same
   keys the About tab's Model settings card writes to
   `strata-<model>.json`. Retry with a seed only to reproduce, not to
   fix.
4. **Tool calls mis-shaped:** `"tool_call_recovery": true` reads the
   common mis-shapes (details + measurement in
   [DETAILS.md](DETAILS.md#using-it)). Otherwise fix the tool schema on
   the agent side — the server passes `tools` through.
5. **Context errors (`prompt ... exceeds the context`, 400s):** see
   §8.7 — `fit_max_tokens`, smaller `max_tokens`, compact the history,
   or rerun setup with `--setup --context <bigger>` (needs the RAM/VRAM
   for the KV cache).
6. **Slow tool loop:** `none`/`low` effort for tool steps, one request
   at a time (no `parallel` for a single agent), pictures only when the
   turn needs them (§8.5), and check Monitor's context-fill and last
   request's prefill/decode rates before blaming the model.

### 8.10 Playwright + vision loop (agentic browser test)

No Playwright-specific wiring ships in this repo — the generic MCP-tools
shape applies ([DETAILS.md](DETAILS.md#tools-from-mcp-servers)):

```json
"mcp_servers": {
  "playwright": {"command": "<playwright-mcp-command>", "args": ["<its", "args>"]}
},
"mcp": {"timeout_s": 60, "max_result_chars": 20000, "max_rounds": 8}
```

Install a Playwright MCP server per its own docs, paste its `command`
(and `args`/`cwd`/`env`) as the value above (stdio transport), or its
`url` (+ `headers`) for Streamable HTTP. Restart HyperStrata; the
server window says what each server offers (`MCP server 'playwright': N
tools (...)`), the Monitor tab lists them, and the Sampling drawer has
**Use tools from MCP servers** (on by default). This loop runs **in the
chat page only** (API agents keep their own Playwright tools — §8.8).

The vision half: the Playwright screenshot reaches the model as an
image part (§8.5), so this loop needs `"vision"` in the config and
`/health` `"images": true`. Per-answer caps are `max_rounds` (default 8
tool calls), `timeout_s`, `max_result_chars` above. Long pages come back
cut with a note — ask for a clipped or downscaled screenshot when the
model only needs the layout. End-to-end Playwright+vision timing on the
reference rig is PENDING (no measured run claimed).

## 9. When something fails

| Symptom | What to do |
| --- | --- |
| `the NVIDIA driver is too old` | The user updates the driver (NVIDIA App or nvidia.com/drivers) and restarts; rerun setup. |
| Download or install stopped | Rerun the same setup command; it continues. |
| `port 8080 is already in use` | PolyStrata is already running (check `/health`), or use `--port 8081`. |
| Very slow, disk busy, or "the engine stopped unexpectedly" | Not enough free RAM: close programs, or set up a smaller size (`--setup --model Q2_0` or IQ2_XS). |
| `ExpertCache: cudaMalloc(...) failed: out of memory` with free VRAM (Windows) | The page file is off or tiny: set a fixed size (Custom size, the same initial and maximum size, e.g. 65536 MB) and restart; a page file Windows grows on demand ("System managed") may not grow in time (#60). |
| `prompt ... exceeds the context` | Rerun setup with `--setup --context <bigger>`. |
| No AMD GPU found (Linux) | The amdgpu driver lists no GPU; check the card and driver. Integrated GPUs are not supported. |
| Python or build tools could not be installed | Install what the message names (links are printed), then rerun. |

More: [TROUBLESHOOTING.md](TROUBLESHOOTING.md) and the [full table](DETAILS.md#troubleshooting). If it still fails,
collect `strata-<model>.log` and the setup output, and if the public repo has issues enabled suggest an issue at https://github.com/theacidblack/HyperStrata/issues; if not, use whatever contact channel the public repo page lists.

## Alternative: the MCP server

PolyStrata also has an MCP server, so an AI tool can check, install, start and stop PolyStrata through tool calls
(`strata_status`, `strata_models`, `strata_install`, `strata_start`, `strata_stop`, `strata_logs`) instead of the
shell commands above. See [MCP_SERVER.md](MCP_SERVER.md).

← [Prev](INSTALL.md) · [Index](INDEX.md) · [Next](MODELS.md) →
