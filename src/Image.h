#pragma once

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include "common.h"
#include "Config.h"

struct Image {
    sf::Image sfml_image;
    sf::Sprite sprite;
    sf::Texture texture;
};

Image image_new();

void image_clear(Image& self, Color c, uint8_t a);
void image_pixel(Image& self, int x, int y, Color c, uint8_t a);

// present the renderer on its window
void image_show(Image& self, sf::RenderWindow& window);
