#include <iterator>

#include "emulation/Console.h"
#include "emulation/instructions.h"
#include "emulation/ROM.h"
#include "emulation/IORegs.h"

void Console::init(StrView romPath) {
    mmc0.init(romPath);
    bus.attachToCPU(&mmc0);
    bus.attachToPPU(&mmc0);

    if (mmc0.rom.prg.size() == PRG_ROM_LOW.size()) {
        disassembler.init(mmc0.rom.prg, PRG_ROM_UP.start);
    } else {
        disassembler.init(mmc0.rom.prg, PRG_ROM_LOW.start);
    }

    init();
}

void Console::init() {
    ram.init();
    bus.attachToCPU(&ram);

    io.init();
    bus.attachToCPU(&io);

    ppu.init(&bus);
    bus.attachToCPU(&ppu);

    cpu.init(&bus);

    cycles = 0;
}

void Console::reset() {
    bus.reset();
    cpu.reset();
    cycles = 0;
}

void Console::clock(IRenderer* renderer) {
    ppu.clock(renderer);

    // because ppu is 3x faster than cpu
    if (cycles % 3 == 0) {
        cpu.clock();
    }

    cycles++;
}

void Console::input(JoyPadInput joypad) {
    // TODO
}

void Disassembler::init(const Vec<uint8_t>& data, uint16_t addr) {
    uint8_t const* mem = data.data();
    int size = data.size();

    while (size > 0) {
        int consumedBytes = 1;
        Str instr;
        Str a(memory::tmp()), b(memory::tmp());

        auto name = instructionSet[mem[0]].name;
        if (name == "???") {
            str_push(instr, "{:02X} ?????", mem[0]);
        } else {
            str_push(instr, name);
        }

        if (size >= 2) { str_push(a, "{:02X}", mem[1]); }
        else           { a = "??"; }

        if (size >= 3) { str_push(b, "{:02X}", mem[1]); }
        else           { b = "??"; }

        switch (instructionSet[mem[0]].mode ) {
        case AddressMode::Implicit:
            break;
        case AddressMode::Accumulator:
            str_push(instr, " A");
            break;
        case AddressMode::Immediate:
            str_push(instr, " #{}", a);
            consumedBytes++;
            break;
        case AddressMode::ZeroPage:
            str_push(instr, " ${}", a);
            consumedBytes++;
            break;
        case AddressMode::ZeroPageX:
            str_push(instr, " ${}, X", a);
            consumedBytes++;
            break;
        case AddressMode::ZeroPageY:
            str_push(instr, " ${}, Y", a);
            consumedBytes++;
            break;
        case AddressMode::Relative:
            if (size >= 2) { a.clear(); str_push(a, "{:+}", int8_t(mem[1])); }
            else           { a = "??"; }

            str_push(instr, " {}", a);
            consumedBytes++;
            break;
        case AddressMode::Absolute:
            str_push(instr, " ${}{}", b, a);
            consumedBytes += 2;
            break;
        case AddressMode::AbsoluteX:
            str_push(instr, " ${}{}, X", b, a);
            consumedBytes += 2;
            break;
        case AddressMode::AbsoluteY:
            str_push(instr, " ${}{}, Y", b, a);
            consumedBytes += 2;
            break;
        case AddressMode::Indirect:
            str_push(instr, " (${}{})", b, a);
            consumedBytes += 2;
            break;
        case AddressMode::IndexedIndirect:
            str_push(instr, " (${}, X)", a);
            consumedBytes++;
            break;
        case AddressMode::IndirectIndexed:
            str_push(instr, " (${}, Y)", a);
            consumedBytes++;
            break;
        default: unreachable();
        }

        assembly[addr] = instr;

        size -= consumedBytes;
        mem  += consumedBytes;
        addr += consumedBytes;
    }
}

Vec<Str> Disassembler::get(const uint16_t addr, const uint16_t n) const {
    Vec<Str> ret(2*n+1, "$????: ???");

    auto tmpItr = assembly.lower_bound(addr);
    auto fItr = tmpItr; // forward itr, addr : end
    auto rItr = make_reverse_iterator(tmpItr); // reverse itr, addr-1 : rend

    // addr
    if (fItr != assembly.end()) {
        if (fItr->first == addr) {
            ret[n] = str_format("${:04X}: {}", fItr->first, fItr->second);
            fItr++;
        } else {
            ret[n] = str_format("${:04X}: ???", addr);
        }
    }

    // [addr+1..addr+n]
    for (auto i = n+1; fItr != assembly.end() && i < ret.size(); fItr++, i++) {
        ret[i] = str_format("${:04X}: {}", fItr->first, fItr->second);
    }

    // [addr-n..addr-1]
    for (auto i = n-1; rItr != assembly.rend() && i >= 0; rItr++, i--) {
        ret[i] = str_format("${:04X}: {}", rItr->first, rItr->second);
    }

    return ret;
}
