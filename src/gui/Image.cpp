#include "gui/Image.h"
#include "Config.h"

Image image_new() {
    Image self {};
    self.sfml_image.create(Config::resolution.w, Config::resolution.h);

    if (!self.texture.create(Config::resolution.w, Config::resolution.h)) {
        panic("texture.create failed");
    }

    return self;
}

void image_pixel(Image& self, int x, int y, Color c, uint8_t a) {
    self.sfml_image.setPixel(x, y, sf::Color(c.r, c.g, c.b, a));
}

void image_clear(Image& self, Color c, uint8_t a) {
    self.sfml_image.create(Config::resolution.w, Config::resolution.h);
}

void image_show(Image& self, sf::RenderWindow& window) {
    self.texture.update(self.sfml_image);
    self.sprite.setTexture(self.texture);

    window.setView(sf::View(sf::FloatRect(0, 0, Config::resolution.w, Config::resolution.h)));

    window.draw(self.sprite);

    window.setView(window.getDefaultView());
}
