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
        pause = SDL_SCANCODE_P, 
        exit = SDL_SCANCODE_ESCAPE, 
        reset = SDL_SCANCODE_BACKSPACE, 
        start = SDL_SCANCODE_RETURN, 
        select = SDL_SCANCODE_TAB,
        debug = SDL_SCANCODE_D, 
        showMem = SDL_SCANCODE_M;

    constexpr VideoSystem sys = NTSC;

    constexpr SDL_Rect resolution{0, 0, NTSC.resolution.width, NTSC.resolution.height};
    constexpr SDL_Rect totalWindSize{0, 0, resolution.w * 3, resolution.h * 3};

    constexpr char* fontPath = "zig.ttf";
}