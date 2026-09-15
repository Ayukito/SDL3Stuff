#ifndef Sprite_h
#define Sprite_h

#include "platform.h"
#include "Renderer.h"

class Sprite{
public:
    Sprite() = default;
    ~Sprite();

    bool load( Renderer& renderer, const std::string& physfsPath );
    void destroy();

    void draw( Renderer& renderer, float x, float y ) const;

    int getWidth() const{ return mWidth; }
    int getHeight() const{ return mHeight; }

private:
    SDL_Texture* mTexture{ nullptr };
    int mWidth{ 0 };
    int mHeight{ 0 };
};

#endif
