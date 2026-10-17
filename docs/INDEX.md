← [Prev](../README.md) · [Index](INDEX.md) · [Next](HOW-TO-INSTALL.md) →

# Docs index

Start at README, follow Next page by page.

Read Start in order, Run in any order after Start, Measure on demand.

## Start
- [README](../README.md): What is this project and where do I start?
- [How to install](HOW-TO-INSTALL.md): How do I install and get one model chatting?
- [Install](INSTALL.md): What PC is needed and how is server verified?
- [AI setup](AI_SETUP.md): How do I connect my apps and agents to the local API?

## Run
### Configure — choose model, weights, cache format, and backend.
- [Models](MODELS.md): Which upstream size tables are kept for reference?
- [Coverage](COVERAGE.md): Which weight files load and what is still unmeasured?
- [KV cache](KV_CACHE.md): Which `--kv` and `--draft-kv` formats exist and which should I use?
- [Q8 KV](Q8_KV.md): What does the q8_0 KV measurement cover?
- [Backends](BACKENDS.md): Which engines run which models and how to switch?
### Scale — serve more load across hardware; Details is the reference.
- [Details](DETAILS.md): Where are all settings, API, and measured numbers?
- [Batching](BATCHING.md): How do I serve several requests at once?
- [Multi GPU](MULTI_GPU.md): How do I split layers across several GPUs?
- [How it works](HOW_IT_WORKS.md): How does the model share work across PC?
### Connect — link apps, images, and per-request controls.
- [Images](IMAGES.md): How do I generate images with Qwen-Image?
- [MCP server](MCP_SERVER.md): How does my assistant install and run HyperStrata?
- [Llama swap](LLAMA_SWAP.md): How do I run HyperStrata behind llama-swap?
- [Logit bias](LOGIT_BIAS.md): How do I bias tokens per request?
- [Benchmarks](BENCHMARKS.md): Which runs were measured and on what rig?

## Measure
- [Community benchmarks](COMMUNITY_BENCHMARKS.md): How do I report a run from other hardware?
- [Test requests](TEST_REQUESTS.md): Which hardware and runs are still wanted?
- [Troubleshooting](TROUBLESHOOTING.md): Setup or inference failed — what do I check first?
- [FAQ](FAQ.md): Where are the answers to common questions?
- [Flags](../--help.md): What flags do launchers and setup accept?

## About
- [Changelog](../CHANGELOG.md): What changed in each release?
- [Contributing](../CONTRIBUTING.md): How do I report bugs if PRs are closed?
- [Security](../SECURITY.md): How do I report a security issue?
- [Upstream divergence](../UPSTREAM_DIVERGENCE.md): What does this fork change relative to upstream?
- [License](../LICENSE): Under what terms is the software distributed?

← [Prev](../README.md) · [Index](INDEX.md) · [Next](HOW-TO-INSTALL.md) →
