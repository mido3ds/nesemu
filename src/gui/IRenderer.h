#pragma once

#include "emulation/common.h"
#include "utils.h"

class Font;

struct IRenderer {
    virtual void clear(Color c, uint8_t a) =0;
    virtual void pixel(int x, int y, Color c, uint8_t a) =0;

    virtual int text(const Str& s, int x, int y, double scaleW, double scaleH, Font* font, Color c, int* newW, int* newH) =0;

    // present the renderer on its window
    virtual void show() =0;
};
