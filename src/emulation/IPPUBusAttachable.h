#pragma once

struct IPPUBusAttachable {
    virtual void reset() =0;

    virtual bool ppuRead(uint16_t addr, uint8_t& data) =0;
    virtual bool ppuWrite(uint16_t addr, uint8_t data) =0;
};
