#pragma once

#include <array>

#include "emulation/ICPUBusAttachable.h"

struct RAM: public ICPUBusAttachable {
    std::array<uint8_t, 0x07FF+1> data;

    virtual void reset();
    virtual bool read(uint16_t addr, uint8_t& data);
    virtual bool write(uint16_t addr, uint8_t data);
};
