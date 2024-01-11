#include "PPU.h"
#include "Config.h"
#include "Image.h"

PPU ppu_new(Console* console) {
    return PPU {
        .console = console,
    };
}

void ppu_clock(PPU& self, Image* image) {
    // TODO
    // for now we will draw random white-black pixels
    if (image) {
        for (int i = 0; i < Config::sys.resolution.height; i++) {
            for (int j = 0; j < Config::sys.resolution.width; j++) {
                image_pixel(*image, j, i, rand()%10 == 0? RGBAColor{}:RGBAColor{255,255,255,255});
            }
        }
    }
}

void ppu_reset(PPU& self) {
    // TODO
    self.cycles = 0;
}

bool ppu_read(PPU& self, uint16_t addr, uint8_t& data) {
    // TODO
    switch (addr) {
    case 0x0000: // Control
        break;
    case 0x0001: // Mask
        break;
    case 0x0002: // Status
        break;
    case 0x0003: // OAM Address
        break;
    case 0x0004: // OAM Data
        break;
    case 0x0005: // Scroll
        break;
    case 0x0006: // PPU Address
        break;
    case 0x0007: // PPU Data
        break;
    }
    return false;
}

bool ppu_write(PPU& self, uint16_t addr, uint8_t data) {
    // TODO
    switch (addr) {
    case 0x0000: // Control
        break;
    case 0x0001: // Mask
        break;
    case 0x0002: // Status
        break;
    case 0x0003: // OAM Address
        break;
    case 0x0004: // OAM Data
        break;
    case 0x0005: // Scroll
        break;
    case 0x0006: // PPU Address
        break;
    case 0x0007: // PPU Data
        break;
    }
    return false;
}

RGBAColor ppu_get_bg_color(const PPU& self, uint8_t index) {
    auto index_into_palette = index >> 2;
    if (index_into_palette > 4) {
        mu::panic("only 4 palettes to select from, found index = {}", index_into_palette);
    }
    auto index_into_indices = index & 0b11;
    uint8_t palette_index = self.bg_palette[index_into_palette].index[index_into_indices];
    return color_from_palette(palette_index);
}

RGBAColor ppu_get_sprite_color(const PPU& self, uint8_t index) {
    auto index_into_palette = index >> 2;
    if (index_into_palette > 4) {
        mu::panic("only 4 palettes to select from, found index = {}", index_into_palette);
    }
    auto index_into_indices = index & 0b11;
    uint8_t palette_index = self.sprite_palette[index_into_palette].index[index_into_indices];
    return color_from_palette(palette_index);
}
