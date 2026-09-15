# SDL3Stuff

A cross-platform C++ game project built on **SDL3**, **SDL3_image**, and **PhysFS**, targeting Windows, macOS, Linux, iOS, Android, PSP, Vita, and Switch. Windows/macOS/Linux build via CMake or the original makefile; the other platforms use their own platform-specific build files.

This is a learning project — first time using SDL3, and still picking up C++ and makefiles along the way. Windows is the primary, actively-developed platform right now; the rest are in various states of catching up (see [Platform status](#platform-status)).

## Project structure

```
src/
├── main.cpp      # Entry point: SDL/window/PhysFS init and the event loop
├── engine/       # Reusable SDL wrapper classes — no game-specific logic
│   ├── platform.h    # Per-OS #ifdefs, central SDL3/SDL3_image/PhysFS include hub, getBasePath()
│   ├── Renderer.*     # RAII wrapper around SDL_Renderer
│   ├── Viewport.*     # Wraps an SDL_Rect as a renderer viewport
│   ├── Sprite.*       # Loads/draws a texture (PhysFS -> SDL3_image -> SDL_Texture)
│   └── SDLPhysFS.*    # PhysFS-backed file reading helpers
└── game/         # Game-specific logic (currently just the demo scene)
    └── Game.*         # Owns the demo viewport/sprite and per-frame draw logic

Assets/            # Files mounted into PhysFS at runtime (e.g. sprite.png, test.txt)
deps/<Platform>/   # Vendored third-party headers/libs per platform (SDL3, SDL3_image, PhysFS)
build/             # Build output (gitignored)
```

Build definitions live at the repo root: `CMakeLists.txt` + `CMakePresets.json` and `makefile` (both Windows/macOS/Linux), `Switch.mk`, `Vita.mk`, `Xcode.xcodeproj` (macOS/iOS), and `AndroidProject/` (Gradle).

`engine/` code should stay generic and reusable; anything specific to this particular game belongs in `game/`.

## Dependencies

- **[SDL3](https://github.com/libsdl-org/SDL)** — windowing, rendering, input.
- **[SDL3_image](https://github.com/libsdl-org/SDL_image)** — PNG loading for `Sprite`. Currently vendored for **Windows only**; other platforms still need this added under `deps/<Platform>/` before sprite loading will work there.
- **[PhysFS](https://icculus.org/physfs/)** — mounts `Assets/` as a virtual filesystem so game code reads assets through a portable path (`/sprite.png`) instead of platform-specific filesystem paths.

SDL3, SDL3_image, and PhysFS are all licensed under the [zlib license](https://opensource.org/license/zlib); see `deps/Windows/include/SDL3/LICENSE.txt`, `deps/Windows/include/SDL3_image/LICENSE.txt`, and `deps/include/PhysFS/LICENSE.txt`.

## Building

Windows, macOS, and Linux can be built either with **CMake** (works in VS Code and the terminal) or with the original **makefile**. Both build the same `src/` tree and produce an equivalent binary — CMake is the newer, recommended path; the makefile is kept for compatibility and because Switch/Vita/iOS/Android still depend on their own build files (`Switch.mk`, `Vita.mk`, `Xcode.xcodeproj`, `AndroidProject/`), not on either of these.

### CMake (Windows / macOS / Linux)

`CMakeLists.txt` mirrors the makefile's per-platform logic (same include/lib paths under `deps/<Platform>/`, same linked libraries, same Windows DLL/Assets copying, same always-`-O3`-plus-optional-`-g3` flags). `CMakePresets.json` defines configure/build presets per platform and build type.

**In VS Code:** install the *CMake Tools* extension, open this folder, and pick a preset (e.g. `windows-debug`) from the status bar — Configure/Build/Debug all work from there. `.vscode/settings.json` already points `cmake.sourceDirectory` at the repo root and defaults the generator to Ninja.

**In a terminal** (Windows needs Ninja and CMake on `PATH` — both installable via MSYS2's `pacman -S mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja`):

```
cmake --preset windows-debug
cmake --build --preset windows-debug
# binary + Assets/ + runtime DLLs land in build/windows-debug/bin/
```

Swap `windows-debug` for `windows-release`, `windows-release-static` (statically linked, no DLL copying — see the `BUILD_STATIC` caveat in `CMakeLists.txt`), `macos-debug`/`macos-release`, or `linux-debug`/`linux-release`. Unlike the makefile (see the [Windows build note](#windows) below), the CMake/Ninja path works fine from both PowerShell and Git Bash on Windows — it doesn't depend on the `OS` environment variable propagating into the build.

The macOS and Linux CMake presets are translated from the makefile's logic but haven't been build-tested (this repo is developed on Windows) — see the comments in `CMakeLists.txt` for known caveats (e.g. macOS's `deps/MacOS` currently only vendors an SDL2 framework, not SDL3).

### makefile (all platforms)

#### Windows

Builds with MinGW64 via MSYS2 (Visual Studio or a standalone MinGW should work too, with some adjustment).

- SDL3 and SDL3_image headers/DLLs are sourced from their official MinGW devel packages.
- PhysFS's DLL is built with MinGW64; headers are from the 3.0.2 release.
- Builds dynamically by default, linking `libgcc_s_seh-1`, `libstdc++-6`, and `libwinpthread-1` from MinGW64. The makefile has a `DYNAMIC` flag to build statically instead (static libs for that path aren't checked into the repo).

```
make OS=Windows_NT
```

**Note:** on this project's dev machine, `make` must be run from PowerShell, not Git Bash — Git Bash's MSYS `make` doesn't pass the `OS` environment variable through to the build, so it silently picks no build target. The CMake path above doesn't have this problem.

#### macOS

Builds with clang. SDL3 and PhysFS are installed via Homebrew; static libs targeting macOS 10.9 are included in `deps/MacOS/`. The makefile links statically and targets macOS 10.9.

```
make
```

#### Linux

Builds with g++ against `libsdl3-dev` and `libphysfs-dev` from your package manager. Compiled with `-no-pie` — without it, the binary gets treated as a shared library for some reason. Static linking on Linux hasn't been tried yet.

```
make
```

### iOS

Built with Xcode (`Xcode.xcodeproj`), targeting iOS 9.0 minimum (13.0+ needed for controller support). Vendored headers/static libs and the assets needed for the app bundle are included under `deps/iOS/`.

**Note:** iOS is still on the old SDL2-based setup (`deps/iOS/include/SDL2`) — it hasn't been ported to SDL3 yet. See [Platform status](#platform-status).

### Vita and Switch

Both build against their respective homebrew SDKs and SDL3/PhysFS ports:

```
make vita     # runs Vita.mk
make switch   # runs Switch.mk
```

- Switch platform code is gated behind `__SWITCH__`; Vita's is gated behind `SDL_PLATFORM_VITA`.
- `Switch.mk` dumps all `.o` files directly into `/build` rather than mirroring `src/`'s folder structure.

### PSP and 3DS

Both are work-in-progress / unproven — no confirmation yet that they actually build. 3DS is additionally limited to PhysFS 3.0.2; every other platform can use 3.2.0.

### Android

`AndroidProject/` is a Gradle scaffold carried over from an SDL2 project template and hasn't been updated for this game yet — nothing here builds or runs the actual game yet.

## Platform status

| Platform | SDL version | Status |
|---|---|---|
| Windows | SDL3 | Actively developed, working end-to-end |
| macOS | SDL3 | Builds, not actively exercised |
| Linux | SDL3 | Builds, not actively exercised |
| Vita | SDL3 | Builds against homebrew SDK, not actively exercised |
| Switch | SDL3 | Builds against homebrew SDK, not actively exercised |
| iOS | SDL2 | Not yet ported to SDL3 |
| Android | SDL2 (template only) | Scaffold only, not integrated |
| PSP | — | Unproven / WIP |
| 3DS | — | Unproven / WIP |

## To-do

- [x] Windows — SDL3
- [ ] Port SDL3_image + current `engine/`/`game/` code to: macOS, Linux, Vita, Switch (SDL3 core is already there for these; verify the sprite/asset pipeline actually works)
- [ ] Port iOS from SDL2 to SDL3
- [ ] Port Android from the SDL2 template to SDL3
- [ ] PSP, 3DS
- [ ] Upgrade select platforms to PhysFS 3.2
- [ ] Actually make something cool
