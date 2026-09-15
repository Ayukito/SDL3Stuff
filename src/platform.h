#ifndef platform_h
#define platform_h

#include <string>

#ifdef _WIN64
    //define something for Windows (64-bit)
#elifdef _WIN32
    //define something for Windows (32-bit
#elifdef SDL_PLATFORM_APPLE
    #include "TargetConditionals.h"
    #if TARGET_OS_IPHONE
        // iOS
    #elif TARGET_OS_MAC
        // MacOS
    #endif
#elifdef SDL_PLATFORM_ANDROID
    // Android
#elifdef SDL_PLATFORM_LINUX
    // linux
#elifdef __SWITCH__
    #include <switch.h>
#elifdef SDL_PLATFORM_VITA
    // vita
#elifdef SDL_PLATFORM_PSP
    // psp
#elifdef SDL_PLATFORM_3DS
    // 3ds
#endif

#include <PhysFS/physfs.h>
#include <SDL3/SDL.h>
// was an indef android, but we'll see if that's still needed

inline std::string getBasePath(){
    #if defined(__SWITCH__) || defined(SDL_PLATFORM_3DS)
        return "romfs:/"; // The Switch SDL doesn't seem to want to work properly, the following function just crashes the system .-.
    #elifdef SDL_PLATFORM_ANDROID
        return "";
    #endif

    const char* base = SDL_GetBasePath();

    return base ? std::string(base) : std::string();
}

#endif