#pragma once

#include <thread>
#include <string>

#include "emulation/RAM.h"
#include "emulation/Bus.h"
#include "emulation/CPU.h"
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
    void clock();

    Disassembler& getDisassembler();
    CPU& getCPU();
    RAM& getRAM();

    void input(JoyPadInput joypad);

private:
    Bus bus;
    shared_ptr<RAM> ram;
    CPU cpu;
    Disassembler disassembler;
};