#ifndef SDLPhysFS_h
#define SDLPhysFS_h

#include "platform.h"

namespace SDLPhysFS{
    std::string readFile(const std::string& path);

    bool dummyRead();
}

#endif