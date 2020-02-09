#pragma once

#include <SDL2/SDL.h>

#include "sdttype.h"
#include "renderer.h"
#include "console.h"

class App {
protected:
    Console* dev = nullptr;
    SDL_Window *mainWind = nullptr, *debugWind = nullptr;
    Renderer mainRenderer, debugRenderer;
    TTF_Font* mainFont = nullptr;

    bool quit = false, pause = false, 
        debugging = true, showMem = true,
        stepping = true;

    u16_t memBeggining = 0;
    f64_t fps;

    void toggleDebugger();
    void renderMem();

    int debuggerTick();
    int mainTick();
public:
    int init(string title, Console* dev);

    int mainLoop();

    ~App();
};