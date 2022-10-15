#pragma once

#include "emulation/ICPUBusAttachable.h"

struct IORegs: public ICPUBusAttachable {
    void init();

    virtual void reset();

    virtual bool read(uint16_t addr, uint8_t& data);
    virtual bool write(uint16_t addr, uint8_t data);
};
