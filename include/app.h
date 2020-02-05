#pragma once

#include <SDL2/SDL.h>

#include "renderer.h"
#include "console.h"

class App {
protected:
    Console* dev = nullptr;
    SDL_Window *mainWind = nullptr, *debugWind = nullptr, *memWind = nullptr;
    Renderer mainRenderer, debugRenderer, memRenderer;
    TTF_Font* mainFont = nullptr;

    bool quit = false, pause = false, 
        debugging = true, showMem = true,
        stepping = true;
    double fps;

    void toggleDebugger();
    void toggleMemWind();

    int debuggerTick();
    int memTick();
    int mainTick();
public:
    int init(string title, Console* dev);

    int mainLoop();

    ~App();
};