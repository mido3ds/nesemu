#include "gui/Renderer.h"
#include "log.h"
#include "Config.h"

int Renderer::init(sf::RenderWindow* window, Config::Rect resolution, Config::Rect windSize) {
    if (resolution.w <= 0 || resolution.h <= 0 || 
        windSize.w <= 0 || windSize.h <= 0) {
        ERROR("invalid resolution or windsize");
        return 1;
    }

    this->resolution = resolution;
    this->windSize = windSize;

    this->window = window;
    if (!window) {
        ERROR("null window");
        return 1;
    }

    return 0;
}

void Renderer::pixel(int x, int y, Color c, u8_t a) {
    // TODO: scale to resolution
    sf::CircleShape p(1);
    p.setFillColor(sf::Color(c.r, c.g, c.b, a));
    p.setPosition(x, y);
    // p.scale(windSize.h/float(resolution.h), windSize.h/float(resolution.h));
    window->draw(p);
}

void Renderer::clear(Color c, u8_t a) {
    window->clear(sf::Color(c.r,c.g,c.b,a));
}

int Renderer::text(string s, int x, int y, f64_t scaleW, f64_t scaleH, sf::Font* font, Color c, int* newW, int* newH) {
    if (!font) {
        ERROR("null font");
        return 1;
    }

    sf::Text text;
    text.setFont(*font);
    text.setString(s);
    text.setPosition(x, y);
    text.setScale(scaleW, scaleH);
    text.setColor(sf::Color(c.r,c.g,c.b));
    text.setCharacterSize(Config::fontSize);
    text.setStyle(sf::Text::Underlined);

    auto sc = text.getScale();
    if (newW) { *newW = sc.x; }
    if (newH) { *newH = sc.y; }

    window->draw(text);

    return 0;
}

void Renderer::show() {
    window->display();
}