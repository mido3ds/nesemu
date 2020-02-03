#pragma once

#include <SDL2/SDL.h>

#include "renderer.h"
#include "console.h"

class App {
protected:
    Console* dev;
    SDL_Window* window = nullptr;
    Renderer renderer;
    TTF_Font* mainFont;

    bool quit = false, pause = false;

public:
    int init(string title, Console* dev);

    int mainLoop();

    ~App();
};