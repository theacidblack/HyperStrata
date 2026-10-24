# HyperStrata Protocol Notes

Phase 0. Engine-side companion to the Exit-OS integration contract. Plain words, measured only. This engine stays independently usable: no client-specific headers, slots, or cache fields on the standard paths.

## Routes

- `POST /v1/chat/completions` — OpenAI chat, stream and non-stream, tools. The only chat route. There is no `POST /` chat route: unknown paths return 404 (`serve/server.py:5873-5874`). Clients that strip `/v1` from the base URL must still send the full path.
- `POST /v1/messages` — Anthropic messages, stream and non-stream, tools. `?beta=true` tolerated (`serve/server.py:5768`).
- `POST /v1/responses` — stateless only. Retrieval, cancel, and compact of a response id always return 404 (`serve/server.py:5811-5813`). No polling, no background.
- `GET /v1/models`, `/models` — model list with modalities and `n_ctx` (`serve/server.py:5705-5722`).
- `GET /props` — engine template, defaults, capability hints (`serve/server.py:5723-5725`).
- `GET /slots` — slot/KV state (`serve/server.py:5726-5744`).
- `GET /status`, `/v1/status`, `/health` — liveness vs readiness are distinct: health is the process, model readiness is the loaded state (`hyper.py:55-56`).
- `GET /metrics`, `/metrics/prometheus`, `/mcp` — telemetry and MCP surface.
- `POST /api/models/load`, `/unload`, `POST /load`, `/unload` — model control (`serve/server.py:5777-5851`).
- Not offered: `POST /v1/completions` (legacy), `/v1/embeddings`. Requesting them returns 404. Hosted tools (`web_search`, `file_search`, `computer_use`, `code_interpreter`, `image_generation`, `local_shell`, `mcp`, `tool_search`) are skipped with a log line, never executed (`serve/server.py:6389`).

## Chat semantics

- Request fields: `model`, `messages`, `tools`, `tool_choice`, `reasoning_effort`, `max_tokens`, `temperature`, `top_p`, `top_k`, `seed`, `stop` (string or up to 4), `response_format` (`json_object` / `json_schema`), `stream`.
- Multimodal content: `content` blocks `{type: text}` + `{type: image_url: {url: data:...}}` need a multimodal config (`serve/models.py:111`); images go through the resident `hyper-vision` encoder (`serve/server.py:2243-2269`).
- `finish_reason` truth table (target behavior):
  - `stop` — clean text stop, no pending tool call.
  - `tool_calls` — one or more COMPLETE calls with fully streamed arguments.
  - `length` — output budget hit; arguments may be partial and must never be executed.
- Known gap #211: a call cut off inside its output can report `stop` with partial streamed args and no final `tool_call` (`serve/server.py:5118-5120`). Until fixed, clients must treat `stop` + calls as suspect. The fix belongs here, not in every client.
- `tool_choice`: unknown shapes are a 400 (`serve/frontend.py:793-811`); known-but-unhonourable (bad name, `required` with no tools) still degrades to `auto`. `none` drops tools before the engine.
- Chat route accepts `POST /chat/completions` as well as `/v1/chat/completions` (trailing slash and query tolerated); foreign-page guard covers both.

## Errors

- `400 invalid_request_error` — schema problems, context overflow (prompt + `max_tokens` over context, with `code: context_length_exceeded`), unsupported `tool_choice`. Bodies carry what did not fit.
- `401 authentication_error` — bad or blank key (`serve/server.py:5582`).
- `404` — unknown route or response id.
- `409 model_busy` — another sequence holds the single FIFO lane.
- `502 structured_output_failed` — `response_format` could not be satisfied.
- `503 server_error` + `Retry-After` — GPU busy or engine starting; worth retrying.
- `503 EngineDied` — the engine ended; the next request restarts it.
- Cancellation: the server polls for client disconnect every 0.5 s and stops the sequence (`serve/server.py:5497-5520`). Stale responses are never delivered after disconnect.

## Discovery

- `/props`, `/v1/models`, and health must agree with runtime: reported `n_ctx`, modalities (vision only when `mmproj` is loaded), and tool support describe what the engine actually does.
- Do not advertise embeddings, hosted tools, or vision beyond the loaded `mmproj` reality.

## Defaults

- Listener `127.0.0.1:8080`. Beyond loopback requires `--api-key` (Bearer or `x-api-key`, multi-key CSV).
- One model, one FIFO sequence at a time. Parallel clients serialize server-side; `409/503` signals backpressure.
- State (weights, run outputs) lives outside any client tree: `HyperStrata-data/`, `state_dir`. Engine KV state is in-memory; persistence is opt-in (`--slot-save-path`).

## Test map (no GPU needed)

- `python -m unittest serve.test_server` — full protocol suite (~329 cases).
- `serve/test_context_overflow.py`, `serve/test_frontend.py` — overflow + tool-shape validation.
- `tools/test_setup_*.py` — installer paths, mocked engine/downloads.
