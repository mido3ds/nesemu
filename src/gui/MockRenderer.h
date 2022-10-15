#pragma once

#include <queue>

#include "emulation/common.h"
#include "gui/IRenderer.h"

struct PixelOperation { int x; int y; Color c; uint8_t a; };
struct TextOperation { Str s; int x; int y; double scaleW; double scaleH; Font* font; Color c; int* newW; int* newH; };

struct MockRenderer: public IRenderer {
    int clearOps = 0;
    int showOps = 0;
    queue<PixelOperation> pixelOps;
    queue<TextOperation> textOps;

    virtual void clear(Color c, uint8_t a);
    bool hasCleared();

    virtual void pixel(int x, int y, Color c, uint8_t a);
    bool hasPixeled(PixelOperation& po);

    virtual int text(const Str& s, int x, int y, double scaleW, double scaleH, Font* font, Color c, int* newW, int* newH);
    bool hasTexted(TextOperation& to);

    // present the renderer on its window
    virtual void show();
    bool hasShown();
};
