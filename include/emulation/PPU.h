#pragma once

#include "emulation/Bus.h"
#include "emulation/BusAttachable.h"

class PPU: public BusAttachable {
public:
    int init(Bus* bus);

    void clock();

    virtual void reset();
    
    virtual bool read(u16_t addr, u8_t& data);
    virtual bool write(u16_t addr, u8_t data);

private:
    Bus* bus;
};