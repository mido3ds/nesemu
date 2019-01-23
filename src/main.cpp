/* 
    Mahmoud Adas, 2019
    NES-6502 Emulator
*/

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <array>
#include <string>
#include <functional>
#include <chrono>
#include <thread>
#include "Logger.h"

using namespace std;

#define int8_sign(n) n>>7
#define int16_sign(n) n>>15
#define int32_sign(n) n>>31
#define int64_sign(n) n>>63

struct Color {
    uint8_t r, g, b;
};

class NES6502_DEVICE {
protected:
    // approx time in nanoseconds for one cycle
    constexpr static int NTSC_CYCLE_NS  = 559;
    constexpr static int PAL_CYCLE_NS   = 601;
    constexpr static int DENDY_CYCLE_NS = 564; // TODO: detect video system

    constexpr static Color DEFAULT_COLOR = Color({0, 0, 0});

    array<uint8_t, UINT16_MAX+1> memory;

    struct Registers {
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
    } regs; 

    struct Instruction {
        function<void()> exec;
        uint8_t bytes;
        uint16_t cycles;
    };

    // opcode -> instruction data
    array<Instruction, UINT8_MAX+1> instrucSet;

    uint32_t cycles; // TODO: cycles++ if page crossed

public:

    NES6502_DEVICE() {
        logInfo("started building NES6502 device");

        /*http://obelisk.me.uk/6502/reference.html*/
        logInfo("filling instruction set data");
        
        instrucSet.fill({[this]() {
            logError("invalid/unsupported opcode(0x%02x) called", readMem(regs.pc));
        },0,0});

        /*ADC*/ {
            auto adc = [this](uint8_t v) {
                regs.p.bits.c = (v == UINT8_MAX && regs.p.bits.c) 
                             || (v + regs.p.bits.c) > UINT8_MAX - regs.a; 
                regs.p.bits.v = int8_sign(v) == int8_sign(regs.a) 
                             && int8_sign(v+regs.a+regs.p.bits.c) != int8_sign(v); 

                regs.a += v + regs.p.bits.c;

                regs.p.bits.z = regs.a == 0;
                regs.p.bits.n = regs.a >> 7;
            };
            instrucSet[0x69] = {[this,&adc]() {
                adc(readMem(regs.pc+1));  
            }, 2, 2};
            instrucSet[0x65] = {[this,&adc]() {
                adc(readMem(zeroPageAddress(readMem(regs.pc+1))));
            }, 2, 3};
            instrucSet[0x75] = {[this,&adc]() {
                adc(readMem(indexedZeroPageAddress(readMem(regs.pc+1), regs.x)));
            }, 2, 4};
            instrucSet[0x6D] = {[this,&adc]() {
                adc(readMem(absoluteAddress(readMem(regs.pc+1), readMem(regs.pc+2))));
            }, 3, 4};
            instrucSet[0x7D] = {[this,&adc]() {
                adc(readMem(indexedAbsoluteAddress(readMem(regs.pc+1), readMem(regs.pc+2), regs.x)));
            }, 3, 4};
            instrucSet[0x79] = {[this,&adc]() {
                adc(readMem(indexedAbsoluteAddress(readMem(regs.pc+1), readMem(regs.pc+2), regs.y)));
            }, 3, 4};
            instrucSet[0x61] = {[this,&adc]() {
                adc(readMem(indexedIndirectAddress(readMem(regs.pc+1), regs.x)));
            }, 2, 6};
            instrucSet[0x71] = {[this,&adc]() {
                adc(readMem(indirectIndexedAddress(readMem(regs.pc+1), regs.y)));
            }, 2, 5};
        }

        /*AND*/ {
            auto and = [this](uint8_t v) {
                regs.a |= v;
                regs.p.bits.z = regs.a == 0;
                regs.p.bits.n = regs.a >> 7;
            };
            instrucSet[0x29] = {[this,&and]() {
                and(readMem(regs.pc+1));
            }, 2, 2};
            instrucSet[0x25] = {[this,&and]() {
                and(readMem(zeroPageAddress(readMem(regs.pc+1))));
            }, 2, 3};
            instrucSet[0x35] = {[this,&and]() {
                and(readMem(indexedZeroPageAddress(readMem(regs.pc+1), regs.x)));
            }, 2, 4};
            instrucSet[0x2D] = {[this,&and]() {
                and(readMem(absoluteAddress(readMem(regs.pc+1), readMem(regs.pc+2))));
            }, 3, 4};
            instrucSet[0x3D] = {[this,&and]() {
                and(readMem(indexedAbsoluteAddress(readMem(regs.pc+1), readMem(regs.pc+2), regs.x)));
            }, 3, 4};
            instrucSet[0x39] = {[this,&and]() {
                and(readMem(indexedAbsoluteAddress(readMem(regs.pc+1), readMem(regs.pc+2), regs.y)));
            }, 3, 4};
            instrucSet[0x21] = {[this,&and]() {
                and(readMem(indexedIndirectAddress(readMem(regs.pc+1), regs.x)));
            }, 2, 6};
            instrucSet[0x31] = {[this,&and]() {
                and(readMem(indirectIndexedAddress(readMem(regs.pc+1), regs.y)));
            }, 2, 5};
        }

        /*ASL*/ {
            auto asl = [this](uint8_t v) -> uint8_t {
                regs.p.bits.c = v >> 7;
                v <<= 1;
                regs.p.bits.z = v == 0; // TODO: not sure if Accumulator only or any value
                regs.p.bits.n = v >> 7;

                return v;
            };
            instrucSet[0x0A] = {[this,&asl]() {
                regs.a = asl(regs.a);
            }, 1, 2};
            instrucSet[0x06] = {[this,&asl]() {
                auto addr = zeroPageAddress(readMem(regs.pc+1));
                writeMem(addr, asl(readMem(addr)));
            }, 2, 5};
            instrucSet[0x16] = {[this,&asl]() {
                auto addr = indexedZeroPageAddress(readMem(regs.pc+1), regs.x);
                writeMem(addr, asl(readMem(addr)));
            }, 2, 6};
            instrucSet[0x0E] = {[this,&asl]() {
                auto addr = absoluteAddress(readMem(regs.pc+1), readMem(regs.pc+2));
                writeMem(addr, asl(readMem(addr)));
            }, 3, 6};
            instrucSet[0x1E] = {[this,&asl]() {
                auto addr = indexedAbsoluteAddress(readMem(regs.pc+1), readMem(regs.pc+2), regs.x);
                writeMem(addr, asl(readMem(addr)));
            }, 3, 7};
        }

        /*BCC*/ {
            instrucSet[0x90] = {[this]() {
                if (!regs.p.bits.c) {
                    regs.pc += (int8_t)readMem(regs.pc+1);
                    cycles++;
                }
            }, 2, 2};
        }

        /*BCS*/ {
            instrucSet[0xB0] = {[this]() {
                if (regs.p.bits.c) {
                    regs.pc += (int8_t)readMem(regs.pc+1);
                    cycles++;
                }
            }, 2, 2};
        }

        /*BEQ*/ {
            instrucSet[0xF0] = {[this]() {
                if (regs.p.bits.z) {
                    regs.pc += (int8_t)readMem(regs.pc+1);
                    cycles++;
                }
            }, 2, 2};
        }

        /*BIT*/ {
            auto bit = [this](uint8_t v) {
                regs.p.bits.z = v & regs.a == 0;
                regs.p.bits.v = v >> 6;
                regs.p.bits.n = v >> 7;
            };
            instrucSet[0x24] = {[this,&bit]() {
                bit(readMem(zeroPageAddress(readMem(regs.pc+1))));
            }, 2, 3};
            instrucSet[0x2C] = {[this,&bit]() {
                bit(readMem(absoluteAddress(readMem(regs.pc+1), readMem(regs.pc+2))));
            }, 3, 4};
        }

        /*BMI*/ {
            instrucSet[0x30] = {[this]() {
                if (regs.p.bits.n) {
                    regs.pc += (int8_t)readMem(regs.pc+1);
                    cycles++;
                }
            }, 2, 2};
        }

        /*BNE*/ {
            instrucSet[0xD0] = {[this]() {
                if (!regs.p.bits.z) {
                    regs.pc += (int8_t)readMem(regs.pc+1);
                    cycles++;
                }
            }, 2, 2};
        }

        /*BPL*/ {
            instrucSet[0x10] = {[this]() {
                if (!regs.p.bits.n) {
                    regs.pc += (int8_t)readMem(regs.pc+1);
                    cycles++;
                }
            }, 2, 2};
        }

        logInfo("finished building NES6502 device");
    }

    /*addressing modes for 6502
    from Appendix E: http://www.nesdev.com/NESDoc.pdf
    */
    inline uint16_t zeroPageAddress(const uint8_t bb) {
        return bb;
    }

    // indexed
    inline uint16_t indexedZeroPageAddress(const uint8_t bb, const uint8_t i) {
        return (bb+i) % 0xFF;
    }

    inline uint16_t absoluteAddress(const uint8_t bb, const uint8_t cc) {
        return cc << 8 | bb;
    }

    // indexed
    inline uint16_t indexedAbsoluteAddress(const uint8_t bb, const uint8_t cc, const uint8_t i) {
        return absoluteAddress(bb, cc) + i;
    }

    inline uint16_t indirectAddress(const uint8_t bb, const uint8_t cc) {
        uint16_t ccbb = absoluteAddress(bb, cc);
        return absoluteAddress(readMem(ccbb), readMem(ccbb+1));
    }

    inline uint16_t indexedIndirectAddress(const uint8_t bb, const uint8_t i) {
       return absoluteAddress(readMem(bb+i), readMem(bb+i+1));
    }

    inline uint16_t indirectIndexedAddress(const uint8_t bb, const uint8_t i) {
        return absoluteAddress(readMem(bb), readMem(bb+1)) + i;
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
        logInfo("start resetting");
        cycles = 0;

        /*https://wiki.nesdev.com/w/index.php/CPU_ALL#After_reset*/
        regs.sp -= 3;
        regs.p.bits.i = 1;
        //TODO: apuMemory[0x4015] = 0;

        logInfo("finished resetting");
    }

    void powerOn() {
        logInfo("start powering on");
        cycles = 0;

        memory.fill(0);
        memset(&regs, 0, sizeof regs);

        /*https://wiki.nesdev.com/w/index.php/CPU_ALL#At_power-up*/
        regs.p.byte = 0x34;
        regs.sp = 0xFD;

        //TODO: All 15 bits of noise channel LFSR = $0000[4]. 
        //The first time the LFSR is clocked from the all-0s state, it will shift in a 1.

        powerOnApu();

        logInfo("finished powering on");
    }

    inline void powerOnApu() {
        logInfo("start powering on apu");

        //TODO

        logInfo("finished powering on apu");
    }

    // nes color palatte -> RGB color
    static Color palatteToColor(const uint8_t palatte) {
        if (palatte > 0x3F) {
            logError("invalid palatte color(0x%02x), returning default color", palatte);
            return DEFAULT_COLOR;
        }

        static constexpr array<Color, 0x3F + 1> colorPalatte = {
            Color({0x75, 0x75, 0x75}),
            Color({0x27, 0x1B, 0x8F}),
            Color({0x00, 0x00, 0xAB}),
            Color({0x47, 0x00, 0x9F}),
            Color({0x8F, 0x00, 0x77}),
            Color({0xAB, 0x00, 0x13}),
            Color({0xA7, 0x00, 0x00}),
            Color({0x7F, 0x0B, 0x00}),
            Color({0x43, 0x2F, 0x00}),
            Color({0x00, 0x47, 0x00}),
            Color({0x00, 0x51, 0x00}),
            Color({0x00, 0x3F, 0x17}),
            Color({0x1B, 0x3F, 0x5F}),
            Color({0x00, 0x00, 0x00}),
            Color({0x00, 0x00, 0x00}),
            Color({0x00, 0x00, 0x00}),
            Color({0xBC, 0xBC, 0xBC}),
            Color({0x00, 0x73, 0xEF}),
            Color({0x23, 0x3B, 0xEF}),
            Color({0x83, 0x00, 0xF3}),
            Color({0xBF, 0x00, 0xBF}),
            Color({0xE7, 0x00, 0x5B}),
            Color({0xDB, 0x2B, 0x00}),
            Color({0xCB, 0x4F, 0x0F}),
            Color({0x8B, 0x73, 0x00}),
            Color({0x00, 0x97, 0x00}),
            Color({0x00, 0xAB, 0x00}),
            Color({0x00, 0x93, 0x3B}),
            Color({0x00, 0x83, 0x8B}),
            Color({0x00, 0x00, 0x00}),
            Color({0x00, 0x00, 0x00}),
            Color({0x00, 0x00, 0x00}),
            Color({0xFF, 0xFF, 0xFF}),
            Color({0x3F, 0xBF, 0xFF}),
            Color({0x5F, 0x97, 0xFF}),
            Color({0xA7, 0x8B, 0xFD}),
            Color({0xF7, 0x7B, 0xFF}),
            Color({0xFF, 0x77, 0xB7}),
            Color({0xFF, 0x77, 0x63}),
            Color({0xFF, 0x9B, 0x3B}),
            Color({0xF3, 0xBF, 0x3F}),
            Color({0x83, 0xD3, 0x13}),
            Color({0x4F, 0xDF, 0x4B}),
            Color({0x58, 0xF8, 0x98}),
            Color({0x00, 0xEB, 0xDB}),
            Color({0x00, 0x00, 0x00}),
            Color({0x00, 0x00, 0x00}),
            Color({0x00, 0x00, 0x00}),
            Color({0xFF, 0xFF, 0xFF}),
            Color({0xAB, 0xE7, 0xFF}),
            Color({0xC7, 0xD7, 0xFF}),
            Color({0xD7, 0xCB, 0xFF}),
            Color({0xFF, 0xC7, 0xFF}),
            Color({0xFF, 0xC7, 0xDB}),
            Color({0xFF, 0xBF, 0xB3}),
            Color({0xFF, 0xDB, 0xAB}),
            Color({0xFF, 0xE7, 0xA3}),
            Color({0xE3, 0xFF, 0xA3}),
            Color({0xAB, 0xF3, 0xBF}),
            Color({0xB3, 0xFF, 0xCF}),
            Color({0x9F, 0xFF, 0xF3}),
            Color({0x00, 0x00, 0x00}),
            Color({0x00, 0x00, 0x00}),
            Color({0x00, 0x00, 0x00})
        };
        
        return colorPalatte[palatte];
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
