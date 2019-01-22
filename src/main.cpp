/* 
    Mahmoud Adas, 2019
    NES-6502 Emulator
*/

#include <cstdint>
#include <array>
#include <string>
#include <cstdio>
#include <functional>
#include "Logger.h"

using namespace std;

class NES6502_DEVICE {
protected:

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

public:

    NES6502_DEVICE(string romPath) {
        instrucSet.fill([]() {
            logError("invalid/unsupported opcode called");
        });
    }

    uint8_t readMem(const uint16_t address) {
        return memory[address];
    }

    void writeMem(const uint16_t address, const uint8_t value) {
        memory[address] = value;
    }

    void reset() {
        logInfo("reset");
    }

    void run() {
        logInfo("run");
        reset();
    }
};

int main(int argc, char const *argv[]) {
    if (argc != 2) {
        printf("Usage: NesEmu /path/to/rom\n");
    } else {
        NES6502_DEVICE(argv[1]).run();
    }
}
