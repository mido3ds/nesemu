// #include <sstream>
// #include <tuple>
// #include <map>
// #include <iterator>

// #include "emulation/console.h"
// #include "log.h"

// TODO: move those to ppu and dma

// u8_t Console::read(u16_t address) {
//     if (address == PPU_STS_REG) { return readPPUStatusRegister(); }
//     if (address == VRAM_IO_REG) {
//         if (vramAddrReg1.state & VRamAddrReg1::CAN_READ
//         || (vramAddrReg1.state & VRamAddrReg1::READ_BUFFERED && vramAddrReg1.addr >= IMG_PLT.start)) {
//             auto v = vram[vramAddrReg1.addr];
//             vramAddrReg1.addr += controlReg->getPPUIncrementRate();
//             return v;
//         }

//         if (vramAddrReg1.state & VRamAddrReg1::READ_BUFFERED) {
//             vramAddrReg1.state |= VRamAddrReg1::CAN_READ;
//         }
//     }

//     return memory[address];
// }

// void Console::write(u16_t address, u8_t value)  {
//     memory[address] = value;

//     // apply mirroring
//     for (auto mirror: MEM_MIRRORS) {
//         for (auto mirrorAddr: mirror.getAdresses(address)) {
//             memory[mirrorAddr] = value;
//         }
//     }

//     if (address == SPRITE_DMA_REG) {
//         // DMA from memory -> sprram
//         memcpy(sprram.data(), memory.data() + 0x100 * u8_t(value), sprram.size());
//     } else if (address == SPRRAM_IO_REG) {
//         sprram[memory[SPRRAM_ADDR_REG]] = value;
//     } else if (address == VRAM_ADDR_REG1) {
//         if (vramAddrReg1.state & (VRamAddrReg1::EMPTY | VRamAddrReg1::FULL)) {
//             vramAddrReg1.state = VRamAddrReg1::LSN;
//             vramAddrReg1.addr = value;
//         } else if (vramAddrReg1.state & VRamAddrReg1::LSN) {
//             vramAddrReg1.state = VRamAddrReg1::FULL | VRamAddrReg1::CAN_WRITE | VRamAddrReg1::READ_BUFFERED;
//             vramAddrReg1.addr |= value << 8;
//         }
//     }
// }

// u16_t Console::readVRam16(u16_t address) { return readVRam(address) | readVRam(address+1) << 8; }
// void Console::writeVRam16(u16_t address, u16_t v) { writeVRam(address, v & 255); writeVRam(address+1, (v >> 8) & 255); }

// u8_t Console::readVRam(u16_t address) {
//     return vram[address];
// }

// void Console::writeVRam(u16_t address, u8_t value) {
//     vram[address] = value;

//     // apply mirroring
//     for (auto mirror: VRAM_MIRRORS) {
//         for (auto mirrorAddr: mirror.getAdresses(address)) {
//             vram[mirrorAddr] = value;
//         }
//     }

//     if (address == 0x3F00) {
//         for (auto& mirrorAddr: {0x3F04, 0x3F08, 0x3F0C, 0x3F10, 0x3F14, 0x3F18, 0x3F1C}) {
//             vram[mirrorAddr] = value;
//         }
//     }
// }

// u16_t Console::getSpriteAddr(SpriteInfo* inf, SpriteType type) {
//     if (type == SpriteType::S8x8) return PATT_TBL0.start + inf->i * SPRITE_8x8_SIZE;
//     if (inf->i % 2 == 0) return PATT_TBL0.start + inf->i * SPRITE_8x16_SIZE;
//     return PATT_TBL1.start + inf->i * SPRITE_8x16_SIZE;
// }

// u8_t Console::readPPUStatusRegister() {
//     memory[VRAM_ADDR_REG0] = memory[VRAM_ADDR_REG1] = 0;
//     u8_t old = ppuStatusReg->byte;
//     ppuStatusReg->byte &= ~(1 << 4);
//     return old;
// }
