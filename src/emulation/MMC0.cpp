#include "emulation/MMC0.h"
#include "emulation/common.h"

void mmc0_load_rom(MMC0& self, const Str& rom_path) {
    self.rom.load(rom_path);

    const bool validMMC0Rom = self.rom.get_mapper_number() == 0 &&
        (self.rom.prg.size() % (16*1024) == 0) &&
        (self.rom.chr.size() == (8*1024));
    if (!validMMC0Rom) {
        panic("only supports MMC0");
    }
}

void MMC0::reset() {}

bool MMC0::read(uint16_t addr, uint8_t& data) {
    if (rom.prg.size() > 0 && PRG_REGION.contains(addr)) {
        data = rom.prg[(addr - PRG_ROM_LOW.start) % rom.prg.size()];

        return true;
    }

    return false;
}

bool MMC0::write(uint16_t addr, uint8_t data) {
    if (rom.prg.size() > 0 && PRG_REGION.contains(addr)) {
        rom.prg[(addr - PRG_ROM_LOW.start) % rom.prg.size()] = data;

        return true;
    }

    return false;
}

bool MMC0::ppu_read(uint16_t addr, uint8_t& data) {
    // TODO
	return false;
}

bool MMC0::ppu_write(uint16_t addr, uint8_t data) {
    // TODO
	return false;
}
