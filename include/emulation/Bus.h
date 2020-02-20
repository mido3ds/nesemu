#pragma once

#include <thread>

#include "stdtype.h"
#include "emulation/MMC.h"
#include "emulation/RAM.h"
#include "emulation/IORegs.h"

class Bus {
public:
    int init(RAM ram, shared_ptr<MMC> mmc, IORegs io);

    void reset();

    bool read(u16_t addr, u8_t& data);
    bool read16(u16_t addr, u16_t& data);
    bool write(u16_t addr, u8_t data);
    bool write16(u16_t addr, u16_t data);

private:
    RAM ram;
    IORegs io;
    shared_ptr<MMC> mmc;
};