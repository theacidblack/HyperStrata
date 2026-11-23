# HyperStrata v0.1.0

**First release · 2026-11-23**

HyperStrata is a fork of PolyStrata `0.1.45-poly.10`, adding Q4_0 KV-cache support for long-context inference.

## Changes

### 4-bit KV cache

Added the `--kv q4_0` option.

- Stores the KV cache in 4-bit blocks.
- Uses 288 bytes per row, compared with 544 bytes for the existing 8-bit format.
- Attention kernels read packed cache values directly.
- Reduces VRAM consumption, leaving more memory available for model weights and other runtime allocations.

Implemented in two commits:

- `b329b5b19` — 4-bit cache storage for Flint.
- `c08cbacfc` — Direct packed-cache reads in the attention kernel.

Supported and tested model configurations:
- Qwen3.6-35B-A3B
- Flint 40B-A2.5B

## Benchmarks

<p><sub><em>RTX 3070 Ti Laptop · 8 GB VRAM · 32 GB DDR5 · 6 CPU threads · Windows · CUDA 13.3 · 131,072-token context</em></sub></p>

| Model | Decode | Prompt processing |
|---|---:|---:|
| Qwen3.6-35B-A3B | 98–102 t/s | 1,519–1,769 t/s |
| Flint 40B-A2.5B | 56–63 t/s | 1,075 t/s |

Measurements are taken from the engine logs. Qwen3.6 has also reached approximately 80 t/s average decode throughput over longer sessions. Flint results are preliminary, based on a limited number of runs.

## Requirements

The tested configuration uses:
- NVIDIA RTX 3070 Ti Laptop GPU with 8 GB VRAM.
- 32 GB system RAM.
- Windows.
- CUDA Toolkit 13.3.

Build dependencies and instructions are provided in [docs/BUILD.md](docs/BUILD.md).

No prebuilt binaries are included. The engine must be built from source.

## Compatibility

This release is based on PolyStrata `0.1.45-poly.10`.

Newer upstream releases are not automatically merged. Configuration files and command-line options from newer versions may differ and should not be assumed compatible.

## Testing scope

Verified on:
- One RTX 3070 Ti Laptop GPU.
- Qwen3.6-35B-A3B.
- Flint 40B-A2.5B.
- Windows with CUDA 13.3.

Not verified:
- Other GPU models.
- Linux.
- Other model quantizations.
- Configurations below 8 GB VRAM or 32 GB system RAM.

## Credits

HyperStrata is based on [PolyStrata](https://github.com/VecSzn/PolyStrata), built on [Strata](https://github.com/Niko1221/Strata) by [Niko1221](https://github.com/Niko1221).

The upstream projects provide the inference engine and model support. The Q4_0 KV-cache implementation and accompanying documentation were added in this fork.