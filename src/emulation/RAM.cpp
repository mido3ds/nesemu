#include "emulation/RAM.h"
#include "emulation/common.h"

void RAM::init() {
    data.fill(0);
}

void RAM::reset() {}

bool RAM::read(uint16_t addr, uint8_t& data) {
    if (RAM_REGION.contains(addr)) {
        data = this->data[addr & RRAM.end];
        return true;
    }
    return false;
}

bool RAM::write(uint16_t addr, uint8_t data) {
    if (RAM_REGION.contains(addr)) {
        this->data[addr & RRAM.end] = data;
        return true;
    }
    return false;
}
