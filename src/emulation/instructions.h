#pragma once

#include <functional>
#include <array>

#include "utils.h"
#include "emulation/common.h"
#include "emulation/CPU.h"

struct Instruction {
    std::function<void(CPU&)> exec;
    StrView name;
    AddressMode mode;
    uint16_t cycles;
    bool crossPagePenalty;
};

typedef Arr<Instruction, 0xFF+1> InstructionSet;
extern const InstructionSet instructionSet;
