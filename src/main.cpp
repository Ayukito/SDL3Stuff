#include "platform.h"
#include <sdl3/SDL_main.h>
#include "SDLPhysFS.h"

bool init();

void close();

int main( int argc, char *args[] );

void draw();

SDL_Window* gWindow{ nullptr };

SDL_Surface* gScreenSurface{ nullptr };

bool exists{ false };





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
        else{
            //Get window surface
            gScreenSurface = SDL_GetWindowSurface( gWindow );
        }
    }

    return success;
}

void close(){
    //Clean up surface
    gScreenSurface = nullptr;
    
    //Destroy window
    SDL_DestroyWindow( gWindow );
    gWindow = nullptr;
    if (PHYSFS_isInit()) {
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
        SDL_FillSurfaceRect( gScreenSurface, nullptr, SDL_MapSurfaceRGB( gScreenSurface, 255, 0, 0 ) );
        SDL_UpdateWindowSurface( gWindow );

        if (PHYSFS_init(NULL)){
            #ifdef __SWITCH__
                romfsInit();
                socketInitializeDefault();
                nxlinkStdio();
            #endif

            exists = SDLPhysFS::dummyRead();

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

void draw(){
    //Fill the surface
    if(exists){
        SDL_FillSurfaceRect( gScreenSurface, nullptr, SDL_MapSurfaceRGB( gScreenSurface, 0, 0, 255 ) );
    }
    else{
        SDL_FillSurfaceRect( gScreenSurface, nullptr, SDL_MapSurfaceRGB( gScreenSurface, 255, 0, 0 ) );
    }
    
    //Update the surface
    SDL_UpdateWindowSurface( gWindow );
}