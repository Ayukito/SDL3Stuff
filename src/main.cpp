#include "platform.h"
#include <SDL3/SDL_main.h>
#include "SDLPhysFS.h"
#include "Renderer.h"
#include "Viewport.h"
#include "Sprite.h"

SDL_Window* gWindow{ nullptr };

Renderer gRenderer;
Viewport gViewport;
Sprite gTestSprite;

bool exists{ false };

std::string gAssetsPath;




bool init(){
    //Initialization flag
    bool success{ true };

    //Initialize SDL
    if( SDL_Init( SDL_INIT_VIDEO ) == false ){
        SDL_Log( "SDL could not initialize! SDL error: %s\n", SDL_GetError() );
        success = false;
    }
    else{
        //Create window
        if( gWindow = SDL_CreateWindow( "Testing", 1280, 720, 0 ); gWindow == nullptr ){
            SDL_Log( "Window could not be created! SDL error: %s\n", SDL_GetError() );
            success = false;
        }
        else if( gRenderer.init( gWindow ) == false ){
            success = false;
        }
        else{
            //Centered sub-rect, smaller than the window, to make the viewport's effect visible
            gViewport.set( 240, 135, 800, 450 );
        }
    }

    return success;
}

void close(){
    gTestSprite.destroy();
    gRenderer.destroy();

    //Destroy window
    SDL_DestroyWindow( gWindow );
    gWindow = nullptr;

    if (PHYSFS_isInit()) {
        if( !gAssetsPath.empty() ){
            PHYSFS_unmount( gAssetsPath.c_str() );
        }
        PHYSFS_deinit();
    }
    //Quit SDL subsystems
    SDL_Quit();
}

void draw(){
    //Clear the screen
    if(exists){
        gRenderer.clear( 0, 0, 255 );
    }
    else{
        gRenderer.clear( 255, 0, 0 );
    }

    gViewport.apply( gRenderer );

    //Draw the test sprite centered within the viewport
    const SDL_Rect& viewportRect = gViewport.getRect();
    float x = ( viewportRect.w - gTestSprite.getWidth() ) / 2.0f;
    float y = ( viewportRect.h - gTestSprite.getHeight() ) / 2.0f;
    gTestSprite.draw( gRenderer, x, y );

    gRenderer.present();
}

int main( int argc, char *args[] ){
    //Final exit code
    int exitCode{ 0 };

    //Initialize
    if( init() == false ){
        SDL_Log( "Unable to initialize program!\n" );
        exitCode = 1;
    }
    else{
        // test
        gRenderer.clear( 255, 0, 0 );
        gRenderer.present();

        if (PHYSFS_init(NULL)){
            #ifdef __SWITCH__
                romfsInit();
                socketInitializeDefault();
                nxlinkStdio();
            #endif

            //Mount the Assets directory for the lifetime of the app
            gAssetsPath = getBasePath() + "Assets";
            if( PHYSFS_mount( gAssetsPath.c_str(), "/", 1 ) == 0 ){
                SDL_Log( "PhysFS Error: %s", PHYSFS_getErrorByCode( PHYSFS_getLastErrorCode() ) );
            }

            exists = SDLPhysFS::dummyRead();

            gTestSprite.load( gRenderer, "/sprite.png" );

            //The quit flag
            bool quit{ false };

            //The event data
            SDL_Event e;
            SDL_zero( e );

            //The main loop
            while( quit == false )
            {
                //Get event data
                while( SDL_PollEvent( &e ) == true )
                {
                    //If event is quit type
                    if( e.type == SDL_EVENT_QUIT )
                    {
                        //End the main loop
                        quit = true;
                    }
                }

                draw();

                SDL_Delay( 16 ); // ~60fps
            }
        } else{
            SDL_Log("PhysFS Error: %s", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
        }
    }

    //Clean up
    close();

    return exitCode;
}
