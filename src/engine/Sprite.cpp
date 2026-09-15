#include "Sprite.h"
#include "SDLPhysFS.h"

Sprite::~Sprite(){
    destroy();
}

bool Sprite::load( Renderer& renderer, const std::string& physfsPath ){
    std::string data = SDLPhysFS::readFile( physfsPath );

    if( data.empty() ){
        SDL_Log( "Sprite could not read file from PhysFS: %s\n", physfsPath.c_str() );
        return false;
    }

    SDL_IOStream* io = SDL_IOFromConstMem( data.data(), data.size() );

    if( io == nullptr ){
        SDL_Log( "Sprite could not create IOStream! SDL error: %s\n", SDL_GetError() );
        return false;
    }

    SDL_Surface* surface = IMG_Load_IO( io, true );

    if( surface == nullptr ){
        SDL_Log( "Sprite could not decode image! SDL_image error: %s\n", SDL_GetError() );
        return false;
    }

    mTexture = SDL_CreateTextureFromSurface( renderer.get(), surface );

    if( mTexture == nullptr ){
        SDL_Log( "Sprite could not create texture! SDL error: %s\n", SDL_GetError() );
        SDL_DestroySurface( surface );
        return false;
    }

    mWidth = surface->w;
    mHeight = surface->h;

    SDL_DestroySurface( surface );

    return true;
}

void Sprite::destroy(){
    if( mTexture != nullptr ){
        SDL_DestroyTexture( mTexture );
        mTexture = nullptr;
    }

    mWidth = 0;
    mHeight = 0;
}

void Sprite::draw( Renderer& renderer, float x, float y ) const{
    SDL_FRect dst{ x, y, static_cast<float>( mWidth ), static_cast<float>( mHeight ) };
    SDL_RenderTexture( renderer.get(), mTexture, nullptr, &dst );
}
