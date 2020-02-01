#pragma once

#include <SDL2/SDL.h>

#include "config.h"
#include "renderer.h"
#include "console.h"

class Window {
protected:
    SDL_Window* window = nullptr;
    Renderer* renderer = nullptr;
    Config const* config;

    bool quit = false, pause = false;

public:
    Window(string title, Config const* config);

    ~Window();

    bool loop(Console* dev);
};