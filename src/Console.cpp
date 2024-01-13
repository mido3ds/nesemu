#include "Console.h"

void console_init(Console& self, const mu::Str& rom_path) {
    self = {};

    if (!rom_path.empty()) {
        rom_from_ines_file(self.rom, rom_path);
        self.assembly = bytecodes_disassemble(self.rom.prg);
    }

    self.ppu = ppu_new(&self);
    self.cpu = cpu_new(&self);

    self.screen_buf = screenbuf_new(Config::resolution.w, Config::resolution.h);
}

void console_reset(Console& self) {
    ppu_reset(self.ppu);
    cpu_reset(self.cpu);
    self.cycles = 0;
}

void console_clock(Console& self) {
    ppu_clock(self.ppu);

    // because ppu is 3x faster than cpu
    if (self.cycles % 3 == 0) {
        cpu_clock(self.cpu);
    }

    self.cycles++;
}

void console_input(Console& self, JoyPadInput joypad) {
    // TODO
}
