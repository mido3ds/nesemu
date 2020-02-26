#pragma once

#include <array>

#include "emulation/ICPUBusAttachable.h"
#include "stdtype.h"

class RAM: public ICPUBusAttachable {
public:
    void init();

    virtual void reset();
    
    virtual bool read(u16_t addr, u8_t& data);
    virtual bool write(u16_t addr, u8_t data);

private:
    std::array<u8_t, 0x07FF+1> data;
};