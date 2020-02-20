#include "emulation/Console.h"
#include "log.h"

int Console::init(string romPath) {
    ROM rom;
    if (rom.init(romPath)) { return 1; }

    disassembler.init(rom.prg, PRG_ROM_LOW.start);

    shared_ptr<MMC> mmc;
    if (!(mmc = MMC::fromROM(rom))) { return 1; }

    ram.init();

    IORegs io;
    io.init();

    if (bus.init(ram, mmc, io)) { return 1; }
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
    return ram;
}

void Console::input(JoyPadInput joypad) {
    // TODO
}