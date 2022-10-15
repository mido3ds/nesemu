#pragma once

#include <functional>
#include <array>

#include "emulation/common.h"
#include "emulation/CPU.h"

struct Instruction {
    function<void(CPU&)> exec;
    string name;
    AddressMode mode;
    uint16_t cycles;
    bool crossPagePenalty;
};

typedef array<Instruction, 0xFF+1> InstructionSet;
extern const InstructionSet instructionSet;
