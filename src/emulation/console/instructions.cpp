#include <sstream>
#include <tuple>
#include <map>
#include <iterator>

#include "emulation/console.h"
#include "logger.h"

static inline u8_t crossPagePenalty(u16_t const& oldpc, u16_t const& newpc) {
    // INFO("newpc=%04X, oldpc=")
    return (newpc>>8 == oldpc>>8) ? 0:1;
}

static inline void branch(u16_t& pc, u16_t& cpuCycles, u8_t const& fetched) {
    const auto oldpc = pc;
    pc += fetched;
    cpuCycles += 1 + crossPagePenalty(oldpc, pc);
}

void Console::loadInstructions() {
    instrucSet.fill({[this]() {
        ERROR("invalid/unsupported opcode(0x%02X) called [pc=0x%04X]", read(regs.pc-1), regs.pc-1);
    },"???", AddressMode::Implicit, 0});

    /*ADC*/ {
        auto adc = [this](u8_t v) {
            u16_t result = regs.a + v + regs.flags.bits.c;

            regs.flags.bits.c = (u16_t)result > UINT8_MAX; 
            regs.flags.bits.v = (i16_t)result > INT8_MAX || (i16_t)result < INT8_MIN; 

            regs.a = (u8_t)result;

            regs.flags.bits.z = regs.a == 0;
            regs.flags.bits.n = regs.a >> 7;
        };
        instrucSet[0x69] = {[this,adc]() {
            adc(fetch());  
        }, "ADC", AddressMode::Immediate, 2};
        instrucSet[0x65] = {[this,adc]() {
            adc(read(zeroPageAddress(fetch())));
        }, "ADC", AddressMode::ZeroPage, 3};
        instrucSet[0x75] = {[this,adc]() {
            adc(read(indexedZeroPageAddress(fetch(), regs.x)));
        }, "ADC", AddressMode::ZeroPageX, 4};
        instrucSet[0x6D] = {[this,adc]() {
            adc(read(absoluteAddress(fetch(), fetch())));
        }, "ADC", AddressMode::Absolute, 4};
        instrucSet[0x7D] = {[this,adc]() {
            adc(read(indexedAbsoluteAddress(fetch(), fetch(), regs.x)));
        }, "ADC", AddressMode::AbsoluteX, 4};
        instrucSet[0x79] = {[this,adc]() {
            adc(read(indexedAbsoluteAddress(fetch(), fetch(), regs.y)));
        }, "ADC", AddressMode::AbsoluteY, 4};
        instrucSet[0x61] = {[this,adc]() {
            adc(read(indexedIndirectAddress(fetch(), regs.x)));
        }, "ADC", AddressMode::IndexedIndirect, 6};
        instrucSet[0x71] = {[this,adc]() {
            adc(read(indirectIndexedAddress(fetch(), regs.y)));
        }, "ADC", AddressMode::IndirectIndexed, 5};
    }

    /*AND*/ {
        auto andd = [this](u8_t v) {
            regs.a = regs.a & v;
            regs.flags.bits.z = regs.a == 0;
            regs.flags.bits.n = regs.a >> 7;
        };
        instrucSet[0x29] = {[this,andd]() {
            andd(fetch());
        }, "AND", AddressMode::Immediate, 2};
        instrucSet[0x25] = {[this,andd]() {
            andd(read(zeroPageAddress(fetch())));
        }, "AND", AddressMode::ZeroPage, 3};
        instrucSet[0x35] = {[this,andd]() {
            andd(read(indexedZeroPageAddress(fetch(), regs.x)));
        }, "AND", AddressMode::ZeroPageX, 4};
        instrucSet[0x2D] = {[this,andd]() {
            andd(read(absoluteAddress(fetch(), fetch())));
        }, "AND", AddressMode::Absolute, 4};
        instrucSet[0x3D] = {[this,andd]() {
            andd(read(indexedAbsoluteAddress(fetch(), fetch(), regs.x)));
        }, "AND", AddressMode::AbsoluteX, 4};
        instrucSet[0x39] = {[this,andd]() {
            andd(read(indexedAbsoluteAddress(fetch(), fetch(), regs.y)));
        }, "AND", AddressMode::AbsoluteY, 4};
        instrucSet[0x21] = {[this,andd]() {
            andd(read(indexedIndirectAddress(fetch(), regs.x)));
        }, "AND", AddressMode::IndexedIndirect, 6};
        instrucSet[0x31] = {[this,andd]() {
            andd(read(indirectIndexedAddress(fetch(), regs.y)));
        }, "AND", AddressMode::IndirectIndexed, 5};
    }

    /*ASL*/ {
        auto asl = [this](u8_t v) -> u8_t {
            regs.flags.bits.c = v >> 7;
            v <<= 1;
            regs.flags.bits.z = v == 0; // TODO: not sure if Accumulator only or any value
            regs.flags.bits.n = v >> 7;

            return v;
        };
        instrucSet[0x0A] = {[this,asl]() {
            regs.a = asl(regs.a);
        }, "ASL", AddressMode::Accumulator, 2};
        instrucSet[0x06] = {[this,asl]() {
            auto addr = zeroPageAddress(fetch());
            write(addr, asl(read(addr)));
        }, "ASL", AddressMode::ZeroPage, 5};
        instrucSet[0x16] = {[this,asl]() {
            auto addr = indexedZeroPageAddress(fetch(), regs.x);
            write(addr, asl(read(addr)));
        }, "ASL", AddressMode::ZeroPageX, 6};
        instrucSet[0x0E] = {[this,asl]() {
            auto addr = absoluteAddress(fetch(), fetch());
            write(addr, asl(read(addr)));
        }, "ASL", AddressMode::Absolute, 6};
        instrucSet[0x1E] = {[this,asl]() {
            auto addr = indexedAbsoluteAddress(fetch(), fetch(), regs.x);
            write(addr, asl(read(addr)));
        }, "ASL", AddressMode::AbsoluteX, 7};
    }

    /*BCC*/ {
        instrucSet[0x90] = {[this]() {
            auto fetched = (i8_t)fetch();
            if (!regs.flags.bits.c) { branch(regs.pc, cpuCycles, fetched); }
        }, "BCC", AddressMode::Implicit, 2};
    }

    /*BCS*/ {
        instrucSet[0xB0] = {[this]() {
            auto fetched = (i8_t)fetch();
            if (regs.flags.bits.c) { branch(regs.pc, cpuCycles, fetched); }
        }, "BCS", AddressMode::Implicit, 2};
    }

    /*BEQ*/ {
        instrucSet[0xF0] = {[this]() {
            auto fetched = (i8_t)fetch();
            if (regs.flags.bits.z) { branch(regs.pc, cpuCycles, fetched); }
        }, "BEQ", AddressMode::Implicit, 2};
    }

    /*BIT*/ {
        auto bit = [this](u8_t v) {
            regs.flags.bits.z = (v & regs.a) == 0;
            regs.flags.bits.v = v >> 6;
            regs.flags.bits.n = v >> 7;
        };
        instrucSet[0x24] = {[this,bit]() {
            bit(read(zeroPageAddress(fetch())));
        }, "BIT", AddressMode::Implicit, 3};
        instrucSet[0x2C] = {[this,bit]() {
            bit(read(absoluteAddress(fetch(), fetch())));
        }, "BIT", AddressMode::Implicit, 4};
    }

    /*BMI*/ {
        instrucSet[0x30] = {[this]() {
            auto fetched = (i8_t)fetch();
            if (regs.flags.bits.n) { branch(regs.pc, cpuCycles, fetched); }
        }, "BMI", AddressMode::Implicit, 2};
    }

    /*BNE*/ {
        instrucSet[0xD0] = {[this]() {
            auto fetched = (i8_t)fetch();
            if (!regs.flags.bits.z) { branch(regs.pc, cpuCycles, fetched); }
        }, "BNE", AddressMode::Implicit, 2};
    }

    /*BPL*/ {
        instrucSet[0x10] = {[this]() {
            auto fetched = (i8_t)fetch();
            if (!regs.flags.bits.n) { branch(regs.pc, cpuCycles, fetched); }
        }, "BPL", AddressMode::Implicit, 2};
    }

    /*BRK*/ {
        instrucSet[0x00] = {[this]() {
            if (regs.flags.bits.i == 1) return;

            push(regs.pc);
            push(regs.flags.byte);
            regs.pc = read16(IRQ); 
            regs.flags.bits.b = 1;
            regs.flags.bits.i = 1;
        }, "BRK", AddressMode::Implicit, 7};
    }

    /*BVC*/ {
        instrucSet[0x50] = {[this]() {
            auto fetched = (i8_t)fetch();
            if (!regs.flags.bits.v) { branch(regs.pc, cpuCycles, fetched); }
        }, "BVC", AddressMode::Implicit, 2};
    }

    /*BVS*/ {
        instrucSet[0x50] = {[this]() {
            auto fetched = (i8_t)fetch();
            if (regs.flags.bits.v) { branch(regs.pc, cpuCycles, fetched); }
        }, "BVS", AddressMode::Implicit, 2};
    }

    /*CLC*/ {
        instrucSet[0x18] = {[this]() {
            regs.flags.bits.c = 0;
        }, "CLC", AddressMode::Implicit, 2};
    }

    /*CLD*/ {
        instrucSet[0xD8] = {[this]() {
            regs.flags.bits.d = 0;
        }, "CLC", AddressMode::Implicit, 2};
    }

    /*CLI*/ {
        instrucSet[0x58] = {[this]() {
            regs.flags.bits.i = 0;
        }, "CLI", AddressMode::Implicit, 2};
    }

    /*CLV*/ {
        instrucSet[0xB8] = {[this]() {
            regs.flags.bits.v = 0;
        }, "CLV", AddressMode::Implicit, 2};
    }

    /*CMP*/ {
        auto cmp = [this](u8_t v) {
            u8_t result = regs.a - v;
            regs.flags.bits.c = result > 0;
            regs.flags.bits.z = result == 0;
            regs.flags.bits.n = result >> 7;
        };
        instrucSet[0xC9] = {[this,cmp]() {
            cmp(fetch());  
        }, "CMP", AddressMode::Immediate, 2};
        instrucSet[0xC5] = {[this,cmp]() {
            cmp(read(zeroPageAddress(fetch())));
        }, "CMP", AddressMode::ZeroPage, 3};
        instrucSet[0xD5] = {[this,cmp]() {
            cmp(read(indexedZeroPageAddress(fetch(), regs.x)));
        }, "CMP", AddressMode::ZeroPageX, 4};
        instrucSet[0xCD] = {[this,cmp]() {
            cmp(read(absoluteAddress(fetch(), fetch())));
        }, "CMP", AddressMode::Absolute, 4};
        instrucSet[0xDD] = {[this,cmp]() {
            cmp(read(indexedAbsoluteAddress(fetch(), fetch(), regs.x)));
        }, "CMP", AddressMode::AbsoluteX, 4};
        instrucSet[0xD9] = {[this,cmp]() {
            cmp(read(indexedAbsoluteAddress(fetch(), fetch(), regs.y)));
        }, "CMP", AddressMode::AbsoluteY, 4};
        instrucSet[0xC1] = {[this,cmp]() {
            cmp(read(indexedIndirectAddress(fetch(), regs.x)));
        }, "CMP", AddressMode::IndexedIndirect, 6};
        instrucSet[0xD1] = {[this,cmp]() {
            cmp(read(indirectIndexedAddress(fetch(), regs.y)));
        }, "CMP", AddressMode::IndirectIndexed, 5};
    }

    /*CPX*/ {
        auto cpx = [this](u8_t v) {
            u8_t result = regs.x - v;
            regs.flags.bits.c = result > 0;
            regs.flags.bits.z = result == 0;
            regs.flags.bits.n = result >> 7;
        };
        instrucSet[0xE0] = {[this,cpx]() {
            cpx(fetch());  
        }, "CPX", AddressMode::Immediate, 2};
        instrucSet[0xE4] = {[this,cpx]() {
            cpx(read(zeroPageAddress(fetch())));
        }, "CPX", AddressMode::ZeroPage, 3};
        instrucSet[0xEC] = {[this,cpx]() {
            cpx(read(absoluteAddress(fetch(), fetch())));
        }, "CPX", AddressMode::Absolute, 4};
    }

    /*CPY*/ {
        auto cpy = [this](u8_t v) {
            u8_t result = regs.y - v;
            regs.flags.bits.c = result > 0;
            regs.flags.bits.z = result == 0;
            regs.flags.bits.n = result >> 7;
        };
        instrucSet[0xC0] = {[this,cpy]() {
            cpy(fetch());  
        }, "CPY", AddressMode::Immediate, 2};
        instrucSet[0xC4] = {[this,cpy]() {
            cpy(read(zeroPageAddress(fetch())));
        }, "CPY", AddressMode::ZeroPage, 3};
        instrucSet[0xCC] = {[this,cpy]() {
            cpy(read(absoluteAddress(fetch(), fetch())));
        }, "CPY", AddressMode::Absolute, 4};
    }

    /*DEC*/ {
        auto dec = [this](u8_t v) -> u8_t {
            v--;

            regs.flags.bits.z = v == 0;
            regs.flags.bits.n = v >> 7;

            return v;
        };
        instrucSet[0xC6] = {[this,dec]() {
            auto addr = zeroPageAddress(fetch());
            write(addr, dec(read(addr)));
        }, "DEC", AddressMode::ZeroPage, 5};
        instrucSet[0xD6] = {[this,dec]() {
            auto addr = indexedZeroPageAddress(fetch(), regs.x);
            write(addr, dec(read(addr)));
        }, "DEC", AddressMode::ZeroPageX, 6};
        instrucSet[0xCE] = {[this,dec]() {
            auto addr = absoluteAddress(fetch(), fetch());
            write(addr, dec(read(addr)));
        }, "DEC", AddressMode::Absolute, 6};
        instrucSet[0xDE] = {[this,dec]() {
            auto addr = indexedAbsoluteAddress(fetch(), fetch(), regs.x);
            write(addr, dec(read(addr)));
        }, "DEC", AddressMode::AbsoluteX, 7};
    }

    /*DEX*/ {
        instrucSet[0xCA] = {[this]() {
            regs.x--;
            regs.flags.bits.z = regs.x == 0;
            regs.flags.bits.n = regs.x >> 7;
        }, "DEX", AddressMode::Implicit, 2};
    }

    /*DEY*/ {
        instrucSet[0x88] = {[this]() {
            regs.y--;
            regs.flags.bits.z = regs.y == 0;
            regs.flags.bits.n = regs.y >> 7;
        }, "DEY", AddressMode::Implicit, 2};
    }

    /*EOR*/ {
        auto eor = [this](u8_t v) {
            regs.a ^= v;

            regs.flags.bits.z = regs.a == 0;
            regs.flags.bits.n = regs.a >> 7;
        };
        instrucSet[0x49] = {[this,eor]() {
            eor(fetch());  
        }, "EOR", AddressMode::Immediate, 2};
        instrucSet[0x45] = {[this,eor]() {
            eor(read(zeroPageAddress(fetch())));
        }, "EOR", AddressMode::ZeroPage, 3};
        instrucSet[0x55] = {[this,eor]() {
            eor(read(indexedZeroPageAddress(fetch(), regs.x)));
        }, "EOR", AddressMode::ZeroPageX, 4};
        instrucSet[0x4D] = {[this,eor]() {
            eor(read(absoluteAddress(fetch(), fetch())));
        }, "EOR", AddressMode::Absolute, 4};
        instrucSet[0x5D] = {[this,eor]() {
            eor(read(indexedAbsoluteAddress(fetch(), fetch(), regs.x)));
        }, "EOR", AddressMode::AbsoluteX, 4};
        instrucSet[0x59] = {[this,eor]() {
            eor(read(indexedAbsoluteAddress(fetch(), fetch(), regs.y)));
        }, "EOR", AddressMode::AbsoluteY, 4};
        instrucSet[0x41] = {[this,eor]() {
            eor(read(indexedIndirectAddress(fetch(), regs.x)));
        }, "EOR", AddressMode::IndexedIndirect, 6};
        instrucSet[0x51] = {[this,eor]() {
            eor(read(indirectIndexedAddress(fetch(), regs.y)));
        }, "EOR", AddressMode::IndirectIndexed, 5};
    }

    /*INC*/ {
        auto inc = [this](u8_t v) -> u8_t {
            v++;

            regs.flags.bits.z = v == 0;
            regs.flags.bits.n = v >> 7;

            return v;
        };
        instrucSet[0xE6] = {[this,inc]() {
            auto addr = zeroPageAddress(fetch());
            write(addr, inc(read(addr)));
        }, "INC", AddressMode::ZeroPage, 5};
        instrucSet[0xF6] = {[this,inc]() {
            auto addr = indexedZeroPageAddress(fetch(), regs.x);
            write(addr, inc(read(addr)));
        }, "INC", AddressMode::ZeroPageX, 6};
        instrucSet[0xEE] = {[this,inc]() {
            auto addr = absoluteAddress(fetch(), fetch());
            write(addr, inc(read(addr)));
        }, "INC", AddressMode::Absolute, 6};
        instrucSet[0xFE] = {[this,inc]() {
            auto addr = indexedAbsoluteAddress(fetch(), fetch(), regs.x);
            write(addr, inc(read(addr)));
        }, "INC", AddressMode::AbsoluteX, 7};
    }

    /*INX*/ {
        instrucSet[0xE8] = {[this]() {
            regs.x++;
            regs.flags.bits.z = regs.x == 0;
            regs.flags.bits.n = regs.x >> 7;
        }, "INX", AddressMode::Implicit, 2};
    }

    /*INY*/ {
        instrucSet[0xC8] = {[this]() {
            regs.y++;
            regs.flags.bits.z = regs.y == 0;
            regs.flags.bits.n = regs.y >> 7;
        }, "INY", AddressMode::Implicit, 2};
    }

    /*JMP*/ {
        instrucSet[0x4C] = {[this]() {
            regs.pc += read(absoluteAddress(fetch(), fetch()));
        }, "JMP", AddressMode::Absolute, 3};
        instrucSet[0x6C] = {[this]() {
            regs.pc += read(indirectAddress(fetch(), fetch()));
        }, "JMP", AddressMode::Indirect, 5};
    }

    /*JSR*/ {
        instrucSet[0x20] = {[this]() {
            push16(regs.pc);
            regs.pc += read(absoluteAddress(fetch(), fetch()));
        }, "JSR", AddressMode::Absolute, 6};
    }

    /*LDA*/ {
        auto lda = [this](u8_t v) {
            regs.a = v;

            regs.flags.bits.z = regs.a == 0;
            regs.flags.bits.n = regs.a >> 7;
        };
        instrucSet[0xA9] = {[this,lda]() {
            lda(fetch());  
        }, "LDA", AddressMode::Immediate, 2};
        instrucSet[0xA5] = {[this,lda]() {
            lda(read(zeroPageAddress(fetch())));
        }, "LDA", AddressMode::ZeroPage, 3};
        instrucSet[0xB5] = {[this,lda]() {
            lda(read(indexedZeroPageAddress(fetch(), regs.x)));
        }, "LDA", AddressMode::ZeroPageX, 4};
        instrucSet[0xAD] = {[this,lda]() {
            lda(read(absoluteAddress(fetch(), fetch())));
        }, "LDA", AddressMode::Absolute, 4};
        instrucSet[0xBD] = {[this,lda]() {
            lda(read(indexedAbsoluteAddress(fetch(), fetch(), regs.x)));
        }, "LDA", AddressMode::AbsoluteX, 4};
        instrucSet[0xB9] = {[this,lda]() {
            lda(read(indexedAbsoluteAddress(fetch(), fetch(), regs.y)));
        }, "LDA", AddressMode::AbsoluteY, 4};
        instrucSet[0xA1] = {[this,lda]() {
            lda(read(indexedIndirectAddress(fetch(), regs.x)));
        }, "LDA", AddressMode::IndexedIndirect, 6};
        instrucSet[0xB1] = {[this,lda]() {
            lda(read(indirectIndexedAddress(fetch(), regs.y)));
        }, "LDA", AddressMode::IndirectIndexed, 5};
    }

    /*LDX*/ {
        auto ldx = [this](u8_t v) {
            regs.x = v;

            regs.flags.bits.z = regs.x == 0;
            regs.flags.bits.n = regs.x >> 7;
        };
        instrucSet[0xA2] = {[this,ldx]() {
            ldx(fetch());  
        }, "LDX", AddressMode::Immediate, 2};
        instrucSet[0xA6] = {[this,ldx]() {
            ldx(read(zeroPageAddress(fetch())));
        }, "LDX", AddressMode::ZeroPage, 3};
        instrucSet[0xB6] = {[this,ldx]() {
            ldx(read(indexedZeroPageAddress(fetch(), regs.y)));
        }, "LDX", AddressMode::ZeroPageY, 4};
        instrucSet[0xAE] = {[this,ldx]() {
            ldx(read(absoluteAddress(fetch(), fetch())));
        }, "LDX", AddressMode::Absolute, 4};
        instrucSet[0xBE] = {[this,ldx]() {
            ldx(read(indexedAbsoluteAddress(fetch(), fetch(), regs.y)));
        }, "LDX", AddressMode::AbsoluteY, 4};
    }

    /*LDY*/ {
        auto ldy = [this](u8_t v) {
            regs.y = v;

            regs.flags.bits.z = regs.y == 0;
            regs.flags.bits.n = regs.y >> 7;
        };
        instrucSet[0xA0] = {[this,ldy]() {
            ldy(fetch());  
        }, "LDY", AddressMode::Immediate, 2};
        instrucSet[0xA4] = {[this,ldy]() {
            ldy(read(zeroPageAddress(fetch())));
        }, "LDY", AddressMode::ZeroPage, 3};
        instrucSet[0xB4] = {[this,ldy]() {
            ldy(read(indexedZeroPageAddress(fetch(), regs.x)));
        }, "LDY", AddressMode::ZeroPageX, 4};
        instrucSet[0xAC] = {[this,ldy]() {
            ldy(read(absoluteAddress(fetch(), fetch())));
        }, "LDY", AddressMode::Absolute, 4};
        instrucSet[0xBC] = {[this,ldy]() {
            ldy(read(indexedAbsoluteAddress(fetch(), fetch(), regs.x)));
        }, "LDY", AddressMode::AbsoluteX, 4};
    }

    /*LSR*/ {
        auto lsr = [this](u8_t v) -> u8_t {
            regs.flags.bits.c = v & 1;
            
            v >>= 1;

            regs.flags.bits.z = v == 0;
            regs.flags.bits.n = 0;

            return v;
        };
        instrucSet[0x4A] = {[this,lsr]() {
            regs.a = lsr(regs.a);
        }, "LSR", AddressMode::Accumulator, 2};
        instrucSet[0x46] = {[this,lsr]() {
            auto addr = read(zeroPageAddress(fetch()));
            write(addr, lsr(addr));
        }, "LSR", AddressMode::ZeroPage, 5};
        instrucSet[0x56] = {[this,lsr]() {
            auto addr = read(indexedZeroPageAddress(fetch(), regs.x));
            write(addr, lsr(addr));
        }, "LSR", AddressMode::ZeroPageX, 6};
        instrucSet[0x4E] = {[this,lsr]() {
            auto addr = read(absoluteAddress(fetch(), fetch()));
            write(addr, lsr(addr));
        }, "LSR", AddressMode::Absolute, 6};
        instrucSet[0x5E] = {[this,lsr]() {
            auto addr = read(indexedAbsoluteAddress(fetch(), fetch(), regs.x));
            write(addr, lsr(addr));
        }, "LSR", AddressMode::AbsoluteX, 7};
    }

    /*NOP*/ {
        for (auto& adr: vector<u8_t>{0xEA ,0x1C ,0x3C ,0x5C ,0x7C ,0xDC ,0xFC}) {
            instrucSet[adr] = {[](){}, "NOP", AddressMode::Implicit, 2};
        }
    }

    /*ORA*/ {
        auto ora = [this](u8_t v) {
            regs.a |= v;

            regs.flags.bits.z = regs.a == 0;
            regs.flags.bits.n = regs.a >> 7;
        };
        instrucSet[0x09] = {[this,ora]() {
            ora(fetch());  
        }, "ORA", AddressMode::Immediate, 2};
        instrucSet[0x05] = {[this,ora]() {
            ora(read(zeroPageAddress(fetch())));
        }, "ORA", AddressMode::ZeroPage, 3};
        instrucSet[0x15] = {[this,ora]() {
            ora(read(indexedZeroPageAddress(fetch(), regs.x)));
        }, "ORA", AddressMode::ZeroPageX, 4};
        instrucSet[0x0D] = {[this,ora]() {
            ora(read(absoluteAddress(fetch(), fetch())));
        }, "ORA", AddressMode::Absolute, 4};
        instrucSet[0x1D] = {[this,ora]() {
            ora(read(indexedAbsoluteAddress(fetch(), fetch(), regs.x)));
        }, "ORA", AddressMode::AbsoluteX, 4};
        instrucSet[0x19] = {[this,ora]() {
            ora(read(indexedAbsoluteAddress(fetch(), fetch(), regs.y)));
        }, "ORA", AddressMode::AbsoluteY, 4};
        instrucSet[0x01] = {[this,ora]() {
            ora(read(indexedIndirectAddress(fetch(), regs.x)));
        }, "ORA", AddressMode::IndexedIndirect, 6};
        instrucSet[0x11] = {[this,ora]() {
            ora(read(indirectIndexedAddress(fetch(), regs.y)));
        }, "ORA", AddressMode::IndirectIndexed, 5};
    }

    /*PHA*/ {
        instrucSet[0x48] = {[this]() {
            push(regs.a);
        }, "PHA", AddressMode::Implicit, 3};
    }

    /*PHP*/ {
        instrucSet[0x08] = {[this]() {
            push(regs.flags.byte);
        }, "PHP", AddressMode::Implicit, 3};
    }

    /*PLA*/ {
        instrucSet[0x68] = {[this]() {
            regs.a = pop();
        }, "PLA", AddressMode::Implicit, 4};
    }

    /*PLP*/ {
        instrucSet[0x28] = {[this]() {
            regs.flags.byte = pop();
        }, "PLP", AddressMode::Implicit, 4};
    }

    /*ROL*/ {
        auto rol = [this](u8_t v) -> u8_t {
            u8_t oldCarry = regs.flags.bits.c;
            regs.flags.bits.c = v >> 7;
            
            v <<= 1;
            v |= oldCarry;

            regs.flags.bits.z = v == 0;
            regs.flags.bits.n = v >> 7;

            return v;
        };
        instrucSet[0x2A] = {[this,rol]() {
            regs.a = rol(regs.a);
        }, "ROL", AddressMode::Accumulator, 2};
        instrucSet[0x26] = {[this,rol]() {
            auto addr = read(zeroPageAddress(fetch()));
            write(addr, rol(addr));
        }, "ROL", AddressMode::ZeroPage, 5};
        instrucSet[0x36] = {[this,rol]() {
            auto addr = read(indexedZeroPageAddress(fetch(), regs.x));
            write(addr, rol(addr));
        }, "ROL", AddressMode::ZeroPageX, 6};
        instrucSet[0x2E] = {[this,rol]() {
            auto addr = read(absoluteAddress(fetch(), fetch()));
            write(addr, rol(addr));
        }, "ROL", AddressMode::Absolute, 6};
        instrucSet[0x3E] = {[this,rol]() {
            auto addr = read(indexedAbsoluteAddress(fetch(), fetch(), regs.x));
            write(addr, rol(addr));
        }, "ROL", AddressMode::AbsoluteX, 7};
    }

    /*ROR*/ {
        auto ror = [this](u8_t v) -> u8_t {
            u8_t oldCarry = regs.flags.bits.c;
            regs.flags.bits.c = v & 1;
            
            v >>= 1;
            v |= oldCarry << 7;

            regs.flags.bits.z = v == 0;
            regs.flags.bits.n = oldCarry;

            return v;
        };
        instrucSet[0x6A] = {[this,ror]() {
            regs.a = ror(regs.a);
        }, "ROR", AddressMode::Accumulator, 2};
        instrucSet[0x66] = {[this,ror]() {
            auto addr = read(zeroPageAddress(fetch()));
            write(addr, ror(addr));
        }, "ROR", AddressMode::ZeroPage, 5};
        instrucSet[0x76] = {[this,ror]() {
            auto addr = read(indexedZeroPageAddress(fetch(), regs.x));
            write(addr, ror(addr));
        }, "ROR", AddressMode::ZeroPageX, 6};
        instrucSet[0x6E] = {[this,ror]() {
            auto addr = read(absoluteAddress(fetch(), fetch()));
            write(addr, ror(addr));
        }, "ROR", AddressMode::Absolute, 6};
        instrucSet[0x7E] = {[this,ror]() {
            auto addr = read(indexedAbsoluteAddress(fetch(), fetch(), regs.x));
            write(addr, ror(addr));
        }, "ROR", AddressMode::AbsoluteX, 7};
    }

    /*RTI*/ {
        instrucSet[0x40] = {[this]() {
            regs.flags.byte = pop();
            regs.pc = pop16();
        }, "RTI", AddressMode::Implicit, 6};
    }

    /*RTS*/ {
        instrucSet[0x60] = {[this]() {
            regs.pc = pop16() + 1;
        }, "RTS", AddressMode::Implicit, 6};
    }

    /*SBC*/ {
        auto sbc = [this](u8_t v) {
            u16_t result = regs.a - v - (~ regs.flags.bits.c);

            regs.flags.bits.c = (u16_t)result > UINT8_MAX; 
            regs.flags.bits.v = (i16_t)result > INT8_MAX || (i16_t)result < INT8_MAX; 

            regs.a = (u8_t)result;

            regs.flags.bits.z = regs.a == 0;
            regs.flags.bits.n = regs.a >> 7;
        };
        instrucSet[0xE9] = {[this,sbc]() {
            sbc(fetch());  
        }, "SBC", AddressMode::Immediate, 2};
        instrucSet[0xE5] = {[this,sbc]() {
            sbc(read(zeroPageAddress(fetch())));
        }, "SBC", AddressMode::ZeroPage, 3};
        instrucSet[0xF5] = {[this,sbc]() {
            sbc(read(indexedZeroPageAddress(fetch(), regs.x)));
        }, "SBC", AddressMode::ZeroPageX, 4};
        instrucSet[0xED] = {[this,sbc]() {
            sbc(read(absoluteAddress(fetch(), fetch())));
        }, "SBC", AddressMode::Absolute, 4};
        instrucSet[0xFD] = {[this,sbc]() {
            sbc(read(indexedAbsoluteAddress(fetch(), fetch(), regs.x)));
        }, "SBC", AddressMode::AbsoluteX, 4};
        instrucSet[0xF9] = {[this,sbc]() {
            sbc(read(indexedAbsoluteAddress(fetch(), fetch(), regs.y)));
        }, "SBC", AddressMode::AbsoluteY, 4};
        instrucSet[0xE1] = {[this,sbc]() {
            sbc(read(indexedIndirectAddress(fetch(), regs.x)));
        }, "SBC", AddressMode::IndexedIndirect, 6};
        instrucSet[0xF1] = {[this,sbc]() {
            sbc(read(indirectIndexedAddress(fetch(), regs.y)));
        }, "SBC", AddressMode::IndirectIndexed, 5};
    }

    /*SEC*/ {
        instrucSet[0x38] = {[this]() {
            regs.flags.bits.c = 1;
        }, "SEC", AddressMode::Implicit, 2};
    }

    /*SED*/ {
        instrucSet[0xF8] = {[this]() {
            regs.flags.bits.d = 1;
        }, "SED", AddressMode::Implicit, 2};
    }

    /*SEI*/ {
        instrucSet[0x78] = {[this]() {
            regs.flags.bits.i = 1;
        }, "SEI", AddressMode::Implicit, 2};
    }

    /*STA*/ {
        instrucSet[0x85] = {[this]() {
            write(read(zeroPageAddress(fetch())), regs.a);
        }, "STA", AddressMode::ZeroPage, 3};
        instrucSet[0x95] = {[this]() {
            write(read(indexedZeroPageAddress(fetch(), regs.x)), regs.a);
        }, "STA", AddressMode::ZeroPageX, 4};
        instrucSet[0x8D] = {[this]() {
            write(read(absoluteAddress(fetch(), fetch())), regs.a);
        }, "STA", AddressMode::Absolute, 4};
        instrucSet[0x9D] = {[this]() {
            write(read(indexedAbsoluteAddress(fetch(), fetch(), regs.x)), regs.a);
        }, "STA", AddressMode::AbsoluteX, 5};
        instrucSet[0x99] = {[this]() {
            write(read(indexedAbsoluteAddress(fetch(), fetch(), regs.y)), regs.a);
        }, "STA", AddressMode::AbsoluteY, 5};
        instrucSet[0x81] = {[this]() {
            write(read(indexedIndirectAddress(fetch(), regs.x)), regs.a);
        }, "STA", AddressMode::AbsoluteX, 6};
        instrucSet[0x91] = {[this]() {
            write(read(indirectIndexedAddress(fetch(), regs.y)), regs.a);
        }, "STA", AddressMode::IndirectIndexed, 6};
    }

    /*STX*/ {
        instrucSet[0x86] = {[this]() {
            write(read(zeroPageAddress(fetch())), regs.x);
        }, "STX", AddressMode::ZeroPage, 3};
        instrucSet[0x96] = {[this]() {
            write(read(indexedZeroPageAddress(fetch(), regs.y)), regs.x);
        }, "STX", AddressMode::ZeroPageY, 4};
        instrucSet[0x8E] = {[this]() {
            write(read(absoluteAddress(fetch(), fetch())), regs.x);
        }, "STX", AddressMode::Absolute, 4};
    }

    /*STY*/ {
        instrucSet[0x84] = {[this]() {
            write(read(zeroPageAddress(fetch())), regs.y);
        }, "STY", AddressMode::ZeroPage, 3};
        instrucSet[0x94] = {[this]() {
            write(read(indexedZeroPageAddress(fetch(), regs.x)), regs.y);
        }, "STY", AddressMode::ZeroPageX, 4};
        instrucSet[0x8C] = {[this]() {
            write(read(absoluteAddress(fetch(), fetch())), regs.y);
        }, "STY", AddressMode::Absolute, 4};
    }        

    /*TAX*/ {
        instrucSet[0xAA] = {[this]() {
            regs.x = regs.a;
            regs.flags.bits.z = regs.x == 0;
            regs.flags.bits.n = regs.x >> 7;
        }, "TAX", AddressMode::Implicit, 2};
    }

    /*TAY*/ {
        instrucSet[0xA8] = {[this]() {
            regs.y = regs.a;
            regs.flags.bits.z = regs.y == 0;
            regs.flags.bits.n = regs.y >> 7;
        }, "TAY", AddressMode::Implicit, 2};
    }

    /*TSX*/ {
        instrucSet[0xBA] = {[this]() {
            regs.x = regs.sp;
            regs.flags.bits.z = regs.x == 0;
            regs.flags.bits.n = regs.x >> 7;
        }, "TSX", AddressMode::Implicit, 2};
    }

    /*TXA*/ {
        instrucSet[0x8A] = {[this]() {
            regs.a = regs.x;
            regs.flags.bits.z = regs.x == 0;
            regs.flags.bits.n = regs.x >> 7;
        }, "TXA", AddressMode::Implicit, 2};
    }

    /*TXS*/ {
        instrucSet[0x9A] = {[this]() {
            regs.sp = regs.x;
            regs.flags.bits.z = regs.x == 0;
            regs.flags.bits.n = regs.x >> 7;
        }, "TXS", AddressMode::Implicit, 2};
    }

    /*TYA*/ {
        instrucSet[0x98] = {[this]() {
            regs.a = regs.y;
            regs.flags.bits.z = regs.a == 0;
            regs.flags.bits.n = regs.a >> 7;
        }, "TYA", AddressMode::Implicit, 2};
    }
}

int Console::init(ROM* rom) {
    loadInstructions();

    if (rom) {
        int err = rom->copyToMemory(&memory);
        if (err != 0) {
            return err;
        }
    } else {
        INFO("no rom");
    }

    powerOn();

    INFO("finished initalizing Console device");
    return 0;
}