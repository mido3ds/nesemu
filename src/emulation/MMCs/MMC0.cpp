#include "emulation/MMCs/MMC0.h"
#include "emulation/common.h"

bool MMC0::valid(ROM const& rom) {
    return rom.getMapperNumber() == 0 &&
        (rom.prg.size() % (16*1024) == 0) &&
        (rom.chr.size() == (8*1024));
}

void MMC0::reset() {}

bool MMC0::read(u16_t addr, u8_t& data) {
    if (PRG_REGION.contains(addr)) {
        data = rom.prg[(addr - PRG_ROM_LOW.start) % rom.prg.size()];

        return true;
    }

    return false;
}

bool MMC0::write(u16_t addr, u8_t data) {
    if (PRG_REGION.contains(addr)) {
        rom.prg[(addr - PRG_ROM_LOW.start) % rom.prg.size()] = data;

        return true;
    }

    return false;
}

bool MMC0::ppuRead(u16_t addr, u8_t& data) {
    // TODO
	return false;
}

bool MMC0::ppuWrite(u16_t addr, u8_t data) {
    // TODO
	return false;
}
