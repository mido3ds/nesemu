#pragma once

#include <string>
#include <thread>

#include "stdtype.h"
#include "emulation/ROM.h"

class MMC {
public:
    static shared_ptr<MMC> fromROM(ROM rom);

    virtual void reset() =0;
    virtual bool read(u16_t addr, u8_t& data) =0;
    virtual bool write(u16_t addr, u8_t data) =0;

private:
    ROM rom;
};