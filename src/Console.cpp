#include <iterator>

#include "Console.h"
#include "ROM.h"

void console_init(Console& self, const Str& rom_path) {
    self = {};

    if (!rom_path.empty()) {
        rom_load(self.rom, rom_path);
    }

    self.ppu = ppu_new(&self);
    self.cpu = cpu_new(&self);
}

void console_reset(Console& self) {
    ppu_reset(self.ppu);
    cpu_reset(self.cpu);
    self.cycles = 0;
}

void console_clock(Console& self, Image* image) {
    ppu_clock(self.ppu, image);

    // because ppu is 3x faster than cpu
    if (self.cycles % 3 == 0) {
        cpu_clock(self.cpu);
    }

    self.cycles++;
}

void console_input(Console& self, JoyPadInput joypad) {
    // TODO
}
