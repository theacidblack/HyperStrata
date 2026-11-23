# Build the engine

The engine is a single program called `strata-poly`, and you compile it yourself. Here are the
commands for Windows and Linux, and the one setting you can't skip: the graphics card architecture.

## 1. Check your PC

| Item | What you need |
| --- | --- |
| System | Windows or Linux, x86-64 |
| CPU | x86-64 |
| Graphics card | NVIDIA RTX 20, 30, 40 or 50 series |
| CUDA toolkit | 13.x (13.0 or newer for RTX 50 cards) |
| C++ compiler | Visual Studio 2022 Build Tools with the C++ workload, or GCC/Clang |
| CMake | 3.24 or newer |
| Disk | About 10 GB free for the build |
| Network | One download during the first configure step |

## 2. Pick your GPU setting

The build makes code for the cards you name. A build for one card does not run on another.

| Card | Value |
| --- | --- |
| RTX 20 series | `75` |
| A100, A-series | `80` |
| RTX 30 series | `86` |
| RTX 40 series | `89` |
| RTX 50 series | `120` |
| This PC only | `native` |

- The default is `120`. Set the flag for your card, or the program runs on RTX 50 only.
- RTX 50 code needs CUDA 13.0 or newer. Older toolkits build code that crashes on long prompts.
- Cards below compute capability 7.5 do not build.

## 3. Build on Windows

1. Open **x64 Native Tools Command Prompt for VS 2022** in the repository folder.
2. Clear the `CL` variable. A set `CL` breaks the CUDA compiler:
   ```bat
   set "CL="
   ```
3. Configure the build. Change `86` to your value from step 2:
   ```bat
   cmake -S . -B build -G "Visual Studio 17 2022" -A x64 ^
         -DCMAKE_BUILD_TYPE=Release ^
         -DSTRATA_ENABLE_CUDA=ON ^
         -DCMAKE_CUDA_ARCHITECTURES=86
   ```
4. Build the engine:
   ```bat
   cmake --build build --config Release --target strata-poly -j 6
   ```
5. The program is `build\Release\strata-poly.exe`.

The first configure downloads a pinned copy of the low-level tensor library. It needs network access one time.

## 4. Build on Linux

1. Configure the build. Change `86` to your value from step 2:
   ```sh
   cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
         -DSTRATA_ENABLE_CUDA=ON \
         -DCMAKE_CUDA_ARCHITECTURES=86
   ```
2. Build the engine:
   ```sh
   cmake --build build --target strata-poly -j"$(nproc)"
   ```
3. The program is `build/strata-poly`.

## 5. Make a build for other PCs

Add this option when you configure:

```sh
-DSTRATA_PORTABLE=ON
```

This sets the CPU code to the AVX2 baseline and links the Visual C++ runtime into the program.
Use it when you share the program with another person. Do not use it for your own PC.

## 6. Use the installer instead

`setup.py` can configure, build and install a model in one step:

```bat
python setup.py --family qwen36 --model UD-Q4_K_M --build --no-start
```

It downloads the model file too. Use the manual steps above when you already have a GGUF file.

## 7. Check the build

1. Run the program with no arguments. It prints its usage and option list.
2. Load a model with a small context:
   ```bat
   build\Release\strata-poly.exe --model "path\to\model.gguf" --max-context 4096 --serve
   ```
3. The last lines must say `loaded in ...` and `ready for requests`.
4. Stop the program with Ctrl+C.

## 8. If the build fails

- **`C1004` or a strange library error:** the `CL` variable is set. Clear it and configure again.
- **`LNK1136` or a broken object file:** delete the `build` folder and configure again.
- **`no kernel image is available` at run time:** the build has no code for your card.
  Repeat step 2 with the correct value.
- **The program crashes on a long prompt on an RTX 50 card:** use CUDA 13.0 or newer.
- **The linker runs out of memory:** close other programs, or build with fewer jobs (`-j 2`).
