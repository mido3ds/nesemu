#include "renderer.h"

Renderer::Renderer(SDL_Window* window, Config const* config) {
    this->config = config;

    renderer = SDL_CreateRenderer(
        window, 
        -1, 
            SDL_RENDERER_ACCELERATED 
        | SDL_RENDERER_PRESENTVSYNC 
        | SDL_RENDERER_TARGETTEXTURE
    );

    backBuffer = SDL_CreateTexture(
        renderer,
        SDL_GetWindowPixelFormat(window), 
        SDL_TEXTUREACCESS_TARGET,
        config->windowSize.w, 
        config->windowSize.h
    );

    SDL_SetRenderTarget(renderer, backBuffer);
}

Renderer::~Renderer() {
    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }

    if (backBuffer) {
        SDL_DestroyTexture(backBuffer);
    }
}

void Renderer::clear(Color c, uint8_t a) {
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, a);
    SDL_RenderClear(renderer);
}

void Renderer::pixel(int x, int y, Color c, uint8_t a) {
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, a);
    SDL_RenderDrawPoint(renderer, x, y);
}

void Renderer::render() {
    // clear window
    SDL_SetRenderTarget(renderer, NULL);
    clear();

    // render backBuffer onto screen at (0,0) correclty sized
    SDL_RenderCopy(renderer, backBuffer, &config->resolution, &config->windowSize);     
    SDL_RenderPresent(renderer);

    // clear backbuffer
    SDL_SetRenderTarget(renderer, backBuffer);
    clear();
}