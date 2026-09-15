#ifndef Viewport_h
#define Viewport_h

#include "platform.h"
#include "Renderer.h"

class Viewport{
public:
    Viewport() = default;
    Viewport( int x, int y, int w, int h );

    void set( int x, int y, int w, int h );

    void apply( const Renderer& renderer ) const;
    void reset( const Renderer& renderer ) const;

    const SDL_Rect& getRect() const{ return mRect; }

private:
    SDL_Rect mRect{ 0, 0, 0, 0 };
};

#endif
