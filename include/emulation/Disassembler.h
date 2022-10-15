#pragma once

#include <map>

#include "emulation/ROM.h"

struct Disassembler {
    std::map<uint16_t, string> assembly;

    // `addr` is the starting address of data
    void init(const vector<uint8_t>& data, uint16_t addr);

    // if addr is in data range, returns `n` assembly
    // otherwise returns "???"
    vector<string> get(const uint16_t addr, const uint16_t n) const;

    // dissasmble one instruction with size <= `size`
    // returns assembly and number of consumed bytes
    static tuple<string, int> dissasmble(uint8_t const* mem, uint32_t size);
};
