#pragma once

#include "emulation/Bus.h"
#include "emulation/ICPUBusAttachable.h"
#include "gui/IRenderer.h"

class PPU: public ICPUBusAttachable {
public:
    int init(Bus* bus);

    void clock(IRenderer* renderer);

    virtual void reset();
    
    virtual bool read(u16_t addr, u8_t& data);
    virtual bool write(u16_t addr, u8_t data);

private:
    Bus* bus;

    u16_t cycles = 0, row = 0, col = 0;
};