#include "Renderer.h"

Renderer::~Renderer(){
    destroy();
}

bool Renderer::init( SDL_Window* window ){
    mRenderer = SDL_CreateRenderer( window, nullptr );

    if( mRenderer == nullptr ){
        SDL_Log( "Renderer could not be created! SDL error: %s\n", SDL_GetError() );
        return false;
    }

    return true;
}

void Renderer::destroy(){
    if( mRenderer != nullptr ){
        SDL_DestroyRenderer( mRenderer );
        mRenderer = nullptr;
    }
}

void Renderer::clear( Uint8 r, Uint8 g, Uint8 b, Uint8 a ){
    SDL_SetRenderDrawColor( mRenderer, r, g, b, a );
    SDL_RenderClear( mRenderer );
}

void Renderer::present(){
    SDL_RenderPresent( mRenderer );
}
