#include <memory>

#include "emulation/Console.h"
#include "emulation/ROM.h"
#include "emulation/MMC.h"
#include "emulation/IORegs.h"

int Console::init(StrView romPath) {
    ROM rom;
    if (rom.init(romPath)) { return 1; }

    disassembler.init(rom.prg, PRG_ROM_LOW.start);
    if (rom.prg.size() == PRG_ROM_LOW.size()) {
        disassembler.init(rom.prg, PRG_ROM_UP.start);
    }

    // mmc
    auto mmc = MMC::fromROM(rom);
    if (!mmc) { return 1; }
    if (bus.attach(std::static_pointer_cast<ICPUBusAttachable>(mmc))) { return 1; }
    if (bus.attach(std::static_pointer_cast<IPPUBusAttachable>(mmc))) { return 1; }

    return init();
}

int Console::init() {
    // ram
    ram = std::make_shared<RAM>();
    ram->init();
    if (bus.attach(ram)) { return 1; }

    // io
    auto io = std::make_shared<IORegs>();
    io->init();
    if (bus.attach(io)) { return 1; }

    // ppu
    ppu = std::make_shared<PPU>();
    if (ppu->init(&bus)) { return 1; }
    if (bus.attach(ppu)) { return 1; }

    if (cpu.init(&bus)) { return 1; }

    cycles = 0;

    return 0;
}

void Console::reset() {
    bus.reset();
    cpu.reset();
    cycles = 0;
}

void Console::clock(IRenderer* renderer) {
    ppu->clock(renderer);

    // because ppu is 3x faster than cpu
    if (cycles % 3 == 0) {
        cpu.clock();
    }

    cycles++;
}

void Console::input(JoyPadInput joypad) {
    // TODO
}
