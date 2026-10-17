# HyperStrata

This is my personal playground. I learn how engines work here, by taking one apart and putting it back together.

This is not a competing engine, and it's not for wide use. It's just where I experiment.

It's tuned for my rig only: RTX 3070 Ti Laptop 8GB with 32GB RAM, on Windows with NVIDIA and CUDA. That's the only machine I can speak for.

Other RTX cards might load in theory, but I have no proof of that. No promise there.

I forked this from PolyStrata 0.1.44.1, commit 51dc2427, which itself comes from Strata. I'm not syncing with upstream anymore, this goes its own way now.

- Strata: https://github.com/Niko1221/Strata
- PolyStrata: https://github.com/VecSzn/PolyStrata
- HyperStrata: https://github.com/theacidblack/HyperStrata

Upstream never approved, supported, or endorsed this. License is MIT.

What runs here, on my rig:

- Q4_K_M MTP at 95.7 / 40.0 / 51.3 tok/s, 42 turns, 31160 tok in 607s, prefill 1.5K cold, PASS
- Q5_K_M MTP at 70.9 / 29.8 / 41.1 tok/s, 61 turns, 29615 tok in 721s, prefill 1.1K cold, PASS

Both at ctx 131072, temp 0.6, cap 4096, reserve 256, sitting around 65C and 0.775V on the 6 last cores. Full table is in docs/BENCHMARKS.md. Logs on file.

There's no ready engine here.
- How to install: ask any CLI agent to follow docs/HOW-TO-INSTALL.md, or do it by hand. Then build with BUILD_HYPER.bat. You bring your own weights. No claim which agent installs best.
- How speed was measured: Pi agent running a 4-phase game build load, raw speed only. Q4/Q5 numbers above are from Pi runs, logs on file. Other agents untested for measuring speed, no guarantee.

- Site: https://theacidblack.github.io/HyperStrata/
- Install: https://theacidblack.github.io/HyperStrata/docs/HOW-TO-INSTALL.html

Public issues are off. Talk in Dev discussions or owner Threads. I like a rig plus server log with reports. Fake numbers close the report.
