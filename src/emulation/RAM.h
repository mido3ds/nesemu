#pragma once

#include "utils.h"

struct RAM {
    Arr<uint8_t, 0x07FF+1> data;

    void reset();
    bool read(uint16_t addr, uint8_t& data);
    bool write(uint16_t addr, uint8_t data);
};
