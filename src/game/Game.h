#ifndef Game_h
#define Game_h

#include "../engine/platform.h"
#include "../engine/Renderer.h"
#include "../engine/Viewport.h"
#include "../engine/Sprite.h"

class Game{
public:
    Game() = default;

    bool init( Renderer& renderer );
    void destroy();

    void draw( Renderer& renderer ) const;

private:
    Viewport mViewport;
    Sprite mTestSprite;
    bool mColorFlip{ false };
};

#endif
