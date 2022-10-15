#pragma once

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include "emulation/common.h"
#include "Config.h"
#include "IRenderer.h"

struct SFMLRenderer: public IRenderer {
    sf::RenderWindow* window = nullptr;
    Config::Rect resolution;
    sf::RenderTexture texture;
    sf::Sprite sprite;

    int init(sf::RenderWindow* window, Config::Rect resolution);

    virtual void clear(Color c, uint8_t a);
    virtual void pixel(int x, int y, Color c, uint8_t a);

    virtual int text(string s, int x, int y, double scaleW, double scaleH, Font* font, Color c, int* newW, int* newH);

    // present the renderer on its window
    virtual void show();
};
