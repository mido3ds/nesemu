#pragma once

#include "emulation/ROM.h"
#include "emulation/ICPUBusAttachable.h"
#include "emulation/IPPUBusAttachable.h"

struct MMC0: public ICPUBusAttachable, public IPPUBusAttachable {
    ROM rom;

    void init(StrView romPath);

    // from both ICPUBusAttachable, IPPUBusAttachable
    virtual void reset();

    // from ICPUBusAttachable
    virtual bool read(uint16_t addr, uint8_t& data);
    virtual bool write(uint16_t addr, uint8_t data);

    // from IPPUBusAttachable
    virtual bool ppuRead(uint16_t addr, uint8_t& data);
    virtual bool ppuWrite(uint16_t addr, uint8_t data);
};
