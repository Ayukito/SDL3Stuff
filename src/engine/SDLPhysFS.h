#ifndef SDLPhysFS_h
#define SDLPhysFS_h

#include "platform.h"

namespace SDLPhysFS{
    std::string readFile(const std::string& path);

    bool dummyRead();

    // Mounts the game's Assets/ for reading via PHYSFS. On desktop/console platforms this
    // mounts assetsPath as a real directory. On Android, Assets/ can't be mounted as a real
    // path — it's packaged into the APK, only reachable through AAssetManager — so instead
    // this reads Assets.zip (see AndroidProject/app/build.gradle, which zips Assets/ into the
    // APK at build time) via SDL_LoadFile(), which transparently reads through AAssetManager
    // on Android, and mounts it straight from memory with PHYSFS_mountMemory().
    // Returns the identifier to later pass to PHYSFS_unmount() (matches assetsPath on every
    // platform except Android), or an empty string on failure.
    std::string mountAssets(const std::string& assetsPath);
}

#endif