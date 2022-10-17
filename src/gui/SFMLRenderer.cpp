#include "gui/SFMLRenderer.h"
#include "Config.h"

void SFMLRenderer::init(sf::RenderWindow* window, Config::Rect resolution) {
    if (resolution.w <= 0 || resolution.h <= 0) {
        panic("invalid resolution and/or windsize");
    }

    this->resolution = resolution;

    this->window = window;
    my_assert(window);

    if (!texture.create(window->getSize().x, window->getSize().y)) {
        panic("coulndt create texture");
    }
    texture.clear();

    sf::View view(sf::FloatRect(0, 0, resolution.w, resolution.h));
    texture.setView(view);
}

void SFMLRenderer::pixel(int x, int y, Color c, uint8_t a) {
    sf::CircleShape p(1);
    p.setFillColor(sf::Color(c.r, c.g, c.b, a));
    p.setPosition(x, y);
    texture.draw(p);
}

void SFMLRenderer::clear(Color c, uint8_t a) {
    texture.clear(sf::Color(c.r,c.g,c.b,a));
}

int SFMLRenderer::text(const Str& s, int x, int y, double scaleW, double scaleH, Font* font, Color c, int* newW, int* newH) {
    if (!font) {
        ERROR("null font");
        return 1;
    }

    sf::Text text;
    text.setFont(*((sf::Font*)font));
    text.setString(s.c_str());
    text.setPosition(x, y);
    text.setScale(scaleW, scaleH);
    text.setColor(sf::Color(c.r,c.g,c.b));
    text.setCharacterSize(Config::font_size);
    text.setStyle(sf::Text::Underlined);

    auto sc = text.getScale();
    if (newW) { *newW = sc.x; }
    if (newH) { *newH = sc.y; }

    texture.draw(text);

    return 0;
}

void SFMLRenderer::show() {
    sprite.setTexture(texture.getTexture());
    sprite.setScale(1.0, -1.0);
    sprite.setPosition(0, window->getSize().y);

    window->draw(sprite);
}
