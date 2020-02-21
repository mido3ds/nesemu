#include "emulation/Bus.h"
#include "log.h"

int Bus::attach(shared_ptr<BusAttachable> attachment) {
    if (attachment) {
        attachments.push_back(attachment);
        return 0;
    }

    ERROR("attaching null to bus");
    return 1;
}

void Bus::reset() {
    for (auto& at:attachments) {
        at->reset();
    }
}

bool Bus::read(u16_t addr, u8_t& data) {
    for (auto& at:attachments) {
        if (at->read(addr, data)) {
            return true;
        }
    }

    ERROR("bus: read from unregistered address %d", addr);
    return false;
}

bool Bus::read16(u16_t addr, u16_t& data) {
    u8_t low, up;
    bool res = read(addr, low) && read(addr+1, up);
    data = low | up << 8;
    return res;
}

bool Bus::write(u16_t addr, u8_t data) {
    for (auto& at:attachments) {
        if (at->write(addr, data)) {
            return true;
        }
    }

    ERROR("bus: write to unregistered address %d", addr);
    return false;
}

bool Bus::write16(u16_t addr, u16_t data) {
    return write(addr, data & 0x00FF) && write(addr+1, (data & 0xFF00) >> 8);
}