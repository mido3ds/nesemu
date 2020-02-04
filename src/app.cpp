#include <SDL2/SDL_ttf.h>
#include <unistd.h>

#include "app.h"
#include "logger.h"
#include "config.h"

int App::init(string title, Console* dev) {
    int err;

    this->dev = dev;
    if (dev == nullptr) {
        logError("dev null");
        return 1;
    }

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    TTF_Init();

    mainWind = SDL_CreateWindow(
        title.c_str(),
        Config::mainWindPos.x, Config::mainWindPos.y,
        Config::mainWind.w, Config::mainWind.h,
        SDL_WINDOW_SHOWN
    );
    if (mainWind == nullptr) {
        logError("null mainWind");
        return 1;
    }

    err = mainRenderer.init(mainWind, Config::resolution, Config::mainWind);
    if (err != 0) {
        return err;
    }

    debugWind = SDL_CreateWindow(
        "Debugger",
        Config::debugWindPos.x, Config::debugWindPos.y,
        Config::debugWind.w, Config::debugWind.h,
        SDL_WINDOW_SHOWN
    );
    if (debugWind == nullptr) {
        logError("null debugWind");
        return 1;
    }

    err = debugRenderer.init(debugWind, Config::debugWind, Config::debugWind);
    if (err != 0) {
        return err;
    }

    memWind = SDL_CreateWindow(
        "Memory",
        Config::memWindPos.x, Config::memWindPos.y,
        Config::memWind.w, Config::memWind.h,
        SDL_WINDOW_SHOWN
    );
    if (memWind == nullptr) {
        logError("null memWind");
        return 1;
    }

    err = memRenderer.init(memWind, Config::memWind, Config::memWind);
    if (err != 0) {
        return err;
    }

    mainFont = TTF_OpenFont(Config::fontPath, 13);
    if (mainFont == nullptr) {
        logError("cant load main font");
        return 1;
    }

    return 0;
}

App::~App() {
    if (mainFont) {
        TTF_CloseFont(mainFont);
    }

    if (memWind) {
        SDL_DestroyWindow(memWind);
    }

    if (debugWind) {
        SDL_DestroyWindow(memWind);
    }

    if (mainWind) {
        SDL_DestroyWindow(mainWind);
    }

    TTF_Quit();
    SDL_Quit();
}

void App::toggleDebugger() {
    if (debugging) {
        SDL_HideWindow(debugWind);
    } else {
        SDL_ShowWindow(debugWind);
    }
    debugging = !debugging;
}

void App::toggleMemWind() {
    if (showMem) {
        SDL_HideWindow(memWind);
    } else {
        SDL_ShowWindow(memWind);
    }
    showMem = !showMem;
}

int App::mainLoop() {
    int err;

    logInfo("start executing");
    int j = 0;

    while (!quit) {
        SDL_Event event; 
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                logInfo("exit");
                return 0;
            case SDL_KEYUP:
                switch (event.key.keysym.sym) {
                case Config::reset:
                    err = dev->reset();
                    if (err != 0) {
                        return err;
                    }
                    break;
                case Config::exit:
                    logInfo("exit");
                    return 0;
                case Config::pause:
                    if (pause) { logInfo("pause"); }
                    else       { logInfo("unpause"); }
                    
                    pause = !pause;
                    break;
                case Config::debug:
                    toggleDebugger();
                    break;
                case Config::showMem:
                    toggleMemWind();
                    break;
                }
            }
        }
        if (pause) { continue; }

        auto keyb = SDL_GetKeyboardState(NULL);
        dev->joypad0.up      = keyb[Config::up];
        dev->joypad0.down    = keyb[Config::down];
        dev->joypad0.left    = keyb[Config::left];
        dev->joypad0.right   = keyb[Config::right];
        dev->joypad0.a       = keyb[Config::a];
        dev->joypad0.b       = keyb[Config::b];
        dev->joypad0.start   = keyb[Config::start];
        dev->joypad0.select  = keyb[Config::select];

        err = dev->oneCPUCycle();
        if (err != 0) {
            return err;
        }

        err = dev->onePPUCycle(&mainRenderer);
        if (err != 0) {
            return err;
        }

        mainRenderer.clear({0,0,0},0);
        for (int i = 0; i < Config::resolution.w; i++) {
            mainRenderer.pixel(i, j, {255,255,255}, 255);
        }
        mainRenderer.endPixels();

        j++;
        j %= Config::mainWind.h;

        mainRenderer.text("fuck youuuu", 0, 0, 1, 1, mainFont, {255,0,0});
        mainRenderer.text("fuck you again", 100, 100, 3, 3, mainFont, {255,0,0});

        mainRenderer.show();

        debugRenderer.clear({0,0,255},0);
        for (int i = 0; i < Config::resolution.w; i++) {
            debugRenderer.pixel(i, j, {255,255,255}, 255);
        }
        debugRenderer.endPixels();

        j++;
        j %= Config::mainWind.h;

        debugRenderer.text("fuck youuuu", 0, 0, 1, 1, mainFont, {255,0,0});
        debugRenderer.text("fuck you again", 100, 100, 3, 3, mainFont, {255,0,0});

        debugRenderer.show();

        memRenderer.clear({0, 255, 0}, 255);
        memRenderer.endPixels();
        memRenderer.show();
    }

    // impossible to reach
    return 1;
}