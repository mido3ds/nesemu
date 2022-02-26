#pragma once

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include "stdtype.h"
#include "emulation/common.h"
#include "Config.h"
#include "IRenderer.h"

class SFMLRenderer: public IRenderer {
private:
    sf::RenderWindow* window = nullptr;
    Config::Rect resolution;
    sf::RenderTexture texture;
    sf::Sprite sprite;

public:
    int init(sf::RenderWindow* window, Config::Rect resolution);

    virtual void clear(Color c, u8_t a);
    virtual void pixel(int x, int y, Color c, u8_t a);

    virtual int text(string s, int x, int y, f64_t scaleW, f64_t scaleH, Font* font, Color c, int* newW, int* newH);

    // present the renderer on its window
    virtual void show();
};
