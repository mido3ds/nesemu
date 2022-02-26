#pragma once

#include "emulation/MMC.h"

class MMC0: public MMC {
public:
    static bool valid(ROM const& rom);

    // from both ICPUBusAttachable, IPPUBusAttachable
    virtual void reset();

    // from ICPUBusAttachable
    virtual bool read(u16_t addr, u8_t& data);
    virtual bool write(u16_t addr, u8_t data);

    // from IPPUBusAttachable
    virtual bool ppuRead(u16_t addr, u8_t& data);
    virtual bool ppuWrite(u16_t addr, u8_t data);
};
