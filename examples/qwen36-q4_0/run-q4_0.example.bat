@echo off
setlocal
title HyperStrata example run qwen36 UD-Q4_K_M q4_0 KV
REM Example launcher for the q4_0 KV run. NOT a real path: edit the four
REM variables below, then run. It derives a throwaway config in %TEMP% and
REM never edits your installed config in place. Needs a local-built engine
REM with the q4_0 kernels (see docs/KV_CACHE.md) and a model file you
REM provide yourself (see README in this folder). Port stays 8080.
REM PROOF it ran q4_0: the engine log must contain "q4_0 KV cache:".

REM --- edit these four ---
set REPO_ROOT=<ABSOLUTE_PATH_TO_THIS_REPO>
set MODEL_FILE=<ABSOLUTE_PATH_TO_Qwen3.6-35B-A3B-UD-Q4_K_M.gguf>
set PACKS_DIR=<ABSOLUTE_PATH_TO_PACKS_DIR_qwen36-ud-q4_k_m>
set CUDA_LIB_DIR=<ABSOLUTE_PATH_TO_CUDA_BIN_x86_64>

set SRC=%REPO_ROOT%\strata-qwen36-ud-q4_k_m.json
set TMP_JSON=%TEMP%\strata-qwen36-ud-q4_k_m-q4_0.json
set Q4_LOG=%REPO_ROOT%\strata-qwen36-ud-q4_k_m-q4_0.log

if not exist "%MODEL_FILE%" (
  echo ERROR: model file missing: %MODEL_FILE%
  echo This repo ships no weights. See README in this folder.
  pause
  exit /b 1
)
if not exist "%REPO_ROOT%\serve\server.py" (
  echo ERROR: not a repo checkout: %REPO_ROOT%
  pause
  exit /b 1
)

REM Pick python the same way the server needs it.
set PY=
if exist "%REPO_ROOT%\.venv\Scripts\python.exe" set PY="%REPO_ROOT%\.venv\Scripts\python.exe"
if not defined PY set PY=python

REM Derive temp q4_0 config from the installed one (or from the example shape):
REM append --kv q4_0 --draft-kv q4_0 --vram-reserve-mib 256, repoint log,
REM inject temperature 0.6 + thinking budget 4096. Refuses when the source
REM already carries --kv/--draft-kv (the installed default must stay fp16).
set SRC_JSON=%SRC%
set TMP_OUT=%TMP_JSON%
set Q4LOG_OUT=%Q4_LOG%
%PY% -c "import json,os,sys; src=os.environ.get('SRC_JSON'); tmp=os.environ.get('TMP_OUT'); qlog=os.environ.get('Q4LOG_OUT'); d=json.load(open(src,encoding='utf-8')); args=list(d.get('args',[])); (print('ERROR: source config already contains --kv, refusing q4_0 override') or sys.exit(10)) if ('--kv' in args) else None; (print('ERROR: source config already contains --draft-kv, refusing q4_0 override') or sys.exit(10)) if ('--draft-kv' in args) else None; args=args+['--kv','q4_0','--draft-kv','q4_0','--vram-reserve-mib','256']; d['args']=args; d['log']=qlog; s=dict(d.get('sampling') or {}); s['temperature']=0.6; d['sampling']=s; d['reasoning_budget_tokens']=4096; json.dump(d,open(tmp,'w',encoding='utf-8'),indent=1); print('Wrote temp q4_0 config: '+tmp)"
if errorlevel 1 (
  echo q4_0 config derivation failed, server NOT started.
  pause
  exit /b 1
)

cd /d "%REPO_ROOT%"
%PY% "%REPO_ROOT%\serve\server.py" --engine strata --config "%TMP_JSON%" --port 8080 --open
if errorlevel 1 pause
