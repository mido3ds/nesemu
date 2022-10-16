#pragma once

#include <thread>

#include "emulation/RAM.h"
#include "emulation/Bus.h"
#include "emulation/CPU.h"
#include "emulation/PPU.h"
#include "emulation/Disassembler.h"

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
