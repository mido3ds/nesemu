#pragma once

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include "sdttype.h"
#include "common.h"
#include "config.h"

class Renderer {
private:
    sf::RenderWindow* window = nullptr;
    Config::Rect resolution, windSize;

public:
    int init(sf::RenderWindow* window, Config::Rect resolution, Config::Rect windSize);

    void clear(Color c, u8_t a);
    void pixel(int x, int y, Color c, u8_t a);

    int text(string s, int x, int y, f64_t scaleW, f64_t scaleH, sf::Font* font, Color c, int* newW, int* newH);

    // present the renderer on its window
    void show();
};