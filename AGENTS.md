
# AGENTS.md

HyperStrata is an experimental inference engine for the 8 GB VRAM + 32 GB RAM
tier: Qwen3.6-35B-A3B (Unsloth UD quants, q4–q6) on one NVIDIA card on Windows,
via CUDA. Reference rig: RTX 3070 Ti Laptop 8 GB + 32 GB RAM. It has a C++/CUDA
engine (`src/`, `include/`), a Python server with an OpenAI- and
Anthropic-compatible API and a web app (`serve/`), and a one-click installer
(`setup.py`, started by `START-HERE.bat`).

The model runs on the `hyper` engine (`src/poly/`), architecture
`qwen35moe`, from its GGUF files: [docs/BACKENDS.md](docs/BACKENDS.md),
[docs/MODELS.md](docs/MODELS.md). Pictures go through the `hyper-vision`
helper: [docs/DETAILS.md](docs/DETAILS.md). HyperStrata is a fork of
[PolyStrata](https://github.com/VecSzn/PolyStrata), which builds on
[Strata](https://github.com/Niko1221/Strata); the engine, installer, server and
web app come from there. Their licenses apply; nothing here claims their
endorsement.

## Installing HyperStrata for a user

Follow **[docs/HOW-TO-INSTALL.md](docs/HOW-TO-INSTALL.md)** (the one install
page): check the PC, pick the model by
VRAM and RAM ([docs/MODELS.md](docs/MODELS.md)), run setup, start and verify
the server, and connect the user's apps. Never expose the server beyond
`127.0.0.1` without `--api-key`.

## Working on the code

- How the engine works, every measured number, the API and all settings:
  [docs/DETAILS.md](docs/DETAILS.md) and the [paper](docs/paper/Strata-Paper.pdf).
- Which weight files load and which paths are still slow:
  [docs/MODELS.md](docs/MODELS.md).
- Full doc map: [docs/INDEX.md](docs/INDEX.md).
- Experiment-first, no guarantees: house rig measured only; rest untested/PENDING.
- Setup's own tests run without a GPU or downloads:
  `python tools/test_setup_<name>.py` (for example `tools/test_setup_amd.py`,
  `tools/test_setup_choices.py`).
- Keep the docs' style: plain words, measured numbers with what they were
  measured on, no claims without a measurement.
- HyperStrata is pure experimentation: low stability is accepted while looking
  for what runs well on limited hardware. Measure, change one thing, measure
  again, and keep what the numbers support.

## Contributing a change or report

- Pull requests are not accepted right now. Owner reviews reports and implements what covers many users.
- Reports welcome: rig + server log. Bench reports follow
  [docs/BENCHMARKS.md](docs/BENCHMARKS.md) with logs, compare against a same-day run of the build
  testing, and say what you did not test.
- A new feature is opt-in, and the default path stays byte-identical to the last release. Say how you checked.
- Change a default only where you measured it faster, and show the numbers with what they were measured on.
- A report from hardware the owner does not have is unverified here. Fake numbers close the report.
- Open test requests and the hardware that is wanted are listed in [docs/BENCHMARKS.md](docs/BENCHMARKS.md).
