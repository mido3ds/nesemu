#include "emulation/RAM.h"

void RAM::init() {
    data.fill(0);
}

void RAM::reset() {}

bool RAM::read(u16_t addr, u8_t& data) {
    if (addr <= 0x07FF || (addr >= 0x0800 && addr <= 0x1FFF)) {
        data = this->data[addr & 0x07FF];
        return true;
    }
    return false;
}

bool RAM::write(u16_t addr, u8_t data) {
    if (addr <= 0x07FF || (addr >= 0x0800 && addr <= 0x1FFF)) {
        this->data[addr & 0x07FF] = data;
        return true;
    }
    return false;
}
