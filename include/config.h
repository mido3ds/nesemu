#pragma once

#include <SDL2/SDL.h>

#include "common.h"

namespace Config {
    // read keyboard state
    constexpr SDL_Scancode 
        up = SDL_SCANCODE_UP, 
        down = SDL_SCANCODE_DOWN, 
        left = SDL_SCANCODE_LEFT, 
        right = SDL_SCANCODE_RIGHT, 
        a = SDL_SCANCODE_A, 
        b = SDL_SCANCODE_S, 
        start = SDL_SCANCODE_RETURN, 
        select = SDL_SCANCODE_TAB,

        // debugging
        scrollMemDown = SDL_SCANCODE_J,
        scrollMemUp = SDL_SCANCODE_K;
    
    constexpr SDL_Keycode
        pause = SDLK_p,
        exit = SDLK_ESCAPE, 
        reset = SDLK_r, 

        // debugging
        nextInstr = SDLK_F10,
        debug = SDLK_d, 
        showMem = SDLK_m,
        toggleStepping = SDLK_F5;

    constexpr VideoSystem sys = NTSC;

    constexpr SDL_Rect resolution{0, 0, NTSC.resolution.width, NTSC.resolution.height};
    
    constexpr SDL_Rect mainWind{0, 0, resolution.w * 3, resolution.h * 3}; 
    constexpr SDL_Rect debugWind{0, 0, 230, mainWind.h};
    constexpr SDL_Rect memWind{0, 0, mainWind.w, 100};

    constexpr char* fontPath = "zig.ttf";
    constexpr int fontSize = 13;
}