#include "gui/SFMLImageRenderer.h"
#include "Config.h"

void SFMLImageRenderer::init(sf::RenderWindow* window, Config::Rect resolution) {
    if (resolution.w <= 0 || resolution.h <= 0) {
        panic("invalid resolution and/or windsize");
    }

    this->resolution = resolution;
    this->window = window;
    my_assert(window);

    image.create(resolution.w, resolution.h);

    if (!texture.create(resolution.w, resolution.h)) {
        panic("texture.create failed");
    }
}

void SFMLImageRenderer::pixel(int x, int y, Color c, uint8_t a) {
    image.setPixel(x, y, sf::Color(c.r, c.g, c.b, a));
}

void SFMLImageRenderer::clear(Color c, uint8_t a) {
    image.create(resolution.w, resolution.h);
}

int SFMLImageRenderer::text(const Str& s, int x, int y, double scaleW, double scaleH, Font* font, Color c, int* newW, int* newH) {
    panic("SFMLImageRenderer doesnt render text!");
    return 1;
}

void SFMLImageRenderer::show() {
    texture.update(image);
    sprite.setTexture(texture);

    window->setView(sf::View(sf::FloatRect(0, 0, resolution.w, resolution.h)));

    window->draw(sprite);

    window->setView(window->getDefaultView());
}
