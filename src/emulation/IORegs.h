#pragma once

#include <cstdint>

struct IORegs {
    void reset();
    bool read(uint16_t addr, uint8_t& data);
    bool write(uint16_t addr, uint8_t data);
};
