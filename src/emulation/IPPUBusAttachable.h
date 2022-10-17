#pragma once

struct IPPUBusAttachable {
    virtual void reset() =0;

    virtual bool ppu_read(uint16_t addr, uint8_t& data) =0;
    virtual bool ppu_write(uint16_t addr, uint8_t data) =0;
};
