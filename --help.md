
← [Prev](docs/TROUBLESHOOTING.md) · [Index](docs/INDEX.md) · [Next](CHANGELOG.md) →

# Launcher flags reference (`--help.md`)

How to build a launcher JSON: the JSON carries engine flags in
`args`, server behaviour in top-level keys, and text style in
`sampling`. Setup writes this file for you — hand-edit only to
change one thing you can name.

Rule of thumb: touch `sampling.temperature`, `reasoning_budget_tokens`,
and `port`. Leave everything else alone unless a log line tells you
otherwise.

Shape reference (placeholders only — put your own paths here):

```json
{
  "exe": "<engine-folder>\\hyper.exe",
  "args": [
    "--model", "<models-folder>\\Qwen3.6-35B-A3B-UD-Q4_K_M.gguf",
    "--max-context", "131072",
    "--threads", "6",
    "--tail-skip", "6",
    "--expert-profile", "<data-folder>\\packs\\qwen36-ud-q4_k_m\\expert-profile.txt",
    "--kv", "q4_0",
    "--draft-kv", "q4_0",
    "--vram-reserve-mib", "256"
  ],
  "cwd": "<install-folder>",
  "tokenizer": "<data-folder>\\packs\\qwen36-ud-q4_k_m\\tokenizer",
  "model_name": "Qwen3.6-35B-A3B-MTP-UD-Q4_K_M-Vision",
  "display_name": "Qwen3.6-35B-A3B-MTP-UD-Q4_K_M-Vision",
  "log": "<logs-folder>\\Qwen3.6-35B-A3B-MTP-UD-Q4_K_M-Vision.log",
  "lib_dirs": ["<python-site-packages>\\nvidia\\cu13\\bin\\x86_64"],
  "port": 8080,
  "sampling": { "temperature": 0.6 },
  "reasoning_budget_tokens": 4096,
  "vision": {
    "exe": "<engine-folder>\\hyper-vision.exe",
    "mmproj": "<models-folder>\\mmproj-F16.gguf",
    "model": "<models-folder>\\Qwen3.6-35B-A3B-UD-Q4_K_M.gguf",
    "gpu": false
  }
}
```

A model without the draft block (base file, no MTP) uses `--no-draft`
instead of `--draft-kv`, and has no `reasoning_budget_tokens` or
`vision` keys unless it reads images.

---

Most-used flags (kept visible):
- `--model <file>` — which weight file loads (`--config` names the launcher JSON on every real start)
- `--max-context 131072` — tokens the caches hold (prompt + answer together)
- `--kv q4_0` / `--draft-kv q4_0` — the VRAM knob (main + draft KV format)
- `sampling.temperature 0.6` + `reasoning_budget_tokens 4096` — the two text knobs worth turning
- `"port": 8080` — where the server listens

<details><summary>All flags</summary>

## 1. Engine launch flags (`args` array — what the engine does)

These live inside the JSON `"args"` list. Source of truth is the
engine's own usage text in `src/poly/main.cpp`.
REPORTED against hyper.exe --help output 2026-10-17, no log on file.

```json
"args": [
  "--model", "<models-folder>\\Qwen3.6-35B-A3B-UD-Q4_K_M.gguf",
  "--max-context", "131072",
  "--threads", "6",
  "--tail-skip", "6",
  "--kv", "q4_0",
  "--draft-kv", "q4_0",
  "--vram-reserve-mib", "256"
]
```

| Flag | What it does | When to touch it |
| --- | --- | --- |
| `--model FILE.gguf` (`src/poly/main.cpp:1031`) | Which weight file loads. | Only to point at a different downloaded file. Usually never. |
| `--serve` (implied by server; `src/poly/main.cpp:1031`) | Runs resident behind `serve/server.py` instead of one-shot `--tokens`. | Never — the server adds it. |
| `--tokens ID,...` / `--max-new N` (`src/poly/main.cpp:1031`) | One-shot run from token ids; `--max-new` caps how many new tokens are written. | Never in a launcher (server uses `--serve`). |
| `--max-context N` (`src/poly/main.cpp:1032`, default `8192`) | Tokens the attention caches hold (prompt + answer together). Setup writes e.g. `131072`. | Only when you need longer chats AND have the VRAM/RAM. Bigger = more KV memory. Usually never. |
| `--threads N` (`src/poly/main.cpp:1033`, default all usable) | CPU threads for experts not in VRAM. | Usually never (setup picks it). |
| `--vram-reserve-mib N` (`src/poly/main.cpp:1034-1037`) | VRAM left free beside the expert cache (min 256). Setup default `256`; engine fallback auto-sizes. | Only if another program (game, second server, GPU vision) needs room. Raise to free VRAM, lower to fit more experts. |
| `--layer-split S` (`src/poly/main.cpp:1038-1040`) | Splits layers across GPUs: `auto` or first layer of each later card (`20` or `20,30`). Default first card alone. | Multi-GPU only. Usually never. |
| `--expert-profile FILE` (`src/poly/main.cpp:1041-1042`) | Expert-use ranking: which experts pre-fill VRAM. | Usually never (setup writes the path). |
| `--no-gpu-experts` (`src/poly/main.cpp:1043`) | Keeps every routed expert on CPU. | Diagnostics only. Usually never. |
| `--any-node` (`src/poly/main.cpp:1044-1045`) | Lets CPU threads run on any NUMA node. Default: the node holding most experts. | Multi-socket PCs only. Usually never. |
| `--window N` (`src/poly/main.cpp:1046-1049`, default `8`, `4` for GLM) | Draft window: tokens guessed ahead per step (block guesses 3 of the 8). `1` = no guessing. | Usually never. |
| `--lookup N` (`src/poly/main.cpp:1050-1051`, default `3`, `0` = never) | Guesses repeats from earlier matching text. | Usually never. |
| `--no-draft` (`src/poly/main.cpp:1052`) | Turns off the block that guesses ahead. Base (non-MTP) launchers carry this. | Only for files with no draft block in them. Usually never. |
| `--draft-model FILE` (`src/poly/main.cpp:1053-1054`) | Takes the guessing block from another file when `--model` has none. | Usually never. |
| `--draft-vocab N` (`src/poly/main.cpp:1055-1056`, default `131072` for Qwen3.6/Ornith/Tiel, else whole vocab; `0` = all) | How many of the vocabulary's first tokens the draft guesses among. Smaller = less VRAM, slightly faster English. | Only via setup `--draft-vocab en\|cyrillic\|fr\|cjk` to save VRAM. Usually never. |
| `--draft-kv fmt` (`src/poly/main.cpp:1057-1058`, `fp16` default) | Draft block's KV cache format: `fp16`, `q8_0`, `q4_0`. Independent of `--kv`. CUDA only. | Only to save VRAM on tight cards (`q4_0`). Usually never. |
| `--no-graphs` (`src/poly/main.cpp:1059-1060`) | Launches each token's kernels one by one instead of replaying a recorded CUDA graph. | Diagnostics only. Usually never. |
| `--kv-whole` (`src/poly/main.cpp:1061-1062`) | Gives attention caches VRAM for the whole context at load. Default: grows with the chat, expert cache keeps the rest until then. | Usually never. |
| `--kv fmt` (`src/poly/main.cpp:1063-1064`, `fp16` default) | Main attention KV format: `fp16`, `q8_0`, `q4_0`. Less VRAM per token, slightly different answers. CUDA only on hyper families. | Only to fit longer context (`q4_0` halves KV). Usually never. |
| `--expert-swaps N` (`src/poly/main.cpp:1065-1067`, default `64`, `0` = frozen cache) | Experts swapped into VRAM mid-answer. | Usually never. |
| `--prompt-batch N` (`src/poly/main.cpp:1068`, default `1024`) | Prompt tokens read at a time (~1.4 MiB VRAM each). | Usually never. |
| `--stage-mib N` (`src/poly/main.cpp:1069-1071`, default `2048` or one layer's experts if less) | VRAM staging for experts outside the cache during a prompt batch. | Usually never. |
| `--stage-ahead N` (`src/poly/main.cpp:1072-1073`, default `1024`, `0` = never) | From N tokens, copies those experts before routing. | Usually never. |
| `--cpu-share F` (`src/poly/main.cpp:1074-1075`) | Share of off-cache experts the CPU computes meanwhile. Default auto from timings; `0` = none. | Usually never. |
| `--prompt-cache N` (`src/poly/main.cpp:1076-1077`, default `6`, `0` = re-read from start) | Conversation checkpoints kept (~150 MB RAM each). | Usually never. |
| `--prompt-cache-every N` (`src/poly/main.cpp:1078-1079`, default `2048`, `0` = none) | Checkpoint every N prompt tokens. | Usually never. |
| `--conversation-cache-mib N` (`src/poly/main.cpp:1080-1081`, default `0` = off) | RAM for parked conversations so several take turns without re-reading. | Opt-in for multi-chat servers. Usually never. |
| `--conversation-cache-slots N` (`src/poly/main.cpp:1082`, default `4`) | Max parked conversations. | With the above. Usually never. |
| `--conversation-cache-min-tokens N` (`src/poly/main.cpp:1083`, default `1024`) | Shorter conversations are not parked. | With the above. Usually never. |
| `--conversation-cache-min-free-mib N` (`src/poly/main.cpp:1084-1085`, default `2560`) | Parks only while this much RAM stays free. | With the above. Usually never. |
| `--batch N` / `--slots N` (`src/poly/main.cpp:1086-1088`, default `0` = one at a time, max `8`) | N answers written together. JSON key `"parallel"` writes `--batch` for you (`serve/server.py:2651-2669`, cap `serve/server.py:2648`). | Opt-in for serving several people. Usually never (each slot takes expert-cache VRAM). |
| `--check-batch K` (`src/poly/main.cpp:1089-1090`) | With `--tokens` and `--batch` K or more: answers K prompts alone then together, compared token by token. | Never in a launcher. |
| `--eos-ids ID,...` (`src/poly/main.cpp:1091`, default GGUF's eos/eot/eom) | Tokens that end a reply. | Usually never. |
| `--temperature F --top-k N --top-p F --min-p F --seed N` (`src/poly/main.cpp:1092`, default greedy) | Sampling for one-shot `--tokens` runs only. Server requests carry their own (section 4). | Never in a launcher. |
| `--images FILE` / `--image-pad ID` (`src/poly/main.cpp:1093-1094`) | One-shot picture input / placeholder token. | Never in a launcher (server `vision` section handles images). |
| `--dump-layers` (`src/poly/main.cpp:1095`) | Prints a checksum of every layer's stages to stderr. | Diagnostics only. Usually never. |
| `--top-probs N` (`src/poly/main.cpp:1096`) | With `--tokens`: prints the N most likely ids of every step to stderr. | Diagnostics only. Usually never. |
| `--follow ID,...` (`src/poly/main.cpp:1097-1098`) | With `--tokens`: writes these tokens in place of the ones picked, for `--top-probs` of the same answer under other settings. | Diagnostics only. Usually never. |
| `--tail-skip N` (`src/poly/main.cpp:1099-1101`, default `0`; setup gives Qwen3.6 `6`) | Skips a rarely-used off-VRAM expert: faster, answers move a little. `STRATA_ROUTE_TAIL_SKIP=N` env does the same (`src/poly/main.cpp:1102`; `0` turns it off). | Usually never (setup sets the family value). |
| `--expert-profile-save P` / `--expert-profile-save-every MIN` — base `strata` engine only, the `hyper` engine rejects this flag (`src/program/generate.cpp:869-872`, default every 10, `0` = at QUIT only) | Saves what the adaptive tier learned. JSON keys `expert_profile_save` / `expert_profile_save_every` (`serve/server.py:2687+`). | Opt-in, base `strata` engine only. Usually never. |
| `--vram-elastic` / `--vram-segment-mib N` — base `strata` engine only, the `hyper` engine rejects this flag (JSON `vram_elastic` / `vram_segment_mib`, `serve/server.py:2637-2643`, default segment `512`) | Expert cache in segments so `/v1/vram` can lend VRAM out and back. | Opt-in, base `strata` engine only. Usually never. |
| `--split-skip-if-fits` — base `strata` engine only, the `hyper` engine rejects this flag (JSON `split_skip_if_fits`, `serve/server.py:2634-2636`) | Auto multi-GPU split runs on one card when everything fits. | Multi-GPU opt-in, base `strata` engine only. Usually never. |
| `--vision` — base `strata` engine only, the `hyper` engine rejects this flag (auto-added for non-poly engines with a vision section, `serve/server.py:2611-2631`) | Lets the engine accept picture embeddings. Hyper (`hyper`/`strata-poly` binary) needs no flag — pictures work whenever a request has them. | Never hand-add; setup writes it. |

---

## 2. Server flags (`python -m serve.server ...` CLI)

Command-line switches for the server process itself.
`serve/server.py:7091-7136`. Every one (except `--engine`/`--script`)
has a JSON twin — prefer the JSON key so restarts keep it.

```json
{
  "port": 8080,
  "host": "127.0.0.1",
  "gpu": 0,
  "fit_max_tokens": false,
  "api_key": "put-a-key-here-when-exposed",
  "idle_unload_s": 0,
  "min_free_vram_mib": 0,
  "api_monitor": false
}
```

| Flag (+ JSON twin) | What it does | When to touch it |
| --- | --- | --- |
| `--engine mock\|hyper\|strata` (`serve/server.py:7093`, default `mock`) | `mock` = scripted answers for tests; `hyper`/`strata` = the real engine via `--config`. | Never in daily use (launchers use `hyper`). |
| `--config FILE` (`serve/server.py:7094-7095`) | The launcher JSON this document describes. | Always — every real start names one. |
| `--host ADDR` / `"host"` (`serve/server.py:7096-7098`, default `127.0.0.1`) | Listen address. `127.0.0.1` = this PC only; `0.0.0.0` = other devices too (then set an API key). | Only to reach it from another device. Usually never. |
| `--script TEXT` (repeatable; `serve/server.py:7099-7101`) | Mock engine's reply. | Mock testing only. Usually never. |
| `--port N` / `"port"` (`serve/server.py:7102`, default `8095`; setup installs use `8080`) | Port to listen on. | Only when the port is taken or you run two models. |
| `--gpu G` / `"gpu"` (`serve/server.py:7103-7104`; `"0,2"` or `[0,2]` = layer split, `serve/server.py:2512-2519`) | Card(s) as `nvidia-smi` numbers them; first is main. | Multi-GPU only. Usually never. |
| `--tokenizer DIR` (`serve/server.py:7105-7106`) | Pack tokenizer directory (falls back to byte tokens if missing). | Never — JSON `"tokenizer"` already names it. |
| `--open` / `"open_browser"` (`serve/server.py:7107`; `serve/runconfig.py:40`) | Opens the chat page when ready. | Personal taste. Usually never. |
| `--fit-max-tokens` / `"fit_max_tokens"` (`serve/server.py:7108-7110`; `serve/runconfig.py:26`) | Clamps an over-long `max_tokens` instead of answering 400. Default: reject. | Compatibility with fussy clients. Usually never. |
| `--api-key KEY` / `"api_key"` (`serve/server.py:7111-7113`) | Requires the key on `/v1/*` (`Bearer` or `x-api-key`); commas separate several. Empty key = no auth plus a warning (`serve/server.py:7208-7221`). | Always set one when `--host 0.0.0.0`. Otherwise usually never. |
| `--mcp-config FILE` / `"mcp_servers"` (`serve/server.py:7114-7116`) | Tool servers for the web app's chat (Claude-Desktop format). Only requests with `"strata_mcp": true` see them. | Only when adding tools. Usually never. |
| `--lazy` / `"lazy_load"` (`serve/server.py:7117`; `serve/runconfig.py:37`) | Starts API unloaded; engine (+text) loads on first request. Text-only: refused with a vision section (`serve/runconfig.py:96-97`). | Slow starters / shared rigs. Usually never. |
| `--api-monitor` / `"api_monitor"` (`serve/server.py:7118-7120`; `serve/runconfig.py:39`) | Keeps last 100 prompts+answers in memory at `/api-monitor`. Off by default. | Debugging only (needs API key if exposed). Usually never. |
| `--idle-unload SEC` / `"idle_unload_s"` (`serve/server.py:7121-7124`; `serve/runconfig.py:36`) | Unloads the model after SEC idle seconds so games/other servers get VRAM; next request reloads. Default never. | Shared-GPU PCs. Usually never. |
| `--min-free-vram-mib N` / `"min_free_vram_mib"` (`serve/server.py:7125-7127`) | Loads an unloaded model only with this much free VRAM, else 503. Default always loads. | With idle-unload on busy PCs. Usually never. |
| `--before-load CMD` / `"before_load"` (+ `before_load_timeout_s`, `before_load_ignore_failure`, `before_load_strict`; `serve/server.py:7128-7129,7243-7250`) | Command run before reloading (e.g. unload another server). String or list. | Multi-server rigs. Usually never. |
| `--slot-save-path DIR` / `"slot_save_path"` (`serve/server.py:7130-7132,7251-7260`) | Turns on `POST /slots/0?action=save\|restore` (llama-server API) into `DIR/NAME` (~1.2 GB per 63K-token chat — PENDING: no measured log on file). | Session hand-off. Usually never. |
| `--replicas N` / `"replicas"` (`serve/server.py:7133-7136`) | Data-parallel replicas: N engines on equal GPU groups, requests spread over them. | Multi-GPU throughput. Usually never. |

---

## 3. JSON keys (types + defaults)

Top-level launcher keys. Types: `str` = text, `int`/`num` = number,
`bool` = `true`/`false`. "Default" = what happens when the key is absent.

```json
{
  "exe": "<engine-folder>\\hyper.exe",
  "args": ["--model", "<models-folder>\\model.gguf"],
  "cwd": "<install-folder>",
  "tokenizer": "<data-folder>\\packs\\model\\tokenizer",
  "model_name": "my-model",
  "display_name": "My Model",
  "log": "<logs-folder>\\model.log",
  "lib_dirs": ["<cuda-libs-folder>"],
  "port": 8080,
  "gpu": 0
}
```

| Key | Type / default | What it does |
| --- | --- | --- |
| `"exe"` (`serve/server.py:6977,2790+`) | `str`, required | Engine binary. Relative paths resolve against `"cwd"`. |
| `"args"` (`serve/server.py:2611-2645`) | list of `str`, required | Engine flags from section 1, plus what the server adds itself: `--layer-split`, `--batch` (from `"parallel"`), `--vision` (non-poly engines only), and the opt-in strata-only keys `--split-skip-if-fits`, `--vram-elastic` / `--vram-segment-mib`, `--expert-profile-save` — the `hyper` engine rejects those three, so leave them off for hyper. |
| `"cwd"` (`serve/server.py:2697-2702,6975-6977`) | `str`, default server's folder | Engine's working directory; relative `"exe"`/vision paths resolve here. |
| `"tokenizer"` (`serve/server.py:7149-7154`) | `str` | Pack tokenizer dir (must hold `vocab.json`). |
| `"model_name"` / `"display_name"` | `str` | API model id / browser tab name. |
| `"aliases"` (`serve/server.py:7202-7207`; `serve/runconfig.py:35`) | list of `str` | Extra names the model answers to. |
| `"log"` (`serve/server.py:815,827-831`) | `str` | Engine stderr log (start narrator + per-request lines read from here). |
| `"lib_dirs"` (`serve/server.py:2841`) | list of `str` | Extra DLL folders searched (CUDA libs). Missing folders skipped. |
| `"env"` (`serve/server.py:2839`) | object | Extra environment for the engine (e.g. AMD tuning table). |
| `"port"`, `"host"`, `"gpu"` | `int` / `str` / `int\|list\|str` | See section 2. |
| `"layer_split"` (`serve/server.py:2572-2602`) | `"auto"` or e.g. `"18"` / `[18, 32]`; default `"auto"` | First layer of each later GPU (counts per card are rejected with an example). |
| `"backend"` (`serve/server.py:2811,2834,7304`) | `"cuda"` / `"hip"` | Which GPU stack's readings/visibility apply. |
| `"vram_reserve_mib"` (`serve/runconfig.py:41-42`) | `int>=0`, engine default auto-sized (a thirty-second of the card, 256-768; or a sixteenth, 512-1024, when free RAM is within 4 GiB of the model files), min 256 | Same as `--vram-reserve-mib` but editable from Settings; stored into `args`. |
| `"parallel"` (`serve/server.py:2651-2669`) | `int`, default off | Up to N requests decode together (`--batch N`, max `8`). `1`/absent = one at a time. |
| `"vram_elastic"` / `"vram_segment_mib"` (`serve/server.py:2637-2643`) | `bool` / `int`, default off / `512` | Segmented expert cache for `/v1/vram` lending. Base `strata` engine only: it adds `--vram-elastic`, which the `hyper` engine rejects. |
| `"split_skip_if_fits"` (`serve/server.py:2634-2636`) | `bool`, default off | One-card run when the split fits there. Base `strata` engine only: it adds `--split-skip-if-fits`, which the `hyper` engine rejects. |
| `"vram"`-related / `"expert_profile_save"` (+ `_every`) (`serve/server.py:2687+`) | `str` / `num` | Learned-profile save path/interval (opt-in). Base `strata` engine only: it adds `--expert-profile-save`, which the `hyper` engine rejects. |
| `"engine_silence_s"` (`serve/server.py:2561-2569`; `serve/runconfig.py:38`, default `300`, `0` = wait forever) | `num>=0` | Ends a request whose engine prints nothing for this long. |
| `"repeat_stop_tokens"` (`serve/server.py:7278-7281`, default `256`, `0` = off) | `int>=0` | Ends a reply repeating one token this many times. |
| `"reasoning_loop_recovery"` (`serve/server.py:7292-7298`, default off) | `false` / `"stop"` / `"recover"` (`true` = recover) | What to do when thinking repeats whole passages. |
| `"reasoning_close_retry"` (`serve/server.py:7170`, default off) | `bool` | Continues once a reply that stops inside `<think>` with thinking closed. |
| `"tool_call_recovery"` (`serve/server.py:7265-7268`, default off) | `bool` | Repairs tool calls. |
| `"warm_start"` (`serve/server.py:7269-7272`, default off) | `bool` | Keeps the system-prompt prefix cached. |
| `"preserve_thinking"` (`serve/server.py:7172`; `serve/runconfig.py:29-30`, default off) | `bool` | Keeps earlier turns' thinking in the prompt (Qwen3.6/Ornith/Tiel templates). |
| `"drop_oldest"` (`serve/server.py:7171`; `serve/runconfig.py:27-28`, default off) | `bool` | Drops oldest turns instead of 400 when a chat overflows. |
| `"anthropic_thinking"` (`serve/server.py:7261-7264`; `serve/runconfig.py:31-32`, default `"model"`) | `"model"` / `"on_request"` | Anthropic requests without thinking asked: think as the model does, or not. |
| `"effort_position"` (`serve/server.py:2536-2558`; `serve/runconfig.py:33-34`, default `"start"`) | `"start"` / `"end"` | Where a non-default effort goes (`"end"` keeps the cache; needs engine 0.1.39+). |
| `"reasoning_effort"` (`serve/server.py:7192-7199,6829-6854`) | `"none"` / `"low"` / `"medium"` / `"high"` | Thinking level for requests naming none. NOTE: never put `--reasoning-effort` in `args` — it is lifted out into this key with a notice. |
| `"cors_origins"` / `"trusted_origins"` / `"allowed_hosts"` (`serve/server.py:7222-7230`) | list / list / name-or-list (`"*"` = any host) | Browser/API access control. Leave alone unless exposing the server. |
| `"prevent_sleep"` (`serve/server.py:7188-7191`, default off, Windows only) | `bool` | Keeps the PC awake while serving. |
| `"mcp_servers"`, `"before_load*"`, `"slot_save_path"`, `"idle_unload_s"`, `"min_free_vram_mib"`, `"api_monitor"`, `"lazy_load"`, `"fit_max_tokens"`, `"open_browser"`, `"api_key"` | see sections 2/4 | Server behaviour keys (defaults off/empty = previous behaviour). |
| `"vision"` | object (section 5) | Image encoder config. Absent = text only. |
| `"images"` (preview) (`serve/server.py:7173-7187`) | object | Image *generation* (Qwen-Image via `serve/images.py`), separate from `vision` (reading pictures). Missing files = requests fail until setup re-runs. |
| `"codex_thread_titles"` / `"codex_compaction_cache"` (`serve/server.py:7200-7201`, default off) | `bool` | Opt-in Codex behaviours. |
| `"memory_debug"` (`serve/server.py:7308`; env `$STRATA_MEMORY_DEBUG`, `serve/server.py:3153-3154`) | `bool` | Extra free-VRAM logging. |
| `"env_to_user"` (`serve/server.py:7273-7277`, default off) | `bool` | Forwards environment text as a user turn. |

Settings-view shortcut: `sampling.*`, `reasoning_budget_tokens`,
`fit_max_tokens`, `drop_oldest`, `preserve_thinking`,
`anthropic_thinking`, `effort_position`, `aliases`, `idle_unload_s`,
`lazy_load`, `engine_silence_s`, `api_monitor`, `open_browser`,
`vram_reserve_mib` are the editable keys (`serve/runconfig.py:19-43`).

---

## 4. Sampling keys (`sampling` block — how the text sounds)

Defaults for requests that send no sampling of their own. A request's
own fields always win; `null` falls back to the default; a bad value
refuses to start the server (`serve/server.py:6862-6925`). Without a
block the engine is greedy. Aliases `repeat_penalty` /
`repeat_last_n` are llama.cpp spellings of the same two settings
(`serve/server.py:6857-6877`).

```json
"sampling": {
  "temperature": 0.6,
  "top_p": 0.95,
  "top_k": 20,
  "min_p": 0.0,
  "repetition_penalty": 1.0,
  "penalty_last_n": 64,
  "seed": 1234
}
```

| Key (rule) | What it does | When to touch it |
| --- | --- | --- |
| `"temperature"` (`>=0`, `0` = greedy; `serve/server.py:6882-6885`; engine key `serve/server.py:1162-1164`) | Randomness. Reference install uses `0.6`. | The one knob worth turning: lower = steadier, higher = wilder. |
| `"top_p"` (`0 < x <= 1`; `serve/server.py:6886-6889`) | Picks from the smallest set covering probability P. | Fine-tuning variety. Usually never. |
| `"min_p"` (`0 <= x <= 1`; `serve/server.py:6890-6893`) | Drops tokens far less likely than the best. | Fine-tuning. Usually never. |
| `"top_k"` (integer `1..64`; `serve/server.py:6894-6897`; wider/`0` become `64`, `serve/server.py:1168-1171`) | Picks among the K most likely tokens. | Fine-tuning. Usually never. |
| `"presence_penalty"` (`>=0`; `serve/server.py:6898-6901`) | Discourages reusing any token already used. | Repetitive answers. Usually never. |
| `"frequency_penalty"` (`>=0`; `serve/server.py:6903-6905`) | Discourages tokens by how often used. | Repetitive answers. Usually never. |
| `"repetition_penalty"` (`>0`, `1` = off; `serve/server.py:6906-6909`) | Penalises used tokens (`penalty_repeat` engine-side, `serve/server.py:1175-1193`). | Repetitive answers. Usually never. |
| `"penalty_last_n"` (non-negative int; `serve/server.py:6910-6913`; default window `64`, `serve/server.py:1187-1193`) | How many last tokens penalties look at. | With one of the penalties. Usually never. |
| `"seed"` (positive int; `serve/server.py:6914-6917`; engine key `serve/server.py:1194-1196`) | Repeatable answers. | Testing only. Usually never. |
| `"experimental_speed_projection"` (`true`/`false`; `serve/server.py:6918-6922`; engine key `cvec=`, `serve/server.py:1231-1236`) | Per-request on/off for the experimental control vector (engine needs `--control-vector-scaled`). | Experiment only. Usually never. |

Request-only keys (never in the `sampling` block): `image_pad`,
`strata_tune` (`pcie_frac`, `spec_min_p`, `aux_cpus`),
`strata_checkpoint: false` (one-shot, no checkpoint),
`strata_img_keys`, `strata_prefix`, `logit_bias`
(`serve/server.py:1197-1229`). Shared chat settings also accept
`max_tokens` and `reasoning_effort` (`serve/server.py:6788-6826`).

---

## 5. Vision section (letting the model read images)

Pictures go through the `hyper-vision` helper + mmproj — see [BACKENDS.md](docs/BACKENDS.md).

```json
"vision": {
  "exe": "<engine-folder>\\hyper-vision.exe",
  "mmproj": "<models-folder>\\mmproj-F16.gguf",
  "model": "<models-folder>\\Qwen3.6-35B-A3B-UD-Q4_K_M.gguf",
  "gpu": false
}
```

| Key | What it does | When to touch it |
| --- | --- | --- |
| `"exe"` (`serve/server.py:2269`) | Vision helper binary. | Never (setup writes it). |
| `"mmproj"` (`serve/server.py:2269`) | Image encoder file for this model. | Never (setup downloads it). |
| `"model"` (`serve/server.py:2269`) | Same GGUF as `--model` (tells the encoder the text model). | Never. |
| `"gpu"` (`serve/server.py:2270-2271`; setup `--vision yes\|no\|gpu\|cpu`, `setup.py:6939-6940`) | `true`/`"gpu"`/`"yes"` = encoder on GPU (default 1024 image tokens); `false`/`"cpu"` = CPU (default 300). A GPU encoder + vision section without `--vram-reserve-mib` gets reserve `700` on old engines (`serve/server.py:2627-2629`). | Only via setup `--vision`. Usually never. |
| `"threads"` (`serve/server.py:2272-2273`) | Encoder CPU threads. | Usually never. |
| `"max_tokens"` (`serve/server.py:2274-2275`; setup `--vision-tokens N`, `setup.py:6941-6944`) | Most image tokens one picture becomes (GPU default `1024`, CPU `300`). More = reads small text better, slower to encode, remembered per model. | Only for tiny text/charts. Usually never. |
| `"min_tokens"` (`serve/server.py:2276-2277`, hand-edited) | mtmd's `image_min_tokens`. | Usually never. |
| `"elastic"` + top-level `"vram_elastic"` (`serve/server.py:2284-2290,6948-6960`) | Encoder starts with the first picture, stops after idle; expert cache keeps `elastic_mib` free meanwhile. Needs both flags or it starts with the model (with a notice). | VRAM-tight + rare pictures. Usually never. |
| `"elastic_mib"` (`serve/server.py:2288`, default `2048`) | VRAM kept free for the elastic encoder. | With elastic. Usually never. |
| `"elastic_idle_s"` (`serve/server.py:2289`, default `120`) | Seconds without a picture before it stops. | With elastic. Usually never. |
| `"cuda_device"` (`serve/server.py:2891-2904`) | Puts the encoder on one card alone (`CUDA_VISIBLE_DEVICES` / `HIP_VISIBLE_DEVICES`). | Spare-GPU rigs. Usually never. |

Temp files avoid paths with spaces (user names with spaces break
`ENC` lines), preferring a spaceless temp dir, then cwd, then drive
root (`serve/server.py:2248-2265`).

---

## 6. Thinking budget (how long it thinks before answering)

```json
{
  "reasoning_budget_tokens": 4096,
  "reasoning_effort": "high"
}
```

| Key | What it does | When to touch it |
| --- | --- | --- |
| `"reasoning_budget_tokens"` (`serve/server.py:3454-3465,7283-7291`; editable, `serve/runconfig.py:25`) | Caps every request's thinking at N tokens (`0`/absent = no cap; a request can set its own or `0` to opt out; non-integer = 400). When the budget would eat the answer, at least `512` tokens stay for it (`serve/server.py:158,4415`). Thinking closes with the model's end tag (`serve/server.py:159-163`). | The second knob worth turning: lower = sooner answers, higher = harder problems. Reference install uses `4096`. |
| `"reasoning_effort"` (`serve/server.py:7192-7199`; allowed `none\|low\|medium\|high`, `serve/server.py:6788-6826`) | Thinking level for requests naming none. Per-request `reasoning_effort` / Responses-style `reasoning.effort` / `chat_template_kwargs` win (`serve/frontend.py:576-586`). Budget asks imply thinking (`serve/frontend.py:678-685`). | Taste / task difficulty. Usually the budget matters more. |
| `"effort_position"` (`"start"` default / `"end"`; `serve/server.py:2536-2558`) | Where non-default effort goes. `"end"` keeps the cached chat when effort changes (needs engine `--tail-role-token`, i.e. 0.1.39+; strata-only — on hyper it falls back to `"start"` with a notice). | Cache-sensitive apps. Usually never. |
| `"reasoning_loop_recovery"` / `"reasoning_close_retry"` | Loop handling (`false`/`"stop"`/`"recover"`) and one continuation when a reply stops inside `<think>` (both opt-in, off default). | Stuck-thinking diagnoses. Usually never. |

Effort sentences (low/medium/xhigh) are fixed prompts
(`serve/server.py:137-145`); request level maps from budget size
(`serve/frontend.py:162`).

---

## 7. Environment variables (advanced — prefer flags/JSON)

Plain rule: if a flag or JSON key exists, use it — env vars are for
harness runs and debugging. Dual-read note: **only the four timeouts
below read `HYPER_*` first and fall back to legacy `STRATA_*`** (with a
deprecation print, `serve/server.py:250-259`). Everything else in this
checkout is `STRATA_*`-only.

```powershell
$env:STRATA_API_KEY = "key-here"
$env:HYPER_ENGINE_READY_S = "900"
```

| Variable | Default | What it does |
| --- | --- | --- |
| `HYPER_ENGINE_READY_S` / legacy `STRATA_ENGINE_READY_S` (`serve/server.py:280`) | `900` s, `0` = wait forever | How long the engine's first `READY` may take (cold load, slow disk). |
| `HYPER_VISION_READY_S` / legacy `STRATA_VISION_READY_S` (`serve/server.py:278`) | `300` s | How long the vision encoder's `READY` may take. |
| `HYPER_VISION_ENCODE_S` / legacy `STRATA_VISION_ENCODE_S` (`serve/server.py:279`) | `300` s | How long one picture encode may take. |
| `HYPER_ENGINE_STALL_S` / legacy `STRATA_ENGINE_STALL_S` (`serve/server.py:268`, needs `psutil`) | `90` s, `0` = off | Frozen-engine watchdog: silent AND no CPU/I-O for this long = ended (slow-but-working engines are never ended, only reported). |
| `STRATA_API_KEY` (`serve/server.py:7111,7208-7218`) | empty = no auth | Same as `--api-key` / `"api_key"`. Empty-but-set warns. |
| `STRATA_ALLOWED_HOSTS` (`serve/server.py:7225`) | empty | Extra `Host:` names beside `"allowed_hosts"` (DNS-rebinding guard). |
| `STRATA_START_RETRIES` (`serve/server.py:544`) | `3`, `0` = never | Retries of a start that dies inside GPU-runtime init (≤30 s, runtime marks or silent exit — never a crash, `serve/server.py:547-570`). Waits 2/4/8 s. |
| `STRATA_REQUEST_LINES` (`serve/server.py:574-593,823-825`) | off (`=1` on) | One stdout line per finished request (for trays/supervisors). |
| `STRATA_BATCH_TRACE` (`serve/server.py:739`) | off | Batch debug chatter. |
| `STRATA_PARALLEL_SOLO` (`serve/server.py:1503-1505`) | on (`=0` keeps solo) | Batch-slot cache behaviour detail. |
| `STRATA_BATCH_MTP` (`serve/server.py:2666`) | `0` | Lets `"parallel"` above `8` through (engine warns/adjusts). |
| `STRATA_MEMORY_DEBUG` (`serve/server.py:3153-3154`) | off | Free-VRAM logging around loads. |
| `STRATA_DEBUG` (`serve/server.py:4830,4936-4937`) | off (`=1` on) | Raw-model-text / blank-turn diagnostics. |
| `STRATA_MAX_BODY_MIB` (`serve/server.py:5312-5315`) | built-in limit | Raises the HTTP body cap (`0`/unreadable = default). |
| `STRATA_HTTP_BACKLOG` (`serve/server.py:6495-6496`) | `256` (min `5`) | Listen queue depth. |
| `STRATA_REPLICA_START` (`serve/server.py:7024`) | parallel (`=serial` serialises) | Replica engine start order. |
| `STRATA_DROP_CACHE_ON_EXIT` (`serve/cache_drop.py:1-16`) | off (`=1` on, non-Windows) | Gives model files' page cache back on stop. |
| `STRATA_KEEP_EMPTY_TURNS` (`serve/frontend.py:65-67`) | new behaviour (`=1` old) | Renders empty turns the old way. |
| `STRATA_SYCL_BIN` (`serve/server.py:2522-2533`) | `build-sycl-aot/strata` | Binary behind the Intel `strata-sycl.sh` launcher. |
| `STRATA_ROUTE_TAIL_SKIP` (engine env, `src/poly/main.cpp:1102`) | model's default | Same as `--tail-skip` (`0` off). Engine flag wins ties conceptually — set one, not both. |
| Setup-time only (`setup.py`): `STRATA_SOURCE` (`setup.py:145`, `--source`, `setup.py:6992-6995`), `STRATA_CUDA` (`setup.py:6999`), `STRATA_PREBUILT_URL` (`setup.py:7003,5829`), `STRATA_POLY_PREBUILT_URL` (`setup.py:755`), `STRATA_ISA_FLOOR` (`setup.py:1238`), `STRATA_EXPERIMENTAL_SM60` (`setup.py:1412,1427`), `STRATA_NVCC` (`setup.py:1848,4075`), `STRATA_VCVARS` (`setup.py:1891`), `STRATA_ROCM_VERSION` / `STRATA_ROCM_INDEX` / `STRATA_EXPERIMENTAL_GFX1103` (`setup.py:2446-2472`), `STRATA_SKIP_SHA256` (`setup.py:3708`), `STRATA_EXECV` (`setup.py:5455`), `HF_ENDPOINT` / `MODELSCOPE_ENDPOINT` (`setup.py:98-128`) | various | Download host, toolchain picks, build overrides. Set only when the corresponding setup flag cannot say it. Usually never. |

---

## 8. Setup / build flags (getting the engine + files — not launchers)

Setup asks four questions (model, size, context, images) and remembers
them; flags below pre-answer or override. Install procedure: [HOW-TO-INSTALL.md](docs/HOW-TO-INSTALL.md). Full help in
`setup.py:32-48,6908-7038`.

```powershell
python setup.py --family qwen36 --model UD-Q4_K_M --context 131072 --vision yes
```

| Flag | What it does | When to touch it |
| --- | --- | --- |
| `--family qwen\|swift\|coder\|unsloth\|nvfp4\|glm\|deepseek\|lfm25\|qwen36\|ornith\|gemma4\|...` (`setup.py:6912-6917`) | Model line. This fork's verified path is `--family qwen36` (Qwen3.6-35B-A3B on hyper). | Every fresh install names one. |
| `--model UD-Q4_K_M\|UD-Q5_K_M\|...` (`setup.py:6918-6920` lists every family's sizes; sizes `setup.py:268-356,539-610`) | Quant size. Small = less RAM/VRAM, weaker; big = reverse. Only the qwen36 UD sizes have fork coverage (see docs/MODELS.md); other families' sizes are upstream-only, untested here. | Every fresh install (or accept setup's recommendation). |
| `--context N` (`setup.py:6921`; choices `setup.py:428`; trained length `262144`) | Context to serve. Past trained length auto-adds yarn scaling (factor = final/262144). | Only for longer chats with the RAM/VRAM to hold them. |
| `--rope-scaling none\|linear\|yarn` / `--rope-scale F` (`setup.py:6922-6928`) | Extension past `262144`. Omitted = yarn; `none` refused there. | Usually never. |
| `--kv int8\|q4_0\|k8v4` (Strata engine) or `fp16\|q8_0\|q4_0` (hyper `qwen35moe`: qwen36/ornith/tiel/...) (`setup.py:6929-6934`; formats `setup.py:770`) | KV precision. `q4_0` halves KV memory, slightly less precise; `k8v4` hybrid on the old engine. Defaults: `int8` (old engine), `fp16` (hyper). | Only to fit longer context. Usually never. |
| `--draft-kv fp16\|q8_0\|q4_0` (`setup.py:6935-6938`) | Draft KV format where a draft block runs; CUDA only. | Tight VRAM only. Usually never. |
| `--vision yes\|no\|gpu\|cpu` (`setup.py:6939-6940`) + `--vision-tokens N` (`setup.py:6941-6944`) | Image reading + tokens per picture (GPU `1024`, CPU `300`). | When you want pictures. |
| `--gguf-dir DIR` / `--models-dir DIR` / `--data-dir DIR` (`setup.py:6978-6982`) | Reuse local GGUFs / place downloads. | Re-installs, moved disks. |
| `--backend cuda\|hip\|sycl` (`setup.py:7034-7037`) | NVIDIA / AMD / Intel Arc. Auto-picks HIP with no usable NVIDIA card. | Usually never (auto is right). |
| `--cuda 12\|13\|auto` (`setup.py:6999-7002`) | CUDA toolkit line. `13` default; `12` for Pascal/Volta cards or old drivers. | Old cards only. |
| `--build` (`setup.py:6991`) | Compiles instead of the ready-made engine. | Only when no ready-made build fits. Needs compilers + time. |
| `--low-ram auto\|on\|off\|resident\|mmap` (`setup.py:7013-7017`) | Reads experts from disk instead of copying all to RAM. | Big-GPU/little-RAM PCs. Usually never. |
| `--resident-budget-gib N` (`setup.py:7018-7020`) | GiB of experts kept in RAM (default RAM minus 24, 40 on 64 GB). | Unsloth big files on mid RAM. Usually never. |
| `--kv-streaming auto\|on\|off` (`setup.py:7031-7033`) | From 64K: KV in RAM, attention window in VRAM (more experts fit GPU). | Long context on small VRAM. Usually never. |
| `--draft-vocab cjk\|en\|cyrillic\|fr` (`setup.py:7008-7012,5467`) | Draft token subset (`en` saves ~110 MiB VRAM, 1–2% faster English; `cyrillic` ~30% faster decode there; `fr` +18% accepts in French). | Tight VRAM or those languages. Usually never. |
| `--gpus` / `--gpu` / `--layer-split` / `--no-remote-expert-opt` (`setup.py:6960-6969`) | Multi-GPU placement. | Multi-GPU only. |
| `--parallel N` / `--replicas N` (`setup.py:7024-7030`) | Batch slots / independent engines (section 1/2). | Serving many. Usually never. |
| `--port` / `--host` / `--api-key` / `--browser\|--no-browser` (`setup.py:6959-6977`) | Where it listens, key, browser open. `--host 0.0.0.0` needs `--api-key`. | Exposure changes only. |
| `--source auto\|huggingface\|modelscope` (`setup.py:6992-6995`) | Download host (China: `modelscope`). | Region-blocked downloads. |
| `--check` / `--calibrate` / `--update` / `--rollback-engine` / `--setup` / `--no-start` / `--yes` (`setup.py:6983-6990,7005-7007`) | Inspect/tune/update/re-configure without starting. | Maintenance. |
| `--vram-reserve-mib N` (`setup.py:7021-7023`, engine default auto-sized: a thirty-second of the card, 256-768, or a sixteenth, 512-1024, when free RAM is within 4 GiB of the model files; min 256) | Same reserve as section 1, set at install. | Shared-GPU installs. |

Build (compile-it-yourself) cmake defines (`CMakeLists.txt`, `setup.py:3358-3363,4315-4332,6146-6208`): `STRATA_ENABLE_CUDA=ON` (NVIDIA) /
`STRATA_ENABLE_HIP=ON` + `CMAKE_HIP_ARCHITECTURES` (AMD) /
`STRATA_ENABLE_SYCL` (Intel Arc, experimental, exclusive) ·
`CMAKE_CUDA_ARCHITECTURES=120` (default; sm_75+; `STRATA_EXPERIMENTAL_SM60=ON`
for Pascal sm_60/Volta sm_70) · `STRATA_MMQ_KQUANTS=ON` (Unsloth K-quant
prompt kernels) · `STRATA_Q6K_EXPERTS=ON` (Q5/Q6_K grouped experts) ·
`STRATA_PREFILL_MMQ=ON` (HIP-only) · `STRATA_PORTABLE=OFF` (AVX2 baseline
release) / `STRATA_ISA_FLOOR` (`""`/`avx`/`none`, older CPUs) ·
`STRATA_BUILD_TESTS=OFF` for installs · vision helper:
`LLAMA_DIR` + `STRATA_VISION_CUDA=ON|OFF`. Touch these only when
compiling for a card the ready-made engine does not cover — usually
never.

</details>

---

*Flag counts not checked here — count from the tables above. Sources: `serve/server.py` (CLI `7093-7136`; sampling
`6862-6925`; engine keys `1160-1236`; vision `2267-2290,2891-2904`;
parallel `2648-2669`; engine-args `2611-2645`; budget `3454-3465`;
timeouts `250-280`), `src/poly/main.cpp` (usage `1029-1102`), `setup.py`
(CLI `6912-7038`; sizes `268-610`; KV `770`; build `3358-4332`),
`serve/runconfig.py` (editable keys `19-43`), `CMakeLists.txt`
(`12-92,153-204`).*

← [Prev](docs/TROUBLESHOOTING.md) · [Index](docs/INDEX.md) · [Next](CHANGELOG.md) →
