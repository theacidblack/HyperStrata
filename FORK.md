# HyperStrata — an experimental fork

This repository is a personal fork of [PolyStrata](https://github.com/VecSzn/PolyStrata).
It follows upstream `main` and adds a small amount of work done on one machine:
an RTX 3070 Ti Laptop with 8 GB of VRAM and 32 GB of RAM.

The addition with a purpose is a **q4_0 KV cache** for the `strata-poly` engine:
`--kv q4_0` stores 288 bytes a row instead of 544, and the decode kernel reads the
packed nibbles directly, so the smaller cache does not cost speed. At 8 GB of VRAM
that leaves room for more experts and more context. It runs for the `qwen35moe`
models (Tiel, Ornith, Occamy) and for Flint, whose 35 sliding-window layers keep
their rows in a ring.

There is no reason to use this over PolyStrata. This is not better than upstream,
not a product, not supported and has no releases. Upstream is the project; this is
where I experiment, measure and keep notes, and only what holds up on my card lands
here. Most of the experimenting happens outside this repository.

Upstream is the source of truth for code, builds, models and support.
Licence: the same as upstream, see `LICENSE`.
