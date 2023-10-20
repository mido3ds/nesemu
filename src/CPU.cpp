#include "CPU.h"
#include "Console.h"
#include "instructions.h"
#include <mu/utils.h>

CPU cpu_new(Console* console) {
    CPU self {};

    self.console = console;
    my_assert(console);

    // https://wiki.nesdev.com/w/index.php/CPU_power_up_state#At_power-up
    self.regs.pc = self.read16(RH);
    mu::log_debug("PC = memory[0xFFFC] = 0x{:04X}", self.regs.pc);

    self.regs.sp = 0xFD;
    self.regs.flags.byte = 0x34; // IRQ disabled
    self.regs.a = self.regs.x = self.regs.y = 0;

    // TODO: All 15 bits of noise channel LFSR = $0000[4].
    //The first time the LFSR is clocked from the all-0s state, it will shift in a 1.

    // TODO: 2A03G: APU Frame Counter reset.
    // (but 2A03letterless: APU frame counter powers up at a value equivalent to 15)

    // https://wiki.nesdev.com/w/index.php/PPU_power_up_state
    // TODO: set all ppu state

    return self;
}

void cpu_reset(CPU& self) {
    self.regs.pc = self.read16(RH);
    mu::log_debug("PC = memory[0xFFFC] = 0x{:04X}", self.regs.pc);

    self.regs.sp = 0xFD;
    self.regs.flags.byte = 0;
    self.regs.a = self.regs.x = self.regs.y = 0;

    // vram[0x4015] = 0; TODO move to PPU
    self.cycles += 8;
}

void cpu_clock(CPU& self) {
    if (self.cycles > 0) {
        self.cycles--;
        return;
    }

    auto& inst = instruction_set[self.fetch()];

    // prepare arg
    // TODO: optimize using function instead
    // of switch case
    self.mode = inst.mode;
    switch (self.mode) {
    case AddressMode::Implicit:
        break;
    case AddressMode::Accumulator:
        self.arg_value = self.regs.a;
        break;
    case AddressMode::Relative:
    case AddressMode::Immediate:
        self.arg_value = self.fetch();
        break;
    case AddressMode::ZeroPage:
        self.arg_addr = self.zero_page_address(self.fetch());
        self.arg_value = self.read(self.arg_addr);
        break;
    case AddressMode::ZeroPageX:
        self.arg_addr = self.indexed_zero_page_address(self.fetch(), self.regs.x);
        self.arg_value = self.read(self.arg_addr);
        break;
    case AddressMode::ZeroPageY:
        self.arg_addr = self.indexed_zero_page_address(self.fetch(), self.regs.y);
        self.arg_value = self.read(self.arg_addr);
        break;
    case AddressMode::Absolute: {
        auto bb = self.fetch(), cc = self.fetch();
        self.arg_addr = self.absolute_address(bb, cc);
        self.arg_value = self.read(self.arg_addr);
        break;
    }
    case AddressMode::AbsoluteX: {
        auto bb = self.fetch(), cc = self.fetch();
        self.arg_addr = self.indexed_absolute_address(bb, cc, self.regs.x);
        self.arg_value = self.read(self.arg_addr);
        break;
    }
    case AddressMode::AbsoluteY: {
        auto bb = self.fetch(), cc = self.fetch();
        self.arg_addr = self.indexed_absolute_address(bb, cc, self.regs.y);
        self.arg_value = self.read(self.arg_addr);
        break;
    }
    case AddressMode::Indirect: {
        auto bb = self.fetch(), cc = self.fetch();
        self.arg_addr = self.indirect_address(bb, cc);
        self.arg_value = self.read(self.arg_addr);
        break;
    }
    case AddressMode::IndexedIndirect:
        self.arg_addr = self.indexed_indirect_address(self.fetch(), self.regs.x);
        self.arg_value = self.read(self.arg_addr);
        break;
    case AddressMode::IndirectIndexed:
        self.arg_addr = self.indirect_indexed_address(self.fetch(), self.regs.y);
        self.arg_value = self.read(self.arg_addr);
        break;
    }

    const auto oldpc = self.regs.pc;
    self.cross_page_penalty = true;

    inst.exec(self);

    self.cycles += inst.cycles;

    if (self.cross_page_penalty && inst.cross_page_penalty == 1) {
        self.cycles += (self.regs.pc>>8 == oldpc>>8) ? 0:1;
    }
}

// TODO: is this only for JMP?
void CPU::reprepare_jmp_arg() {
    auto fpc = regs.pc-2;
    if ((fpc & 0x00FF) != 0x00FF) { return; }

    auto a = read(fpc);
    auto b = read(fpc & 0xFF00);

    switch (mode) {
    case AddressMode::Absolute:
        arg_addr = absolute_address(a, b);
        break;
    case AddressMode::Indirect:
        arg_addr = indirect_address(a, b);
        break;
    default:
        mu::log_error("not JMP address mode");
        return;
    }

    arg_value = read(arg_addr);
}

void CPU::write_arg(uint8_t v) {
    switch (this->mode) {
    case AddressMode::Accumulator:
        arg_value = regs.a;
        break;
    case AddressMode::Implicit:
    case AddressMode::Relative:
    case AddressMode::Immediate:
        break;
    default:
        write(arg_addr, v);
        break;
    }
}

uint8_t CPU::read(uint16_t address) {
    uint8_t data = 0;
    bool success = rom_read(console->rom, address, data)
        || ram_read(console->ram, address, data)
        || ppu_read(console->ppu, address, data);
    if (!success) {
       mu::log_warning("read from unregistered address 0x{:02X}", address);
    }
    return data;
}

void CPU::write(uint16_t address, uint8_t data)  {
    bool success = rom_write(console->rom, address, data)
        || ram_write(console->ram, address, data)
        || ppu_read(console->ppu, address, data);
    if (!success) {
       mu::log_warning("write to unregistered address 0x{:02X}", address);
    }
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

uint16_t CPU::zero_page_address(const uint8_t bb) {
    return bb;
}

uint16_t CPU::indexed_zero_page_address(const uint8_t bb, const uint8_t i) {
    return (bb+i) & 0xFF;
}

uint16_t CPU::absolute_address(const uint8_t bb, const uint8_t cc) {
    return cc << 8 | bb;
}

uint16_t CPU::indexed_absolute_address(const uint8_t bb, const uint8_t cc, const uint8_t i) {
    return absolute_address(bb, cc) + i;
}

uint16_t CPU::indirect_address(const uint8_t bb, const uint8_t cc) {
    uint16_t ccbb = absolute_address(bb, cc);
    return absolute_address(read(ccbb), read(ccbb+1));
}

uint16_t CPU::indexed_indirect_address(const uint8_t bb, const uint8_t i) {
    return absolute_address(read((bb+i) & 0x00FF), read((bb+i+1) & 0x00FF));
}

uint16_t CPU::indirect_indexed_address(const uint8_t bb, const uint8_t i) {
    return absolute_address(read(bb), read(bb+1)) + i;
}

uint8_t CPU::fetch() { return read(regs.pc++); }

uint16_t CPU::read16(uint16_t address) {
    return read(address) | read(address+1) << 8;
}

void CPU::write16(uint16_t address, uint16_t v) {
    write(address, v & 0x00FF);
    write(address+1, (v & 0xFF00) >> 8);
}

uint16_t CPU::pop16() { return pop() | pop() << 8; }
void CPU::push16(uint16_t v) { push(v & 255); push((v >> 8) & 255); }
