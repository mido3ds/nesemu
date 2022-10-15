#pragma once

#include "emulation/MMC.h"

class MMC0: public MMC {
public:
    static bool valid(ROM const& rom);

    // from both ICPUBusAttachable, IPPUBusAttachable
    virtual void reset();

    // from ICPUBusAttachable
    virtual bool read(uint16_t addr, uint8_t& data);
    virtual bool write(uint16_t addr, uint8_t data);

    // from IPPUBusAttachable
    virtual bool ppuRead(uint16_t addr, uint8_t& data);
    virtual bool ppuWrite(uint16_t addr, uint8_t data);
};
