#ifndef SDLPhysFS_h
#define SDLPhysFS_h

#include "platform_include.h"

namespace SDLPhysFS{
    string PHYSFS_readFile( string path);

    char* file_read(const char* filename);

    bool PhysFS_DummyRead();
}

#endif