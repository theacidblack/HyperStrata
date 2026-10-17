@echo off
rem BUILD_HYPER.bat - build the HyperStrata engine (hyper.exe) for your NVIDIA card, in one double-click.
rem
rem What it does: finds your Visual Studio + CUDA toolkit, configures a stock CUDA build into
rem build-hyper\, compiles the "hyper" target with 8 parallel jobs, copies hyper.exe into
rem engine-hyper\, then runs a smoke check (hyper.exe --help, which loads no model and starts no server).
rem Takes 10-20 minutes the first time, a few minutes after a source change (only what changed rebuilds).
rem
rem Before you run it: close the HyperStrata server / any run-*.bat window first. Building while the
rem model is loaded can run this PC out of memory. Nothing here uploads anything or starts a server.
rem
rem Needs (plain words): an NVIDIA driver, the NVIDIA CUDA Toolkit, Visual Studio 2019 or 2022 with the
rem "Desktop development with C++" workload, and cmake + ninja (setup's .venv already has them, else
rem ".venv\Scripts\python.exe -m pip install cmake ninja"). The first build also needs internet once:
rem CMake fetches the pinned ggml commit by itself (see STRATA_GGML_DIR below).
rem
rem If a step fails, the message says what is missing and what to paste in a report - see each :fail block.
setlocal
title HyperStrata - build engine
cd /d "%~dp0"

rem ============ Build settings: every active flag, one per line ============
rem CUDA_ARCH - which card generation to build code for. This MUST match your card; the CMake default
rem   is 120 (RTX 50), which will not run on older cards. When to change it: set it to YOUR card.
rem   75 = RTX 20 series, 86 = RTX 30 series (incl. the reference RTX 3070 Ti Laptop, Ampere SM86),
rem   89 = RTX 40 series, 120 = RTX 50 series. Several cards / unknown card: 75;86;89;120 (slower build).
set "CUDA_ARCH=86"
rem BUILD_TYPE - Release = fast, optimized engine. When to change it: Debug only if a developer asks
rem   for a debuggable build (it runs much slower). CMake defaults to Release on its own too.
set "BUILD_TYPE=Release"
rem JOBS - parallel compile jobs. When to change it: lower (e.g. 4) if the build runs out of RAM;
rem   raise it toward your CPU thread count if the disk/CPU sit idle. README's procedure uses 8.
set "JOBS=8"
rem ---- The cmake flags below are fixed by these settings (each does one thing): ----
rem -DSTRATA_ENABLE_CUDA=ON : build the CUDA GPU targets. OFF would skip the engine itself - never
rem   turn it off in this file (AMD cards need a different HIP build; run setup instead).
rem -DSTRATA_BUILD_TESTS=OFF : skip the ~122-test suite (~4 min). Turn ON (with ctest after) if you
rem   want to verify the build beyond the smoke check at the end.
rem -DCMAKE_CUDA_ARCHITECTURES=%CUDA_ARCH% : which card generation the kernels compile for (see CUDA_ARCH).
rem -DCMAKE_CUDA_COMPILER=<found nvcc> : pin the exact toolkit found below, so CMake cannot mix one
rem   toolkit's headers with another toolkit's libraries.
rem -G Ninja + -DCMAKE_MAKE_PROGRAM=<found ninja> : the build tool (same as setup.py uses).
rem (STRATA_GGML_DIR is deliberately NOT passed: empty means CMake fetches the pinned ggml commit itself.)
rem
rem ============ Optional flags (commented out): uncomment to use ============
rem For Pascal / Volta cards (sm_60, sm_61, sm_70, older than RTX 20): needs a CUDA 12.x toolkit,
rem   because CUDA 13 cannot compile for them. Uncomment both lines:
rem set "CUDA_ARCH=60;61;70"
rem set "EXTRA_DEFS=-DSTRATA_EXPERIMENTAL_SM60=ON"
rem For a card you don't know: build for all supported generations (slow build, runs anywhere sm_75+):
rem set "CUDA_ARCH=75;86;89;120"
rem Portable engine for OTHER PCs (AVX2 baseline + static MSVC runtime, no redistributable needed):
rem set "EXTRA_DEFS=-DSTRATA_PORTABLE=ON"
rem Drop the Unsloth K-quant prompt kernels (smaller build; UD-Q4/Q5/Q6 files then run the FP16 prompt path):
rem set "EXTRA_DEFS=-DSTRATA_MMQ_KQUANTS=OFF"
rem Drop the Q6_K grouped expert kernels (only matters for UD-Q6_K_XL / UD-Q5_K_XL layer 2):
rem set "EXTRA_DEFS=-DSTRATA_Q6K_EXPERTS=OFF"
rem Reuse your own llama.cpp checkout for ggml instead of fetching the pinned commit (needs internet once
rem   otherwise); path must contain ggml\CMakeLists.txt:
rem set "EXTRA_DEFS=-DSTRATA_GGML_DIR=C:\path\to\llama.cpp"
rem AMD cards are NOT this file: they need -DSTRATA_ENABLE_HIP=ON -DSTRATA_ENABLE_CUDA=OFF plus ROCm,
rem   which setup wires up for you - run START-HERE.bat instead of uncommenting anything here.
rem The image encoder (hyper-vision.exe) is NOT this file either: it is a separate project in
rem   tools\vision (STRATA_VISION_CUDA) that setup builds on demand when you pick --vision.
set "EXTRA_DEFS="

set "SRC=%~dp0"
set "BUILDDIR=%~dp0build-hyper"
set "ENGDIR=%~dp0engine-hyper"
set "SMOKE=%TEMP%\hyper-smoke.txt"

rem cmake/ninja from setup's private environment first, so they work even when not on PATH
if exist "%~dp0.venv\Scripts\cmake.exe" set "PATH=%~dp0.venv\Scripts;%PATH%"

rem ---- 1. Visual Studio (gives vcvars64.bat, the compiler environment) ----
set "VCVARS="
if defined STRATA_VCVARS if exist "%STRATA_VCVARS%" set "VCVARS=%STRATA_VCVARS%"
if not defined VCVARS (
  set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
  if exist "%VSWHERE%" (
    for /f "usebackq delims=" %%i in (`""%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath"`) do set "VSINSTALL=%%i"
    if defined VSINSTALL if exist "%VSINSTALL%\VC\Auxiliary\Build\vcvars64.bat" set "VCVARS=%VSINSTALL%\VC\Auxiliary\Build\vcvars64.bat"
  )
)
if not defined VCVARS goto fail_vcvars

rem ---- 2. CUDA toolkit (gives nvcc, the GPU compiler) ----
set "NVCC="
if defined STRATA_NVCC if exist "%STRATA_NVCC%" set "NVCC=%STRATA_NVCC%"
if not defined NVCC for /f "delims=" %%i in ('where nvcc 2^>nul') do if not defined NVCC set "NVCC=%%i"
if not defined NVCC if defined CUDA_PATH if exist "%CUDA_PATH%\bin\nvcc.exe" set "NVCC=%CUDA_PATH%\bin\nvcc.exe"
if not defined NVCC if defined CUDA_HOME if exist "%CUDA_HOME%\bin\nvcc.exe" set "NVCC=%CUDA_HOME%\bin\nvcc.exe"
if not defined NVCC (
  for /d %%d in ("C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v*") do if exist "%%d\bin\nvcc.exe" set "NVCC=%%d\bin\nvcc.exe"
)
if not defined NVCC goto fail_cuda
if not exist "%NVCC%" goto fail_cuda

rem ---- 3. cmake + ninja (the build driver) ----
set "CMAKE_BIN="
set "NINJA_BIN="
for /f "delims=" %%i in ('where cmake 2^>nul') do if not defined CMAKE_BIN set "CMAKE_BIN=%%i"
for /f "delims=" %%i in ('where ninja 2^>nul') do if not defined NINJA_BIN set "NINJA_BIN=%%i"
if not defined CMAKE_BIN goto fail_tools
if not defined NINJA_BIN goto fail_tools

echo Building the HyperStrata engine for sm_%CUDA_ARCH% ...
echo   compiler: %VCVARS%
echo   cuda:     %NVCC%
echo.

rem ---- 4. compiler environment (this window only; needed for every compile below) ----
call "%VCVARS%" >nul
if errorlevel 1 goto fail_vcvars_run

rem ---- 5. configure (source dir = this folder, build dir = build-hyper) ----
"%CMAKE_BIN%" -S "%SRC%" -B "%BUILDDIR%" -G Ninja "-DCMAKE_MAKE_PROGRAM=%NINJA_BIN%" -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DSTRATA_ENABLE_CUDA=ON -DSTRATA_BUILD_TESTS=OFF "-DCMAKE_CUDA_ARCHITECTURES=%CUDA_ARCH%" "-DCMAKE_CUDA_COMPILER=%NVCC%" %EXTRA_DEFS%
if errorlevel 1 goto fail_configure

rem ---- 6. build the hyper target ----
"%CMAKE_BIN%" --build "%BUILDDIR%" --target hyper -j %JOBS%
if errorlevel 1 goto fail_build
if not exist "%BUILDDIR%\hyper.exe" goto fail_build

rem ---- 7. install into engine-hyper (the folder the run scripts use) ----
rem NOTE: no BUILD.json is written on purpose. setup.py treats an engine folder without that stamp as
rem the user's own build and leaves it alone; a forged stamp would only confuse its rebuild check.
if not exist "%ENGDIR%" mkdir "%ENGDIR%" >nul 2>nul
copy /y "%BUILDDIR%\hyper.exe" "%ENGDIR%\hyper.exe" >nul
if errorlevel 1 goto fail_copy
if not exist "%ENGDIR%\hyper.exe" goto fail_copy

rem ---- 8. smoke check: --help prints usage WITHOUT loading a model or starting a server ----
rem (hyper.exe has no --version flag; --help is the documented check and always exits with code 2,
rem  so success is "the word usage: appeared", not "exit code 0".)
"%ENGDIR%\hyper.exe" --help > "%SMOKE%" 2>&1
findstr /c:"usage:" "%SMOKE%" >nul
if errorlevel 1 goto fail_smoke
del "%SMOKE%" >nul 2>nul

echo.
echo Build OK: %ENGDIR%\hyper.exe
echo The engine is ready. Start a model the usual way (START-HERE.bat or a run-*.bat).
echo This window can be closed.
pause
exit /b 0

:fail_vcvars
echo.
echo BUILD STOPPED: no Visual Studio C++ build tools found.
echo Install Visual Studio 2019 or 2022 with the workload "Desktop development with C++",
echo then run this file again. (CUDA 13.0-13.2 do not accept a Visual-Studio-2026-only install.
echo If your install is somewhere unusual, set STRATA_VCVARS to your vcvars64.bat and retry.)
echo What to paste in a report: this whole window text, your Windows version, and which Visual
echo Studio version you installed, if any.
pause
exit /b 1

:fail_cuda
echo.
echo BUILD STOPPED: no NVIDIA CUDA Toolkit found (no nvcc).
echo Install the NVIDIA CUDA Toolkit 13.x (13.0 is what the ready-made engine uses), then run this
echo file again. If it is installed somewhere unusual, set STRATA_NVCC to its nvcc.exe and retry.
echo (RTX 50 cards need CUDA 13.0 or newer; cards older than RTX 20 need CUDA 12.x plus the
echo STRATA_EXPERIMENTAL_SM60 lines at the top of this file.)
echo What to paste in a report: this whole window text, your GPU model, and your driver version
echo ("nvidia-smi" in a terminal prints both).
pause
exit /b 1

:fail_tools
echo.
echo BUILD STOPPED: cmake or ninja not found.
echo If this folder has a .venv, run ".venv\Scripts\python.exe -m pip install cmake ninja" once,
echo then run this file again. Otherwise install cmake and ninja and put them on PATH.
echo What to paste in a report: this whole window text.
pause
exit /b 1

:fail_vcvars_run
echo.
echo BUILD STOPPED: the compiler setup (vcvars64.bat) did not run.
echo Repair or reinstall the Visual Studio C++ workload, restart Windows, and run this file again.
echo What to paste in a report: this whole window text plus the full path printed after "compiler:".
pause
exit /b 1

:fail_configure
echo.
echo BUILD STOPPED: the configure step failed (the error lines are just above this).
echo Common causes: an old build-hyper folder configured for another GPU or toolkit (delete the
echo build-hyper folder and run again), no internet on the first run (ggml is fetched once),
echo or a CUDA toolkit your Visual Studio version does not accept.
echo What to paste in a report: this whole window text (from the top), the output of
echo "nvcc --version", and your Visual Studio version.
pause
exit /b 1

:fail_build
echo.
echo BUILD STOPPED: the compile failed (the error lines are just above this).
echo Common causes: the model/server still running while building (close it, run again), a CUDA
echo toolkit too old for your card, or a half-finished earlier build (delete the build-hyper
echo folder and run again; only what is missing recompiles, so retrying is cheap).
echo What to paste in a report: this whole window text (from the top), your GPU model, the output
echo of "nvcc --version", and your Visual Studio version.
pause
exit /b 1

:fail_copy
echo.
echo BUILD STOPPED: hyper.exe was built but could not be copied into engine-hyper.
echo Check that no antivirus is quarantining hyper.exe and that engine-hyper is writable,
echo then run this file again.
echo What to paste in a report: this whole window text.
pause
exit /b 1

:fail_smoke
echo.
echo BUILD STOPPED: hyper.exe was copied but the smoke check failed (it did not print its help text).
echo Something is wrong with the new binary - do NOT use it. Most likely the CUDA_ARCH at the top of
echo this file does not match your card (86 = RTX 30, 89 = RTX 40, 120 = RTX 50, 75 = RTX 20).
echo What to paste in a report: this whole window text, your GPU model, and the file %SMOKE% if it exists.
pause
exit /b 1
