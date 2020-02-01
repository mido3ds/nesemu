#pragma once

#include <SDL2/SDL.h>
#include <yaml-cpp/yaml.h>

#include "common.h"

struct Config {
    SDL_Scancode 
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
        select = SDL_SCANCODE_TAB;

    VideoSystem sys = NTSC;

    SDL_Rect windowSize = {0, 0, NTSC.resolution.width * 3, NTSC.resolution.height * 3}, 
            resolution = {0, 0, NTSC.resolution.width, NTSC.resolution.height};

    static Config fromFile(string path);
};