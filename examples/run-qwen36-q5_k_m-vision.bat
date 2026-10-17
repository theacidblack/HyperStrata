@echo off
REM Preset run-qwen36-q5_k_m-vision.bat (MTP + images UD-Q5_K_M). Copy next to your install. 3 EDITS below: REPO_ROOT, MODEL_JSON, PY.
REM 1. REPO_ROOT = your install folder (has serve\server.py). 2. MODEL_JSON = your copied qwen36-q5_k_m-vision.json path.
REM 3. PY = a python with the server's packages (install's .venv\Scripts\python.exe, else python).
REM qwen36-q5_k_m-vision.json needs its own edits first (engine exe, MTP model gguf, vision mmproj with gpu false).
REM Starts python serve\server.py --engine hyper --config ... --port 8080 --open; chat at http://127.0.0.1:8080.
setlocal
set REPO_ROOT=OWN-PATHS-HERE\install-folder
set MODEL_JSON=OWN-PATHS-HERE\qwen36-q5_k_m-vision.json
set PY=OWN-PATHS-HERE\.venv\Scripts\python.exe

if not exist "%REPO_ROOT%\serve\server.py" (
  echo ERROR: no checkout at REPO_ROOT: %REPO_ROOT%
  pause
  exit /b 1
)
if not exist "%MODEL_JSON%" (
  echo ERROR: config missing: %MODEL_JSON%
  echo Copy examples\qwen36-q5_k_m-vision.json, make its edits, set MODEL_JSON to it.
  pause
  exit /b 1
)
cd /d "%REPO_ROOT%"
%PY% "%REPO_ROOT%\serve\server.py" --engine hyper --config "%MODEL_JSON%" --port 8080 --open
if errorlevel 1 pause
