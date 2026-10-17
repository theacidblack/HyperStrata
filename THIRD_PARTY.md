
← [Prev](UPSTREAM_DIVERGENCE.md) · [Index](docs/INDEX.md) · [Next](AGENTS.md) →

# Third-party attributions

*Needs: nothing (reference page) · Gives: every third-party piece, where it is pinned, and its license · Next: none*

One record for every third-party piece in this tree: what it is, where it is pinned, and under what license.

| component | what | why | source + pin | license | license text where |
| --- | --- | --- | --- | --- | --- |
| `LICENSE` | repo license, upstream MIT kept verbatim | base terms | Niko1221/Strata | MIT, Niko1221 + Strata contributors | `LICENSE` |
| `third_party/ggml/` | `ggml-common.h` vendored header | engine compiles against it | llama.cpp tree at engine pin below | MIT, ggml authors 2023-2026 | `third_party/ggml/LICENSE` |
| `third_party/llama.cpp/` | full upstream checkout | build source: mtmd, gguf-py, vendor | `github.com/ggml-org/llama.cpp` @ `3cf03257f219afbe7334045ff7c6a06ac68c627d`, ZIP `setup.py:187-188` | MIT, ggml authors | `third_party/llama.cpp/LICENSE` |
| vision checkout | second llama.cpp checkout, hyper-vision only | MiniCPM-V 4.7 mtmd is newer than the engine pin | same repo @ `69f201a2051ea9b9e9b50c3cc56afd9e2ae64414`, ZIP `setup.py:191-192` | MIT, ggml authors | checkout `LICENSE` |
| gguf-py 0.19.0 | GGUF read/write lib | tooling | nested in checkout, `pyproject.toml` version 0.19.0 | MIT, Georgi Gerganov 2023 | nested `LICENSE` |
| `tools/mtmd` | multimodal front end | vision path | nested in checkout, same engine pin | MIT, upstream | checkout `LICENSE` |
| `vendor/` | cpp-httplib, miniaudio, stb, hash, sheredon, nlohmann/json | upstream deps, as shipped | nested in checkout | json MIT (`licenses/LICENSE-jsonhpp`); rest per files as upstream ships them | `vendor/*/LICENSE*`, in-file headers |
| i-quants | IQ2/IQ3/... formats | weight formats | inside ggml (`ggml.h` `GGML_TYPE_IQ*`), not separate | MIT, ggml authors | checkout `LICENSE` |
| MMQ / strata_kq | prompt kernels; `strata_kq` lib is the fork's own wrapper | prompt path | fork `src/prefill/`; kernels derived from llama.cpp ggml-cuda | MIT | `LICENSE` + checkout `LICENSE` |
| NVFP4 port | NVFP4 engine path, ported from sergqwer/strata-nvfp4 | NVFP4 packs | sergqwer/strata-nvfp4 (MIT, `setup.py:227`); packs `Maximilian228/*` pinned by revision+sha256 (`setup.py:234-264`) | MIT | `setup.py:227` + upstream repo |
| python deps | numpy, jinja2, regex, pyyaml, tqdm, requests, cmake, ninja, pillow, psutil + markupsafe, certifi, charset-normalizer, idna, urllib3, colorama; CUDA wheels nvidia-cublas/cuda-runtime 13.0.x (cu12 variants) | setup installs into `.venv` | `requirements.txt` pinned; `setup.py:208,216,224-225` | UNCONFIRMED (no `.venv` on disk) | installed dist-info `METADATA` when present |

Weights: user-supplied Unsloth UD GGUFs, never committed or redistributed (`README.md:48`; `.gitignore:6` `data/**/*.gguf`). setup.py per-model strings: Swift Open License 1.0 (swift, `setup.py:443`), Qwen Community License 1.0 (NVFP4, `setup.py:348`), NVIDIA Open Model License + Qwen (NVFP4-NVIDIA, `setup.py:353-355`), Apache 2.0 (qwen36-strata, `setup.py:721`). Unsloth UD files are Qwen-derived; license is per model card — check the card, no claim beyond.

Python: every package above installs into `.venv` and is never committed (`/.venv/` in `.gitignore:30`). No `.venv` exists on this disk, so each package license is UNCONFIRMED until read from installed dist-info `METADATA` `License:`/`Classifier` fields. Nothing here is stated from memory.

Fetch policy: `/third_party/llama.cpp*` is git-ignored (`.gitignore:32`) and fetched reproducibly at build/setup time from the pinned ZIPs. Deliberate policy: the repo stays small and the build stays reproducible. Not a hole. `third_party/ggml/VERSION.txt` carries the same pin as `LLAMA_CPP_COMMIT`.

← [Prev](UPSTREAM_DIVERGENCE.md) · [Index](docs/INDEX.md) · [Next](AGENTS.md) →
