# Onboarding gaps — public-clone walk to first chat

Walked as a non-technical user from a fresh public clone to first chat. Each item is one line: stage — gap — what must exist to close it.

## Stage 0 — clone and prereqs

1. Clone — no git version or install link, zip-vs-clone tradeoffs unstated — close: prereq box with minimum git version plus when zip is fine.
2. Clone — no total clone size or time on slow disks — close: stated checkout size and unzip time range.
3. Clone — no Python version stated up front — close: minimum Python stated here, not only inside the launcher script.
4. Clone — winget-vs-python.org fallback needs network/admin unstated — close: what each Python-install path needs and what to do offline.
5. Clone — no Visual Studio edition/version/workload stated up front — close: exact VS year plus workload name before any build step.
6. Clone — no CUDA Toolkit version stated up front — close: exact CUDA line for RTX 20/30/40/50 plus the old-card exception.
7. Clone — no NVIDIA driver minimum stated up front — close: minimum driver line plus where to read it.
8. Clone — no disk-space number stated up front and three numbers disagree later — close: one authoritative free-space figure per model choice.
9. Clone — no RAM/VRAM minimum stated up front — close: minimum-and-reference tiers stated before download.
10. Clone — no AVX2/CPU floor stated up front — close: CPU floor plus what happens without it.
11. Clone — no admin-rights statement — close: what needs admin and what never does.
12. Clone — no network/firewall/proxy statement — close: hosts/ports to allow plus offline limit.

## Stage 1 — README pointer

13. README How-I-run-it — single command shown with no meaning for each flag — close: one plain-words line per flag in place.
14. README How-I-run-it — `--gguf-dir <folder>` placeholder with no example shape — close: folder-content example plus trailing-slash rule.
15. README How-I-run-it — `--no-start` unexplained — close: what it skips and what to run next.
16. README How-I-run-it — `UD-Q4_K_M is tight, use UD-Q2_K_XL if setup warns` has no warning text quoted — close: exact warning string plus the switch command.
17. README How-I-run-it — no family/model picker, only one model named — close: two-sentence picker pointing at the coverage list.
18. README How-I-run-it — no download-vs-local choice — close: when to pass a local folder vs let setup download.
19. README How-I-run-it — no download size or time — close: size plus time at two speeds plus metered-connection warning.
20. README How-I-run-it — no where-files-land statement — close: data-folder location rule in one line.
21. README Docs list — eight docs named with no read order — close: numbered read order for install vs troubleshoot.

## Stage 2 — INSTALL Path A (agent does it)

22. Path A — `tell any coding agent two things` does not give the exact sentence to paste — close: copy-paste prompt template with folder and model slots.
23. Path A — agent `compares against COVERAGE §§1-3` but those sections use LOADS vs live-DONE vs PENDING without a decision script — close: exact covered/not-covered verdict wording the agent must say.
24. Path A — off-list request has no stop-or-continue rule — close: what the agent must refuse vs warn-and-continue.
25. Path A — `--check` output fields unexplained — close: sample verdict output plus pass/fail meanings.
26. Path A — `--yes` consequences unstated — close: list of defaults it accepts with no prompts.
27. Path A — `--family/--model/--context/--gguf-dir/--data-dir/--port` have no fork-correct values here — close: fork-correct example command in place.
28. Path A — `127.0.0.1 never 0.0.0.0 without --api-key` has no key-making step — close: key generation plus storage rule.
29. Path A — `first start takes a while` has no minutes or progress signal — close: expected wait plus which window line means progress vs stuck.
30. Path A — `later starts reuse downloads` has no location to verify — close: how to confirm reuse vs redownload.

## Stage 3 — INSTALL Path B (manual) and START-HERE

31. Path B step 1 — `free disk for tens of GB` is not a number — close: per-model GB figure in place.
32. Path B step 2 — clone URL vs zip choice has no verification step — close: how to confirm the checkout is complete.
33. Path B step 3 — example command has no flag glossary — close: one line per flag beside the command.
34. Path B step 3 — `without --gguf-dir setup downloads instead` states no size/destination/verify — close: size, destination rule, and hash/resume statement.
35. Path B step 3 — `setup provides the engine` names a folder that does not match the shipped engine folder — close: one canonical engine-folder name used everywhere.
36. Path B step 3 — PENDING says no standalone build script exists yet the repo ships one — close: corrected step naming the shipped script and when to use it vs setup.
37. Path B step 3 — `BUILD.json stamp` unexplained — close: what the stamp proves and what its absence means.
38. Path B step 4 — `copy examples plus 3 path edits` does not show a filled example — close: before/after snippet with placeholders replaced.
39. Path B step 4 — engine-exe vs model-gguf vs mmproj edits have no format checks — close: extension and existence checks per path.
40. Path B step 4 — `shape: run-qwen36-ud-q4_k_m.bat` points at a checked-in personal launcher — close: example-only launcher clearly marked non-canonical.
41. Path B step 5 — `curl /health` has no Windows equivalent when curl is missing — close: PowerShell one-liner plus expected JSON.
42. Path B step 5 — `close the server window to stop` has no graceful-wait note — close: expected unload delay plus when to force-close.
43. START-HERE — Python 3.10-vs-3.12 floor differs between script and prose — close: single minimum version everywhere.
44. START-HERE — Microsoft Store Python stub failure mode undocumented — close: detection note plus fix.
45. START-HERE — `.venv` creation failure has no disk/permissions guidance — close: causes plus retry steps.

## Stage 4 — build script and engine folder

46. Build script — default CUDA arch vs user card mapping buried in comments — close: lookup table plus how to find your arch.
47. Build script — CUDA 13 default breaks older cards with no up-front stop — close: preflight card-vs-CUDA rule before configure.
48. Build script — Visual-Studio-2026-only rejection noted only in a failure branch — close: supported VS list at the top.
49. Build script — cmake/ninja via private env vs PATH precedence unclear — close: which copy wins and the install command.
50. Build script — first-build internet need for the pinned ggml fetch unstated up front — close: one-time download size plus offline alternative.
51. Build script — `build-hyper` stale-config failure tells user to delete the folder with no backup note — close: safe-clean command plus what is preserved.
52. Build script — JOBS default has no RAM guidance — close: lower-it rule with symptom to watch.
53. Build script — BUILD.json deliberately absent but setup treats absence as user build — close: plain-words consequence for later setup runs.
54. Build script — smoke check exit-code-2 vs `usage:` string confuses users — close: success criterion stated as text match, not exit code.
55. Build script — vision helper explicitly not built here with no pointer — close: where the vision binary comes from and when it is needed.
56. Build script — antivirus-quarantine failure appears only here, not in install/troubleshooting — close: shared AV note linked from both pages.

## Stage 5 — examples and configs

57. Examples — three example JSON shapes disagree on binary name and keys — close: one canonical shape plus what the others are for.
58. Examples — `OWN-PATHS-HERE` vs angle-bracket vs `REPO_ROOT` placeholder styles differ — close: single placeholder style with a filled example.
59. Examples — absolute-vs-relative path rule unstated — close: which keys require absolute paths and why.
60. Examples — `cwd` purpose unexplained — close: what breaks when it points at the wrong folder.
61. Examples — `tokenizer` pack source unexplained — close: who creates it, path shape, and fallback when missing.
62. Examples — `expert-profile.txt` source unexplained — close: who writes it and what to do when absent.
63. Examples — `lib_dirs` CUDA path unexplained — close: how to find the correct folder on your PC.
64. Examples — vision block delete-vs-fill decision unexplained — close: text-only vs images rule with mmproj picker.
65. Examples — mmproj must match the weight file but mismatch error is undocumented — close: matching rule plus mismatch symptom.
66. Examples — `--kv/--draft-kv` vs `--no-draft` choice unexplained — close: MTP-vs-base rule in one line.
67. Examples — `run-model.example.bat` REPO_ROOT/MODEL_JSON/PY edits have no validation beyond existence — close: per-variable fix for each error line.
68. Examples — launcher hardcodes port 8080 with no conflict path — close: where to change the port in both files.
69. Examples — launcher passes `--open` with no browserless alternative — close: flag to skip opening the browser.
70. Examples — qwen36-q4_0 throwaway-config script refuses when source already has KV flags but gives no next step — close: what to do when refused.

## Stage 6 — flags reference

71. Flags reference — PENDING says it is not written yet it ships complete — close: corrected status plus entry point from INSTALL.
72. Flags reference — engine vs server vs JSON vs sampling vs env layers overwhelm a new user — close: three-knobs-first box at the top.
73. Flags reference — `Usually never` on dozens of rows hides the few that matter here — close: fork-relevant subset called out.
74. Flags reference — `--max-context` bigger-equals-more-memory lacks numbers — close: per-context memory guidance link.
75. Flags reference — `--vram-reserve-mib` defaults disagree across pages — close: single installed default stated once.
76. Flags reference — setup/build cmake defines duplicate the build script with no precedence — close: which source wins on conflict.

## Stage 7 — agent setup and coverage check

77. Agent setup — title and folder names still say the old project name — close: fork-correct names and paths throughout.
78. Agent setup — model table is upstream-first with no fork-default row — close: fork-default family/model row at the top.
79. Agent setup — download sizes disagree across pages — close: one per-model size table used everywhere.
80. Agent setup — RAM table has no 16 GB row and no below-minimum stop — close: explicit too-small verdict plus smaller-model or stop rule.
81. Agent setup — AMD path documented though the fork cuts AMD — close: single AMD verdict with stop rule.
82. Agent setup — iGPU/laptop-iGPU never mentioned — close: explicit unsupported verdict.
83. Agent setup — old-GPU and no-AVX2 experimental paths lack opt-in wording — close: exact flag plus slow/experimental warning.
84. Agent setup — Linux/sudo/ROCm sections apply to a Windows-first fork path — close: Windows-first ordering with Linux as appendix.
85. Agent setup — `While it downloads` gives one speed example only — close: size-to-time table plus resume statement.
86. Agent setup — start/verify commands use old run names and old engine flag — close: fork-correct start and verify commands.
87. Agent setup — `/health` vs `/v1/models` vs chat-completion check order unstated — close: numbered verify sequence with expected outputs.
88. Agent setup — app-connection section lists five clients with no first choice — close: browser-first rule, agents second.
89. Agent setup — failure table misses wrong-gguf-path, mmproj mismatch, cmake/nvcc fail, OOM-at-load, firewall prompt — close: one row per failure with fix.
90. Agent setup — log-file name still uses the old prefix — close: fork-correct log name and location.
91. Agent setup — MCP-server alternative has no when-to-use rule — close: shell-vs-MCP decision line.

## Stage 8 — first chat

92. First chat — no URL plus what to expect on first paint — close: address plus ready-vs-loading states.
93. First chat — no first message suggestion — close: one short hello prompt plus where to type it.
94. First chat — no thinking-effort guidance for a fast first reply — close: default effort plus quick-test setting.
95. First chat — no latency expectation for first vs later replies — close: wait ranges with progress signal.
96. First chat — PC-freeze warning lives far from the chat step — close: one-line freeze note repeated at start.
97. First chat — context-overflow and new-chat rule unexplained — close: symptom plus fix in place.
98. First chat — images-first-try path unexplained — close: vision-on vs text-only check before attaching a picture.
99. First chat — API-key/firewall prompts on first open undocumented — close: allow-vs-block rule for the local port.
100. First chat — where chats persist (browser only) unstated at first chat — close: one-line persistence note before they type.
