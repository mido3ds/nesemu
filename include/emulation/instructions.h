#pragma once

#include <functional>

#include "emulation/common.h"
#include "emulation/CPU.h"

struct Instruction {
    function<void(CPU&)> exec;
    string name;
    AddressMode mode;
    u16_t cycles;
    bool crossPagePenalty;
};

typedef array<Instruction, 0xFF+1> InstructionSet;
extern const InstructionSet instructionSet;