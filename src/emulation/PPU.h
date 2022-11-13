#pragma once

#include "gui/IRenderer.h"

struct Console;

struct PPU {
    Console* console;
    uint16_t cycles = 0, row = 0, col = 0;
};

PPU ppu_new(Console* console);
void ppu_clock(PPU& self, IRenderer* renderer);
void ppu_reset(PPU& self);
bool ppu_read(PPU& self, uint16_t addr, uint8_t& data);
bool ppu_write(PPU& self, uint16_t addr, uint8_t data);
