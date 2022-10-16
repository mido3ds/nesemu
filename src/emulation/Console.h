#pragma once

#include <thread>
#include <map>

#include "emulation/RAM.h"
#include "emulation/Bus.h"
#include "emulation/CPU.h"
#include "emulation/PPU.h"
#include "emulation/MMC0.h"
#include "emulation/IORegs.h"

struct Disassembler {
    std::pmr::map<uint16_t, Str> assembly;
};

// `addr` is the starting address of data
Disassembler disassembler_new(const Vec<uint8_t>& data, uint16_t addr);

// if addr is in data range, returns `n` assembly
// otherwise returns "???"
Vec<Str> disassembler_get(const Disassembler& self, const uint16_t addr, const uint16_t n);

struct Console {
    CPU cpu;
    PPU ppu;
    RAM ram;
    Bus bus;
    MMC0 mmc0;
    IORegs io;
    uint64_t cycles;
    Disassembler disassembler;
};

void console_init(Console& self);
void console_load_rom(Console& self, StrView romPath);
void console_reset(Console& self);
void console_clock(Console& self, IRenderer* renderer);

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
