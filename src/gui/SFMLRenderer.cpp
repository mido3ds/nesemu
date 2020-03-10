#include "gui/SFMLRenderer.h"
#include "log.h"
#include "Config.h"

int SFMLRenderer::init(sf::RenderWindow* window, Config::Rect resolution) {
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

    if (!texture.create(window->getSize().x, window->getSize().y)) {
        ERROR("coulndt create texture");
        return 1;
    }
    texture.clear();

    sf::View view(sf::FloatRect(0, 0, resolution.w, resolution.h));
    texture.setView(view);

    return 0;
}

void SFMLRenderer::pixel(int x, int y, Color c, u8_t a) {
    sf::CircleShape p(1);
    p.setFillColor(sf::Color(c.r, c.g, c.b, a));
    p.setPosition(x, y);
    texture.draw(p);
}

void SFMLRenderer::clear(Color c, u8_t a) {
    texture.clear(sf::Color(c.r,c.g,c.b,a));
}

int SFMLRenderer::text(string s, int x, int y, f64_t scaleW, f64_t scaleH, Font* font, Color c, int* newW, int* newH) {
    if (!font) {
        ERROR("null font");
        return 1;
    }

    sf::Text text;
    text.setFont(*((sf::Font*)font));
    text.setString(s);
    text.setPosition(x, y);
    text.setScale(scaleW, scaleH);
    text.setColor(sf::Color(c.r,c.g,c.b));
    text.setCharacterSize(Config::fontSize);
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

    window->clear();
    window->draw(sprite);
    window->display();
}