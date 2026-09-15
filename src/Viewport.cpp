#include "Viewport.h"

Viewport::Viewport( int x, int y, int w, int h ){
    set( x, y, w, h );
}

void Viewport::set( int x, int y, int w, int h ){
    mRect = SDL_Rect{ x, y, w, h };
}

void Viewport::apply( const Renderer& renderer ) const{
    SDL_SetRenderViewport( renderer.get(), &mRect );
}

void Viewport::reset( const Renderer& renderer ) const{
    SDL_SetRenderViewport( renderer.get(), nullptr );
}
