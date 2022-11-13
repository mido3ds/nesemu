#include "emulation/RAM.h"
#include "emulation/common.h"

bool ram_read(RAM& self, uint16_t addr, uint8_t& data) {
    if (RAM_REGION.contains(addr)) {
        data = self[addr & RRAM.end];
        return true;
    }
    return false;
}

bool ram_write(RAM& self, uint16_t addr, uint8_t data) {
    if (RAM_REGION.contains(addr)) {
        self[addr & RRAM.end] = data;
        return true;
    }
    return false;
}
