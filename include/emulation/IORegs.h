#pragma once

#include <thread>

#include "stdtype.h"

class IORegs {
public:
    void init();

    void reset();
    bool read(u16_t addr, u8_t& data);
    bool write(u16_t addr, u8_t data);
};