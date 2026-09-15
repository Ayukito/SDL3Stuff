#include "engine/platform.h"
#include <SDL3/SDL_main.h>
#include "engine/Renderer.h"
#include "engine/SDLPhysFS.h"
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

        #ifdef SDL_PLATFORM_ANDROID
            // On Android, PHYSFS_init() doesn't take argv0 — it expects a pointer to a valid
            // PHYSFS_AndroidInit (JNIEnv* + Context jobject, both as void*), which it uses to
            // query the app's data/cache directories. Must be called after SDL_Init(), since
            // that's what sets up SDL's JNI env/activity references on Android.
            PHYSFS_AndroidInit androidInit{ SDL_GetAndroidJNIEnv(), SDL_GetAndroidActivity() };
            const char* physfsArgv0 = reinterpret_cast<const char*>( &androidInit );
        #else
            const char* physfsArgv0 = args[0];
        #endif

        if (PHYSFS_init(physfsArgv0)){
            #ifdef __SWITCH__
                romfsInit();
                socketInitializeDefault();
                nxlinkStdio();
            #endif

            //Mount the Assets directory for the lifetime of the app
            gAssetsPath = SDLPhysFS::mountAssets( getBasePath() + "Assets" );

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
