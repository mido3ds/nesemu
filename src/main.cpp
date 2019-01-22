/* 
    Mahmoud Adas, 2019
    NES-6502 Emulator
*/

#include <cstdint>
#include <array>
#include <functional>
#include "Logger.h"

using namespace std;

class NES6502_DEVICE {
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
    array<function<void(uint8_t)>, 256> instrucSet;

    inline void execute(uint8_t opcode) {
        instrucSet[opcode](opcode);
    }

public:
    NES6502_DEVICE() {
        instrucSet.fill([](uint8_t opcode) {
            logError("invalid/unsupported opcode(0x%02x) called", opcode);
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
        execute(0x15);
    }
};

int main(int argc, char const *argv[])
{
    NES6502_DEVICE().run();
    return 0;
}
