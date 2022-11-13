#pragma once

#include "gui/IRenderer.h"

struct Console;

struct PPU {
    Console* console;

    uint16_t cycles = 0, row = 0, col = 0;

    void reset();
    bool read(uint16_t addr, uint8_t& data);
    bool write(uint16_t addr, uint8_t data);
};

PPU ppu_new(Console* console);
void ppu_clock(PPU& self, IRenderer* renderer);
