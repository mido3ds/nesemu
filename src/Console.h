#pragma once

#include <thread>
#include <map>

#include "RAM.h"
#include "ROM.h"
#include "CPU.h"
#include "PPU.h"

struct Console {
    CPU cpu;
    PPU ppu;
    RAM ram;
    ROM rom;
    uint64_t cycles;
};

void console_init(Console& self, const Str& rom_path = "");
void console_reset(Console& self);
void console_clock(Console& self, Image* image);

struct JoyPadInput {
    bool a;
    bool b;
    bool select;
    bool start;
    bool up;
    bool down;
    bool left;
    bool right;
};

void console_input(Console& self, JoyPadInput joypad);
