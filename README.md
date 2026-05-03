# Basin Engine

A simple 3D rendering engine built with OpenGL.

## Dependencies

- C++17 compiler
- OpenGL
- [GLFW3](https://github.com/glfw/glfw) — window and context
- [Assimp](https://github.com/assimp/assimp) — model loading
- [FreeType](https://freetype.org/) — text rendering
- [GLM](https://github.com/g-truc/glm) — math (found via your package manager, vcpkg, or fetched by CMake if not found)

## Building

This project sets `CMAKE_EXPORT_COMPILE_COMMANDS` to `ON`; after configuring, you can use `build/compile_commands.json` (or a symlink) with clangd and other tools.

If the `build/` tree was created on another machine or the cache is wrong, delete `build/` and configure again.

### Linux (system packages)

Install development packages for the libraries above (names vary: `libglfw3-dev`, `libassimp-dev`, `libfreetype6-dev`, `libglm-dev` on Debian/Ubuntu, plus OpenGL/GLU/Mesa as needed), then:

```bash
cmake -B build
cmake --build build
./build/my_app
# or: cmake --build build --target run
./run.sh
```

### Windows (vcpkg recommended)

1. [Install vcpkg](https://vcpkg.io/en/getting-started) and a C++toolset (Visual Studio 2022 / Build Tools with “Desktop development with C++”).
2. [Install **Ninja](https://ninja-build.org/)** (e.g. `winget install Ninja-build.Ninja`, or a copy ships with some VS installs) so the **Ninja** generator is available. Open **Developer PowerShell for Visual Studio 2022** (or the **x64 Native Tools** prompt) so the MSVC tools are on `PATH`—that matters for a Ninja build, which does not auto-discover a full Visual Studio “instance” the way the Visual Studio *generator* does.
3. From the project root, with `VCPKG_ROOT` set, configure and build using the **Ninja + vcpkg** preset (recommended; see [CMakePresets.json](CMakePresets.json)):
  ```powershell
   # Use "Developer PowerShell for VS 2022" (or x64 Native Tools) so cl.exe is on PATH.
   $env:VCPKG_ROOT = "C:\path\to\vcpkg"
   rmdir -Recurse -ErrorAction SilentlyContinue build
   cmake --preset win-ninja-vcpkg
   cmake --build build
  ```
   The executable is `build\my_app.exe` (Ninja is single-config, **Release** via the preset’s `CMAKE_BUILD_TYPE`).
   Run:
   Or use: `.\run.ps1` (after a successful configure).
4. If you **prefer the Visual Studio IDE generator** and it works on your machine, you can use `--preset win-vs` / `--preset win-vs-vcpkg` and `cmake --build build --config Release` as before.

**If `cmake -B build` or `win-vs` says NMake, `nmake` not found, or `CMAKE_CXX_COMPILER` is not set**  
Plain PowerShell often has no C++ toolset. Use a **VS Developer** shell, or a preset that does not rely on NMake (Ninja, or a Visual Studio *generator* from a shell where CMake’s VS detection works). Delete `build/` and re-run configure after changing the approach.

**If the generator is `Visual Studio 17 2022` and CMake says it “could not find any instance of Visual Studio”**  
C++(e.g. `cl.exe`) may be installed, but the **Visual Studio** generator’s registration check failed (partial install, or CMake cannot see the IDE). This project does not require that generator: use `**cmake --preset win-ninja-vcpkg`** from a **Developer** shell, with **Ninja** on `PATH` (and delete a stale `build/` first). You can also install/repair the **“Desktop development with C++”** workload so the full VS toolset is registered, then `win-vs-vcpkg` may work.

**Visual Studio multi-config** — If you did configure with a Visual Studio generator, pass `--config` when building or running the custom target.

```powershell
cmake --build build --config Release --target run
```

**Multi-config (Visual Studio) vs single-config (Ninja):** with Ninja, the executable is under `build/`. With Visual Studio, it is `build/Release/my_app.exe` (or `Debug`).

`run.ps1` passes `--config` for multi-config generators; you can use it for Ninja in the `else` branch.

**Troubleshooting: “Could not find toolchain file”, Ninja, or `CMAKE_CXX_COMPILER` not set**  

| Symptom | What it means | What to do |
|--------|---------------|------------|
| `Could not find toolchain file: ...\vcpkg.cmake` | `VCPKG_ROOT` does not point at a real vcpkg clone, or vcpkg was never fully installed. | `VCPKG_ROOT` must be the folder that contains **`vcpkg.exe`**, and these files must exist: `scripts\buildsystems\vcpkg.cmake` and (usually) `vcpkg.exe`. Check in PowerShell: `Test-Path "C:\your\path\scripts\buildsystems\vcpkg.cmake"`. [Clone or install vcpkg](https://vcpkg.io/en/getting-started) into that path, *or* set `VCPKG_ROOT` to wherever you actually keep vcpkg (it is not always `C:\dev\vcpkg` unless you put it there). |
| `Could not find a build program corresponding to "Ninja"` / `CMAKE_MAKE_PROGRAM is not set` | The **Ninja** program is not installed or not on this shell’s `PATH`. | `winget install Ninja-build.Ninja` (then open a *new* terminal), or `choco install ninja`, or set `PATH` to the directory that contains `ninja.exe`. Verify: `ninja --version`. |
| `CMAKE_CXX_COMPILER not set` (with the Ninja preset) | **MSVC** (`cl.exe`) is not on `PATH`. | Start **“Developer PowerShell for VS 2022”** or **“x64 Native Tools Command Prompt for VS 2022”** from the Start menu (or run `VsDevCmd.bat` / the matching `vcvars64.bat` for your toolset) so the compiler is available, then run `cmake --preset win-ninja-vcpkg` again. |

Configuring and building in **plain** (non-Developer) **PowerShell** is often wrong for MSVC + Ninja: you need both **Ninja** and **cl** (from the same shell) before CMake can succeed.

### Run target and working directory

The `run` custom target always runs the app with the **source tree** as the working directory so relative paths work (`shaders/`, `scenes/`, `assets/`, `fonts/`).

## Font (text rendering)

The app looks for a TrueType font at `fonts/JetBrainsMonoNerdFont-Regular.ttf` (relative to the process working directory, which is the project root for `run` and `./run.sh`).  
Place that file in the `fonts/` directory, or set the environment variable `BASIN_FONT` to a full path of any `.ttf` file.  
Example: [JetBrains Mono (Nerd Font build)](https://github.com/ryanoasis/nerd-fonts).

## Features

- OBJ model loading via Assimp
- Basic collision detection (AABB)
- First-person camera controller
- Text rendering with Freetype
- Shader system with vertex/fragment shaders
- Primitive generation (cubes, planes, etc.)
- Scene system with JSON-based scene loading

## Controls

- WASD — move
- Mouse — look
- ESC — release cursor

