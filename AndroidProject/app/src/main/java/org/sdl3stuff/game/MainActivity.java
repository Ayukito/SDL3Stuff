package org.sdl3stuff.game;

import org.libsdl.app.SDLActivity;

public class MainActivity extends SDLActivity {
    @Override
    protected String[] getLibraries() {
        return new String[] {
            "SDL3",
            "SDL3_image",
            "game"
        };
    }
}
