#pragma once

#include "emulation/Bus.h"
#include "emulation/ICPUBusAttachable.h"
#include "gui/IRenderer.h"

struct PPU: public ICPUBusAttachable {
    Bus* bus;

    uint16_t cycles = 0, row = 0, col = 0;

    void init(Bus* bus);

    void clock(IRenderer* renderer);

    virtual void reset();

    virtual bool read(uint16_t addr, uint8_t& data);
    virtual bool write(uint16_t addr, uint8_t data);
};
