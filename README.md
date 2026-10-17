# HyperStrata

Run 35-billion-parameter models on your own PC: 8 GB VRAM + 32 GB RAM, NVIDIA, Windows. You bring the weights.

Tested: Qwen3.6-35B-A3B in Q4_K_M and Q5_K_M (MTP). Both measured, logs on file.

Site: https://theacidblack.github.io/HyperStrata/

## Speed

- Q4_K_M MTP: 95.7 / 40.0 / 51.3 tok/s (42 turns, 607 s), prefill ~1.5K cold
- Q5_K_M MTP: 70.9 / 29.8 / 41.1 (61 turns, 721 s), prefill ~1.1K cold

Bench: RTX 3070 Ti Laptop, 65 C CPU, 0.775 V GPU, 6 cores, ctx 131072, temp 0.6. Stock PCs read lower. Full table: [docs/BENCHMARKS.md](docs/BENCHMARKS.md).

## Install

[docs/HOW-TO-INSTALL.md](docs/HOW-TO-INSTALL.md) — checks your PC, runs setup, ends with one chat. Agent method tested with Pi only.

## Use

Server on `http://127.0.0.1:8080`. Any OpenAI-compatible client, key `none`, model id from `/v1/models`. Presets in `examples/`.

## Reports

No pull requests. Bug reports: rig + flags + full server log. Fake numbers close the report.

## License

MIT (Niko1221 and Strata contributors). Built from Strata via PolyStrata. Numbers cover only the measured rig above.
