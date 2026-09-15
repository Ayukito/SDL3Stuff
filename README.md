# SDL3Stuff

A cross-platform C++ game project built on **SDL3**, **SDL3_image**, and **PhysFS**, targeting Windows, macOS, Linux, iOS, Android, PSP, Vita, and Switch. Windows/macOS/Linux build via CMake or the original makefile; Android builds via a CMake-backed Gradle project; macOS/iOS can also generate a real Xcode project via CMake; PSP/Vita/Switch use their own platform-specific build files.

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
cmake/             # Vendored ios.toolchain.cmake (github.com/leetal/ios-cmake), for iOS CMake builds
build/             # Build output (gitignored)
old/               # Archived AndroidProject/Xcode.xcodeproj from before the CMake-based rewrite
```

Build definitions live at the repo root: `CMakeLists.txt` + `CMakePresets.json` (Windows/macOS/Linux/Android/iOS) and `makefile` (Windows/macOS/Linux), plus `Switch.mk`, `Vita.mk` for the remaining platforms. `AndroidProject/` is a Gradle wrapper around the same root `CMakeLists.txt`. `old/` holds the previous `AndroidProject/`/`Xcode.xcodeproj` from before this CMake-based rewrite, kept for reference.

`engine/` code should stay generic and reusable; anything specific to this particular game belongs in `game/`.

## Dependencies

- **[SDL3](https://github.com/libsdl-org/SDL)** — windowing, rendering, input.
- **[SDL3_image](https://github.com/libsdl-org/SDL_image)** — PNG loading for `Sprite`. Vendored for Windows and Android; other platforms (macOS/Linux/iOS/Vita/Switch) still need it added before sprite loading will work there.
- **[PhysFS](https://icculus.org/physfs/)** — mounts `Assets/` as a virtual filesystem so game code reads assets through a portable path (`/sprite.png`) instead of platform-specific filesystem paths. Prebuilt everywhere except Android, where it's compiled directly from the vendored source in `deps/include/PhysFS/` (see `CMakeLists.txt`'s Android branch).

SDL3, SDL3_image, and PhysFS are all licensed under the [zlib license](https://opensource.org/license/zlib); see `deps/Windows/include/SDL3/LICENSE.txt`, `deps/Windows/include/SDL3_image/LICENSE.txt`, and `deps/include/PhysFS/LICENSE.txt`.

## Building

Windows, macOS, and Linux can be built either with **CMake** (works in VS Code and the terminal) or with the original **makefile**. Both build the same `src/` tree and produce an equivalent binary — CMake is the newer, recommended path; the makefile is kept for compatibility. Switch and Vita still depend on their own build files (`Switch.mk`, `Vita.mk`); Android and iOS/macOS-via-Xcode are also CMake-backed now (`AndroidProject/`'s Gradle build, and the `ios-device`/`ios-simulator`/`macos-xcode` presets), just through a different entry point than the plain `cmake --preset` commands below.

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

The macOS and Linux CMake presets are translated from the makefile's logic but haven't been build-tested (this repo is developed on Windows) — see the comments in `CMakeLists.txt` for known caveats. macOS now resolves SDL3/SDL3_image via Homebrew's CMake config (`brew install sdl3 sdl3_image`) instead of the makefile's `-framework SDL3` search, which never actually had an SDL3 framework to find in `deps/MacOS/`.

#### Generating a real Xcode project (macOS / iOS)

CMake's Xcode generator only runs on macOS itself, so this repo can't produce an `.xcodeproj` from Windows — but the presets to do it on a Mac are already set up:

```
cmake --preset macos-xcode        # → build/macos-xcode/SDL3Stuff.xcodeproj
cmake --preset ios-device         # → build/ios-device/SDL3Stuff.xcodeproj
cmake --preset ios-simulator      # → build/ios-simulator/SDL3Stuff.xcodeproj
```

The iOS presets use `cmake/ios.toolchain.cmake` (vendored from [leetal/ios-cmake](https://github.com/leetal/ios-cmake)). **iOS is scaffold-only and not build-tested** — SDL3's only official Apple binary release is a `.dmg` disk image, which can't be read on Windows, so `deps/Apple/SDL3.xcframework` and `deps/Apple/SDL3_image.xcframework` don't exist yet. To finish it on a Mac: download `SDL3-<version>.dmg` from [SDL's releases](https://github.com/libsdl-org/SDL/releases), mount it, and copy `SDL3.xcframework` into `deps/Apple/` (same for `SDL3_image`). `deps/iOS/lib/libphysfs.a` is reused as-is, but it's a device-only fat binary (arm64+armv7, no simulator slice — verified via its Mach-O header), so `ios-simulator` will need a new PhysFS build before it links.

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

`AndroidProject/` is a Gradle project whose native build (`app/build.gradle`'s `externalNativeBuild.cmake`) points directly at the repo's root `CMakeLists.txt` — there's no separate copy of the native build config to keep in sync. SDL3 and SDL3_image come from their official prebuilt Android AARs (`app/libs/`, using [Prefab](https://google.github.io/prefab/) — see each AAR's bundled `INSTALL.md`); PhysFS has no official Android binary, so it's compiled from the vendored source in `deps/include/PhysFS/` as part of the same CMake configure.

**Verified working**: built end-to-end on this machine after installing the Android SDK (cmdline-tools), NDK 28.2.13676358, and CMake 3.22.1 via `sdkmanager`, and produced a real `app-debug.apk` (~16 MB, all four ABIs) via:

```
cd AndroidProject
.\gradlew.bat assembleDebug
```

`local.properties` (gitignored, machine-specific) needs `sdk.dir=<path to your Android SDK>`.

**Assets on Android**: `Assets/` can't be mounted as a real PhysFS directory here — it lives inside the APK, only reachable through `AAssetManager`. Instead, `app/build.gradle`'s `zipAssets` task zips the shared root `Assets/` into `Assets.zip` and packages that as a normal APK asset; `SDLPhysFS::mountAssets()` (`src/engine/SDLPhysFS.cpp`) reads it via `SDL_LoadFile()` (which transparently reads through `AAssetManager` on Android) and mounts the bytes directly with `PHYSFS_mountMemory()` — no extraction to disk needed. Every other platform still mounts `Assets/` as a real directory through the same function. The APK build was confirmed to actually contain `assets/Assets.zip`; the runtime mount/read path itself hasn't been exercised on an actual device/emulator (no AVD set up on this machine).

**`PHYSFS_init()` on Android**: unlike every other platform, PhysFS on Android doesn't take `argv0` — it expects a pointer to a `PHYSFS_AndroidInit` struct (a `JNIEnv*` + the `Context`/`Activity` `jobject`, both typed as `void*` so `physfs.h` doesn't need `jni.h`), which it uses internally to resolve the app's data/cache directories. `main.cpp` builds this from `SDL_GetAndroidJNIEnv()`/`SDL_GetAndroidActivity()` under `#ifdef SDL_PLATFORM_ANDROID`, called after `SDL_Init()` (which is what sets those up on Android) and before `PHYSFS_init()`. Compiles cleanly for all four ABIs; not runtime-verified (no device/emulator on this machine).

## Platform status

| Platform | SDL version | Status |
|---|---|---|
| Windows | SDL3 | Actively developed, working end-to-end (make and CMake) |
| Android | SDL3 | CMake+Gradle build verified (produces a real APK); PhysFS asset mounting not yet implemented on-device; not run on an emulator/device |
| macOS | SDL3 | Builds via makefile/CMake, not actively exercised; CMake now resolves SDL3 via Homebrew, not build-tested |
| Linux | SDL3 | Builds, not actively exercised |
| Vita | SDL3 | Builds against homebrew SDK, not actively exercised |
| Switch | SDL3 | Builds against homebrew SDK, not actively exercised |
| iOS | SDL3 (scaffold only) | CMake/Xcode-generator + iOS toolchain set up, but SDL3.xcframework isn't vendored yet (needs a Mac — see [Building](#generating-a-real-xcode-project-macos--ios)); not build-tested |
| PSP | — | Unproven / WIP |
| 3DS | — | Unproven / WIP |

## To-do

- [x] Windows — SDL3 (make and CMake)
- [x] Android — CMake+Gradle build working; SDL3 core done
- [x] PhysFS asset mounting on Android — `Assets.zip` + `PHYSFS_mountMemory()`, not yet run on-device to confirm
- [ ] Run the Android build on an actual emulator/device
- [ ] Vendor `SDL3.xcframework`/`SDL3_image.xcframework` on a Mac and verify the `ios-device`/`ios-simulator`/`macos-xcode` CMake presets actually build
- [ ] Get a simulator-capable PhysFS build for iOS (current `deps/iOS/lib/libphysfs.a` is device-only)
- [ ] Port SDL3_image + current `engine/`/`game/` code to: macOS, Linux, Vita, Switch (SDL3 core is already there for these; verify the sprite/asset pipeline actually works)
- [ ] PSP, 3DS
- [ ] Upgrade select platforms to PhysFS 3.2
- [ ] Actually make something cool
