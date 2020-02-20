#pragma once

#include "emulation/MMC.h"

class MMC0: public MMC {
public:
    virtual void reset();
    virtual bool read(u16_t addr, u8_t& data);
    virtual bool write(u16_t addr, u8_t data);
};