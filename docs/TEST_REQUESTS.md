# Test requests and hardware wanted

Upstream PolyStrata was built/measured on a few machines (not THIS fork). Wanted reports below are PENDING for HyperStrata; no support implied until measured on THIS tree. Write a report as described in
[COMMUNITY_BENCHMARKS.md](COMMUNITY_BENCHMARKS.md): the build, the card and PCIe width, the CPU and RAM speed, the
model file, the exact command, three runs per size, and what you did not test.

This list is edited by the owner when needed; items carry no response promise. An item stays until a report answers it.

Reports from 8 GB VRAM + 32 GB RAM jump the queue — that is the rig this is built for.

Better rigs are welcome to report, but they sit outside what this is for, so they get less attention.

6 GB VRAM + 32 GB RAM is welcome too, probably works — reports accepted, best-effort, nothing promised.

| Wanted | Why | Example of an earlier answer |
|---|---|---|
| Two NVIDIA cards of different sizes (a large and a small one), in both card orders, with `--layer-split auto` | There is no unequal pair here to check the split planner; the card order and the small card's free memory changed the split | #880, #1238 |
| Three or four NVIDIA GPUs | No multi-GPU NVIDIA rig of that size here | #880 |
| Pascal (sm_61: Tesla P40, P100, GTX 10 series) | Older cards run on the fp32 path and can behave differently from newer ones | #875, #876 |
| A Linux host with 32 GB of RAM or less | The expert file tier and the resident mode depend on how much of the model fits in RAM | #1194 |
| A long soak (an hour or more of back-to-back requests) on a split or resident setup | Shows crashes, drift and heat that a short run does not | #848 |
