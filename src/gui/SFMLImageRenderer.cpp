#include "gui/SFMLImageRenderer.h"
#include "Config.h"

int SFMLImageRenderer::init(sf::RenderWindow* window, Config::Rect resolution) {
    if (resolution.w <= 0 || resolution.h <= 0) {
        ERROR("invalid resolution or windsize");
        return 1;
    }

    this->resolution = resolution;

    this->window = window;
    if (!window) {
        ERROR("null window");
        return 1;
    }

    image.create(resolution.w, resolution.h);

    if (!texture.create(resolution.w, resolution.h)) {
        ERROR("texture.create failed");
        return 1;
    }

    return 0;
}

void SFMLImageRenderer::pixel(int x, int y, Color c, uint8_t a) {
    image.setPixel(x, y, sf::Color(c.r, c.g, c.b, a));
}

void SFMLImageRenderer::clear(Color c, uint8_t a) {
    image.create(resolution.w, resolution.h);
}

int SFMLImageRenderer::text(const Str& s, int x, int y, double scaleW, double scaleH, Font* font, Color c, int* newW, int* newH) {
    ERROR("SFMLImageRenderer doesnt render text!");
    return 1;
}

void SFMLImageRenderer::show() {
    texture.update(image);
    sprite.setTexture(texture);

    window->setView(sf::View(sf::FloatRect(0, 0, resolution.w, resolution.h)));

    window->clear();
    window->draw(sprite);
    window->display();

    window->setView(window->getDefaultView());
}
