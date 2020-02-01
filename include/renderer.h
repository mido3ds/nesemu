#pragma once

#include <SDL2/SDL.h>

#include "config.h"

class Renderer {
private:
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* backBuffer = nullptr;
    Config const* config;

public:
    Renderer(SDL_Window* window, Config const* config);

    ~Renderer();

    void clear(Color c = {0, 0, 0}, uint8_t a = 0);

    void pixel(int x, int y, Color c, uint8_t a = 255);

    void render();
};