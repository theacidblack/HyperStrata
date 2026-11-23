# Make a launcher

A launcher starts the server and the engine with your settings. One launcher is two files:

- a **JSON config** with the engine flags and the sampling settings;
- a **batch file** (Windows) or shell script (Linux) that starts the server.

Copy the examples below and change the paths.

## 1. Make the tokenizer pack

The server needs the model's tokenizer as a small folder. Make it from your GGUF file:

```bat
python tools\strata_tokenizer.py --gguf "path\to\model.gguf" --out "packs\my-model"
```

The result is `packs\my-model\tokenizer`. Point the config at that folder.

## 2. Make the expert profile (optional)

The expert profile is a text file of expert use counts. The engine reads it at the start and
writes it again at the stop, so the file becomes better with use.

- Without the flag, the engine keeps its counts for one run only.
- With `--expert-profile "path\to\expert-profile.txt"`, the counts survive between runs.
- Start with an empty file. The engine fills it.

## 3. Write the config

Save this as `settings\qwen36-q4.json`. Change every path to your own.

```json
{
  "exe": "C:\\HyperStrata\\build\\Release\\strata-poly.exe",
  "cwd": "C:\\HyperStrata",
  "tokenizer": "C:\\HyperStrata\\packs\\my-model\\tokenizer",
  "model_name": "Qwen3.6-35B-A3B-MTP-UD-Q4_K_M",
  "log": "C:\\HyperStrata\\logs\\qwen36.log",
  "port": 8080,
  "args": [
    "--model", "C:\\models\\Qwen3.6-35B-A3B-UD-Q4_K_M.gguf",
    "--max-context", "131072",
    "--threads", "6",
    "--prompt-batch", "1024",
    "--window", "8",
    "--expert-profile", "C:\\HyperStrata\\packs\\my-model\\expert-profile.txt",
    "--tail-skip", "6",
    "--kv", "q4_0",
    "--expert-swaps", "64"
  ],
  "sampling": {
    "temperature": 0.0,
    "top_p": 0.95,
    "top_k": 20,
    "min_p": 0.0,
    "presence_penalty": 0.0
  },
  "reasoning_budget_tokens": 4096,
  "preserve_thinking": true
}
```

### Engine flags

| Flag | What it does | Qwen3.6 | Flint |
| --- | --- | --- | --- |
| `--model` | The GGUF file. | yes | yes |
| `--max-context` | Tokens the attention caches hold. | 131072 | 131072 |
| `--threads` | CPU threads for the experts outside VRAM. | 6 | 6 |
| `--prompt-batch` | Prompt tokens read at a time. | 1024 | 2048 |
| `--window` | Rows the engine writes in one step. | 8 | default |
| `--kv` | Attention cache format. Use `q4_0`. | `q4_0` | `q4_0` |
| `--expert-profile` | File of expert use counts. | yes | yes |
| `--tail-skip` | Leave out an expert that every token ranks below this place. | 6 | default |
| `--expert-swaps` | Experts kept moving into VRAM during an answer. | 64 | default |
| `--no-draft` | Do not run the guess-ahead block. | no | yes |
| `--ram-experts-mib` | RAM held for the experts outside VRAM. `-1` holds all. | default | `-1` |
| `--vram-reserve-mib` | VRAM kept free beside the expert cache. | default | `256` |

- `--kv q4_0` is the change of this fork. It works for Qwen3.6-35B-A3B and Flint 40B-A2.5B.
- Sampling lives in the JSON `sampling` block, not in engine flags.

### Server settings

| Key | What it does |
| --- | --- |
| `exe` | Full path to the engine program. |
| `cwd` | Folder the engine starts in. |
| `tokenizer` | Folder with the tokenizer pack. |
| `model_name` | Name the API shows. |
| `log` | File for the server and engine log. |
| `port` | Port on `127.0.0.1`. Use one port, one server at a time. |
| `args` | The engine flags, one string per item. |
| `sampling` | Default sampling for every request. |
| `reasoning_budget_tokens` | Most tokens the model may think. |
| `preserve_thinking` | Keep earlier thinking in the prompt. |

## 4. Test the server by hand

```bat
python serve\server.py --engine strata --config "settings\qwen36-q4.json" --port 8080 --open
```

Watch the window. The last lines must say `loaded in ...` and `ready: http://127.0.0.1:8080/v1`.
The browser opens the chat page. Then press Ctrl+C to stop.

## 5. Write the start script

Save this as `Run-Qwen3.6.bat` in the repository folder, next to `serve`. Change the paths.

```bat
@echo off
setlocal
set "GGUF=C:\models\Qwen3.6-35B-A3B-UD-Q4_K_M.gguf"
set "CFG=%~dp0settings\qwen36-q4.json"
if not exist "%GGUF%" echo [FAIL] missing model: "%GGUF%" & pause & exit /b 1
if not exist "%CFG%" echo [FAIL] missing config: "%CFG%" & pause & exit /b 1
cd /d "%~dp0"
echo starting the model on 127.0.0.1:8080
start "HyperStrata" cmd /k python -u serve\server.py --engine strata --config "%CFG%" --port 8080 --open
exit /b 0
```

- Double-click the file to start. One window holds the server and the engine.
- Close that window to stop both.
- If port 8080 is busy, stop the other server first. The server reads `127.0.0.1` only.

## 6. Use the API

The server speaks the OpenAI and Anthropic APIs:

- `http://127.0.0.1:8080/v1/chat/completions`
- `http://127.0.0.1:8080/v1/messages`
- `http://127.0.0.1:8080/v1/models`
- `http://127.0.0.1:8080/health`

Point your client at `http://127.0.0.1:8080/v1` and use any model name.
