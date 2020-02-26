#include "emulation/Console.h"
#include "emulation/ROM.h"
#include "emulation/MMC.h"
#include "emulation/IORegs.h"
#include "log.h"

int Console::init(string romPath) {
    ROM rom;
    if (rom.init(romPath)) { return 1; }

    disassembler.init(rom.prg, PRG_ROM_LOW.start);

    // mmc
    auto mmc = MMC::fromROM(rom);
    if (!mmc) { return 1; }
    if (bus.attach(static_pointer_cast<ICPUBusAttachable>(mmc))) { return 1; }
    if (bus.attach(static_pointer_cast<IPPUBusAttachable>(mmc))) { return 1; }

    return init();
}

int Console::init() {
    // ram
    ram = make_shared<RAM>();
    ram->init();
    if (bus.attach(ram)) { return 1; }

    // io
    auto io = make_shared<IORegs>();
    io->init();
    if (bus.attach(io)) { return 1; }

    // ppu
    ppu = make_shared<PPU>();
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

Disassembler* Console::getDisassembler() {
    return &disassembler;
}

CPU* Console::getCPU() {
    return &cpu;
}

RAM* Console::getRAM() {
    return ram.get();
}

void Console::input(JoyPadInput joypad) {
    // TODO
}