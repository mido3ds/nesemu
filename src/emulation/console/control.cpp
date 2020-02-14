#include <sstream>
#include <tuple>
#include <map>
#include <iterator>

#include "emulation/console.h"
#include "emulation/instructions.h"
#include "logger.h"

int Console::init(ROM* rom) {
    if (rom) {
        int err = rom->copyToMemory(&memory);
        if (err != 0) {
            return err;
        }
    } else {
        INFO("no rom");
    }

    powerOn();

    INFO("finished initalizing Console device");
    return 0;
}

void Console::reset() {
    regs.pc = read16(RH);
    INFO("PC = memory[0xFFFC] = 0x%04X", regs.pc);
    
    regs.sp = 0xFD;
    regs.flags.byte = 0;
    regs.a = regs.x = regs.y = 0;

    vram[0x4015] = 0;
    cpuCycles += 8;
    INFO("done resetting");
}

void Console::powerOn() {
    // https://wiki.nesdev.com/w/index.php/CPU_power_up_state#At_power-up
    regs.pc = read16(RH);
    INFO("PC = memory[0xFFFC] = 0x%04X", regs.pc);

    regs.sp = 0xFD;
    regs.flags.byte = 0x34; // IRQ disabled
    regs.a = regs.x = regs.y = 0;

    cpuCycles = 0;
    INFO("intialized CPU");

    // TODO: All 15 bits of noise channel LFSR = $0000[4]. 
    //The first time the LFSR is clocked from the all-0s state, it will shift in a 1.

    // TODO: 2A03G: APU Frame Counter reset. 
    // (but 2A03letterless: APU frame counter powers up at a value equivalent to 15)

    // https://wiki.nesdev.com/w/index.php/PPU_power_up_state
    // TODO: set all ppu state
    INFO("initialized PPU");
}

// get arg given address mode of instruction
void Console::prepareArg(AddressMode mode) {
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
void Console::reprepareJMPArg() {
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

u8_t Console::getArgValue() {
    return argValue;
}

u16_t Console::getArgAddr() {
    return argAddr;
}

void Console::writeArg(u8_t v) {
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

void Console::noCrossPage() {
    cpp = false;
}

void Console::oneCPUCycle() {
    if (cpuCycles > 0) {
        cpuCycles--;
        return;
    }

    auto& inst = instructionSet[fetch()];

    prepareArg(inst.mode);
    auto oldpc = regs.pc;
    cpp = true;

    inst.exec(*this);

    cpuCycles += inst.cpuCycles;

    if (cpp && inst.crossPagePenalty == 1) {
        cpuCycles += (regs.pc>>8 == oldpc>>8) ? 0:1;
    }
}

void Console::onePPUCycle(Renderer* renderer) {
    // TODO
}

void Console::oneAPUCycle() {
    // TODO
}
