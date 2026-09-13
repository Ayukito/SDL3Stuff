#ifndef platform_include
#define platform_include

#include <iostream>
#include <string>
#include <string.h>
using namespace std;
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
    #include <SDL.h>
    #include <physfs.h>
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

#ifndef SDL_PLATFORM_ANDROID
    #include <PhysFS/physfs.h>
    #include <SDL3/SDL.h>
#endif

inline string GetBasePath(){
    #if defined(__SWITCH__) || defined(SDL_PLATFORM_3DS)
        return "romfs:/"; // The Switch SDL doesn't seem to want to work properly, the following function just crashes the system .-.
    #elifdef SDL_PLATFORM_ANDROID
        return "";
    #endif
    return string(SDL_GetBasePath());
}
#endif