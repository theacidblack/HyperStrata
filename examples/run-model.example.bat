@echo off
REM Copy this + model.example.json next to your install. 3 EDITS below: REPO_ROOT, MODEL_JSON, PY.
REM 1. REPO_ROOT = your install folder (has serve\server.py). 2. MODEL_JSON = your copied .json name.
REM 3. PY = a python with the server's packages (install's .venv\Scripts\python.exe, else python).
REM model.example.json needs its own 3 edits first (engine exe, model gguf, mmproj or no vision block).
REM Starts python serve\server.py --engine hyper --config ... --port 8080 --open; chat at http://127.0.0.1:8080.
setlocal
set REPO_ROOT=OWN-PATHS-HERE\install-folder
set MODEL_JSON=OWN-PATHS-HERE\model-file.json
set PY=OWN-PATHS-HERE\.venv\Scripts\python.exe

if not exist "%REPO_ROOT%\serve\server.py" (
  echo ERROR: no checkout at REPO_ROOT: %REPO_ROOT%
  pause
  exit /b 1
)
if not exist "%MODEL_JSON%" (
  echo ERROR: config missing: %MODEL_JSON%
  echo Copy examples\model.example.json, make its 3 edits, set MODEL_JSON to it.
  pause
  exit /b 1
)
cd /d "%REPO_ROOT%"
%PY% "%REPO_ROOT%\serve\server.py" --engine hyper --config "%MODEL_JSON%" --port 8080 --open
if errorlevel 1 pause
