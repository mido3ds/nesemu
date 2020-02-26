#pragma once

#include <queue>

#include "emulation/common.h"
#include "gui/IRenderer.h"

struct PixelOperation { int x; int y; Color c; u8_t a; };
struct TextOperation { string s; int x; int y; f64_t scaleW; f64_t scaleH; Font* font; Color c; int* newW; int* newH; };

class MockRenderer: public IRenderer {
private:
    int clearOps = 0; 
    int showOps = 0;
    queue<PixelOperation> pixelOps;
    queue<TextOperation> textOps;

public:
    virtual void clear(Color c, u8_t a);
    bool hasCleared();

    virtual void pixel(int x, int y, Color c, u8_t a);
    bool hasPixeled(PixelOperation& po);

    virtual int text(string s, int x, int y, f64_t scaleW, f64_t scaleH, Font* font, Color c, int* newW, int* newH);
    bool hasTexted(TextOperation& to);

    // present the renderer on its window
    virtual void show();
    bool hasShown();
};