cmake_minimum_required(VERSION 3.21)
project(SDL3Stuff C CXX)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

option(BUILD_STATIC "Statically link the runtime and dependencies where possible (Windows only)" OFF)

# Mirrors the old makefile's flags exactly: always -O3, -g3 only in Debug,
# rather than CMake's own per-config defaults (which would add -O0/-DNDEBUG).
set(CMAKE_CXX_FLAGS_DEBUG "")
set(CMAKE_CXX_FLAGS_RELEASE "")

file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")

if(ANDROID)
    # Android needs a shared library (loaded via System.loadLibrary() from Java/SDLActivity),
    # not an executable. SDL3's SDL_main.h (already included by src/main.cpp via platform.h)
    # handles redirecting our plain int main() into the right entry point on Android too.
    add_library(game SHARED ${GAME_SOURCES})
else()
    add_executable(game ${GAME_SOURCES})
    set_target_properties(game PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
endif()

target_include_directories(game PRIVATE "${CMAKE_SOURCE_DIR}/deps/include")

target_compile_options(game PRIVATE -Wall -O3 $<$<CONFIG:Debug>:-g3>)

if(NOT ANDROID)
    # Bundle Assets/ next to the built executable, same as the makefile did. Android instead
    # packages Assets/ into the APK via AndroidProject/app/build.gradle's assets sourceSet.
    add_custom_command(TARGET game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_SOURCE_DIR}/Assets" "$<TARGET_FILE_DIR:game>/Assets"
        COMMENT "Copying Assets/"
    )
endif()

if(ANDROID)
    # SDL3 core is a real, verified dependency here (unlike Apple): the official
    # SDL3-devel-android.zip release ships a Prefab-enabled AAR with prebuilt
    # armeabi-v7a/arm64-v8a/x86/x86_64 shared libs + bundled Java glue (SDLActivity etc.
    # come from the AAR's classes.jar — no manual .java copying needed). Gradle's Prefab
    # integration (buildFeatures.prefab true in app/build.gradle) exposes it to CMake as an
    # ordinary package.
    find_package(SDL3 REQUIRED CONFIG)
    find_package(SDL3_image REQUIRED CONFIG)

    # PhysFS has no official Android binary release, so it's compiled here from the vendored
    # source in deps/include/PhysFS (which already carries physfs_platform_android.c). This
    # mirrors upstream PhysFS's own CMakeLists.txt: every physfs_platform_*.c/archiver_*.c
    # file is compiled unconditionally on every platform, gated internally by #ifdefs, so a
    # plain glob of deps/include/PhysFS/*.c is exactly upstream's PHYSFS_SRCS list (confirmed
    # against PhysFS 3.2.0's CMakeLists.txt) — this excludes physfs_platform_apple.m and
    # physfs_platform_haiku.cpp/winrt.cpp automatically, since those aren't .c files.
    file(GLOB PHYSFS_ANDROID_SRCS "${CMAKE_SOURCE_DIR}/deps/include/PhysFS/*.c")
    add_library(physfs STATIC ${PHYSFS_ANDROID_SRCS})
    target_include_directories(physfs PRIVATE "${CMAKE_SOURCE_DIR}/deps/include/PhysFS")

    # KNOWN GAP: PHYSFS_mount() in main.cpp mounts a real filesystem directory
    # (getBasePath() + "Assets"), but on Android, Assets/ lives inside the APK's zip and is
    # only reachable via AAssetManager, not a plain path — this will need Android-specific
    # handling (e.g. extracting Assets/ to SDL_GetAndroidInternalStoragePath() on first
    # launch) before asset loading actually works on-device. Not implemented yet.

    target_link_libraries(game PRIVATE
        SDL3::SDL3
        SDL3_image::SDL3_image
        physfs
        log
    )

elseif(WIN32)
    # SDL3/SDL3_image come from MSYS2's mingw-w64 packages (`pacman -S mingw-w64-x86_64-sdl3
    # mingw-w64-x86_64-sdl3-image`) instead of vendored deps/Windows headers/libs — both ship
    # a proper CMake config (confirmed: SDL3Config.cmake/SDL3_imageConfig.cmake under
    # mingw64/lib/cmake/), so this is the same find_package() pattern as the macOS/Homebrew
    # branch. MinGW64's g++ already searches its own include/lib dirs by default, so no
    # explicit -I/-L is needed for them.
    find_package(SDL3 REQUIRED CONFIG)
    find_package(SDL3_image REQUIRED CONFIG)

    # PhysFS has no MSYS2 package, and rather than keep a prebuilt libphysfs.dll vendored in
    # deps/Windows/lib, it's built directly from the vendored source in deps/include/PhysFS —
    # the same source Android already compiles from (see the ANDROID branch above). SHARED by
    # default to match SDL3/SDL3_image's dynamic linking here; BUILD_STATIC switches it to a
    # static lib instead, which finally makes a real fully-static Windows build possible (this
    # used to be a dead end, since only a prebuilt .dll existed and no static import lib).
    file(GLOB PHYSFS_WIN_SRCS "${CMAKE_SOURCE_DIR}/deps/include/PhysFS/*.c")
    if(BUILD_STATIC)
        add_library(physfs STATIC ${PHYSFS_WIN_SRCS})
    else()
        add_library(physfs SHARED ${PHYSFS_WIN_SRCS})
        # physfs.h's PHYSFS_DECL only requests __declspec(dllexport) for MSVC (_MSC_VER); for
        # GCC/MinGW it falls back to __attribute__((visibility("default"))), which doesn't
        # populate a Windows export table on its own. --export-all-symbols makes MinGW's
        # linker export every global symbol instead, the same way it would by default anyway
        # for a DLL with no explicit exports — made explicit here rather than relied upon.
        target_link_options(physfs PRIVATE -Wl,--export-all-symbols)
        set_target_properties(physfs PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
    endif()
    target_include_directories(physfs PRIVATE "${CMAKE_SOURCE_DIR}/deps/include/PhysFS")

    # libgcc_s_seh-1.dll/libstdc++-6.dll/libwinpthread-1.dll ship with the MinGW-w64 toolchain
    # itself (not something we build) — pulled straight from wherever this build's compiler
    # lives, rather than vendoring a copy that could drift out of sync with the actual
    # compiler version in use.
    get_filename_component(MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)

    if(BUILD_STATIC)
        # SDL3::SDL3 / SDL3_image::SDL3_image alias to the *shared* target by default (see
        # SDL3Config.cmake) — link the -static variants explicitly, or -static below would
        # just add system-lib flags on top of a still-dynamically-linked SDL3.
        target_link_libraries(game PRIVATE mingw32 SDL3::SDL3-static SDL3_image::SDL3_image-static physfs)
    else()
        target_link_libraries(game PRIVATE mingw32 SDL3::SDL3 SDL3_image::SDL3_image physfs)
    endif()

    target_compile_options(game PRIVATE $<$<NOT:$<CONFIG:Debug>>:-w>)
    target_link_options(game PRIVATE $<$<NOT:$<CONFIG:Debug>>:-Wl,-subsystem,windows>)

    if(BUILD_STATIC)
        # -luuid: the static SDL3 references some COM interface GUIDs (IID_ITaskbarList3,
        # IID_IShellItem, GUID_NULL) that only libuuid provides — without it this fails to
        # link. Confirmed via the equivalent makefile DYNAMIC=false path hitting the same
        # missing-symbol error first.
        target_link_options(game PRIVATE
            -static -lkernel32 -ladvapi32 -lgdi32 -limm32 -lmsvcrt
            -lole32 -loleaut32 -lsetupapi -lshell32 -luser32 -lversion -lwinmm -luuid
        )
    else()
        # physfs.dll is built straight into bin/ alongside game.exe (see its
        # RUNTIME_OUTPUT_DIRECTORY above); SDL3/SDL3_image are resolved via their imported
        # CMake targets; the MinGW runtime DLLs come from the compiler's own directory (see
        # MINGW_BIN_DIR above) — nothing here is vendored in the repo.
        add_custom_command(TARGET game POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${MINGW_BIN_DIR}/libgcc_s_seh-1.dll"
                    "${MINGW_BIN_DIR}/libstdc++-6.dll"
                    "${MINGW_BIN_DIR}/libwinpthread-1.dll"
                    "$<TARGET_FILE:SDL3::SDL3>"
                    "$<TARGET_FILE:SDL3_image::SDL3_image>"
                    "$<TARGET_FILE_DIR:game>"
            COMMENT "Copying runtime DLLs"
        )
    endif()

elseif(IOS)
    # SCAFFOLD ONLY, not build-tested — this repo is developed on Windows, and SDL3's only
    # official Apple binary release is a .dmg (verified via the GitHub releases API: no
    # plain zip/tar.gz for Apple platforms), which can't be read/mounted here. To finish
    # this on an actual Mac:
    #   1. Download SDL3-<version>.dmg from https://github.com/libsdl-org/SDL/releases,
    #      mount it, and copy SDL3.xcframework to deps/Apple/SDL3.xcframework (same for
    #      SDL3_image once you've built/obtained an SDL3_image.xcframework).
    #   2. Configure with the ios-device or ios-simulator CMake preset (uses the Xcode
    #      generator + cmake/ios.toolchain.cmake, vendored from
    #      https://github.com/leetal/ios-cmake).
    # deps/iOS/lib/libphysfs.a is reused as-is: it's a fat arm64+armv7 binary (device-only,
    # no simulator slice — verified via its Mach-O fat header), so simulator builds will
    # need a new PhysFS build with a simulator slice before they'll link.
    set(APPLE_DEPS_DIR "${CMAKE_SOURCE_DIR}/deps/Apple")

    set_target_properties(game PROPERTIES
        MACOSX_BUNDLE TRUE
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.sdl3stuff.game"
    )

    target_link_libraries(game PRIVATE
        "${APPLE_DEPS_DIR}/SDL3.xcframework"
        "${APPLE_DEPS_DIR}/SDL3_image.xcframework"
        "${CMAKE_SOURCE_DIR}/deps/iOS/lib/libphysfs.a"
        "-framework UIKit"
        "-framework CoreFoundation"
        "-framework IOKit"
        "-framework Foundation"
    )

elseif(APPLE)
    # macOS. Not build-tested on this machine (Windows).
    #
    # SDL3 is resolved via Homebrew's CMake package config (`brew install sdl3 sdl3_image`,
    # both ship SDL3Config.cmake/SDL3_imageConfig.cmake) rather than the makefile's
    # `-framework SDL3` search against deps/MacOS/Frameworks: that folder only ever vendored
    # an unrelated SDL2.framework, so the makefile's flag would not actually have resolved an
    # SDL3 framework there — this find_package() approach is the correct fix, not a straight
    # translation of that line. deps/MacOS/lib/libphysfs.a is reused unchanged: it's already
    # a universal arm64+x86_64 fat binary (verified via its Mach-O fat header), so it covers
    # both Intel and Apple Silicon without changes.
    #
    # CMAKE_OSX_ARCHITECTURES / CMAKE_OSX_DEPLOYMENT_TARGET are set via the macos-* CMake
    # presets (must be set before `project()` to take effect, so they can't live here).
    find_package(SDL3 REQUIRED CONFIG)
    find_package(SDL3_image REQUIRED CONFIG)

    target_link_libraries(game PRIVATE
        SDL3::SDL3
        SDL3_image::SDL3_image
        "${CMAKE_SOURCE_DIR}/deps/MacOS/lib/libphysfs.a"
        "-framework CoreFoundation"
        "-framework IOKit"
        "-framework Foundation"
    )

elseif(UNIX)
    # Not build-tested on this machine (Windows) — translated from the makefile's Linux
    # branch, which expects SDL3/PhysFS installed as static libs at these fixed paths
    # (matches libsdl3-dev / libphysfs-dev per README).
    target_link_libraries(game PRIVATE
        /usr/local/lib/libSDL3.a
        /usr/local/lib/libphysfs.a
        dl
        pthread
    )
    target_link_options(game PRIVATE -no-pie)
endif()
