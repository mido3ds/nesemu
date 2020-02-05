#include "renderer.h"
#include "logger.h"
#include "config.h"

int Renderer::init(SDL_Window* window, SDL_Rect resolution, SDL_Rect windSize) {
    if (resolution.w <= 0 || resolution.h <= 0 || 
        windSize.w <= 0 || windSize.h <= 0) {
        logError("invalid resolution or windsize");
        return 1;
    }

    this->resolution = resolution;
    this->windSize = windSize;

    renderer = SDL_CreateRenderer(
        window, 
        -1, 
            SDL_RENDERER_ACCELERATED 
        | SDL_RENDERER_PRESENTVSYNC 
        | SDL_RENDERER_TARGETTEXTURE
    );
    if (renderer == nullptr) {
        logError("null renderer");
        return 1;
    }

    backBuffer = SDL_CreateTexture(
        renderer,
        SDL_GetWindowPixelFormat(window), 
        SDL_TEXTUREACCESS_TARGET,
        windSize.w, 
        windSize.h
    );
    if (backBuffer == nullptr) {
        logError("null backbuffer");
        return 1;
    }

    return 0;
}

Renderer::~Renderer() {
    if (backBuffer) {
        SDL_DestroyTexture(backBuffer);
    }

    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }
}

void Renderer::pixel(int x, int y, Color c, uint8_t a) {
    SDL_SetRenderTarget(renderer, backBuffer);
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, a);
    SDL_RenderDrawPoint(renderer, x, y);

    endedPixels = false;
}

void Renderer::allPixels(Color c, uint8_t a) {
    SDL_SetRenderTarget(renderer, backBuffer);
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, a);
    SDL_RenderClear(renderer);

    endedPixels = true;
}

void Renderer::endPixels() {
    // copy backBuffer to renderer, resized from (resolution) to (windSize)
    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderCopy(renderer, backBuffer, &resolution, &windSize);

    endedPixels = true;
}

int Renderer::text(string s, int x, int y, double scaleW, double scaleH, TTF_Font* font, Color c) {
    if (!endedPixels) {
        logWarning("trying to type text with render without finalising pixels buffer");
    }

    SDL_Surface* surface = TTF_RenderText_Solid(font, s.c_str(), {c.r, c.g, c.b});
    if (surface == nullptr) {
        logError("no surface");
        return 1;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == nullptr) {
        logError("no texture");
        SDL_FreeSurface(surface);
        return 1;
    }

    int texW = 0;
    int texH = 0;
    SDL_QueryTexture(texture, NULL, NULL, &texW, &texH);
    SDL_Rect dstrect = {x, y, texW*scaleW, texH*scaleH};

    // copy texture to renderer
    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderCopy(renderer, texture, NULL, &dstrect);

    // cleanup
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

void Renderer::show() {
    // show default target
    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderPresent(renderer);

    // allPixels the default target
    SDL_SetRenderTarget(renderer, NULL);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
}