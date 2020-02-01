#pragma once
#include <string>
#include <vector>

using namespace std;

// disassemble returns array of strings of memory disassembly
vector<string> disassemble(uint8_t* mem, uint16_t size);