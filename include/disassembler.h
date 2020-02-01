#pragma once

#include <string>
#include <vector>

#include "common.h"

using namespace std;

// disassemble returns array of strings of memory disassembly
vector<string> disassemble(uint8_t* mem, uint32_t size, const InstructionSet& instset);