#include <sstream>
#include <tuple>
#include <map>
#include <iterator>

#include "console.h"
#include "logger.h"

u16_t Console::readVRam16(u16_t address) { return readVRam(address) | readVRam(address+1) << 8; }
void Console::writeVRam16(u16_t address, u16_t v) { writeVRam(address, v & 255); writeVRam(address+1, (v >> 8) & 255); }

u8_t Console::readVRam(u16_t address) {
    return vram[address];
}

void Console::writeVRam(u16_t address, u8_t value) {
    vram[address] = value;

    // apply mirroring 
    for (auto mirror: VRAM_MIRRORS) {
        for (auto mirrorAddr: mirror.getAdresses(address)) {
            vram[mirrorAddr] = value;
        }
    }

    if (address == 0x3F00) {
        for (auto& mirrorAddr: {0x3F04, 0x3F08, 0x3F0C, 0x3F10, 0x3F14, 0x3F18, 0x3F1C}) {
            vram[mirrorAddr] = value;
        }
    }
}

u16_t Console::getSpriteAddr(SpriteInfo* inf, SpriteType type) {
    if (type == SpriteType::S8x8) return PATT_TBL0.start + inf->i * SPRITE_8x8_SIZE;
    if (inf->i % 2 == 0) return PATT_TBL0.start + inf->i * SPRITE_8x16_SIZE;
    return PATT_TBL1.start + inf->i * SPRITE_8x16_SIZE;
}

u8_t Console::readPPUStatusRegister() {
    memory[VRAM_ADDR_REG0] = memory[VRAM_ADDR_REG1] = 0;
    u8_t old = ppuStatusReg->byte;
    ppuStatusReg->byte &= ~(1 << 4);
    return old;
}