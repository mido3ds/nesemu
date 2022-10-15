#pragma once

#include <cstdint>

struct ICPUBusAttachable {
    virtual void reset() =0;

    virtual bool read(uint16_t addr, uint8_t& data) =0;
    virtual bool write(uint16_t addr, uint8_t data) =0;
};
