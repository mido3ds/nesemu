#include <iterator>

#include "emulation/Console.h"
#include "emulation/instructions.h"
#include "emulation/ROM.h"
#include "emulation/IORegs.h"

void console_init(Console& self, const Str& rom_path) {
    self = {};

    if (!rom_path.empty()) {
        mmc0_load_rom(self.mmc0, rom_path);

        if (self.mmc0.rom.prg.size() == PRG_ROM_LOW.size()) {
            self.disassembler = disassembler_new(self.mmc0.rom.prg, PRG_ROM_UP.start);
        } else {
            self.disassembler = disassembler_new(self.mmc0.rom.prg, PRG_ROM_LOW.start);
        }
    }

    bus_attach_to_cpu(self.bus, &self.mmc0);
    bus_attach_to_cpu(self.bus, &self.ram);
    bus_attach_to_cpu(self.bus, &self.io);
    bus_attach_to_cpu(self.bus, &self.ppu);

    bus_attach_to_ppu(self.bus, &self.mmc0);

    self.ppu = ppu_new(&self.bus);
    self.cpu = cpu_new(&self.bus);
}

void console_reset(Console& self) {
    bus_reset(self.bus);
    cpu_reset(self.cpu);
    self.cycles = 0;
}

void console_clock(Console& self, IRenderer* renderer) {
    ppu_clock(self.ppu, renderer);

    // because ppu is 3x faster than cpu
    if (self.cycles % 3 == 0) {
        cpu_clock(self.cpu);
    }

    self.cycles++;
}

void console_input(Console& self, JoyPadInput joypad) {
    // TODO
}

Disassembler disassembler_new(const Vec<uint8_t>& data, uint16_t addr) {
    Disassembler self {};

    uint8_t const* mem = data.data();
    int size = data.size();

    while (size > 0) {
        int consumedBytes = 1;
        Str instr;
        Str a(memory::tmp()), b(memory::tmp());

        auto name = instruction_set[mem[0]].name;
        if (name == "???") {
            str_push(instr, "{:02X} ?????", mem[0]);
        } else {
            str_push(instr, name);
        }

        if (size >= 2) { str_push(a, "{:02X}", mem[1]); }
        else           { a = "??"; }

        if (size >= 3) { str_push(b, "{:02X}", mem[1]); }
        else           { b = "??"; }

        switch (instruction_set[mem[0]].mode ) {
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

        self.assembly[addr] = instr;

        size -= consumedBytes;
        mem  += consumedBytes;
        addr += consumedBytes;
    }

    return std::move(self);
}

Vec<Str> disassembler_get(const Disassembler& self, const uint16_t addr, const uint16_t n) {
    Vec<Str> ret(2*n+1, "$????: ???");

    auto tmpItr = self.assembly.lower_bound(addr);
    auto fItr = tmpItr; // forward itr, addr : end
    auto rItr = make_reverse_iterator(tmpItr); // reverse itr, addr-1 : rend

    // addr
    if (fItr != self.assembly.end()) {
        if (fItr->first == addr) {
            ret[n] = str_format("${:04X}: {}", fItr->first, fItr->second);
            fItr++;
        } else {
            ret[n] = str_format("${:04X}: ???", addr);
        }
    }

    // [addr+1..addr+n]
    for (auto i = n+1; fItr != self.assembly.end() && i < ret.size(); fItr++, i++) {
        ret[i] = str_format("${:04X}: {}", fItr->first, fItr->second);
    }

    // [addr-n..addr-1]
    for (auto i = n-1; rItr != self.assembly.rend() && i >= 0; rItr++, i--) {
        ret[i] = str_format("${:04X}: {}", rItr->first, rItr->second);
    }

    return ret;
}
