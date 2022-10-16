#include <memory>

#include "emulation/Console.h"
#include "emulation/ROM.h"
#include "emulation/MMC.h"
#include "emulation/IORegs.h"

void Console::init(StrView romPath) {
    ROM rom;
    rom.init(romPath);

    disassembler.init(rom.prg, PRG_ROM_LOW.start);
    if (rom.prg.size() == PRG_ROM_LOW.size()) {
        disassembler.init(rom.prg, PRG_ROM_UP.start);
    }

    // mmc
    auto mmc = MMC::fromROM(rom);
    bus.attach(std::static_pointer_cast<ICPUBusAttachable>(mmc));
    bus.attach(std::static_pointer_cast<IPPUBusAttachable>(mmc));

    init();
}

void Console::init() {
    // ram
    ram = std::make_shared<RAM>();
    ram->init();
    bus.attach(ram);

    // io
    auto io = std::make_shared<IORegs>();
    io->init();
    bus.attach(io);

    // ppu
    ppu = std::make_shared<PPU>();
    ppu->init(&bus);
    bus.attach(ppu);

    cpu.init(&bus);

    cycles = 0;
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
