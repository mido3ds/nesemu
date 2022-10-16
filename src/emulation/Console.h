#pragma once

#include <thread>
#include <map>

#include "emulation/RAM.h"
#include "emulation/Bus.h"
#include "emulation/CPU.h"
#include "emulation/PPU.h"

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

struct Disassembler {
    std::pmr::map<uint16_t, Str> assembly;

    // `addr` is the starting address of data
    void init(const Vec<uint8_t>& data, uint16_t addr);

    // if addr is in data range, returns `n` assembly
    // otherwise returns "???"
    Vec<Str> get(const uint16_t addr, const uint16_t n) const;
};

struct Console {
    CPU cpu;
    std::shared_ptr<PPU> ppu;
    std::shared_ptr<RAM> ram;
    Bus bus;
    uint64_t cycles = 0;
    Disassembler disassembler;

    void init(StrView romPath); // with rom
    void init();

    void reset();
    void clock(IRenderer* renderer);

    void input(JoyPadInput joypad);
};
