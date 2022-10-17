#include "emulation/Bus.h"
#include "utils.h"

void bus_attach_to_cpu(Bus& self, ICPUBusAttachable* attachment) {
    my_assert(attachment);
    self.cpu_attachments.push_back(attachment);
}

void bus_attach_to_ppu(Bus& self, IPPUBusAttachable* attachment) {
    my_assert(attachment);
    self.ppu_attachments.push_back(attachment);
}

void bus_reset(Bus& self) {
    for (auto& at : self.cpu_attachments) {
        at->reset();
    }

    for (auto& at : self.ppu_attachments) {
        at->reset();
    }
}

bool Bus::read(uint16_t addr, uint8_t& data) {
    bool success = false;
    for (auto& at:cpu_attachments) {
        if (at->read(addr, data)) {
            success = true;
        }
    }

    if (success) { return true; }

    WARNING("bus: read from unregistered address 0x{:02X}", addr);
    return false;
}

bool Bus::read16(uint16_t addr, uint16_t& data) {
    uint8_t low = 0, up = 0;
    bool res = read(addr, low) && read(addr+1, up);
    data = low | up << 8;
    return res;
}

bool Bus::write(uint16_t addr, uint8_t data) {
    bool success = false;
    for (auto& at:cpu_attachments) {
        if (at->write(addr, data)) {
            success = true;
        }
    }

    if (success) { return true; }

    WARNING("bus: write to unregistered address 0x{:02X}", addr);
    return false;
}

bool Bus::write16(uint16_t addr, uint16_t data) {
    return write(addr, data & 0x00FF) && write(addr+1, (data & 0xFF00) >> 8);
}

bool Bus::ppu_read(uint16_t addr, uint8_t& data) {
    bool success = false;
    for (auto& at:ppu_attachments) {
        if (at->ppu_read(addr, data)) {
            success = true;
        }
    }

    if (success) { return true; }

    WARNING("bus: read from unregistered address 0x{:02X}", addr);
    return false;
}

bool Bus::ppu_read16(uint16_t addr, uint16_t& data) {
    uint8_t low, up;
    bool res = read(addr, low) && read(addr+1, up);
    data = low | up << 8;
    return res;
}

bool Bus::ppu_write(uint16_t addr, uint8_t data) {
    bool success = false;
    for (auto& at:ppu_attachments) {
        if (at->ppu_write(addr, data)) {
            success = true;
        }
    }

    if (success) { return true; }

    WARNING("bus: write to unregistered address 0x{:02X}", addr);
    return false;
}

bool Bus::ppu_write16(uint16_t addr, uint16_t data) {
    return write(addr, data & 0x00FF) && write(addr+1, (data & 0xFF00) >> 8);
}
