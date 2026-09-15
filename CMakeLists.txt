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
    # explicit -I/-L is needed for them. PhysFS has no MSYS2 package, so it's still vendored.
    find_package(SDL3 REQUIRED CONFIG)
    find_package(SDL3_image REQUIRED CONFIG)

    set(WINDIR "${CMAKE_SOURCE_DIR}/deps/Windows")
    target_link_directories(game PRIVATE "${WINDIR}/lib")
    target_link_libraries(game PRIVATE mingw32 SDL3::SDL3 SDL3_image::SDL3_image physfs)

    target_compile_options(game PRIVATE $<$<NOT:$<CONFIG:Debug>>:-w>)
    target_link_options(game PRIVATE $<$<NOT:$<CONFIG:Debug>>:-Wl,-subsystem,windows>)

    if(BUILD_STATIC)
        # No static PhysFS import lib is vendored for Windows (only libphysfs.dll),
        # so this mirrors the makefile's DYNAMIC=false path as-is, including that limitation.
        target_link_options(game PRIVATE
            -static -lkernel32 -ladvapi32 -lgdi32 -limm32 -lmsvcrt
            -lole32 -loleaut32 -lsetupapi -lshell32 -luser32 -lversion -lwinmm
        )
    else()
        # PhysFS's DLL is still vendored; SDL3/SDL3_image's now come straight from the
        # MSYS2 install via their imported CMake targets' resolved DLL locations.
        file(GLOB WIN_RUNTIME_DLLS "${WINDIR}/lib/*.dll")
        add_custom_command(TARGET game POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${WIN_RUNTIME_DLLS}
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
