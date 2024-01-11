#pragma once

#include "CPU.h"

struct Instruction {
    std::function<void(CPU&)> exec;
    mu::StrView name;
    AddressMode mode;
    uint16_t cycles;
    bool cross_page_penalty;
};

using InstructionSet = mu::Arr<Instruction, 0xFF+1>;
extern const InstructionSet instruction_set;
