#pragma once

#include "common.h"

struct Image;
struct Console;

struct Palette {
    uint8_t index[4];
};

struct PPU {
    Console* console;
    uint16_t cycles = 0, row = 0, col = 0;

    // vram
    uint8_t universal_bg_index; // $3F00
    Palette bg_palettes[4],      // $3F01 - $3F0F
            sprite_palettes[4];  // $3F11 - $3F1F
};

PPU ppu_new(Console* console);
void ppu_clock(PPU& self, Image* image);
void ppu_reset(PPU& self);
bool ppu_read(PPU& self, uint16_t addr, uint8_t& data);
bool ppu_write(PPU& self, uint16_t addr, uint8_t data);
