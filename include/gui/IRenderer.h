#pragma once

#include "emulation/common.h"

class Font;

class IRenderer {
public:
    virtual void clear(Color c, u8_t a) =0;
    virtual void pixel(int x, int y, Color c, u8_t a) =0;

    virtual int text(string s, int x, int y, f64_t scaleW, f64_t scaleH, Font* font, Color c, int* newW, int* newH) =0;

    // present the renderer on its window
    virtual void show() =0;
};
