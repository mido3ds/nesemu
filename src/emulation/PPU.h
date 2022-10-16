#pragma once

#include "emulation/ICPUBusAttachable.h"
#include "gui/IRenderer.h"

struct Bus;

struct PPU: public ICPUBusAttachable {
    Bus* bus;

    uint16_t cycles = 0, row = 0, col = 0;

    virtual void reset();
    virtual bool read(uint16_t addr, uint8_t& data);
    virtual bool write(uint16_t addr, uint8_t data);
};

PPU ppu_new(Bus* bus);
void ppu_clock(PPU& self, IRenderer* renderer);
