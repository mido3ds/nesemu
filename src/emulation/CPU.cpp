#include "emulation/CPU.h"
#include "emulation/instructions.h"
#include "utils.h"

int CPU::init(Bus* bus) {
    if (!bus) { return 1; }
    this->bus = bus;

    // https://wiki.nesdev.com/w/index.php/CPU_power_up_state#At_power-up
    regs.pc = read16(RH);
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
    return 0;
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
    crossPagePenalty = true;

    inst.exec(*this);

    cycles += inst.cycles;

    if (crossPagePenalty && inst.crossPagePenalty == 1) {
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

uint8_t CPU::getArgValue() {
    return argValue;
}

uint16_t CPU::getArgAddr() {
    return argAddr;
}

void CPU::writeArg(uint8_t v) {
    switch (this->mode) {
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

uint8_t CPU::read(uint16_t address) {
    uint8_t data;
    bus->read(address, data);
    return data;
}

void CPU::write(uint16_t address, uint8_t value)  {
    bus->write(address, value);
}

void CPU::push(uint8_t v) {
    write(STACK.start | regs.sp, v);
    regs.sp--;
}

uint8_t CPU::pop() {
    regs.sp++;
    uint8_t v = read(STACK.start | regs.sp);
    return v;
}

uint16_t CPU::zeroPageAddress(const uint8_t bb) {
    return bb;
}

uint16_t CPU::indexedZeroPageAddress(const uint8_t bb, const uint8_t i) {
    return (bb+i) & 0xFF;
}

uint16_t CPU::absoluteAddress(const uint8_t bb, const uint8_t cc) {
    return cc << 8 | bb;
}

uint16_t CPU::indexedAbsoluteAddress(const uint8_t bb, const uint8_t cc, const uint8_t i) {
    return absoluteAddress(bb, cc) + i;
}

uint16_t CPU::indirectAddress(const uint8_t bb, const uint8_t cc) {
    uint16_t ccbb = absoluteAddress(bb, cc);
    return absoluteAddress(read(ccbb), read(ccbb+1));
}

uint16_t CPU::indexedIndirectAddress(const uint8_t bb, const uint8_t i) {
    return absoluteAddress(read((bb+i) & 0x00FF), read((bb+i+1) & 0x00FF));
}

uint16_t CPU::indirectIndexedAddress(const uint8_t bb, const uint8_t i) {
    return absoluteAddress(read(bb), read(bb+1)) + i;
}

uint8_t CPU::fetch() { return read(regs.pc++); }

uint16_t CPU::read16(uint16_t address) {
    uint16_t data;
    bus->read16(address, data);
    return data;
}

void CPU::write16(uint16_t address, uint16_t v) { bus->write16(address, v); }

uint16_t CPU::pop16() { return pop() | pop() << 8; }
void CPU::push16(uint16_t v) { push(v & 255); push((v >> 8) & 255); }
