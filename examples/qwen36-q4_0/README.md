# Example: q4_0 KV run of Qwen3.6-35B-A3B

Two files, both placeholders — fill in your own paths, nothing here runs
as-is:

- `config.example.json` — the shape of a working run config: model file,
  context 131072, expert profile, tokenizer, `--kv q4_0 --draft-kv q4_0`,
  256 MiB VRAM reserve, temperature 0.6, thinking budget 4096, port 8080.
- `run-q4_0.example.bat` — derives a throwaway config in `%TEMP%` from your
  installed config and starts the server. It never edits the installed
  config; the fp16 default stays clean.

## Weights are yours, not ours

This repo ships **no model weights**. Get the Unsloth `UD-Q4_K_M` GGUF of
`Qwen3.6-35B-A3B` yourself from Hugging Face, put it anywhere, and point
`MODEL_FILE` at it. The normal path is the installer (`START-HERE.bat`),
which writes the real config this example mirrors. What the installer
downloads, and any model-install code inherited from PolyStrata/Strata, is
outside what this fork has audited — you fetch and run weights at your own
risk, under their own licenses. Nothing here redistributes them.

## Proof it ran q4_0

After starting, the engine log must contain both lines:

- `q4_0 KV cache: 288 bytes a row`
- `q4_0 KV cache (draft block): 288 bytes a row`

Absence means the engine is not the q4_0 build. Details:
`docs/KV_CACHE.md`. Measured numbers: root `README.md`.
