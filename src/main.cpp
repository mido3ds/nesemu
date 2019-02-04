/* 
    Mahmoud Adas, 2019
    NES-6502 Emulator
*/

// TODO: cycles++ if page crossed
/* TODO: decide whether to emulate this bug or not:
    An original 6502 has does not correctly fetch the target address 
    if the indirect vector falls on a page boundary 
    (e.g. $xxFF where xx is any value from $00 to $FF). 
    In this case fetches the LSB from $xxFF as expected but takes the MSB from $xx00. 
    This is fixed in some later chips like the 65SC02 so for compatibility 
    always ensure the indirect vector is not at the end of the page.
*/

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <memory>
#include <array>
#include <vector>
#include <string>
#include <functional>
#include <chrono>
#include <thread>
#include "Logger.h"

using namespace std;

static unique_ptr<char> readBinaryFile(string path, size_t* size) {
    ifstream file(path, ios::in|ios::binary|ios::ate);
    if (!file.is_open()) return nullptr;

    *size = (size_t)file.tellg();
    auto buffer = unique_ptr<char>(new char[*size]);

    file.seekg(0, ios::beg);
    file.read(buffer.get(), *size);
    file.close();

    return buffer;
}

struct Color {
    uint8_t r, g, b;
};

struct ROM {
    unique_ptr<char> buffer;
    size_t size;
    string path;

    static ROM fromFile(string path) {
        size_t size;
        return {readBinaryFile(path, &size), size, path};
    }
};

struct Instruction {
    function<void()> exec;
    uint8_t bytes;
    uint16_t cycles;
};

struct Region {
    uint16_t start, end;

    constexpr bool contains(uint16_t addr) {return addr <= end && addr >= start;}  
    constexpr uint16_t size() {return (end + 1) - start;}
};

struct Mirror {
    Region source, dest;

    vector<uint16_t> getAdresses(const uint16_t address) const {
        if (source.contains(address)) {
            uint16_t relativeAdress = address - source.start;
            uint16_t stripe = source.size();
            uint16_t num = dest.size()/stripe;

            vector<uint16_t> adresses(num);
            for (int i = 0; i < num; i++) {
                adresses[i] = dest.start + relativeAdress + stripe * i;
            }
            return adresses;
        }

        return vector<uint16_t>();
    }
};

enum class SpriteType : uint8_t {S8x8 = 0, S8x16 = 1};
enum class ColorMode : uint8_t {Color = 0, Monochrome = 1};

struct SpriteInfo {
    uint8_t y; // Y-coordinate of the top left of the sprite minus 1
    uint8_t i; // Index number of the sprite in the pattern tables.

    union {
        struct {
            uint8_t color:2; // Most significant two bits of the colour
            uint8_t:3;
            uint8_t pritority:1; // Indicates whether this sprite has priority over the background
            uint8_t hFlip:1; // Indicates whether to flip the sprite horizontally
            uint8_t vFlip:1; // Indicates whether to flip the sprite vertically}
        } bits;
        uint8_t byte;
    } attr;
};

constexpr uint8_t SPRITE_8x8_SIZE = 16;
constexpr uint8_t SPRITE_8x16_SIZE = 2 * SPRITE_8x8_SIZE;
constexpr uint32_t MEM_SIZE = 0xFFFF + 1;

constexpr uint16_t PPU_CTRL_REG0 = 0x2000, 
                    PPU_CTRL_REG1 = 0x2001, 
                    PPU_STS_REG = 0x2002,
                    PPU_SCRL_REG = 0x2006;

// approx time in nanoseconds for one cycle
constexpr int NTSC_CYCLE_NS  = 559;
constexpr int PAL_CYCLE_NS   = 601;
constexpr int DENDY_CYCLE_NS = 564; // TODO: detect video system

constexpr Color DEFAULT_COLOR = Color({0, 0, 0});

// memory regions
constexpr Region
    ZERO_PAGE {0x0000, 0x0100-1},
    STACK {0x0100, 0x0200-1},
    RAM {0x0200, 0x0800-1},

    IO_REGS0 {0x2000, 0x2008-1},
    IO_REGS1 {0x4000, 0x4020-1},

    EX_ROM {0x4020, 0x6000-1},
    SRAM {0x6000, 0x8000-1},
    PRG_ROM_LOW {0x8000, 0xC000-1},
    PRG_ROM_UP {0xC000, 0xFFFF};

// varm regions
constexpr Region 
    /* pattern tables */
    PATT_TBL0 {0x0000, 0x1000-1},
    PATT_TBL1 {0x1000, 0x2000-1},

    /* name tables */
    NAME_TBL0 {0x2000, 0x23C0-1},
    ATT_TBL0 {0x23C0, 0x2400-1},
    NAME_TBL1 {0x2400, 0x27C0-1},
    ATT_TBL1 {0x27C0, 0x2800-1},
    NAME_TBL2 {0x2800, 0x2BC0-1},
    ATT_TBL2 {0x2BC0, 0x2C00-1},
    NAME_TBL3 {0x2C00, 0x2FC0-1},
    ATT_TBL3 {0x2FC0, 0x3000-1},

    /* palettes */
    IMG_PLT {0x3F00, 0x3F10-1}, 
    SPR_PLT {0x3F10, 0x3F20-1};

constexpr array<Mirror, 2> MEM_MIRRORS {
    Mirror({{0x0000, 0x07FF}, {0x0800, 0x2000-1}}),
    Mirror({IO_REGS0, {0x2008, 0x4000-1}}),
};

constexpr array<Mirror, 3> VRAM_MIRRORS {
    Mirror({{0x2000, 0x2EFF}, {0x3000, 0x3F00-1}}),
    Mirror({{0x3F00, 0x3F1F}, {0x3F20, 0x4000-1}}),
    Mirror({{0x0000, 0x3FFF}, {0x4000, 0xFFFF}}),
};

// interrupt vector table
constexpr uint16_t
    IRQ = 0xFFFE, NMI = 0xFFFA, RH = 0xFFFC;


class NES6502 {
protected:

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
                uint8_t:1; 
                uint8_t v:1; // overflow flag
                uint8_t n:1; // negative flag
            } bits;
            uint8_t byte;
        } flags; // processor status
    } regs;

    ROM rom;
    array<uint8_t, MEM_SIZE> memory;
    array<uint8_t, MEM_SIZE> vram;
    array<uint8_t, 256> sprram; // sprite ram
    array<Instruction, UINT8_MAX+1> instrucSet;

    union ScrollReg {
        struct {
            uint8_t xScroll:5;
            uint8_t yScroll:5;
            uint8_t bit10:1;
            uint8_t bit11:1;
            uint8_t yOffset:3;
            uint8_t:1;
        } bits;
        uint16_t word;

        void incXScroll() {bits.bit10 ^= (++bits.xScroll == 0);}
        void incYScroll() {
            if (++bits.yScroll == 30) {
                bits.bit11 ^= 1;
                bits.yScroll = 0;
            }
        }
    } *scrollReg = (ScrollReg*)(memory.data() + PPU_SCRL_REG);

    union PPUControlReg {
        struct {
            // reg0
            uint8_t nameTable:2; // name table 0, 1, 2 or 3
            uint8_t ppuIncRate:1; // 0 -> 1 or 1 -> 32
            uint8_t spritesPattTable:1; // pattern table 0 or 1
            uint8_t bckgPattTable:1; // pattern table 0 or 1
            SpriteType spriteType:1;
            uint8_t:1; // Changes PPU between master and slave modes. This is not used by the NES.
            bool nmiEnabled:1; // whether non-maskable-interrupt should occur upon V-Blank or not

            // reg1
            ColorMode colorMode:1;
            bool showLeftBckg:1; // whether to show background in the left 8 pixels or not
            bool showLeftSprites:1; // whether to show sprites in the left 8 pixels or not
            bool showBckg:1;  // whether to show whole background or not
            bool showSprites:1; // whether to show whole sprites or not
            uint8_t color:3; // background colour in monochrome mode or colour intensity in colour mode
        } bits;
        uint8_t reg0, reg1;

        uint16_t getNameTableAddress() {return 0x2000 + bits.nameTable * 0x0400;}
        uint8_t getPPUIncrementRate() {return bits.ppuIncRate ? /*vertical*/32 : /*horizontal*/1;}
    } *controlReg = (PPUControlReg*)(memory.data() + PPU_CTRL_REG0);

    union PPUStatusReg {
        struct {
            uint8_t:1;
            uint8_t:1;
            uint8_t:1;
            uint8_t:1;
            uint8_t ignoreVramWrites:1;
            uint8_t spritesMoreThan8:1;    
            uint8_t sprite0Hit:1; // set when a non-transparent pixel of 
                                // sprite 0 overlaps a non-transparent background pixel
            uint8_t vblank:1; // set when V-Blank is occurring
        } bits;
        uint8_t byte;
    } *statusReg = (PPUStatusReg*)(memory.data() + PPU_STS_REG);

    uint8_t& sprramAddrReg                    = memory[0x2003];
    uint8_t& sprramIOReg                      = memory[0x2004];

    uint8_t& vramAddrReg0                     = memory[0x2005];
    uint8_t& vramAddrReg1                     = memory[0x2006];
    uint8_t& vramIOReg                        = memory[0x2007];

    uint8_t& apuPulse1ControlReg              = memory[0x4000];
    uint8_t& apuPulse1RampControlReg          = memory[0x4001];
    uint8_t& apuPulse1FineTuneReg             = memory[0x4002];
    uint8_t& apuPulse1CoarseTuneReg           = memory[0x4003];
    uint8_t& apuPulse2ControlReg              = memory[0x4004];
    uint8_t& apuPulse2RampControlReg          = memory[0x4005];
    uint8_t& apuPulse2FineTuneReg             = memory[0x4006];
    uint8_t& apuPulse2CoarseTuneReg           = memory[0x4007];
    uint8_t& apuTriangleControlReg1           = memory[0x4008];
    uint8_t& apuTriangleControlReg2           = memory[0x4009];
    uint8_t& apuTriangleFrequencyReg1         = memory[0x400A];
    uint8_t& apuTriangleFrequencyReg2         = memory[0x400B];
    uint8_t& apuNoiseControlReg1              = memory[0x400C];
    uint8_t& apuNoiseFrequencyReg1            = memory[0x400E];
    uint8_t& apuNoiseFrequencyReg2            = memory[0x400F];
    uint8_t& apuDeltaModulationControlReg     = memory[0x4010];
    uint8_t& apuDeltaModulationDAReg          = memory[0x4011];
    uint8_t& apuDeltaModulationAddressReg     = memory[0x4012];
    uint8_t& apuDeltaModulationDataLengthReg  = memory[0x4013];
    uint8_t& apuVerticalClockSignalReg        = memory[0x4015];

    uint8_t& spriteDMAReg                     = memory[0x4014];

    // TODO listen for changes in reg
    struct JoyPadReg {
        union {
            uint8_t data:1; // Reads data from joypad or causes joypad strobe when writing.
            uint8_t:1;
            uint8_t:1;
            uint8_t zapperIsPointing:1;
            uint8_t zapperIsTriggered:1;
            uint8_t:3;
        } bits;
        uint8_t byte;
    } *joypadReg0 = (JoyPadReg*)(memory.data() + 0x4016), 
    *joypadReg1 = (JoyPadReg*)(memory.data() + 0x4017);

    uint16_t cycles;

public:

    NES6502() {
        logInfo("started building NES6502 device");

        // 6502 instruction set reference: http://obelisk.me.uk/6502/reference.html
        logInfo("filling instruction set data");
        
        instrucSet.fill({[this]() {
            logError("invalid/unsupported opcode(0x%02x) called", read<uint8_t>(regs.pc));
        },0,0});

        /*ADC*/ {
            auto adc = [this](uint8_t v) {
                uint16_t result = regs.a + v + regs.flags.bits.c;

                regs.flags.bits.c = (uint16_t)result > UINT8_MAX; 
                regs.flags.bits.v = (int16_t)result > INT8_MAX || (int16_t)result < INT8_MIN; 

                regs.a = (uint8_t)result;

                regs.flags.bits.z = regs.a == 0;
                regs.flags.bits.n = regs.a >> 7;
            };
            instrucSet[0x69] = {[this,&adc]() {
                adc(read<uint8_t>(regs.pc+1));  
            }, 2, 2};
            instrucSet[0x65] = {[this,&adc]() {
                adc(read<uint8_t>(zeroPageAddress(read<uint8_t>(regs.pc+1))));
            }, 2, 3};
            instrucSet[0x75] = {[this,&adc]() {
                adc(read<uint8_t>(indexedZeroPageAddress(read<uint8_t>(regs.pc+1), regs.x)));
            }, 2, 4};
            instrucSet[0x6D] = {[this,&adc]() {
                adc(read<uint8_t>(absoluteAddress(read<uint8_t>(regs.pc+1), read<uint8_t>(regs.pc+2))));
            }, 3, 4};
            instrucSet[0x7D] = {[this,&adc]() {
                adc(read<uint8_t>(indexedAbsoluteAddress(read<uint8_t>(regs.pc+1), read<uint8_t>(regs.pc+2), regs.x)));
            }, 3, 4};
            instrucSet[0x79] = {[this,&adc]() {
                adc(read<uint8_t>(indexedAbsoluteAddress(read<uint8_t>(regs.pc+1), read<uint8_t>(regs.pc+2), regs.y)));
            }, 3, 4};
            instrucSet[0x61] = {[this,&adc]() {
                adc(read<uint8_t>(indexedIndirectAddress(read<uint8_t>(regs.pc+1), regs.x)));
            }, 2, 6};
            instrucSet[0x71] = {[this,&adc]() {
                adc(read<uint8_t>(indirectIndexedAddress(read<uint8_t>(regs.pc+1), regs.y)));
            }, 2, 5};
        }

        /*AND*/ {
            auto and = [this](uint8_t v) {
                regs.a |= v;
                regs.flags.bits.z = regs.a == 0;
                regs.flags.bits.n = regs.a >> 7;
            };
            instrucSet[0x29] = {[this,&and]() {
                and(read<uint8_t>(regs.pc+1));
            }, 2, 2};
            instrucSet[0x25] = {[this,&and]() {
                and(read<uint8_t>(zeroPageAddress(read<uint8_t>(regs.pc+1))));
            }, 2, 3};
            instrucSet[0x35] = {[this,&and]() {
                and(read<uint8_t>(indexedZeroPageAddress(read<uint8_t>(regs.pc+1), regs.x)));
            }, 2, 4};
            instrucSet[0x2D] = {[this,&and]() {
                and(read<uint8_t>(absoluteAddress(read<uint8_t>(regs.pc+1), read<uint8_t>(regs.pc+2))));
            }, 3, 4};
            instrucSet[0x3D] = {[this,&and]() {
                and(read<uint8_t>(indexedAbsoluteAddress(read<uint8_t>(regs.pc+1), read<uint8_t>(regs.pc+2), regs.x)));
            }, 3, 4};
            instrucSet[0x39] = {[this,&and]() {
                and(read<uint8_t>(indexedAbsoluteAddress(read<uint8_t>(regs.pc+1), read<uint8_t>(regs.pc+2), regs.y)));
            }, 3, 4};
            instrucSet[0x21] = {[this,&and]() {
                and(read<uint8_t>(indexedIndirectAddress(read<uint8_t>(regs.pc+1), regs.x)));
            }, 2, 6};
            instrucSet[0x31] = {[this,&and]() {
                and(read<uint8_t>(indirectIndexedAddress(read<uint8_t>(regs.pc+1), regs.y)));
            }, 2, 5};
        }

        /*ASL*/ {
            auto asl = [this](uint8_t v) -> uint8_t {
                regs.flags.bits.c = v >> 7;
                v <<= 1;
                regs.flags.bits.z = v == 0; // TODO: not sure if Accumulator only or any value
                regs.flags.bits.n = v >> 7;

                return v;
            };
            instrucSet[0x0A] = {[this,&asl]() {
                regs.a = asl(regs.a);
            }, 1, 2};
            instrucSet[0x06] = {[this,&asl]() {
                auto addr = zeroPageAddress(read<uint8_t>(regs.pc+1));
                write<uint8_t>(addr, asl(read<uint8_t>(addr)));
            }, 2, 5};
            instrucSet[0x16] = {[this,&asl]() {
                auto addr = indexedZeroPageAddress(read<uint8_t>(regs.pc+1), regs.x);
                write<uint8_t>(addr, asl(read<uint8_t>(addr)));
            }, 2, 6};
            instrucSet[0x0E] = {[this,&asl]() {
                auto addr = absoluteAddress(read<uint8_t>(regs.pc+1), read<uint8_t>(regs.pc+2));
                write<uint8_t>(addr, asl(read<uint8_t>(addr)));
            }, 3, 6};
            instrucSet[0x1E] = {[this,&asl]() {
                auto addr = indexedAbsoluteAddress(read<uint8_t>(regs.pc+1), read<uint8_t>(regs.pc+2), regs.x);
                write<uint8_t>(addr, asl(read<uint8_t>(addr)));
            }, 3, 7};
        }

        /*BCC*/ {
            instrucSet[0x90] = {[this]() {
                if (!regs.flags.bits.c) {
                    regs.pc += (int8_t)read<uint8_t>(regs.pc+1);
                    cycles++;
                }
            }, 2, 2};
        }

        /*BCS*/ {
            instrucSet[0xB0] = {[this]() {
                if (regs.flags.bits.c) {
                    regs.pc += (int8_t)read<uint8_t>(regs.pc+1);
                    cycles++;
                }
            }, 2, 2};
        }

        /*BEQ*/ {
            instrucSet[0xF0] = {[this]() {
                if (regs.flags.bits.z) {
                    regs.pc += (int8_t)read<uint8_t>(regs.pc+1);
                    cycles++;
                }
            }, 2, 2};
        }

        /*BIT*/ {
            auto bit = [this](uint8_t v) {
                regs.flags.bits.z = v & regs.a == 0;
                regs.flags.bits.v = v >> 6;
                regs.flags.bits.n = v >> 7;
            };
            instrucSet[0x24] = {[this,&bit]() {
                bit(read<uint8_t>(zeroPageAddress(read<uint8_t>(regs.pc+1))));
            }, 2, 3};
            instrucSet[0x2C] = {[this,&bit]() {
                bit(read<uint8_t>(absoluteAddress(read<uint8_t>(regs.pc+1), read<uint8_t>(regs.pc+2))));
            }, 3, 4};
        }

        /*BMI*/ {
            instrucSet[0x30] = {[this]() {
                if (regs.flags.bits.n) {
                    regs.pc += (int8_t)read<uint8_t>(regs.pc+1);
                    cycles++;
                }
            }, 2, 2};
        }

        /*BNE*/ {
            instrucSet[0xD0] = {[this]() {
                if (!regs.flags.bits.z) {
                    regs.pc += (int8_t)read<uint8_t>(regs.pc+1);
                    cycles++;
                }
            }, 2, 2};
        }

        /*BPL*/ {
            instrucSet[0x10] = {[this]() {
                if (!regs.flags.bits.n) {
                    regs.pc += (int8_t)read<uint8_t>(regs.pc+1);
                    cycles++;
                }
            }, 2, 2};
        }

        /*BRK*/ {
            instrucSet[0x00] = {[this]() {
                if (regs.flags.bits.i == 0) return;

                push<uint8_t>(regs.pc);
                push<uint8_t>(regs.flags.byte);
                regs.pc = read<uint16_t>(IRQ); 
                regs.flags.bits.b = 1;

                logInfo("program called BRK");
            }, 1, 7};
        }

        /*BVC*/ {
            instrucSet[0x50] = {[this]() {
                if (!regs.flags.bits.v) {
                    regs.pc += (int8_t)read<uint8_t>(regs.pc+1);
                    cycles++;
                }
            }, 2, 2};
        }

        /*BVS*/ {
            instrucSet[0x50] = {[this]() {
                if (regs.flags.bits.v) {
                    regs.pc += (int8_t)read<uint8_t>(regs.pc+1);
                    cycles++;
                }
            }, 2, 2};
        }

        /*CLC*/ {
            instrucSet[0x18] = {[this]() {
                regs.flags.bits.c = 0;
            }, 1, 2};
        }

        /*CLC*/ {
            instrucSet[0xD8] = {[this]() {
                regs.flags.bits.d = 0;
            }, 1, 2};
        }

        /*CLI*/ {
            instrucSet[0x58] = {[this]() {
                regs.flags.bits.i = 0;
            }, 1, 2};
        }

        /*CLV*/ {
            instrucSet[0xB8] = {[this]() {
                regs.flags.bits.v = 0;
            }, 1, 2};
        }

        /*CMP*/ {
            auto cmp = [this](uint8_t v) {
                uint8_t result = regs.a - v;
                regs.flags.bits.c = result > 0;
                regs.flags.bits.z = result == 0;
                regs.flags.bits.n = result >> 7;
            };
            instrucSet[0xC9] = {[this,&cmp]() {
                cmp(read<uint8_t>(regs.pc+1));  
            }, 2, 2};
            instrucSet[0xC5] = {[this,&cmp]() {
                cmp(read<uint8_t>(zeroPageAddress(read<uint8_t>(regs.pc+1))));
            }, 2, 3};
            instrucSet[0xD5] = {[this,&cmp]() {
                cmp(read<uint8_t>(indexedZeroPageAddress(read<uint8_t>(regs.pc+1), regs.x)));
            }, 2, 4};
            instrucSet[0xCD] = {[this,&cmp]() {
                cmp(read<uint8_t>(absoluteAddress(read<uint8_t>(regs.pc+1), read<uint8_t>(regs.pc+2))));
            }, 3, 4};
            instrucSet[0xDD] = {[this,&cmp]() {
                cmp(read<uint8_t>(indexedAbsoluteAddress(read<uint8_t>(regs.pc+1), read<uint8_t>(regs.pc+2), regs.x)));
            }, 3, 4};
            instrucSet[0xD9] = {[this,&cmp]() {
                cmp(read<uint8_t>(indexedAbsoluteAddress(read<uint8_t>(regs.pc+1), read<uint8_t>(regs.pc+2), regs.y)));
            }, 3, 4};
            instrucSet[0xC1] = {[this,&cmp]() {
                cmp(read<uint8_t>(indexedIndirectAddress(read<uint8_t>(regs.pc+1), regs.x)));
            }, 2, 6};
            instrucSet[0xD1] = {[this,&cmp]() {
                cmp(read<uint8_t>(indirectIndexedAddress(read<uint8_t>(regs.pc+1), regs.y)));
            }, 2, 5};
        }

        /*CPX*/ {
            auto cpx = [this](uint8_t v) {
                uint8_t result = regs.x - v;
                regs.flags.bits.c = result > 0;
                regs.flags.bits.z = result == 0;
                regs.flags.bits.n = result >> 7;
            };
            instrucSet[0xE0] = {[this,&cpx]() {
                cpx(read<uint8_t>(regs.pc+1));  
            }, 2, 2};
            instrucSet[0xE4] = {[this,&cpx]() {
                cpx(read<uint8_t>(zeroPageAddress(read<uint8_t>(regs.pc+1))));
            }, 2, 3};
            instrucSet[0xEC] = {[this,&cpx]() {
                cpx(read<uint8_t>(absoluteAddress(read<uint8_t>(regs.pc+1), read<uint8_t>(regs.pc+2))));
            }, 3, 4};
        }

        /*CPY*/ {
            auto cpy = [this](uint8_t v) {
                uint8_t result = regs.y - v;
                regs.flags.bits.c = result > 0;
                regs.flags.bits.z = result == 0;
                regs.flags.bits.n = result >> 7;
            };
            instrucSet[0xC0] = {[this,&cpy]() {
                cpy(read<uint8_t>(regs.pc+1));  
            }, 2, 2};
            instrucSet[0xC4] = {[this,&cpy]() {
                cpy(read<uint8_t>(zeroPageAddress(read<uint8_t>(regs.pc+1))));
            }, 2, 3};
            instrucSet[0xCC] = {[this,&cpy]() {
                cpy(read<uint8_t>(absoluteAddress(read<uint8_t>(regs.pc+1), read<uint8_t>(regs.pc+2))));
            }, 3, 4};
        }

        /*DEC*/ {
            auto dec = [this](uint8_t v) -> uint8_t {
                v--;

                regs.flags.bits.z = v == 0;
                regs.flags.bits.n = v >> 7;

                return v;
            };
            instrucSet[0xC6] = {[this,&dec]() {
                auto addr = zeroPageAddress(read<uint8_t>(regs.pc+1));
                write<uint8_t>(addr, dec(read<uint8_t>(addr)));
            }, 2, 5};
            instrucSet[0xD6] = {[this,&dec]() {
                auto addr = indexedZeroPageAddress(read<uint8_t>(regs.pc+1), regs.x);
                write<uint8_t>(addr, dec(read<uint8_t>(addr)));
            }, 2, 6};
            instrucSet[0xCE] = {[this,&dec]() {
                auto addr = absoluteAddress(read<uint8_t>(regs.pc+1), read<uint8_t>(regs.pc+2));
                write<uint8_t>(addr, dec(read<uint8_t>(addr)));
            }, 3, 6};
            instrucSet[0xDE] = {[this,&dec]() {
                auto addr = indexedAbsoluteAddress(read<uint8_t>(regs.pc+1), read<uint8_t>(regs.pc+2), regs.x);
                write<uint8_t>(addr, dec(read<uint8_t>(addr)));
            }, 3, 7};
        }

        /*DEX*/ {
            instrucSet[0xCA] = {[this]() {
                regs.x--;
                regs.flags.bits.z = regs.x == 0;
                regs.flags.bits.n = regs.x >> 7;
            }, 1, 2};
        }

        /*DEY*/ {
            instrucSet[0x88] = {[this]() {
                regs.y--;
                regs.flags.bits.z = regs.y == 0;
                regs.flags.bits.n = regs.y >> 7;
            }, 1, 2};
        }

        /*EOR*/ {
            auto eor = [this](uint8_t v) {
                regs.a ^= v;

                regs.flags.bits.z = regs.a == 0;
                regs.flags.bits.n = regs.a >> 7;
            };
            instrucSet[0x49] = {[this,&eor]() {
                eor(read<uint8_t>(regs.pc+1));  
            }, 2, 2};
            instrucSet[0x45] = {[this,&eor]() {
                eor(read<uint8_t>(zeroPageAddress(read<uint8_t>(regs.pc+1))));
            }, 2, 3};
            instrucSet[0x55] = {[this,&eor]() {
                eor(read<uint8_t>(indexedZeroPageAddress(read<uint8_t>(regs.pc+1), regs.x)));
            }, 2, 4};
            instrucSet[0x4D] = {[this,&eor]() {
                eor(read<uint8_t>(absoluteAddress(read<uint8_t>(regs.pc+1), read<uint8_t>(regs.pc+2))));
            }, 3, 4};
            instrucSet[0x5D] = {[this,&eor]() {
                eor(read<uint8_t>(indexedAbsoluteAddress(read<uint8_t>(regs.pc+1), read<uint8_t>(regs.pc+2), regs.x)));
            }, 3, 4};
            instrucSet[0x59] = {[this,&eor]() {
                eor(read<uint8_t>(indexedAbsoluteAddress(read<uint8_t>(regs.pc+1), read<uint8_t>(regs.pc+2), regs.y)));
            }, 3, 4};
            instrucSet[0x41] = {[this,&eor]() {
                eor(read<uint8_t>(indexedIndirectAddress(read<uint8_t>(regs.pc+1), regs.x)));
            }, 2, 6};
            instrucSet[0x51] = {[this,&eor]() {
                eor(read<uint8_t>(indirectIndexedAddress(read<uint8_t>(regs.pc+1), regs.y)));
            }, 2, 5};
        }

        /*INC*/ {
            auto inc = [this](uint8_t v) -> uint8_t {
                v++;

                regs.flags.bits.z = v == 0;
                regs.flags.bits.n = v >> 7;

                return v;
            };
            instrucSet[0xE6] = {[this,&inc]() {
                auto addr = zeroPageAddress(read<uint8_t>(regs.pc+1));
                write<uint8_t>(addr, inc(read<uint8_t>(addr)));
            }, 2, 5};
            instrucSet[0xF6] = {[this,&inc]() {
                auto addr = indexedZeroPageAddress(read<uint8_t>(regs.pc+1), regs.x);
                write<uint8_t>(addr, inc(read<uint8_t>(addr)));
            }, 2, 6};
            instrucSet[0xEE] = {[this,&inc]() {
                auto addr = absoluteAddress(read<uint8_t>(regs.pc+1), read<uint8_t>(regs.pc+2));
                write<uint8_t>(addr, inc(read<uint8_t>(addr)));
            }, 3, 6};
            instrucSet[0xFE] = {[this,&inc]() {
                auto addr = indexedAbsoluteAddress(read<uint8_t>(regs.pc+1), read<uint8_t>(regs.pc+2), regs.x);
                write<uint8_t>(addr, inc(read<uint8_t>(addr)));
            }, 3, 7};
        }

        /*INX*/ {
            instrucSet[0xE8] = {[this]() {
                regs.x++;
                regs.flags.bits.z = regs.x == 0;
                regs.flags.bits.n = regs.x >> 7;
            }, 1, 2};
        }

        /*INY*/ {
            instrucSet[0xC8] = {[this]() {
                regs.y++;
                regs.flags.bits.z = regs.y == 0;
                regs.flags.bits.n = regs.y >> 7;
            }, 1, 2};
        }

        /*JMP*/ {
            instrucSet[0x4C] = {[this]() {
                regs.pc += read<uint8_t>(absoluteAddress(read<uint8_t>(regs.pc+1), read<uint8_t>(regs.pc+2)));
            }, 3, 3};
            instrucSet[0x6C] = {[this]() {
                regs.pc += read<uint8_t>(indirectAddress(read<uint8_t>(regs.pc+1), read<uint8_t>(regs.pc+2)));
            }, 3, 5};
        }

        /*JSR*/ {
            instrucSet[0x20] = {[this]() {
                push<uint16_t>(regs.pc);
                regs.pc += read<uint8_t>(absoluteAddress(read<uint8_t>(regs.pc+1), read<uint8_t>(regs.pc+2)));
            }, 3, 6};
        }

        /*LDA*/ {
            auto lda = [this](uint8_t v) {
                regs.a = v;

                regs.flags.bits.z = regs.a == 0;
                regs.flags.bits.n = regs.a >> 7;
            };
            instrucSet[0xA9] = {[this,&lda]() {
                lda(read<uint8_t>(regs.pc+1));  
            }, 2, 2};
            instrucSet[0xA5] = {[this,&lda]() {
                lda(read<uint8_t>(zeroPageAddress(read<uint8_t>(regs.pc+1))));
            }, 2, 3};
            instrucSet[0xB5] = {[this,&lda]() {
                lda(read<uint8_t>(indexedZeroPageAddress(read<uint8_t>(regs.pc+1), regs.x)));
            }, 2, 4};
            instrucSet[0xAD] = {[this,&lda]() {
                lda(read<uint8_t>(absoluteAddress(read<uint8_t>(regs.pc+1), read<uint8_t>(regs.pc+2))));
            }, 3, 4};
            instrucSet[0xBD] = {[this,&lda]() {
                lda(read<uint8_t>(indexedAbsoluteAddress(read<uint8_t>(regs.pc+1), read<uint8_t>(regs.pc+2), regs.x)));
            }, 3, 4};
            instrucSet[0xB9] = {[this,&lda]() {
                lda(read<uint8_t>(indexedAbsoluteAddress(read<uint8_t>(regs.pc+1), read<uint8_t>(regs.pc+2), regs.y)));
            }, 3, 4};
            instrucSet[0xA1] = {[this,&lda]() {
                lda(read<uint8_t>(indexedIndirectAddress(read<uint8_t>(regs.pc+1), regs.x)));
            }, 2, 6};
            instrucSet[0xB1] = {[this,&lda]() {
                lda(read<uint8_t>(indirectIndexedAddress(read<uint8_t>(regs.pc+1), regs.y)));
            }, 2, 5};
        }

        /*LDX*/ {
            auto ldx = [this](uint8_t v) {
                regs.x = v;

                regs.flags.bits.z = regs.x == 0;
                regs.flags.bits.n = regs.x >> 7;
            };
            instrucSet[0xA2] = {[this,&ldx]() {
                ldx(read<uint8_t>(regs.pc+1));  
            }, 2, 2};
            instrucSet[0xA6] = {[this,&ldx]() {
                ldx(read<uint8_t>(zeroPageAddress(read<uint8_t>(regs.pc+1))));
            }, 2, 3};
            instrucSet[0xB6] = {[this,&ldx]() {
                ldx(read<uint8_t>(indexedZeroPageAddress(read<uint8_t>(regs.pc+1), regs.y)));
            }, 2, 4};
            instrucSet[0xAE] = {[this,&ldx]() {
                ldx(read<uint8_t>(absoluteAddress(read<uint8_t>(regs.pc+1), read<uint8_t>(regs.pc+2))));
            }, 3, 4};
            instrucSet[0xBE] = {[this,&ldx]() {
                ldx(read<uint8_t>(indexedAbsoluteAddress(read<uint8_t>(regs.pc+1), read<uint8_t>(regs.pc+2), regs.y)));
            }, 3, 4};
        }

        /*LDY*/ {
            auto ldy = [this](uint8_t v) {
                regs.y = v;

                regs.flags.bits.z = regs.y == 0;
                regs.flags.bits.n = regs.y >> 7;
            };
            instrucSet[0xA0] = {[this,&ldy]() {
                ldy(read<uint8_t>(regs.pc+1));  
            }, 2, 2};
            instrucSet[0xA4] = {[this,&ldy]() {
                ldy(read<uint8_t>(zeroPageAddress(read<uint8_t>(regs.pc+1))));
            }, 2, 3};
            instrucSet[0xB4] = {[this,&ldy]() {
                ldy(read<uint8_t>(indexedZeroPageAddress(read<uint8_t>(regs.pc+1), regs.x)));
            }, 2, 4};
            instrucSet[0xAC] = {[this,&ldy]() {
                ldy(read<uint8_t>(absoluteAddress(read<uint8_t>(regs.pc+1), read<uint8_t>(regs.pc+2))));
            }, 3, 4};
            instrucSet[0xBC] = {[this,&ldy]() {
                ldy(read<uint8_t>(indexedAbsoluteAddress(read<uint8_t>(regs.pc+1), read<uint8_t>(regs.pc+2), regs.x)));
            }, 3, 4};
        }

        /*LSR*/ {
            auto lsr = [this](uint8_t v) -> uint8_t {
                regs.flags.bits.c = v & 0b1;
                
                v >>= 1;

                regs.flags.bits.z = v == 0;
                regs.flags.bits.n = 0;

                return v;
            };
            instrucSet[0x4A] = {[this,&lsr]() {
                regs.a = lsr(regs.a);
            }, 1, 2};
            instrucSet[0x46] = {[this,&lsr]() {
                auto addr = read<uint8_t>(zeroPageAddress(read<uint8_t>(regs.pc+1)));
                write<uint8_t>(addr, lsr(addr));
            }, 2, 5};
            instrucSet[0x56] = {[this,&lsr]() {
                auto addr = read<uint8_t>(indexedZeroPageAddress(read<uint8_t>(regs.pc+1), regs.x));
                write<uint8_t>(addr, lsr(addr));
            }, 2, 6};
            instrucSet[0x4E] = {[this,&lsr]() {
                auto addr = read<uint8_t>(absoluteAddress(read<uint8_t>(regs.pc+1), read<uint8_t>(regs.pc+2)));
                write<uint8_t>(addr, lsr(addr));
            }, 3, 6};
            instrucSet[0x5E] = {[this,&lsr]() {
                auto addr = read<uint8_t>(indexedAbsoluteAddress(read<uint8_t>(regs.pc+1), read<uint8_t>(regs.pc+2), regs.x));
                write<uint8_t>(addr, lsr(addr));
            }, 3, 7};
        }

        /*NOP*/ {
            instrucSet[0xEA] = {[](){}, 1, 2};
        }

        /*ORA*/ {
            auto ora = [this](uint8_t v) {
                regs.a |= v;

                regs.flags.bits.z = regs.a == 0;
                regs.flags.bits.n = regs.a >> 7;
            };
            instrucSet[0x09] = {[this,&ora]() {
                ora(read<uint8_t>(regs.pc+1));  
            }, 2, 2};
            instrucSet[0x05] = {[this,&ora]() {
                ora(read<uint8_t>(zeroPageAddress(read<uint8_t>(regs.pc+1))));
            }, 2, 3};
            instrucSet[0x15] = {[this,&ora]() {
                ora(read<uint8_t>(indexedZeroPageAddress(read<uint8_t>(regs.pc+1), regs.x)));
            }, 2, 4};
            instrucSet[0x0D] = {[this,&ora]() {
                ora(read<uint8_t>(absoluteAddress(read<uint8_t>(regs.pc+1), read<uint8_t>(regs.pc+2))));
            }, 3, 4};
            instrucSet[0x1D] = {[this,&ora]() {
                ora(read<uint8_t>(indexedAbsoluteAddress(read<uint8_t>(regs.pc+1), read<uint8_t>(regs.pc+2), regs.x)));
            }, 3, 4};
            instrucSet[0x19] = {[this,&ora]() {
                ora(read<uint8_t>(indexedAbsoluteAddress(read<uint8_t>(regs.pc+1), read<uint8_t>(regs.pc+2), regs.y)));
            }, 3, 4};
            instrucSet[0x01] = {[this,&ora]() {
                ora(read<uint8_t>(indexedIndirectAddress(read<uint8_t>(regs.pc+1), regs.x)));
            }, 2, 6};
            instrucSet[0x11] = {[this,&ora]() {
                ora(read<uint8_t>(indirectIndexedAddress(read<uint8_t>(regs.pc+1), regs.y)));
            }, 2, 5};
        }

        /*PHA*/ {
            instrucSet[0x48] = {[this]() {
                push<uint8_t>(regs.a);
            }, 1, 3};
        }

        /*PHP*/ {
            instrucSet[0x08] = {[this]() {
                push<uint8_t>(regs.flags.byte);
            }, 1, 3};
        }

        /*PLA*/ {
            instrucSet[0x68] = {[this]() {
                regs.a = pop<uint8_t>();
            }, 1, 4};
        }

        /*PLP*/ {
            instrucSet[0x28] = {[this]() {
                regs.flags.byte = pop<uint8_t>();
            }, 1, 4};
        }

        /*ROL*/ {
            auto rol = [this](uint8_t v) -> uint8_t {
                uint8_t oldCarry = regs.flags.bits.c;
                regs.flags.bits.c = v >> 7;
                
                v <<= 1;
                v |= oldCarry;

                regs.flags.bits.z = v == 0;
                regs.flags.bits.n = v >> 7;

                return v;
            };
            instrucSet[0x2A] = {[this,&rol]() {
                regs.a = rol(regs.a);
            }, 1, 2};
            instrucSet[0x26] = {[this,&rol]() {
                auto addr = read<uint8_t>(zeroPageAddress(read<uint8_t>(regs.pc+1)));
                write<uint8_t>(addr, rol(addr));
            }, 2, 5};
            instrucSet[0x36] = {[this,&rol]() {
                auto addr = read<uint8_t>(indexedZeroPageAddress(read<uint8_t>(regs.pc+1), regs.x));
                write<uint8_t>(addr, rol(addr));
            }, 2, 6};
            instrucSet[0x2E] = {[this,&rol]() {
                auto addr = read<uint8_t>(absoluteAddress(read<uint8_t>(regs.pc+1), read<uint8_t>(regs.pc+2)));
                write<uint8_t>(addr, rol(addr));
            }, 3, 6};
            instrucSet[0x3E] = {[this,&rol]() {
                auto addr = read<uint8_t>(indexedAbsoluteAddress(read<uint8_t>(regs.pc+1), read<uint8_t>(regs.pc+2), regs.x));
                write<uint8_t>(addr, rol(addr));
            }, 3, 7};
        }

        /*ROR*/ {
            auto ror = [this](uint8_t v) -> uint8_t {
                uint8_t oldCarry = regs.flags.bits.c;
                regs.flags.bits.c = v & 0b1;
                
                v >>= 1;
                v |= oldCarry << 7;

                regs.flags.bits.z = v == 0;
                regs.flags.bits.n = oldCarry;

                return v;
            };
            instrucSet[0x6A] = {[this,&ror]() {
                regs.a = ror(regs.a);
            }, 1, 2};
            instrucSet[0x66] = {[this,&ror]() {
                auto addr = read<uint8_t>(zeroPageAddress(read<uint8_t>(regs.pc+1)));
                write<uint8_t>(addr, ror(addr));
            }, 2, 5};
            instrucSet[0x76] = {[this,&ror]() {
                auto addr = read<uint8_t>(indexedZeroPageAddress(read<uint8_t>(regs.pc+1), regs.x));
                write<uint8_t>(addr, ror(addr));
            }, 2, 6};
            instrucSet[0x6E] = {[this,&ror]() {
                auto addr = read<uint8_t>(absoluteAddress(read<uint8_t>(regs.pc+1), read<uint8_t>(regs.pc+2)));
                write<uint8_t>(addr, ror(addr));
            }, 3, 6};
            instrucSet[0x7E] = {[this,&ror]() {
                auto addr = read<uint8_t>(indexedAbsoluteAddress(read<uint8_t>(regs.pc+1), read<uint8_t>(regs.pc+2), regs.x));
                write<uint8_t>(addr, ror(addr));
            }, 3, 7};
        }

        /*RTI*/ {
            instrucSet[0x40] = {[this]() {
                regs.flags.byte = pop<uint8_t>();
                regs.pc = pop<uint16_t>();
            }, 1, 6};
        }

        /*RTS*/ {
            instrucSet[0x60] = {[this]() {
                regs.pc = pop<uint16_t>() + 1;
            }, 1, 6};
        }

        /*SBC*/ {
            auto sbc = [this](uint8_t v) {
                uint16_t result = regs.a - v - (~ regs.flags.bits.c);

                regs.flags.bits.c = (uint16_t)result > UINT8_MAX; 
                regs.flags.bits.v = (int16_t)result > INT8_MAX || (int16_t)result < INT8_MAX; 

                regs.a = (uint8_t)result;

                regs.flags.bits.z = regs.a == 0;
                regs.flags.bits.n = regs.a >> 7;
            };
            instrucSet[0xE9] = {[this,&sbc]() {
                sbc(read<uint8_t>(regs.pc+1));  
            }, 2, 2};
            instrucSet[0xE5] = {[this,&sbc]() {
                sbc(read<uint8_t>(zeroPageAddress(read<uint8_t>(regs.pc+1))));
            }, 2, 3};
            instrucSet[0xF5] = {[this,&sbc]() {
                sbc(read<uint8_t>(indexedZeroPageAddress(read<uint8_t>(regs.pc+1), regs.x)));
            }, 2, 4};
            instrucSet[0xED] = {[this,&sbc]() {
                sbc(read<uint8_t>(absoluteAddress(read<uint8_t>(regs.pc+1), read<uint8_t>(regs.pc+2))));
            }, 3, 4};
            instrucSet[0xFD] = {[this,&sbc]() {
                sbc(read<uint8_t>(indexedAbsoluteAddress(read<uint8_t>(regs.pc+1), read<uint8_t>(regs.pc+2), regs.x)));
            }, 3, 4};
            instrucSet[0xF9] = {[this,&sbc]() {
                sbc(read<uint8_t>(indexedAbsoluteAddress(read<uint8_t>(regs.pc+1), read<uint8_t>(regs.pc+2), regs.y)));
            }, 3, 4};
            instrucSet[0xE1] = {[this,&sbc]() {
                sbc(read<uint8_t>(indexedIndirectAddress(read<uint8_t>(regs.pc+1), regs.x)));
            }, 2, 6};
            instrucSet[0xF1] = {[this,&sbc]() {
                sbc(read<uint8_t>(indirectIndexedAddress(read<uint8_t>(regs.pc+1), regs.y)));
            }, 2, 5};
        }

        /*SEC*/ {
            instrucSet[0x38] = {[this]() {
                regs.flags.bits.c = 1;
            }, 1, 2};
        }

        /*SED*/ {
            instrucSet[0xF8] = {[this]() {
                regs.flags.bits.d = 1;
            }, 1, 2};
        }

        /*SEI*/ {
            instrucSet[0x78] = {[this]() {
                regs.flags.bits.i = 1;
            }, 1, 2};
        }

        /*STA*/ {
            instrucSet[0x85] = {[this]() {
                write<uint8_t>(read<uint8_t>(zeroPageAddress(read<uint8_t>(regs.pc+1))), regs.a);
            }, 2, 3};
            instrucSet[0x95] = {[this]() {
                write<uint8_t>(read<uint8_t>(indexedZeroPageAddress(read<uint8_t>(regs.pc+1), regs.x)), regs.a);
            }, 2, 4};
            instrucSet[0x8D] = {[this]() {
                write<uint8_t>(read<uint8_t>(absoluteAddress(read<uint8_t>(regs.pc+1), read<uint8_t>(regs.pc+2))), regs.a);
            }, 3, 4};
            instrucSet[0x9D] = {[this]() {
                write<uint8_t>(read<uint8_t>(indexedAbsoluteAddress(read<uint8_t>(regs.pc+1), read<uint8_t>(regs.pc+2), regs.x)), regs.a);
            }, 3, 5};
            instrucSet[0x99] = {[this]() {
                write<uint8_t>(read<uint8_t>(indexedAbsoluteAddress(read<uint8_t>(regs.pc+1), read<uint8_t>(regs.pc+2), regs.y)), regs.a);
            }, 3, 5};
            instrucSet[0x81] = {[this]() {
                write<uint8_t>(read<uint8_t>(indexedIndirectAddress(read<uint8_t>(regs.pc+1), regs.x)), regs.a);
            }, 2, 6};
            instrucSet[0x91] = {[this]() {
                write<uint8_t>(read<uint8_t>(indirectIndexedAddress(read<uint8_t>(regs.pc+1), regs.y)), regs.a);
            }, 2, 6};
        }

        /*STX*/ {
            instrucSet[0x86] = {[this]() {
                write<uint8_t>(read<uint8_t>(zeroPageAddress(read<uint8_t>(regs.pc+1))), regs.x);
            }, 2, 3};
            instrucSet[0x96] = {[this]() {
                write<uint8_t>(read<uint8_t>(indexedZeroPageAddress(read<uint8_t>(regs.pc+1), regs.y)), regs.x);
            }, 2, 4};
            instrucSet[0x8E] = {[this]() {
                write<uint8_t>(read<uint8_t>(absoluteAddress(read<uint8_t>(regs.pc+1), read<uint8_t>(regs.pc+2))), regs.x);
            }, 3, 4};
        }

        /*STY*/ {
            instrucSet[0x84] = {[this]() {
                write<uint8_t>(read<uint8_t>(zeroPageAddress(read<uint8_t>(regs.pc+1))), regs.y);
            }, 2, 3};
            instrucSet[0x94] = {[this]() {
                write<uint8_t>(read<uint8_t>(indexedZeroPageAddress(read<uint8_t>(regs.pc+1), regs.x)), regs.y);
            }, 2, 4};
            instrucSet[0x8C] = {[this]() {
                write<uint8_t>(read<uint8_t>(absoluteAddress(read<uint8_t>(regs.pc+1), read<uint8_t>(regs.pc+2))), regs.y);
            }, 3, 4};
        }        

        /*TAX*/ {
            instrucSet[0xAA] = {[this]() {
                regs.x = regs.a;
                regs.flags.bits.z = regs.x == 0;
                regs.flags.bits.n = regs.x >> 7;
            }, 1, 2};
        }

        /*TAY*/ {
            instrucSet[0xA8] = {[this]() {
                regs.y = regs.a;
                regs.flags.bits.z = regs.y == 0;
                regs.flags.bits.n = regs.y >> 7;
            }, 1, 2};
        }

        /*TSX*/ {
            instrucSet[0xBA] = {[this]() {
                regs.x = regs.sp;
                regs.flags.bits.z = regs.x == 0;
                regs.flags.bits.n = regs.x >> 7;
            }, 1, 2};
        }

        /*TXA*/ {
            instrucSet[0x8A] = {[this]() {
                regs.a = regs.x;
                regs.flags.bits.z = regs.x == 0;
                regs.flags.bits.n = regs.x >> 7;
            }, 1, 2};
        }

        /*TXS*/ {
            instrucSet[0x9A] = {[this]() {
                regs.sp = regs.x;
                regs.flags.bits.z = regs.x == 0;
                regs.flags.bits.n = regs.x >> 7;
            }, 1, 2};
        }

        /*TYA*/ {
            instrucSet[0x98] = {[this]() {
                regs.a = regs.y;
                regs.flags.bits.z = regs.a == 0;
                regs.flags.bits.n = regs.a >> 7;
            }, 1, 2};
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
        return absoluteAddress(read<uint8_t>(ccbb), read<uint8_t>(ccbb+1));
    }

    inline uint16_t indexedIndirectAddress(const uint8_t bb, const uint8_t i) {
       return absoluteAddress(read<uint8_t>(bb+i), read<uint8_t>(bb+i+1));
    }

    inline uint16_t indirectIndexedAddress(const uint8_t bb, const uint8_t i) {
        return absoluteAddress(read<uint8_t>(bb), read<uint8_t>(bb+1)) + i;
    }

    void setROM(string romPath) {
        logInfo("using rom: %s", romPath.c_str());
        rom = ROM::fromFile(romPath);
        if (!rom.buffer) {
            logError("couldn't read rom in path: %s", romPath.c_str());
            return;
        }
        if (rom.size == 0) {
            logError("rom is empty");
            return;
        }

        memcpy(&memory[EX_ROM.start], rom.buffer.get(), rom.size);
        logInfo("copied rom");
        // TODO
    }

    void burnCycles() {
        // TODO better burning
        std::this_thread::sleep_for(std::chrono::nanoseconds(cycles * NTSC_CYCLE_NS));
        cycles = 0;
    }

    template<typename T>
    T read(uint16_t address) {
        if (address == PPU_STS_REG) return (T) readPPUStatusRegister();
        else return *((T*) (memory.data() + address));
    }

    template<typename T>
    void write(uint16_t address, T value) {
        *((T*) (memory.data() + address)) = value;

        // apply mirroring 
        for (auto mirror: MEM_MIRRORS) {
            for (auto mirrorAddr: mirror.getAdresses(address)) {
                memory[mirrorAddr] = value;
            }
        }

        if (address == 0x4014) {
            // DMA from memory -> sprram
            memcpy(sprram.data(), memory.data() + 0x100 * value, sprram.size());
        } else if (address == 0x2004) {
            sprram[sprramAddrReg] = value;
        } else {
            // TODO: implement 0x2006 vram io regs reading/writing
            // if (address = 0x2006) {
            //     static int numWrites = 0;
            //     static uint8_t lastWrite = 0;
                
            //     numWrites++;
            //     if (numWrites == 1) {
            //         lastWrite = value;
            //     } else {
            //         numWrites = 0;
            //         uint16_t addr = value << 8 | lastWrite;

            //     }
            // }
            // if (address == 0x2006 && ++numWritesTo0x2006 == 2) {
            //     numWritesTo0x2006 = 0;
            //     vram[vramAddrReg1]
            // }
        }
    }

    template<typename T>
    void push(T v) {
        write<T>(STACK.start + regs.sp, v); // TODO: make it wrap around if stack is full
        regs.sp -= sizeof T;
    }

    template<typename T>
    T pop() {
        T v = read<T>(STACK.end + regs.sp); // TODO: from start or end
        regs.sp += sizeof T;
        return v;
    }

    template<typename T>
    T readVRam(uint16_t address) {
        return *((T*) (vram.data() + address));
    }

    template<typename T>
    void writeVRam(uint16_t address, T value) {
        *((T*) (vram.data() + address)) = value;

        // apply mirroring 
        for (auto mirror: VRAM_MIRRORS) {
            for (auto mirrorAddr: mirror.getAdresses(address)) {
                vram[mirrorAddr] = value;
            }
        }

        if (address == 0x3F00) {
            for (auto& mirrorAddr: {0x3F04, 0x3F08, 0x3F0C, 0x3F10, 0x3F14, 0x3F18, 0x3F1C}) {
                vram[mirrorAddr] = value;
            }
        }
    }

    uint16_t getSpriteAddr(SpriteInfo* inf, SpriteType type) {
        if (type == SpriteType::S8x8) return PATT_TBL0.start + inf->i * SPRITE_8x8_SIZE;
        if (inf->i % 2 == 0) return PATT_TBL0.start + inf->i * SPRITE_8x16_SIZE;
        return PATT_TBL1.start + inf->i * SPRITE_8x16_SIZE;
    }

    inline uint8_t readPPUStatusRegister() {
        vramAddrReg0 = vramAddrReg1 = 0;
        uint8_t old = statusReg->byte;
        statusReg->byte &= ~(1 << 4);
        return old;
    }

    void reset() {
        logInfo("start resetting");
        cycles = 0;

        /*https://wiki.nesdev.com/w/index.php/CPU_ALL#After_reset*/
        regs.sp -= 3;
        regs.flags.bits.i = 1;
        vram[0x4015] = 0;
        regs.pc = read<uint16_t>(RH);

        logInfo("finished resetting");
    }

    void powerOn() {
        logInfo("start powering on");
        cycles = 0;

        memory.fill(0);
        memset(&regs, 0, sizeof regs);

        // https://wiki.nesdev.com/w/index.php/CPU_ALL#At_power-up
        regs.flags.byte = 0x34;
        regs.sp = 0xFD;
        regs.pc = read<uint16_t>(RH);

        //TODO: All 15 bits of noise channel LFSR = $0000[4]. 
        //The first time the LFSR is clocked from the all-0s state, it will shift in a 1.

        powerOnPPU();

        logInfo("finished powering on");
    }

    inline void powerOnPPU() {
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
        NES6502 dev;
        dev.setROM(argv[1]);
        dev.powerOn();
    }
}
