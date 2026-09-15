# SDL3Stuff

A cross-platform C++ game built on SDL3, SDL3_image, and PhysFS. First time using SDL3, still learning C++/CMake/makefiles as I go, so expect some rough edges.

Windows is where this actually gets tested day to day. Everything else ranges from "builds fine" to "scaffolding only" — see [Platform status](#platform-status) below.

## Layout

```
src/
├── main.cpp     # entry point, window/PhysFS init, main loop
├── engine/      # generic SDL wrapper classes (Renderer, Viewport, Sprite, SDLPhysFS, platform.h)
└── game/        # actual game logic (just a demo scene right now)

Assets/     # mounted into PhysFS at runtime
deps/       # vendored third-party stuff, per platform where needed
cmake/      # vendored ios.toolchain.cmake
old/        # previous AndroidProject/Xcode.xcodeproj, kept around for reference
```

`engine/` should stay generic; anything game-specific goes in `game/`.

Build files at the root: `CMakeLists.txt` + `CMakePresets.json` (Windows/macOS/Linux/Android/iOS), the original `makefile` (Windows/macOS/Linux), and `Switch.mk`/`Vita.mk` for those two. `AndroidProject/` is just a Gradle shell around the same root `CMakeLists.txt`.

## Dependencies

- **SDL3** — window/render/input
- **SDL3_image** — PNG loading
- **PhysFS** — mounts `Assets/` as a virtual filesystem, so the game reads `/sprite.png` instead of dealing with per-platform paths

All three are zlib-licensed. Where they come from depends on the platform — package manager where possible (MSYS2 on Windows, Homebrew on macOS, apt on Linux, the official AAR on Android), vendored source otherwise. PhysFS doesn't have a package anywhere I'm targeting, so on Windows/Android it's compiled straight from the vendored source in `deps/include/PhysFS/` instead of linking a prebuilt copy; other platforms still link a prebuilt static lib.

## Building

### Windows

```
pacman -S mingw-w64-x86_64-sdl3 mingw-w64-x86_64-sdl3-image mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja
cmake --preset windows-debug
cmake --build --preset windows-debug
```

Or the old-fashioned way: `make`

Both produce the same thing. CMake's the one I actually use; the makefile still works and is kept around for now. SDL3/SDL3_image come from MSYS2; PhysFS has no MSYS2 package, so it's built straight from the vendored source into `physfs.dll` as part of the build, rather than linking a prebuilt copy.

In VS Code: install the CMake Tools extension, open the folder, pick a preset from the status bar. Should just work.

There's also a `windows-release-static`/`DYNAMIC=false` build that statically links everything (SDL3, SDL3_image, PhysFS, the C++ runtime) into a single standalone `game.exe` — no DLLs to ship alongside it.

### macOS / Linux

```
brew install sdl3 sdl3_image      # macOS
apt install libsdl3-dev           # Linux (or whatever your distro calls it)
cmake --preset macos-debug   # or linux-debug
cmake --build --preset macos-debug
```

Or the old-fashioned way: `make`

Not build-tested by me (this is a Windows machine) — the CMake/makefile logic for these is translated as carefully as I can manage, but if something's wrong, that's why.

### Generating an Xcode project

CMake's Xcode generator only runs on an actual Mac, so I can't produce this from here. The presets exist though:

```
cmake --preset macos-xcode
cmake --preset ios-device
cmake --preset ios-simulator
```

iOS specifically isn't ready — SDL3 only ships Apple binaries as a `.dmg`, which I can't open on Windows, so there's no `SDL3.xcframework` vendored yet. To finish it: grab `SDL3-<version>.dmg` from SDL's releases on an actual Mac, mount it, drop `SDL3.xcframework` into `deps/Apple/`. The vendored PhysFS lib for iOS is device-only (no simulator slice), so that'll need rebuilding too before `ios-simulator` links.

### Android

```
cd AndroidProject
.\gradlew.bat assembleDebug
```

This one's real — builds an actual APK, all four ABIs, using the same root `CMakeLists.txt` as everything else (no separate native build config to keep in sync). SDL3/SDL3_image come from their official AARs; PhysFS gets compiled from source since there's no Android package for it. `Assets/` gets zipped into the APK and mounted with `PHYSFS_mountMemory()` instead of a real path, since Android assets live inside the APK, not on disk.

Opens in Android Studio too (File → Open) — confirmed it builds, runs, and deploys to a device from there.

### Vita / Switch

```
make vita
make switch
```

Own SDKs, own build files, not touched by any of the CMake stuff above.

### PSP / 3DS

Work has not begun but with the libraries used it should theoretically be possible.

## Platform status

| Platform | Status |
|---|---|
| Windows | working, actively used |
| Android | working — builds, runs, and deploys from both Gradle CLI and Android Studio |
| macOS | should build; untested |
| Linux | should build; untested |
| Vita / Switch | build against homebrew SDKs; untested |
| iOS | scaffolded, missing SDL3.xcframework, needs a Mac to finish |
| PSP / 3DS | Theoretical, not started |

## To-do

- [x] Windows
- [x] Android
- [ ] Finish iOS (needs a Mac)
- [ ] Get SDL3_image + the engine/game split working on macOS, Linux, Vita, Switch
- [ ] PSP, 3DS
- [ ] Make something that's actually a game
