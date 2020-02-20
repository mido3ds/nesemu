#pragma once

#include <array>

#include "stdtype.h"

class RAM {
public:
    void init();

    void reset();
    bool read(u16_t addr, u8_t& data);
    bool write(u16_t addr, u8_t data);

private:
    std::array<u8_t, 0x07FF+1> data;
};