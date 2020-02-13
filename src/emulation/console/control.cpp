#include <sstream>
#include <tuple>
#include <map>
#include <iterator>

#include "emulation/console.h"
#include "logger.h"

void Console::reset() {
    regs.pc = read16(RH);
    INFO("PC = memory[0xFFFC] = 0x%04X", regs.pc);
    
    regs.sp = 0xFD;
    regs.flags.byte = 0;
    regs.a = regs.x = regs.y = 0;

    vram[0x4015] = 0;
    cpuCycles += 8;
    INFO("done resetting");
}

void Console::powerOn() {
    // https://wiki.nesdev.com/w/index.php/CPU_power_up_state#At_power-up
    regs.pc = read16(RH);
    INFO("PC = memory[0xFFFC] = 0x%04X", regs.pc);

    regs.sp = 0xFD;
    regs.flags.byte = 0x34; // IRQ disabled
    regs.a = regs.x = regs.y = 0;

    cpuCycles = 0;
    INFO("intialized CPU");

    // TODO: All 15 bits of noise channel LFSR = $0000[4]. 
    //The first time the LFSR is clocked from the all-0s state, it will shift in a 1.

    // TODO: 2A03G: APU Frame Counter reset. 
    // (but 2A03letterless: APU frame counter powers up at a value equivalent to 15)

    // https://wiki.nesdev.com/w/index.php/PPU_power_up_state
    // TODO: set all ppu state
    INFO("initialized PPU");
}

void Console::oneCPUCycle() {
    if (cpuCycles > 0) {
        cpuCycles--;
    }

    auto& inst = instrucSet[fetch()];
    inst.exec();
    cpuCycles += inst.cpuCycles;
}

void Console::onePPUCycle(Renderer* renderer) {
    // TODO
}

void Console::oneAPUCycle() {
    // TODO
}
