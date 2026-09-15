#include "engine/platform.h"
#include <SDL3/SDL_main.h>
#include "engine/Renderer.h"
#include "game/Game.h"

SDL_Window* gWindow{ nullptr };

Renderer gRenderer;
Game gGame;

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
    }

    return success;
}

void close(){
    gGame.destroy();
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

            gGame.init( gRenderer );

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

                gGame.draw( gRenderer );

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
