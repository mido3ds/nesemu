#include <sstream>
#include <tuple>
#include <map>
#include <iterator>

#include "emulation/console.h"
#include "logger.h"

u8_t Console::read(u16_t address) {
    if (address == PPU_STS_REG) { return readPPUStatusRegister(); }
    if (address == VRAM_IO_REG) {
        if (vramAddrReg1.state & VRamAddrReg1::CAN_READ 
        || (vramAddrReg1.state & VRamAddrReg1::READ_BUFFERED && vramAddrReg1.addr >= IMG_PLT.start)) {
            auto v = vram[vramAddrReg1.addr];
            vramAddrReg1.addr += controlReg->getPPUIncrementRate();
            return v;
        }

        if (vramAddrReg1.state & VRamAddrReg1::READ_BUFFERED) {
            vramAddrReg1.state |= VRamAddrReg1::CAN_READ;
        }  
    }

    return memory[address];
}

void Console::write(u16_t address, u8_t value)  {
    memory[address] = value;

    // apply mirroring 
    for (auto mirror: MEM_MIRRORS) {
        for (auto mirrorAddr: mirror.getAdresses(address)) {
            memory[mirrorAddr] = value;
        }
    }

    if (address == SPRITE_DMA_REG) {
        // DMA from memory -> sprram
        memcpy(sprram.data(), memory.data() + 0x100 * u8_t(value), sprram.size());
    } else if (address == SPRRAM_IO_REG) {
        sprram[memory[SPRRAM_ADDR_REG]] = value;
    } else if (address == VRAM_ADDR_REG1) {
        if (vramAddrReg1.state & (VRamAddrReg1::EMPTY | VRamAddrReg1::FULL)) {
            vramAddrReg1.state = VRamAddrReg1::LSN;
            vramAddrReg1.addr = value;
        } else if (vramAddrReg1.state & VRamAddrReg1::LSN) {
            vramAddrReg1.state = VRamAddrReg1::FULL | VRamAddrReg1::CAN_WRITE | VRamAddrReg1::READ_BUFFERED;
            vramAddrReg1.addr |= value << 8;
        }
    }
}

void Console::push(u8_t v) {
    write(STACK.start | regs.sp, v);
    regs.sp--;
}

u8_t Console::pop() {
    regs.sp++;
    u8_t v = read(STACK.start | regs.sp);
    return v;
}

u16_t Console::zeroPageAddress(const u8_t bb) {
    return bb;
}

u16_t Console::indexedZeroPageAddress(const u8_t bb, const u8_t i) {
    return (bb+i) & 0xFF;
}

u16_t Console::absoluteAddress(const u8_t bb, const u8_t cc) {
    return cc << 8 | bb;
}

u16_t Console::indexedAbsoluteAddress(const u8_t bb, const u8_t cc, const u8_t i) {
    return absoluteAddress(bb, cc) + i;
}

u16_t Console::indirectAddress(const u8_t bb, const u8_t cc) {
    u16_t ccbb = absoluteAddress(bb, cc);
    return absoluteAddress(read(ccbb), read(ccbb+1));
}

u16_t Console::indexedIndirectAddress(const u8_t bb, const u8_t i) {
    return absoluteAddress(read(bb+i), read(bb+i+1));
}

u16_t Console::indirectIndexedAddress(const u8_t bb, const u8_t i) {
    return absoluteAddress(read(bb), read(bb+1)) + i;
}

u8_t Console::fetch() { return read(regs.pc++); }

u16_t Console::read16(u16_t address) { return read(address) | read(address+1) << 8; }
u16_t Console::pop16() { return pop() | pop() << 8; }
void Console::write16(u16_t address, u16_t v) { write(address, v & 255); write(address+1, (v >> 8) & 255); }
void Console::push16(u16_t v) { push(v & 255); push((v >> 8) & 255); }