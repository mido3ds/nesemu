#pragma once

#include <thread>
#include <vector>
#include <memory>

using std::vector;
using std::shared_ptr;

#include "emulation/ICPUBusAttachable.h"
#include "emulation/IPPUBusAttachable.h"

struct Bus {
    vector<shared_ptr<ICPUBusAttachable>> cpuAttachments;
    vector<shared_ptr<IPPUBusAttachable>> ppuAttachments;

    int attach(shared_ptr<ICPUBusAttachable> attachment);
    int attach(shared_ptr<IPPUBusAttachable> attachment);

    void reset();

    // CPU
    bool read(uint16_t addr, uint8_t& data);
    bool read16(uint16_t addr, uint16_t& data);

    bool write(uint16_t addr, uint8_t data);
    bool write16(uint16_t addr, uint16_t data);

    // PPU
    bool ppuRead(uint16_t addr, uint8_t& data);
    bool ppuRead16(uint16_t addr, uint16_t& data);

    bool ppuWrite(uint16_t addr, uint8_t data);
    bool ppuWrite16(uint16_t addr, uint16_t data);
};
