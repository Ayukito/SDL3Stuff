#ifndef Renderer_h
#define Renderer_h

#include "platform.h"

class Renderer{
public:
    Renderer() = default;
    ~Renderer();

    bool init( SDL_Window* window );
    void destroy();

    void clear( Uint8 r = 0, Uint8 g = 0, Uint8 b = 0, Uint8 a = 255 );
    void present();

    SDL_Renderer* get() const{ return mRenderer; }

private:
    SDL_Renderer* mRenderer{ nullptr };
};

#endif
