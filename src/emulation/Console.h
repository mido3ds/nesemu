#pragma once

#include <thread>
#include <string>

#include "emulation/RAM.h"
#include "emulation/Bus.h"
#include "emulation/CPU.h"
#include "emulation/PPU.h"
#include "emulation/Disassembler.h"

using namespace std;

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
    shared_ptr<PPU> ppu;
    shared_ptr<RAM> ram;
    Bus bus;
    uint64_t cycles = 0;
    Disassembler disassembler;

    int init(string romPath); // with rom
    int init();

    void reset();
    void clock(IRenderer* renderer);

    void input(JoyPadInput joypad);
};
