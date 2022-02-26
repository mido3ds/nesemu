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

class Console {
public:
    int init(string romPath); // with rom
    int init();

    void reset();
    void clock(IRenderer* renderer);

    Disassembler* getDisassembler();
    CPU* getCPU();
    PPU* getPPU();
    RAM* getRAM();

    void input(JoyPadInput joypad);

private:
    Bus bus;
    shared_ptr<PPU> ppu;
    shared_ptr<RAM> ram;
    CPU cpu;
    Disassembler disassembler;

    u64_t cycles = 0;
};
