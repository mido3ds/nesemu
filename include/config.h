#pragma once

#include <SDL2/SDL.h>

#include "common.h"

namespace Config {
    constexpr SDL_Scancode 
        up = SDL_SCANCODE_UP, 
        down = SDL_SCANCODE_DOWN, 
        left = SDL_SCANCODE_LEFT, 
        right = SDL_SCANCODE_RIGHT, 
        a = SDL_SCANCODE_A, 
        b = SDL_SCANCODE_S, 
        start = SDL_SCANCODE_RETURN, 
        select = SDL_SCANCODE_TAB;
    
    constexpr SDL_Keycode
        pause = SDLK_p,
        exit = SDLK_ESCAPE, 
        reset = SDLK_r, 
        debug = SDLK_d, 
        showMem = SDLK_m;

    constexpr VideoSystem sys = NTSC;

    constexpr SDL_Rect resolution{0, 0, NTSC.resolution.width, NTSC.resolution.height};
    
    constexpr SDL_Rect mainWind{0, 0, resolution.w * 3, resolution.h * 3}, mainWindPos{324, 182};
    constexpr SDL_Rect debugWind{0, 0, 230, mainWind.h}, debugWindPos{1100, 180};
    constexpr SDL_Rect memWind{0, 0, mainWind.w, 100}, memWindPos{327, 44};

    constexpr char* fontPath = "zig.ttf";
}