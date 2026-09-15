#include "Game.h"
#include "../engine/SDLPhysFS.h"

bool Game::init( Renderer& renderer ){
    //Centered sub-rect, smaller than the window, to make the viewport's effect visible
    mViewport.set( 240, 135, 800, 450 );

    mColorFlip = SDLPhysFS::dummyRead();

    return mTestSprite.load( renderer, "/sprite.png" );
}

void Game::destroy(){
    mTestSprite.destroy();
}

void Game::draw( Renderer& renderer ) const{
    //Clear the screen
    if( mColorFlip ){
        renderer.clear( 0, 0, 255 );
    }
    else{
        renderer.clear( 255, 0, 0 );
    }

    mViewport.apply( renderer );

    //Draw the test sprite centered within the viewport
    const SDL_Rect& viewportRect = mViewport.getRect();
    float x = ( viewportRect.w - mTestSprite.getWidth() ) / 2.0f;
    float y = ( viewportRect.h - mTestSprite.getHeight() ) / 2.0f;
    mTestSprite.draw( renderer, x, y );

    renderer.present();
}
