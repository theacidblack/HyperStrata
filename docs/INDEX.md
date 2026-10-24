
We respect everyone who built and contributed to this. Strata and PolyStrata are real engineering. Everything I do after the last synced version (v0.1.45-poly.1) is pure experimentation for 8GB VRAM / 32GB RAM rigs like mine, focused on making things fast on sm86. I have no access to other architectures except through oracle checks and tests. This is not a mainstream engine and does not try to be popular or compete. This is my public research where I experiment. There is no benefit to using this over mainstream builds. Treat it as a personal experiment. I claim no speed benefits or real improvements over mainstream. I am just messing with this because I like it.

← [Prev](../README.md) · [Index](INDEX.md) · [Next](HOW-TO-INSTALL.md) →

# Docs index

Site (same docs, black theme): https://theacidblack.github.io/HyperStrata/

Start at README, follow Next page by page.

Read Start in order, Run in any order after Start, Measure on demand.

## Start
- [README](../README.md): What is this project and where do I start?
- [How to install](HOW-TO-INSTALL.md): How do I install and get one model chatting?

## Run
### Configure — choose model, weights, cache format, and backend.
- [Models](MODELS.md): Which model and quant fit my PC and how much RAM they need?
- [KV cache](KV_CACHE.md): Which `--kv` and `--draft-kv` formats exist and which should I use?
- [Backends](BACKENDS.md): Which engines run which models and how to switch?
### Scale — serve more load across hardware; Details is the reference.
- [Details](DETAILS.md): Where are all settings, API, and measured numbers?
- [Protocol notes](HYPERSTRATA_PROTOCOL.md): Routes, tool-call semantics, errors, and discovery for independent clients.

## Measure
- [Benchmarks](BENCHMARKS.md): Which runs were measured and on what rig?
- [Troubleshooting](TROUBLESHOOTING.md): Setup or inference failed — what do I check first?
- [Flags](../--help.md): What flags do launchers and setup accept?

## About
- [Changelog](../CHANGELOG.md): What changed in each release?
- [Upstream divergence](../UPSTREAM_DIVERGENCE.md): What does this fork change relative to upstream?
- [Third-party](../THIRD_PARTY.md): Which third-party pieces ship and under what licenses?
- [Working on the code](../AGENTS.md): How do I contribute a change or a report?

Reference material for hardware and models out of scope here: [docs/archive/](archive/).

← [Prev](../README.md) · [Index](INDEX.md) · [Next](HOW-TO-INSTALL.md) →
