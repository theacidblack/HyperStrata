# HyperStrata

HyperStrata is a fork of [PolyStrata](https://github.com/VecSzn/PolyStrata) with an additional Q4_0 KV-cache format for reducing VRAM usage during long-context inference.

The fork is tested with Qwen3.6-35B-A3B and Flint 40B-A2.5B on an NVIDIA RTX 3070 Ti Laptop GPU with 8 GB VRAM.

## Features

### Q4_0 KV cache

HyperStrata adds the `--kv q4_0` option for 4-bit KV-cache storage.

- 288 bytes per row, compared with 544 bytes for the existing 8-bit format.
- Attention kernels read packed cache values directly.
- Reduces KV-cache memory requirements at long context lengths.

The purpose is to free VRAM for model weights while retaining long-context support.

### Model placement

HyperStrata uses hybrid GPU/CPU memory placement:

- Selected model weights reside in GPU VRAM.
- Remaining weights reside in system RAM.
- Host-resident weights are transferred to the GPU when required.

This allows models larger than available VRAM to run without loading all weights into the GPU simultaneously.

## Benchmarks

<p><sub><em>Reported engine-log measurements · RTX 3070 Ti Laptop · 8 GB VRAM · 32 GB DDR5 · Windows · CUDA 13.3 · 131,072-token context</em></sub></p>

| Model | Decode | Prompt processing |
|---|---:|---:|
| Qwen3.6-35B-A3B | 98–102 t/s | 1,519–1,769 t/s |
| Flint 40B-A2.5B | 56–63 t/s | 1,075 t/s |

Qwen3.6 typically reaches around 100 t/s decode, with reported long-session throughput around 80 t/s.

Flint measurements are based on a smaller number of runs and should be treated as preliminary.

Results depend on model configuration, context length, and hardware.

## Requirements

Tested configuration:

- NVIDIA RTX 3070 Ti Laptop GPU, 8 GB VRAM
- 32 GB system RAM
- Windows
- CUDA Toolkit 13.3

Build dependencies:

- C++ compiler
- CUDA Toolkit 13.x
- CMake 3.24+
- Python 3.10+

Other GPU configurations and operating systems have not been verified.

## Build

The build process is the same as upstream PolyStrata. No separate build system is required.

For the tested RTX 3070 Ti configuration, the CMake commands are:

```bat
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DSTRATA_ENABLE_CUDA=ON -DCMAKE_CUDA_ARCHITECTURES=86
cmake --build build --target strata-poly -j 6
```

Set `CMAKE_CUDA_ARCHITECTURES` to match your target GPU:

| GPU generation | Architecture |
|---|---:|
| RTX 20 series | 75 |
| RTX 30 series | 86 |
| RTX 40 series | 89 |
| RTX 50 series | 120 |

These are CUDA compilation targets, not hardware validation claims.

For manual setup and platform-specific instructions, see [docs/BUILD.md](docs/BUILD.md). For launcher configuration, see [docs/LAUNCHERS.md](docs/LAUNCHERS.md).

## Compatibility and limitations

- Based on PolyStrata `0.1.45-poly.10`.
- Tested with two model configurations on one GPU.
- Windows is the tested operating system.
- Other GPU generations, Linux, and additional model configurations are not verified.
- Newer upstream changes are not automatically merged. Individual changes may be ported when needed and tested.

## Credits

HyperStrata is based on [PolyStrata](https://github.com/VecSzn/PolyStrata), which builds on [Strata](https://github.com/Niko1221/Strata) by [Niko1221](https://github.com/Niko1221).

The upstream projects provide the inference engine, server, and model support. HyperStrata adds Q4_0 KV-cache support and accompanying documentation.

See [LICENSE](LICENSE) for licensing terms.