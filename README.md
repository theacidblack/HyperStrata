# HyperStrata

My personal playground to learn engines.

## About

This is not competing. It is an experiment, in public.
I learn here by taking an engine apart and putting it back together.

## Support

- Code supports all RTX GPUs, without testing.
- Focus is my spec: RTX 3070 Ti Laptop 8GB + 32GB RAM.
- Compatibility checks sometimes, when I can.

## Runs

- Q4: 95.7 / 40.0 / 51.3 tok/s, 42 turns, 31160 tok in 607s, prefill 1.5K cold, PASS
- Q5: 70.9 / 29.8 / 41.1 tok/s, 61 turns, 29615 tok in 721s, prefill 1.1K cold, PASS
Rig: ctx 131072, temp 0.6, cap 4096, reserve 256, 65C, 0.775V, 6 last cores.
Full table in docs/BENCHMARKS.md. Logs on file.
Measured with Pi 4-phase game load, raw speed only.

## Install

No ready engine here. You bring weights.
- Ask any CLI agent to follow docs/HOW-TO-INSTALL.md, or do it by hand.
- Then build with BUILD_HYPER.bat. No claim which agent installs best.
- Site: https://theacidblack.github.io/HyperStrata/
- Install: https://theacidblack.github.io/HyperStrata/docs/HOW-TO-INSTALL.html

## Reports

Public issues off. Talk in Dev discussions or owner Threads.
- Bring rig + server log. Fake numbers close the report.

## License

Fork from PolyStrata 0.1.44.1, commit 51dc2427, from Strata.
- https://github.com/Niko1221/Strata
- https://github.com/VecSzn/PolyStrata
- https://github.com/theacidblack/HyperStrata
Upstream never approved, supported, or endorsed this. MIT.
