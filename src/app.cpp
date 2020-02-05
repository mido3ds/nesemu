#include <chrono>
#include <sstream>
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
        SDL_WINDOW_HIDDEN
    );
    if (debugWind == nullptr) {
        logError("null debugWind");
        return 1;
    }
    if (debugging) { SDL_ShowWindow(debugWind); }

    err = debugRenderer.init(debugWind, Config::debugWind, Config::debugWind);
    if (err != 0) {
        return err;
    }

    memWind = SDL_CreateWindow(
        "Memory",
        Config::memWindPos.x, Config::memWindPos.y,
        Config::memWind.w, Config::memWind.h,
        SDL_WINDOW_HIDDEN
    );
    if (memWind == nullptr) {
        logError("null memWind");
        return 1;
    }
    if (showMem) { SDL_ShowWindow(memWind); }

    err = memRenderer.init(memWind, Config::memWind, Config::memWind);
    if (err != 0) {
        return err;
    }

    mainFont = TTF_OpenFont(Config::fontPath, Config::fontSize);
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

static string hex8(uint8_t v) {
    char buffer[10] = {0};
    sprintf(buffer, "%02X", v);
    return buffer;
}

static string hex16(uint16_t v) {
    char buffer[10] = {0};
    sprintf(buffer, "%04X", v);
    return buffer;
}

int App::debuggerTick() {
    if (!debugging) { return 0; }

    debugRenderer.allPixels({0,0,0},0);
    debugRenderer.endPixels();

    const int h = Config::fontSize+1, 
        w = Config::debugWind.w/2;
    int i = 0;

    // fps
    debugRenderer.text("FPS: "+to_string(int(fps)), 10,(i++)*h,1,1, mainFont,{255,0,0}, 0, 0);
    i++;

    // regs
    Color c{255,255,0};
    debugRenderer.text("SP: $" + hex8(dev->regs.sp), 10,(i)*h,1,1, mainFont,c, 0, 0);
    debugRenderer.text("A: $" + hex8(dev->regs.a), 10+w,(i++)*h,1,1, mainFont,c, 0, 0);

    debugRenderer.text("X: $" + hex8(dev->regs.x), 10,(i)*h,1,1, mainFont,c, 0, 0);
    debugRenderer.text("Y: $" + hex8(dev->regs.y), 10+w,(i++)*h,1,1, mainFont,c, 0, 0);
    i++;

    debugRenderer.text("C: " + to_string(dev->regs.flags.bits.c), 10,(i)*h,1,1, mainFont,c, 0, 0);
    debugRenderer.text("Z: " + to_string(dev->regs.flags.bits.z), 10+w*2.0/3,(i)*h,1,1, mainFont,c, 0, 0);
    debugRenderer.text("I: " + to_string(dev->regs.flags.bits.i), 10+w*4.0/3,(i++)*h,1,1, mainFont,c, 0, 0);

    debugRenderer.text("D: " + to_string(dev->regs.flags.bits.d), 10,(i)*h,1,1, mainFont,c, 0, 0);
    debugRenderer.text("B: " + to_string(dev->regs.flags.bits.b), 10+w*2.0/3,(i)*h,1,1, mainFont,c, 0, 0);
    debugRenderer.text("V: " + to_string(dev->regs.flags.bits.v), 10+w*4.0/3,(i++)*h,1,1, mainFont,c, 0, 0);

    debugRenderer.text("N: " + to_string(dev->regs.flags.bits.n), 10,(i++)*h,1,1, mainFont,c, 0, 0);
    i++;
    
    // assembly
    const int n = 18;
    int j = 1;
    for (auto& s: dev->getAssembly(dev->regs.pc, n)) {
        c = {r:255, g:255, b:255};
        if (j++ == n+1) {
            c.g = c.b = 0;
        }

        debugRenderer.text(s, 10,(i++)*h,1,1, mainFont,c, 0, 0);
    }

    debugRenderer.show();
    
    return 0;
}

int App::memTick() {
    if (!showMem) { return 0; }

    return 0;
}

int App::mainTick() {
    int err;
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            quit = true;
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
                quit = true;
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
            case Config::toggleStepping:
                stepping = !stepping;
                break;
            }
        }
    }
    if (pause) { return 0; }

    auto keyb = SDL_GetKeyboardState(NULL);
    dev->joypad0.up      = keyb[Config::up];
    dev->joypad0.down    = keyb[Config::down];
    dev->joypad0.left    = keyb[Config::left];
    dev->joypad0.right   = keyb[Config::right];
    dev->joypad0.a       = keyb[Config::a];
    dev->joypad0.b       = keyb[Config::b];
    dev->joypad0.start   = keyb[Config::start];
    dev->joypad0.select  = keyb[Config::select];

    if (!stepping || keyb[Config::nextInstr]) {
        err = dev->oneCPUCycle();
        if (err != 0) {
            return err;
        }

        err = dev->onePPUCycle(&mainRenderer);
        if (err != 0) {
            return err;
        }
    }

    return 0;
}

int App::mainLoop() {
    using namespace chrono;

    logInfo("start executing");
    int err;

    while (!quit) {
        auto t1 = high_resolution_clock::now();

        if ((err = mainTick()) != 0) {
            return err;
        }

        if ((err = debuggerTick()) != 0) {
            return err;
        }

        if ((err = memTick()) != 0) {
            return err;
        }
        
        usleep(1); // guard window from overdrawing

        auto t2 = high_resolution_clock::now();
        auto duration = duration_cast<chrono::milliseconds>(t2 - t1).count();

        fps = 1000.0/duration;
    }

    return 0;
}