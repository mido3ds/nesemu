#pragma once

#include <functional>

#include "emulation/common.h"
#include "emulation/console.h"

struct Instruction {
    function<void(Console&)> exec;
    string name;
    AddressMode mode;
    u16_t cpuCycles;
    bool crossPagePenalty;
};

typedef array<Instruction, 0xFF+1> InstructionSet;
extern const InstructionSet instructionSet;