# HyperStrata

*Needs: nothing · Gives: what this is and where to start · Next: docs/INDEX.md*

Personal experiments, live in public.
Not a competing engine.
Experimental fork to learn how engines work.
Tuned for one rig only: RTX 3070 Ti Laptop 8GB + 32GB.
Other RTX cards: theory only, no proof, no promise.
No wide support. No maintenance promise.
No sync upstream from here. Experiment, not a copy.

Chain: Strata https://github.com/Niko1221/Strata to PolyStrata https://github.com/VecSzn/PolyStrata to HyperStrata.
Upstream never approved, supported, or endorsed this.
Started from PolyStrata 0.1.44.1 commit 51dc2427.
License: MIT.

Measured on that rig only, ctx 131072 temp 0.6 cap 4096 reserve 256:
Q4 95.7 / 40.0 / 51.3 tok/s, 42 turns, 31160 tok / 607 s, prefill 1.5K cold, PASS.
Q5 70.9 / 29.8 / 41.1 tok/s, 61 turns, 29615 tok / 721 s, prefill 1.1K cold, PASS.
Rig: 65C lock, 0.775V, 6 last cores no SMT.
Full table: docs/BENCHMARKS.md.

No ready engine here.
Recommended: ask any CLI agent to follow docs/HOW-TO-INSTALL.md Method 1. Tested with Pi agent only, other agents should work no guarantee.
Or do Method 1 by hand through docs/HOW-TO-INSTALL.md.
Build with BUILD_HYPER.bat. You bring the weights.

Public issues off. Report in Dev discussions or owner Threads.

← Prev none · [Index](docs/INDEX.md) · [Next](docs/INDEX.md) →
