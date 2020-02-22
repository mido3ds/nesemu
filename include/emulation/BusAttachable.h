#pragma once

#include "stdtype.h"

class BusAttachable {
public:
    virtual void reset();

    // from CPU
    virtual bool read(u16_t addr, u8_t& data);
    virtual bool write(u16_t addr, u8_t data);

    // from PPU
    virtual bool ppuRead(u16_t addr, u8_t& data);
    virtual bool ppuWrite(u16_t addr, u8_t data);
};