#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "sdttype.h"
#include "common.h"

class Renderer {
private:
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* backBuffer = nullptr;
    SDL_Rect resolution, windSize;

    bool endedPixels = true;

public:
    int init(SDL_Window* window, SDL_Rect resolution, SDL_Rect windSize);

    void allPixels(Color c, u8_t a);
    void pixel(int x, int y, Color c, u8_t a);

    // copy pixels buffer, must be called before any Renderer#text or Renderer#show
    void endPixels();

    int text(string s, int x, int y, f64_t scaleW, f64_t scaleH, TTF_Font* font, Color c, int* newW, int* newH);

    // present the renderer on its window
    void show();

    ~Renderer();
};