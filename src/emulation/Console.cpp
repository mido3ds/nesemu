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
    if (bus.attach(mmc)) { return 1; }

    // ram
    ram = make_shared<RAM>();
    ram->init();
    if (bus.attach(ram)) { return 1; }

    // io
    auto io = make_shared<IORegs>();
    io->init();
    if (bus.attach(io)) { return 1; }

    cpu.init(&bus);

    return 0;
}

void Console::reset() {
    bus.reset();
    cpu.reset();
}

void Console::clock() {
    cpu.clock();
}

Disassembler& Console::getDisassembler() {
    return disassembler;
}

CPU& Console::getCPU() {
    return cpu;
}

RAM& Console::getRAM() {
    return *ram;
}

void Console::input(JoyPadInput joypad) {
    // TODO
}