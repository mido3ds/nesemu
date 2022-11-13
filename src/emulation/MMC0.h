#pragma once

#include "emulation/ROM.h"

struct MMC0 {
    ROM rom;

    void reset();
    bool read(uint16_t addr, uint8_t& data);
    bool write(uint16_t addr, uint8_t data);
    bool ppu_read(uint16_t addr, uint8_t& data);
    bool ppu_write(uint16_t addr, uint8_t data);
};

void mmc0_load_rom(MMC0& self, const Str& rom_path);
