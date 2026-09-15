cmake_minimum_required(VERSION 3.21)
project(SDL3Stuff CXX)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

option(BUILD_STATIC "Statically link the runtime and dependencies where possible (Windows only)" OFF)

# Mirrors the old makefile's flags exactly: always -O3, -g3 only in Debug,
# rather than CMake's own per-config defaults (which would add -O0/-DNDEBUG).
set(CMAKE_CXX_FLAGS_DEBUG "")
set(CMAKE_CXX_FLAGS_RELEASE "")

file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")

add_executable(game ${GAME_SOURCES})

set_target_properties(game PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

target_include_directories(game PRIVATE "${CMAKE_SOURCE_DIR}/deps/include")

target_compile_options(game PRIVATE -Wall -O3 $<$<CONFIG:Debug>:-g3>)

# Bundle Assets/ next to the built executable, same as the makefile did.
add_custom_command(TARGET game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/Assets" "$<TARGET_FILE_DIR:game>/Assets"
    COMMENT "Copying Assets/"
)

if(WIN32)
    set(WINDIR "${CMAKE_SOURCE_DIR}/deps/Windows")
    target_include_directories(game PRIVATE "${WINDIR}/include")
    target_link_directories(game PRIVATE "${WINDIR}/lib")
    target_link_libraries(game PRIVATE mingw32 SDL3 SDL3_image physfs)

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
        file(GLOB WIN_RUNTIME_DLLS "${WINDIR}/lib/*.dll")
        add_custom_command(TARGET game POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${WIN_RUNTIME_DLLS} "$<TARGET_FILE_DIR:game>"
            COMMENT "Copying runtime DLLs"
        )
    endif()

elseif(APPLE)
    # Not build-tested on this machine (Windows) — translated from the makefile's Darwin
    # branch as closely as possible. deps/MacOS currently only vendors an SDL2 framework,
    # so -framework SDL3 will need an actual SDL3 framework added before this links; see README.
    set(MACDIR "${CMAKE_SOURCE_DIR}/deps/MacOS")
    target_include_directories(game PRIVATE "${MACDIR}/include")
    target_compile_options(game PRIVATE -mmacosx-version-min=10.9)
    target_link_options(game PRIVATE
        -F${MACDIR}/Frameworks
        -Wl,-rpath,@executable_path/../Frameworks
        -mmacosx-version-min=10.9
    )
    target_link_libraries(game PRIVATE
        "-framework SDL3"
        "${MACDIR}/lib/libphysfs.a"
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
