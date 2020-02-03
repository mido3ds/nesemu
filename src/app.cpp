#include <SDL2/SDL_ttf.h>
#include <unistd.h>

#include "app.h"
#include "renderer.h"
#include "logger.h"
#include "config.h"

int App::init(string title, Console* dev) {
    this->dev = dev;
    if (dev == nullptr) {
        logError("dev null");
        return 1;
    }

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    TTF_Init();

    window = SDL_CreateWindow(
        title.c_str(),
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        Config::totalWindSize.w, Config::totalWindSize.h,
        SDL_WINDOW_SHOWN
    );
    if (window == nullptr) {
        logError("null window");
        return 1;
    }

    int err = renderer.init(window, Config::resolution, Config::totalWindSize);
    if (err != 0) {
        return err;
    }

    mainFont = TTF_OpenFont(Config::fontPath, 13);
    if (!mainFont) {
        logError("cant load main font");
        return 1;
    }

    return 0;
}

App::~App() {
    if (window) {
        SDL_DestroyWindow(window);
    }

    TTF_Quit();
    SDL_Quit();
}

int App::mainLoop() {
    int err;

    logInfo("start executing");
    int j = 0;

    while (!quit) {
        SDL_Event event; 
        while (SDL_PollEvent(&event) || pause) {
            if (event.type == SDL_QUIT) {
                return 0;
            } else if (event.type == SDL_KEYDOWN) {
                auto keyb = SDL_GetKeyboardState(NULL);

                if (keyb[Config::reset]) {
                    err = dev->reset();
                    if (err != 0) {
                        return err;
                    }
                } else if (keyb[Config::exit]) {
                    logInfo("exit");
                    return 0;
                } else if (keyb[Config::pause]) {
                    if (pause) { logInfo("pause"); }
                    else       { logInfo("unpause"); }
                    
                    pause = !pause;
                    continue;
                }
            }
        }

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

        err = dev->onePPUCycle(&renderer);
        if (err != 0) {
            return err;
        }

        renderer.clear({0,0,0},0);
        for (int i = 0; i < Config::resolution.w; i++) {
            renderer.pixel(i, j, {255,255,255}, 255);
        }
        renderer.endPixels();

        j++;
        j %= Config::totalWindSize.h;

        renderer.text("fuck youuuu", 0, 0, 1, 1, mainFont, {255,0,0});
        renderer.text("fuck you again", 100, 100, 3, 3, mainFont, {255,0,0});

        renderer.show();
    }

    // impossible to reach
    return 1;
}