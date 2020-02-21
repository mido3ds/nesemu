#pragma once

#include "emulation/BusAttachable.h"

class IORegs: public BusAttachable {
public:
    void init();

    virtual void reset();
    virtual bool read(u16_t addr, u8_t& data);
    virtual bool write(u16_t addr, u8_t data);
};