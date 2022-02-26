#pragma once

#include <map>

#include "emulation/ROM.h"
#include "stdtype.h"

class Disassembler {
public:
    // `addr` is the starting address of data
    void init(const vector<u8_t>& data, u16_t addr);

    // if addr is in data range, returns `n` assembly
    // otherwise returns "???"
    vector<string> get(const u16_t addr, const u16_t n) const;

    // dissasmble one instruction with size <= `size`
    // returns assembly and number of consumed bytes
    static tuple<string, int> dissasmble(u8_t const* mem, u32_t size);

private:
    std::map<u16_t, string> assembly;
};
