#include "emulation/Bus.h"
#include "log.h"

int Bus::init(RAM ram, shared_ptr<MMC> mmc, IORegs io) {
    if (!mmc) { return 1; }
    
    this->ram = ram;
    this->mmc = mmc;
    this->io = io;

    return 0;
}

void Bus::reset() {
    INFO("reset bus");
    ram.reset();
    mmc->reset();
    io.reset();
}

bool Bus::read(u16_t addr, u8_t& data) {
    if (ram.read(addr, data) || 
        mmc->read(addr, data) || 
        io.read(addr, data)) { 
        return true;
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
    if (ram.write(addr, data) || 
        mmc->write(addr, data) || 
        io.write(addr, data)) { 
        return true;
    }

    ERROR("bus: write to unregistered address %d", addr);
    return false;
}

bool Bus::write16(u16_t addr, u16_t data) {
    return write(addr, data & 0x00FF) && write(addr+1, (data & 0xFF00) >> 8);
}