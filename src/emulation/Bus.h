#pragma once

#include <thread>

#include "utils.h"
#include "emulation/ICPUBusAttachable.h"
#include "emulation/IPPUBusAttachable.h"

struct Bus {
    Vec<ICPUBusAttachable*> cpu_attachments;
    Vec<IPPUBusAttachable*> ppu_attachments;

    // CPU
    bool read(uint16_t addr, uint8_t& data);
    bool read16(uint16_t addr, uint16_t& data);

    bool write(uint16_t addr, uint8_t data);
    bool write16(uint16_t addr, uint16_t data);

    // PPU
    bool ppu_read(uint16_t addr, uint8_t& data);
    bool ppu_read16(uint16_t addr, uint16_t& data);

    bool ppu_write(uint16_t addr, uint8_t data);
    bool ppu_write16(uint16_t addr, uint16_t data);
};

void bus_attach_to_cpu(Bus& self, ICPUBusAttachable* attachment);
void bus_attach_to_ppu(Bus& self, IPPUBusAttachable* attachment);
void bus_reset(Bus& self);
