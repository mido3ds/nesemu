#pragma once

#include <thread>
#include <vector>

using std::vector;
using std::shared_ptr;

#include "emulation/ICPUBusAttachable.h"
#include "emulation/IPPUBusAttachable.h"

class Bus {
public:
    int attach(shared_ptr<ICPUBusAttachable> attachment);
    int attach(shared_ptr<IPPUBusAttachable> attachment);

    void reset();

    // CPU
    bool read(u16_t addr, u8_t& data);
    bool read16(u16_t addr, u16_t& data);

    bool write(u16_t addr, u8_t data);
    bool write16(u16_t addr, u16_t data);

    // PPU
    bool ppuRead(u16_t addr, u8_t& data);
    bool ppuRead16(u16_t addr, u16_t& data);

    bool ppuWrite(u16_t addr, u8_t data);
    bool ppuWrite16(u16_t addr, u16_t data);

private:
    vector<shared_ptr<ICPUBusAttachable>> cpuAttachments;
    vector<shared_ptr<IPPUBusAttachable>> ppuAttachments;
};