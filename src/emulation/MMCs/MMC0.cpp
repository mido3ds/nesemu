#include "emulation/MMCs/MMC0.h"
#include "emulation/common.h"

bool MMC0::valid(ROM const& rom) {
    return rom.getMapperNumber() == 0 &&
        (rom.prg.size() % (16*1024) == 0) &&
        (rom.chr.size() == (8*1024));
}

void MMC0::reset() {}

bool MMC0::read(uint16_t addr, uint8_t& data) {
    if (PRG_REGION.contains(addr)) {
        data = rom.prg[(addr - PRG_ROM_LOW.start) % rom.prg.size()];

        return true;
    }

    return false;
}

bool MMC0::write(uint16_t addr, uint8_t data) {
    if (PRG_REGION.contains(addr)) {
        rom.prg[(addr - PRG_ROM_LOW.start) % rom.prg.size()] = data;

        return true;
    }

    return false;
}

bool MMC0::ppuRead(uint16_t addr, uint8_t& data) {
    // TODO
	return false;
}

bool MMC0::ppuWrite(uint16_t addr, uint8_t data) {
    // TODO
	return false;
}
