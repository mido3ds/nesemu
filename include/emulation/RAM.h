#pragma once

#include <array>

#include "emulation/BusAttachable.h"
#include "stdtype.h"

class RAM: public BusAttachable {
public:
    void init();
    
    virtual bool read(u16_t addr, u8_t& data);
    virtual bool write(u16_t addr, u8_t data);

private:
    std::array<u8_t, 0x07FF+1> data;
};