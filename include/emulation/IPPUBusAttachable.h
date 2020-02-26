#pragma once

#include "stdtype.h"

class IPPUBusAttachable {
public:
    virtual void reset() =0;

    virtual bool ppuRead(u16_t addr, u8_t& data) =0;
    virtual bool ppuWrite(u16_t addr, u8_t data) =0;
};