#include <sstream>
#include <tuple>
#include <map>
#include <iterator>

#include "console.h"
#include "logger.h"

int Console::reset() {
    logInfo("start resetting");

    regs.pc = read16(RH);
    regs.sp = 0xFD;
    regs.flags.byte = 0;
    regs.a = regs.x = regs.y = 0;

    vram[0x4015] = 0;
    cpuCycles += 8;

    return 0;
}

int Console::powerOn() {
    // https://wiki.nesdev.com/w/index.php/CPU_power_up_state#At_power-up
    logInfo("start powering on CPU");

    regs.pc = read16(RH);
    regs.sp = 0xFD;
    regs.flags.byte = 0x34; // IRQ disabled
    regs.a = regs.x = regs.y = 0;

    memory.fill(0);
    cpuCycles = 0;

    // TODO: All 15 bits of noise channel LFSR = $0000[4]. 
    //The first time the LFSR is clocked from the all-0s state, it will shift in a 1.

    // TODO: 2A03G: APU Frame Counter reset. 
    // (but 2A03letterless: APU frame counter powers up at a value equivalent to 15)

    // https://wiki.nesdev.com/w/index.php/PPU_power_up_state
    logInfo("start powering on ppu");
    // TODO: set all ppu state

    return 0;
}

int Console::oneCPUCycle() {
    if (cpuCycles > 0) {
        cpuCycles--;
        return 0;
    }

    auto& inst = instrucSet[fetch()];
    inst.exec();
    cpuCycles += inst.cpuCycles;

    return 0;
}

int Console::onePPUCycle(Renderer* renderer) {
    // TODO
    return 0;
}

int Console::oneAPUCycle() {
    // TODO
    return 0;
}
