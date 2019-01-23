/* 
    Mahmoud Adas, 2019
    NES-6502 Emulator
*/

#include <cstdint>
#include <cstdio>
#include <array>
#include <string>
#include <functional>
#include <chrono>
#include <thread>
#include "Logger.h"

using namespace std;

class NES6502_DEVICE {
protected:
    // approx time in nanoseconds for one cycle
    const int NTSC_CYCLE_NS  = 559;
    const int PAL_CYCLE_NS   = 601;
    const int DENDY_CYCLE_NS = 564;

    uint8_t memory[0xffff + 1];

    struct {
        uint16_t pc; // program counter
        uint8_t sp; // stack pointer
        uint8_t a; // accumulator
        uint8_t x; // index
        uint8_t y; // index
        union {
            struct {
                uint8_t c:1; // carry flag
                uint8_t z:1; // zero flag
                uint8_t i:1; // interrupt disable
                uint8_t d:1; // decimal mode
                uint8_t b:1; // break command
                uint8_t __unused__:1; 
                uint8_t v:1; // overflow flag
                uint8_t n:1; // negative flag
            } bits;
            uint8_t byte;
        } p; // processor status
    } registers; 

    // opcode -> function
    array<function<void()>, 256> instrucSet;

    uint32_t cycles;

public:

    NES6502_DEVICE() {
        logInfo("building NES6502 device NTSC");
        logInfo("filling instruction set functions");
        instrucSet.fill([]() {
            logError("invalid/unsupported opcode called");
        });

        /*http://obelisk.me.uk/6502/reference.html*/
        /*ADC*/ {
            instrucSet[0x69] = [this]() {
                
                cycles += 2;                
            };
            instrucSet[0x65] = [this]() {

                cycles += 3;
            };
            instrucSet[0x75] = [this]() {

                cycles += 4;
            };
            instrucSet[0x6D] = [this]() {

                cycles += 4;
            };
            instrucSet[0x7D] = [this]() {

                cycles += 4;
            };
            instrucSet[0x79] = [this]() {

                cycles += 4;
            };
            instrucSet[0x61] = [this]() {

                cycles += 6;
            };
            instrucSet[0x71] = [this]() {

                cycles += 5;
            };
        }
    }

    void setROM(string romPath) {
        //TODO
        logInfo("using rom: %s", romPath.c_str());
    }

    inline void burnCycles() {
        std::this_thread::sleep_for(std::chrono::nanoseconds(cycles * NTSC_CYCLE_NS));
        cycles = 0;
    }

    uint8_t readMem(const uint16_t address) {
        return memory[address];
    }

    void writeMem(const uint16_t address, const uint8_t value) {
        memory[address] = value;
    }

    void reset() {
        logInfo("reset");
        cycles = 0;
    }

    void powerOn() {
        logInfo("power on");
        cycles = 0;
    }
};

int main(int argc, char const *argv[]) {
    if (argc != 2) {
        printf("Usage: NesEmu /path/to/rom\n");
    } else {
        NES6502_DEVICE dev;
        dev.setROM(argv[1]);
        dev.powerOn();
    }
}
