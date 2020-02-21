#pragma once

#include "stdtype.h"

class BusAttachable {
public:
    virtual void reset() =0;
    virtual bool read(u16_t addr, u8_t& data) =0;
    virtual bool write(u16_t addr, u8_t data) =0;
};