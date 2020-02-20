#include "emulation/CPU.h"
#include "emulation/instructions.h"
#include "log.h"

void CPU::init(Bus* bus) {
    this->bus = bus;

    // https://wiki.nesdev.com/w/index.php/CPU_power_up_state#At_power-up
    // regs.pc = read16(RH); TODO
    INFO("PC = memory[0xFFFC] = 0x%04X", regs.pc);

    regs.sp = 0xFD;
    regs.flags.byte = 0x34; // IRQ disabled
    regs.a = regs.x = regs.y = 0;

    cycles = 0;

    // TODO: All 15 bits of noise channel LFSR = $0000[4]. 
    //The first time the LFSR is clocked from the all-0s state, it will shift in a 1.

    // TODO: 2A03G: APU Frame Counter reset. 
    // (but 2A03letterless: APU frame counter powers up at a value equivalent to 15)

    // https://wiki.nesdev.com/w/index.php/PPU_power_up_state
    // TODO: set all ppu state
}

void CPU::reset() {
    regs.pc = read16(RH);
    INFO("PC = memory[0xFFFC] = 0x%04X", regs.pc);
    
    regs.sp = 0xFD;
    regs.flags.byte = 0;
    regs.a = regs.x = regs.y = 0;

    // vram[0x4015] = 0; TODO move to PPU
    cycles += 8;
}

void CPU::clock() {
    if (cycles > 0) {
        cycles--;
        return;
    }

    auto& inst = instructionSet[fetch()];

    prepareArg(inst.mode);
    auto oldpc = regs.pc;
    cpp = true;

    inst.exec(*this);

    cycles += inst.cycles;

    if (cpp && inst.crossPagePenalty == 1) {
        cycles += (regs.pc>>8 == oldpc>>8) ? 0:1;
    }
}

void CPU::prepareArg(AddressMode mode) {
    // TODO: optimize using function instead 
    // of switch case
    switch (mode) {
    case AddressMode::Implicit:
        break;
    case AddressMode::Accumulator:
        argValue = regs.a;
        break;
    case AddressMode::Relative:
    case AddressMode::Immediate:
        argValue = fetch();
        break;
    case AddressMode::ZeroPage:
        argAddr = zeroPageAddress(fetch());
        argValue = read(argAddr);
        break;
    case AddressMode::ZeroPageX:
        argAddr = indexedZeroPageAddress(fetch(), regs.x);
        argValue = read(argAddr);
        break;
    case AddressMode::ZeroPageY:
        argAddr = indexedZeroPageAddress(fetch(), regs.y);
        argValue = read(argAddr);
        break;
    case AddressMode::Absolute: {
        auto bb = fetch(), cc = fetch();
        argAddr = absoluteAddress(bb, cc);
        argValue = read(argAddr);
        break;
    }
    case AddressMode::AbsoluteX: {
        auto bb = fetch(), cc = fetch();
        argAddr = indexedAbsoluteAddress(bb, cc, regs.x);
        argValue = read(argAddr);
        break;
    }
    case AddressMode::AbsoluteY: {
        auto bb = fetch(), cc = fetch();
        argAddr = indexedAbsoluteAddress(bb, cc, regs.y);
        argValue = read(argAddr);
        break;
    }
    case AddressMode::Indirect: {
        auto bb = fetch(), cc = fetch();
        argAddr = indirectAddress(bb, cc);
        argValue = read(argAddr);
        break;
    }
    case AddressMode::IndexedIndirect:
        argAddr = indexedIndirectAddress(fetch(), regs.x);
        argValue = read(argAddr);
        break;
    case AddressMode::IndirectIndexed:
        argAddr = indirectIndexedAddress(fetch(), regs.y);
        argValue = read(argAddr);
        break;
    }

    this->mode = mode;
}

// TODO: is this only for JMP?
void CPU::reprepareJMPArg() {
    auto fpc = regs.pc-2;
    if ((fpc & 0x00FF) != 0x00FF) { return; }

    auto a = read(fpc);
    auto b = read(fpc & 0xFF00);

    switch (mode) {
    case AddressMode::Absolute:
        argAddr = absoluteAddress(a, b);
        break;
    case AddressMode::Indirect:
        argAddr = indirectAddress(a, b);
        break;
    default:
        ERROR("not JMP address mode");
        return;
    }

    argValue = read(argAddr);
}

u8_t CPU::getArgValue() {
    return argValue;
}

u16_t CPU::getArgAddr() {
    return argAddr;
}

void CPU::writeArg(u8_t v) {
    switch (mode) {
    case AddressMode::Accumulator:
        argValue = regs.a;
        break;
    case AddressMode::Implicit:
    case AddressMode::Relative:
    case AddressMode::Immediate:
        break;
    default:
        write(argAddr, v);
        break;
    }
}

void CPU::noCrossPage() {
    cpp = false;
}

u8_t CPU::read(u16_t address) {
    u8_t data;
    bus->read(address, data);
    return data;
}

void CPU::write(u16_t address, u8_t value)  {
    bus->write(address, value);
}

void CPU::push(u8_t v) {
    write(STACK.start | regs.sp, v);
    regs.sp--;
}

u8_t CPU::pop() {
    regs.sp++;
    u8_t v = read(STACK.start | regs.sp);
    return v;
}

u16_t CPU::zeroPageAddress(const u8_t bb) {
    return bb;
}

u16_t CPU::indexedZeroPageAddress(const u8_t bb, const u8_t i) {
    return (bb+i) & 0xFF;
}

u16_t CPU::absoluteAddress(const u8_t bb, const u8_t cc) {
    return cc << 8 | bb;
}

u16_t CPU::indexedAbsoluteAddress(const u8_t bb, const u8_t cc, const u8_t i) {
    return absoluteAddress(bb, cc) + i;
}

u16_t CPU::indirectAddress(const u8_t bb, const u8_t cc) {
    u16_t ccbb = absoluteAddress(bb, cc);
    return absoluteAddress(read(ccbb), read(ccbb+1));
}

u16_t CPU::indexedIndirectAddress(const u8_t bb, const u8_t i) {
    return absoluteAddress(read((bb+i) & 0x00FF), read((bb+i+1) & 0x00FF));
}

u16_t CPU::indirectIndexedAddress(const u8_t bb, const u8_t i) {
    return absoluteAddress(read(bb), read(bb+1)) + i;
}

u8_t CPU::fetch() { return read(regs.pc++); }

u16_t CPU::read16(u16_t address) { 
    u16_t data;
    bus->read16(address, data);
    return data;
}

void CPU::write16(u16_t address, u16_t v) { bus->write16(address, v); }

u16_t CPU::pop16() { return pop() | pop() << 8; }
void CPU::push16(u16_t v) { push(v & 255); push((v >> 8) & 255); }

CPURegs CPU::getRegs() {
    return regs;
}

u16_t CPU::getCycles() {
    return cycles;
}