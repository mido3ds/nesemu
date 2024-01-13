#include "Console.h"

CPU cpu_new(Console* console) {
    CPU self {
        .console = console,
    };

    // https://wiki.nesdev.com/w/index.php/CPU_power_up_state#At_power-up
    self.regs.pc = cpu_read16(self, RH);
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
    self.regs.pc = cpu_read16(self, RH);
    mu::log_debug("PC = memory[0xFFFC] = 0x{:04X}", self.regs.pc);

    self.regs.sp = 0xFD;
    self.regs.flags.byte = 0;
    self.regs.a = self.regs.x = self.regs.y = 0;

    // vram[0x4015] = 0; TODO move to PPU
    self.cycles += 8;
}

// addressing modes for 6502, from Appendix E: http://www.nesdev.com/NESDoc.pdf
uint16_t _zero_page_adr(const uint8_t bb) { return bb; }
uint16_t _idx_zero_page_adr(const uint8_t bb, const uint8_t i) { return (bb+i) & 0xFF; }
uint16_t _abs_adr(const uint8_t bb, const uint8_t cc) { return cc << 8 | bb; }
uint16_t _idx_abs_adr(const uint8_t bb, const uint8_t cc, const uint8_t i) { return _abs_adr(bb, cc) + i; }

uint16_t _indirect_adr(CPU& self, const uint8_t bb, const uint8_t cc) {
    uint16_t ccbb = _abs_adr(bb, cc);
    return _abs_adr(cpu_read(self, ccbb), cpu_read(self, ccbb+1));
}

uint16_t _idx_indirect_adr(CPU& self, const uint8_t bb, const uint8_t i) {
    return _abs_adr(cpu_read(self, (bb+i) & 0x00FF), cpu_read(self, (bb+i+1) & 0x00FF));
}

uint16_t _indirect_idx_adr(CPU& self, const uint8_t bb, const uint8_t i) {
    return _abs_adr(cpu_read(self, bb), cpu_read(self, bb+1)) + i;
}

void cpu_clock(CPU& self) {
    if (self.cycles > 0) {
        self.cycles--;
        return;
    }

    auto& inst = instruction_set[cpu_fetch(self)];

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
        self.arg_value = cpu_fetch(self);
        break;
    case AddressMode::ZeroPage:
        self.arg_addr = _zero_page_adr(cpu_fetch(self));
        self.arg_value = cpu_read(self, self.arg_addr);
        break;
    case AddressMode::ZeroPageX:
        self.arg_addr = _idx_zero_page_adr(cpu_fetch(self), self.regs.x);
        self.arg_value = cpu_read(self, self.arg_addr);
        break;
    case AddressMode::ZeroPageY:
        self.arg_addr = _idx_zero_page_adr(cpu_fetch(self), self.regs.y);
        self.arg_value = cpu_read(self, self.arg_addr);
        break;
    case AddressMode::Absolute: {
        auto bb = cpu_fetch(self), cc = cpu_fetch(self);
        self.arg_addr = _abs_adr(bb, cc);
        self.arg_value = cpu_read(self, self.arg_addr);
        break;
    }
    case AddressMode::AbsoluteX: {
        auto bb = cpu_fetch(self), cc = cpu_fetch(self);
        self.arg_addr = _idx_abs_adr(bb, cc, self.regs.x);
        self.arg_value = cpu_read(self, self.arg_addr);
        break;
    }
    case AddressMode::AbsoluteY: {
        auto bb = cpu_fetch(self), cc = cpu_fetch(self);
        self.arg_addr = _idx_abs_adr(bb, cc, self.regs.y);
        self.arg_value = cpu_read(self, self.arg_addr);
        break;
    }
    case AddressMode::Indirect: {
        auto bb = cpu_fetch(self), cc = cpu_fetch(self);
        self.arg_addr = _indirect_adr(self, bb, cc);
        self.arg_value = cpu_read(self, self.arg_addr);
        break;
    }
    case AddressMode::IndexedIndirect:
        self.arg_addr = _idx_indirect_adr(self, cpu_fetch(self), self.regs.x);
        self.arg_value = cpu_read(self, self.arg_addr);
        break;
    case AddressMode::IndirectIndexed:
        self.arg_addr = _indirect_idx_adr(self, cpu_fetch(self), self.regs.y);
        self.arg_value = cpu_read(self, self.arg_addr);
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
void cpu_reprepare_jmp_arg(CPU& self) {
    auto fpc = self.regs.pc-2;
    if ((fpc & 0x00FF) != 0x00FF) { return; }

    auto a = cpu_read(self, fpc);
    auto b = cpu_read(self, fpc & 0xFF00);

    switch (self.mode) {
    case AddressMode::Absolute:
        self.arg_addr = _abs_adr(a, b);
        break;
    case AddressMode::Indirect:
        self.arg_addr = _indirect_adr(self, a, b);
        break;
    default:
        mu::log_error("not JMP address mode");
        return;
    }

    self.arg_value = cpu_read(self, self.arg_addr);
}

void cpu_write_arg(CPU& self, uint8_t v) {
    switch (self.mode) {
    case AddressMode::Accumulator:
        self.arg_value = self.regs.a;
        break;
    case AddressMode::Implicit:
    case AddressMode::Relative:
    case AddressMode::Immediate:
        break;
    default:
        cpu_write(self, self.arg_addr, v);
        break;
    }
}

uint8_t cpu_read(CPU& self, uint16_t address) {
    uint8_t data = 0;
    bool success = rom_read(self.console->rom, address, data)
        || ram_read(self.console->ram, address, data)
        || ppu_read(self.console->ppu, address, data);
    if (!success) {
       mu::log_warning("read from unregistered address 0x{:02X}", address);
    }
    return data;
}

uint16_t cpu_read16(CPU& self, uint16_t address) {
    return cpu_read(self, address) | cpu_read(self, address+1) << 8;
}

void cpu_write(CPU& self, uint16_t address, uint8_t data)  {
    bool success = rom_write(self.console->rom, address, data)
        || ram_write(self.console->ram, address, data)
        || ppu_read(self.console->ppu, address, data);
    if (!success) {
       mu::log_warning("write to unregistered address 0x{:02X}", address);
    }
}

void cpu_push(CPU& self, uint8_t v) {
    cpu_write(self, STACK.start | self.regs.sp, v);
    self.regs.sp--;
}

uint8_t cpu_pop(CPU& self) {
    self.regs.sp++;
    uint8_t v = cpu_read(self, STACK.start | self.regs.sp);
    return v;
}

uint8_t cpu_fetch(CPU& self) { return cpu_read(self, self.regs.pc++); }

uint16_t cpu_cpu_read16(CPU& self, uint16_t address) {
    return cpu_read(self, address) | cpu_read(self, address+1) << 8;
}

void cpu_write16(CPU& self, uint16_t address, uint16_t v) {
    cpu_write(self, address, v & 0x00FF);
    cpu_write(self, address+1, (v & 0xFF00) >> 8);
}

uint16_t cpu_pop16(CPU& self) {
    return cpu_pop(self) | cpu_pop(self) << 8;
}

void cpu_push16(CPU& self, uint16_t v) {
    cpu_push(self, v & 255); cpu_push(self, (v >> 8) & 255);
}
