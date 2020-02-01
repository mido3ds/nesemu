#include "window.h"
#include "renderer.h"
#include "logger.h"

Window::Window(string title, Config const* config) {
    this->config = config;

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    window = SDL_CreateWindow(
        title.c_str(),
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        config->windowSize.w, config->windowSize.h,
        SDL_WINDOW_SHOWN
    );

    renderer = new Renderer(window, config);
}

Window::~Window() {
    if (window) {
        SDL_DestroyWindow(window);
    }

    if (renderer) {
        delete renderer;
    }

    SDL_Quit();
}

bool Window::loop(Console* dev) {
    logInfo("start executing");

    SDL_Event event;
    
    while (!quit) {
        while (SDL_PollEvent(&event) || pause) {
            if (event.type == SDL_QUIT) {
                return false;
            } 

            if (event.type == SDL_KEYDOWN) {
                const auto* state = SDL_GetKeyboardState(NULL);

                if (state[config->reset]) {
                    logInfo("reset");
                    return true;
                } else if (state[config->exit]) {
                    logInfo("exit");
                    return false;
                } else if (state[config->pause]) {
                    if (pause) logInfo("pause");
                    else logInfo("unpause");
                    
                    pause = !pause;
                    continue;
                }
            }
        }

        const auto* state = SDL_GetKeyboardState(NULL);
        dev->joypad0.up = state[config->up];
        dev->joypad0.down = state[config->down];
        dev->joypad0.left = state[config->left];
        dev->joypad0.right = state[config->right];
        dev->joypad0.a = state[config->a];
        dev->joypad0.b = state[config->b];
        dev->joypad0.start = state[config->start];
        dev->joypad0.select = state[config->select];

        dev->oneCPUCycle();
        dev->onePPUCycle(renderer);

        renderer->render();
    }
    return false;
}