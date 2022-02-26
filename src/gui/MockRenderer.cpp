#include "gui/MockRenderer.h"

void MockRenderer::clear(Color c, u8_t a) {
    clearOps++;
}
bool MockRenderer::hasCleared() {
    if (clearOps > 0) {
        clearOps--;
        return true;
    }
    return false;
}

void MockRenderer::pixel(int x, int y, Color c, u8_t a) {
    pixelOps.push({x, y, c, a});
}
bool MockRenderer::hasPixeled(PixelOperation& po) {
    if (pixelOps.size() > 0) {
        po = pixelOps.front();
        pixelOps.pop();
        return true;
    }
    return false;
}

int MockRenderer::text(string s, int x, int y, f64_t scaleW, f64_t scaleH, Font* font, Color c, int* newW, int* newH) {
    textOps.push(TextOperation {s, x, y, scaleW, scaleH, font, c, newW, newH});
	return 0;
}
bool MockRenderer::hasTexted(TextOperation& to) {
    if (textOps.size() > 0) {
        to = textOps.front();
        textOps.pop();
        return true;
    }
    return false;
}

void MockRenderer::show() {
    showOps++;
}
bool MockRenderer::hasShown() {
    if (showOps > 0) {
        showOps--;
        return true;
    }
    return false;
}
