#include <array>
#include <string>
#include <functional>

#include "emulation/common.h"
#include "emulation/console.h"
#include "emulation/instructions.h"
#include "stdtype.h"
#include "logger.h"

using namespace std;

void ADC(Console& dev) {
    auto v = dev.getArgValue();
    
    u16_t result = dev.regs.a + v + dev.regs.flags.bits.c;

    dev.regs.flags.bits.c = (u16_t)result > UINT8_MAX; 
    dev.regs.flags.bits.v = (i16_t)result > INT8_MAX || (i16_t)result < INT8_MIN; 

    dev.regs.a = (u8_t)result;

    dev.regs.flags.bits.z = dev.regs.a == 0;
    dev.regs.flags.bits.n = dev.regs.a >> 7;
}

void AHX(Console& dev) {
    ERROR("invalid/unsupported opcode was called");
}

void ALR(Console& dev) {
    ERROR("invalid/unsupported opcode was called");
}

void ANC(Console& dev) {
    ERROR("invalid/unsupported opcode was called");
}

void AND(Console& dev) {
    auto v = dev.getArgValue();

    dev.regs.a = dev.regs.a & v;
    dev.regs.flags.bits.z = dev.regs.a == 0;
    dev.regs.flags.bits.n = dev.regs.a >> 7;
}

void ARR(Console& dev) {
    ERROR("invalid/unsupported opcode was called");
}

void ASL(Console& dev) {
    auto v = dev.getArgValue();

    dev.regs.flags.bits.c = v >> 7;
    v <<= 1;
    dev.regs.flags.bits.z = v == 0; // TODO: not sure if Accumulator only or any value
    dev.regs.flags.bits.n = v >> 7;

    dev.writeArg(v);
}

void AXS(Console& dev) {
    ERROR("invalid/unsupported opcode was called");
}

void BCC(Console& dev) {
    if (!dev.regs.flags.bits.c) { 
        auto fetched = (i8_t)dev.getArgValue();
        dev.regs.pc += fetched;
        dev.cpuCycles++;
    } else {
        dev.noCrossPage();
    }
}

void BCS(Console& dev) {
    if (dev.regs.flags.bits.c) { 
        auto fetched = (i8_t)dev.getArgValue();
        dev.regs.pc += fetched;
        dev.cpuCycles++;
    } else {
        dev.noCrossPage();
    }
}

void BEQ(Console& dev) {
    if (dev.regs.flags.bits.z) { 
        auto fetched = (i8_t)dev.getArgValue();
        dev.regs.pc += fetched;
        dev.cpuCycles++;
    } else {
        dev.noCrossPage();
    }
}

void BIT(Console& dev) {
    auto v = dev.getArgValue();

    dev.regs.flags.bits.z = (v & dev.regs.a) == 0;
    dev.regs.flags.bits.v = v >> 6;
    dev.regs.flags.bits.n = v >> 7;
}

void BMI(Console& dev) {
    if (dev.regs.flags.bits.n) { 
        auto fetched = (i8_t)dev.getArgValue();
        dev.regs.pc += fetched;
        dev.cpuCycles++;
    } else {
        dev.noCrossPage();
    }
}

void BNE(Console& dev) {
    if (!dev.regs.flags.bits.z) { 
        auto fetched = (i8_t)dev.getArgValue();
        dev.regs.pc += fetched;
        dev.cpuCycles++;
    } else {
        dev.noCrossPage();
    }
}

void BPL(Console& dev) {
    if (!dev.regs.flags.bits.n) { 
        auto fetched = (i8_t)dev.getArgValue();
        dev.regs.pc += fetched;
        dev.cpuCycles++;
    } else {
        dev.noCrossPage();
    }
}

void BRK(Console& dev) {
    if (dev.regs.flags.bits.i == 1) return;

    dev.push(dev.regs.pc);
    dev.push(dev.regs.flags.byte);
    dev.regs.pc = dev.read16(IRQ); 
    dev.regs.flags.bits.b = 1;
    dev.regs.flags.bits.i = 1;
}

void BVC(Console& dev) {
    if (!dev.regs.flags.bits.v) { 
        auto fetched = (i8_t)dev.getArgValue();
        dev.regs.pc += fetched;
        dev.cpuCycles++;
    } else {
        dev.noCrossPage();
    }
}

void BVS(Console& dev) {
    if (dev.regs.flags.bits.v) { 
        auto fetched = (i8_t)dev.getArgValue();
        dev.regs.pc += fetched;
        dev.cpuCycles++;
    } else {
        dev.noCrossPage();
    }
}

void CLC(Console& dev) {
    dev.regs.flags.bits.c = 0;
}

void CLD(Console& dev) {
    dev.regs.flags.bits.d = 0;
}

void CLI(Console& dev) {
    dev.regs.flags.bits.i = 0;
}

void CLV(Console& dev) {
    dev.regs.flags.bits.v = 0;
}

void CMP(Console& dev) {
    auto v = dev.getArgValue();
    u8_t result = dev.regs.a - v;
    dev.regs.flags.bits.c = result > 0;
    dev.regs.flags.bits.z = result == 0;
    dev.regs.flags.bits.n = result >> 7;
}

void CPX(Console& dev) {
    auto v = dev.getArgValue();
    u8_t result = dev.regs.x - v;
    dev.regs.flags.bits.c = result > 0;
    dev.regs.flags.bits.z = result == 0;
    dev.regs.flags.bits.n = result >> 7;
}

void CPY(Console& dev) {
    auto v = dev.getArgValue();
    u8_t result = dev.regs.y - v;
    dev.regs.flags.bits.c = result > 0;
    dev.regs.flags.bits.z = result == 0;
    dev.regs.flags.bits.n = result >> 7;
}

void DCP(Console& dev) {
    ERROR("invalid/unsupported opcode was called");
}

void DEC(Console& dev) {
    auto v = dev.getArgValue();
    v--;

    dev.regs.flags.bits.z = v == 0;
    dev.regs.flags.bits.n = v >> 7;

    dev.writeArg(v);
}

void DEX(Console& dev) {
    dev.regs.x--;
    dev.regs.flags.bits.z = dev.regs.x == 0;
    dev.regs.flags.bits.n = dev.regs.x >> 7;
}

void DEY(Console& dev) {
    dev.regs.y--;
    dev.regs.flags.bits.z = dev.regs.y == 0;
    dev.regs.flags.bits.n = dev.regs.y >> 7;
}

void EOR(Console& dev) {
    auto v = dev.getArgValue();
    dev.regs.a ^= v;

    dev.regs.flags.bits.z = dev.regs.a == 0;
    dev.regs.flags.bits.n = dev.regs.a >> 7;
}

void INC(Console& dev) {
    auto v = dev.getArgValue();
    v++;

    dev.regs.flags.bits.z = v == 0;
    dev.regs.flags.bits.n = v >> 7;

    dev.writeArg(v);
}

void INX(Console& dev) {
    dev.regs.x++;
    dev.regs.flags.bits.z = dev.regs.x == 0;
    dev.regs.flags.bits.n = dev.regs.x >> 7;
}

void INY(Console& dev) {
    dev.regs.y++;
    dev.regs.flags.bits.z = dev.regs.y == 0;
    dev.regs.flags.bits.n = dev.regs.y >> 7;
}

void ISC(Console& dev) {
    ERROR("invalid/unsupported opcode was called");
}

void JMP(Console& dev) {
    dev.reprepareJMPArg();
    dev.regs.pc = dev.getArgAddr();
}

void JSR(Console& dev) {
    dev.push16(dev.regs.pc);
    dev.regs.pc = dev.getArgAddr();
}

void KIL(Console& dev) {
    ERROR("KIL was called");
}

void LAS(Console& dev) {
    ERROR("invalid/unsupported opcode was called");
}

void LAX(Console& dev) {
    ERROR("invalid/unsupported opcode was called");
}

void LDA(Console& dev) {
    auto v = dev.getArgValue();
    dev.regs.a = v;

    dev.regs.flags.bits.z = dev.regs.a == 0;
    dev.regs.flags.bits.n = dev.regs.a >> 7;
}

void LDX(Console& dev) {
    auto v = dev.getArgValue();
    dev.regs.x = v;

    dev.regs.flags.bits.z = dev.regs.x == 0;
    dev.regs.flags.bits.n = dev.regs.x >> 7;
}

void LDY(Console& dev) {
    auto v = dev.getArgValue();
    dev.regs.y = v;

    dev.regs.flags.bits.z = dev.regs.y == 0;
    dev.regs.flags.bits.n = dev.regs.y >> 7;
}

void LSR(Console& dev) {
    auto v = dev.getArgValue();
    dev.regs.flags.bits.c = v & 1;
            
    v >>= 1;

    dev.regs.flags.bits.z = v == 0;
    dev.regs.flags.bits.n = 0;

    dev.writeArg(v);
}

void NOP(Console& dev) {}

void ORA(Console& dev) {
    auto v = dev.getArgValue();
    dev.regs.a |= v;

    dev.regs.flags.bits.z = dev.regs.a == 0;
    dev.regs.flags.bits.n = dev.regs.a >> 7;
}

void PHA(Console& dev) {
    dev.push(dev.regs.a);
}

void PHP(Console& dev) {
    dev.push(dev.regs.flags.byte);
}

void PLA(Console& dev) {
    dev.regs.a = dev.pop();
}

void PLP(Console& dev) {
    dev.regs.flags.byte = dev.pop();
}

void RLA(Console& dev) {
    ERROR("invalid/unsupported opcode was called");
}

void ROL(Console& dev) {
    auto v = dev.getArgValue();
    u8_t oldCarry = dev.regs.flags.bits.c;
    dev.regs.flags.bits.c = v >> 7;
    
    v <<= 1;
    v |= oldCarry;

    dev.regs.flags.bits.z = v == 0;
    dev.regs.flags.bits.n = v >> 7;

    dev.writeArg(v);
}

void ROR(Console& dev) {
    auto v = dev.getArgValue();
    u8_t oldCarry = dev.regs.flags.bits.c;
    dev.regs.flags.bits.c = v & 1;
    
    v >>= 1;
    v |= oldCarry << 7;

    dev.regs.flags.bits.z = v == 0;
    dev.regs.flags.bits.n = oldCarry;

    dev.writeArg(v);
}

void RRA(Console& dev) {
    ERROR("invalid/unsupported opcode was called");
}

void RTI(Console& dev) {
    dev.regs.flags.byte = dev.pop();
    dev.regs.pc = dev.pop16();
}

void RTS(Console& dev) {
    dev.regs.pc = dev.pop16() + 1;
}

void SAX(Console& dev) {
    ERROR("invalid/unsupported opcode was called");
}

void SBC(Console& dev) {
    auto v = dev.getArgValue();
    u16_t result = dev.regs.a - v - (~ dev.regs.flags.bits.c);

    dev.regs.flags.bits.c = (u16_t)result > UINT8_MAX; 
    dev.regs.flags.bits.v = (i16_t)result > INT8_MAX || (i16_t)result < INT8_MAX; 

    dev.regs.a = (u8_t)result;

    dev.regs.flags.bits.z = dev.regs.a == 0;
    dev.regs.flags.bits.n = dev.regs.a >> 7;
}

void SEC(Console& dev) {
    dev.regs.flags.bits.c = 1;
}

void SED(Console& dev) {
    dev.regs.flags.bits.d = 1;
}

void SEI(Console& dev) {
    dev.regs.flags.bits.i = 1;
}

void SHX(Console& dev) {
    ERROR("invalid/unsupported opcode was called");
}

void SHY(Console& dev) {
    ERROR("invalid/unsupported opcode was called");
}

void SLO(Console& dev) {
    ERROR("invalid/unsupported opcode was called");
}

void SRE(Console& dev) {
    ERROR("invalid/unsupported opcode was called");
}

void STA(Console& dev) {
    dev.writeArg(dev.regs.a);
}

void STX(Console& dev) {
    dev.writeArg(dev.regs.x);
}

void STY(Console& dev) {
    dev.writeArg(dev.regs.y);
}

void TAS(Console& dev) {
    ERROR("invalid/unsupported opcode was called");
}

void TAX(Console& dev) {
    dev.regs.x = dev.regs.a;
    dev.regs.flags.bits.z = dev.regs.x == 0;
    dev.regs.flags.bits.n = dev.regs.x >> 7;
}

void TAY(Console& dev) {
    dev.regs.y = dev.regs.a;
    dev.regs.flags.bits.z = dev.regs.y == 0;
    dev.regs.flags.bits.n = dev.regs.y >> 7;
}

void TSX(Console& dev) {
    dev.regs.x = dev.regs.sp;
    dev.regs.flags.bits.z = dev.regs.x == 0;
    dev.regs.flags.bits.n = dev.regs.x >> 7;
}

void TXA(Console& dev) {
    dev.regs.a = dev.regs.x;
    dev.regs.flags.bits.z = dev.regs.x == 0;
    dev.regs.flags.bits.n = dev.regs.x >> 7;
}

void TXS(Console& dev) {
    dev.regs.sp = dev.regs.x;
    dev.regs.flags.bits.z = dev.regs.x == 0;
    dev.regs.flags.bits.n = dev.regs.x >> 7;
}

void TYA(Console& dev) {
    dev.regs.a = dev.regs.y;
    dev.regs.flags.bits.z = dev.regs.a == 0;
    dev.regs.flags.bits.n = dev.regs.a >> 7;
}

void XAA(Console& dev) {
    ERROR("invalid/unsupported opcode was called");
}

#define _DEF(x) x, #x
const InstructionSet instructionSet{
    Instruction{_DEF(BRK), AddressMode::Implicit,        7, 0},
    Instruction{_DEF(ORA), AddressMode::IndexedIndirect, 6, 0},
    Instruction{_DEF(KIL), AddressMode::Implicit,        0, 0},
    Instruction{_DEF(SLO), AddressMode::IndexedIndirect, 8, 0},
    Instruction{_DEF(NOP), AddressMode::ZeroPage,        3, 0},
    Instruction{_DEF(ORA), AddressMode::ZeroPage,        3, 0},
    Instruction{_DEF(ASL), AddressMode::ZeroPage,        5, 0},
    Instruction{_DEF(SLO), AddressMode::ZeroPage,        5, 0},
    Instruction{_DEF(PHP), AddressMode::Implicit,        3, 0},
    Instruction{_DEF(ORA), AddressMode::Immediate,       2, 0},
    Instruction{_DEF(ASL), AddressMode::Implicit,        2, 0},
    Instruction{_DEF(ANC), AddressMode::Immediate,       2, 0},
    Instruction{_DEF(NOP), AddressMode::Absolute,        4, 0},
    Instruction{_DEF(ORA), AddressMode::Absolute,        4, 0},
    Instruction{_DEF(ASL), AddressMode::Absolute,        6, 0},
    Instruction{_DEF(SLO), AddressMode::Absolute,        6, 0},
    Instruction{_DEF(BPL), AddressMode::Relative,        2, 1},
    Instruction{_DEF(ORA), AddressMode::IndirectIndexed, 5, 1},
    Instruction{_DEF(KIL), AddressMode::Implicit,        0, 0},
    Instruction{_DEF(SLO), AddressMode::IndirectIndexed, 8, 0},
    Instruction{_DEF(NOP), AddressMode::ZeroPageX,       4, 0},
    Instruction{_DEF(ORA), AddressMode::ZeroPageX,       4, 0},
    Instruction{_DEF(ASL), AddressMode::ZeroPageX,       6, 0},
    Instruction{_DEF(SLO), AddressMode::ZeroPageX,       6, 0},
    Instruction{_DEF(CLC), AddressMode::Implicit,        2, 0},
    Instruction{_DEF(ORA), AddressMode::AbsoluteY,       4, 1},
    Instruction{_DEF(NOP), AddressMode::Implicit,        2, 0},
    Instruction{_DEF(SLO), AddressMode::AbsoluteY,       7, 0},
    Instruction{_DEF(NOP), AddressMode::AbsoluteX,       4, 1},
    Instruction{_DEF(ORA), AddressMode::AbsoluteX,       4, 1},
    Instruction{_DEF(ASL), AddressMode::AbsoluteX,       7, 0},
    Instruction{_DEF(SLO), AddressMode::AbsoluteX,       7, 0},
    Instruction{_DEF(JSR), AddressMode::Absolute,        6, 0},
    Instruction{_DEF(AND), AddressMode::IndexedIndirect, 6, 0},
    Instruction{_DEF(KIL), AddressMode::Implicit,        0, 0},
    Instruction{_DEF(RLA), AddressMode::IndexedIndirect, 8, 0},
    Instruction{_DEF(BIT), AddressMode::ZeroPage,        3, 0},
    Instruction{_DEF(AND), AddressMode::ZeroPage,        3, 0},
    Instruction{_DEF(ROL), AddressMode::ZeroPage,        5, 0},
    Instruction{_DEF(RLA), AddressMode::ZeroPage,        5, 0},
    Instruction{_DEF(PLP), AddressMode::Implicit,        4, 0},
    Instruction{_DEF(AND), AddressMode::Immediate,       2, 0},
    Instruction{_DEF(ROL), AddressMode::Implicit,        2, 0},
    Instruction{_DEF(ANC), AddressMode::Immediate,       2, 0},
    Instruction{_DEF(BIT), AddressMode::Absolute,        4, 0},
    Instruction{_DEF(AND), AddressMode::Absolute,        4, 0},
    Instruction{_DEF(ROL), AddressMode::Absolute,        6, 0},
    Instruction{_DEF(RLA), AddressMode::Absolute,        6, 0},
    Instruction{_DEF(BMI), AddressMode::Relative,        2, 1},
    Instruction{_DEF(AND), AddressMode::IndirectIndexed, 5, 1},
    Instruction{_DEF(KIL), AddressMode::Implicit,        0, 0},
    Instruction{_DEF(RLA), AddressMode::IndirectIndexed, 8, 0},
    Instruction{_DEF(NOP), AddressMode::ZeroPageX,       4, 0},
    Instruction{_DEF(AND), AddressMode::ZeroPageX,       4, 0},
    Instruction{_DEF(ROL), AddressMode::ZeroPageX,       6, 0},
    Instruction{_DEF(RLA), AddressMode::ZeroPageX,       6, 0},
    Instruction{_DEF(SEC), AddressMode::Implicit,        2, 0},
    Instruction{_DEF(AND), AddressMode::AbsoluteY,       4, 1},
    Instruction{_DEF(NOP), AddressMode::Implicit,        2, 0},
    Instruction{_DEF(RLA), AddressMode::AbsoluteY,       7, 0},
    Instruction{_DEF(NOP), AddressMode::AbsoluteX,       4, 1},
    Instruction{_DEF(AND), AddressMode::AbsoluteX,       4, 1},
    Instruction{_DEF(ROL), AddressMode::AbsoluteX,       7, 0},
    Instruction{_DEF(RLA), AddressMode::AbsoluteX,       7, 0},
    Instruction{_DEF(RTI), AddressMode::Implicit,        6, 0},
    Instruction{_DEF(EOR), AddressMode::IndexedIndirect, 6, 0},
    Instruction{_DEF(KIL), AddressMode::Implicit,        0, 0},
    Instruction{_DEF(SRE), AddressMode::IndexedIndirect, 8, 0},
    Instruction{_DEF(NOP), AddressMode::ZeroPage,        3, 0},
    Instruction{_DEF(EOR), AddressMode::ZeroPage,        3, 0},
    Instruction{_DEF(LSR), AddressMode::ZeroPage,        5, 0},
    Instruction{_DEF(SRE), AddressMode::ZeroPage,        5, 0},
    Instruction{_DEF(PHA), AddressMode::Implicit,        3, 0},
    Instruction{_DEF(EOR), AddressMode::Immediate,       2, 0},
    Instruction{_DEF(LSR), AddressMode::Implicit,        2, 0},
    Instruction{_DEF(ALR), AddressMode::Immediate,       2, 0},
    Instruction{_DEF(JMP), AddressMode::Absolute,        3, 0},
    Instruction{_DEF(EOR), AddressMode::Absolute,        4, 0},
    Instruction{_DEF(LSR), AddressMode::Absolute,        6, 0},
    Instruction{_DEF(SRE), AddressMode::Absolute,        6, 0},
    Instruction{_DEF(BVC), AddressMode::Relative,        2, 1},
    Instruction{_DEF(EOR), AddressMode::IndirectIndexed, 5, 1},
    Instruction{_DEF(KIL), AddressMode::Implicit,        0, 0},
    Instruction{_DEF(SRE), AddressMode::IndirectIndexed, 8, 0},
    Instruction{_DEF(NOP), AddressMode::ZeroPageX,       4, 0},
    Instruction{_DEF(EOR), AddressMode::ZeroPageX,       4, 0},
    Instruction{_DEF(LSR), AddressMode::ZeroPageX,       6, 0},
    Instruction{_DEF(SRE), AddressMode::ZeroPageX,       6, 0},
    Instruction{_DEF(CLI), AddressMode::Implicit,        2, 0},
    Instruction{_DEF(EOR), AddressMode::AbsoluteY,       4, 1},
    Instruction{_DEF(NOP), AddressMode::Implicit,        2, 0},
    Instruction{_DEF(SRE), AddressMode::AbsoluteY,       7, 0},
    Instruction{_DEF(NOP), AddressMode::AbsoluteX,       4, 1},
    Instruction{_DEF(EOR), AddressMode::AbsoluteX,       4, 1},
    Instruction{_DEF(LSR), AddressMode::AbsoluteX,       7, 0},
    Instruction{_DEF(SRE), AddressMode::AbsoluteX,       7, 0},
    Instruction{_DEF(RTS), AddressMode::Implicit,        6, 0},
    Instruction{_DEF(ADC), AddressMode::IndexedIndirect, 6, 0},
    Instruction{_DEF(KIL), AddressMode::Implicit,        0, 0},
    Instruction{_DEF(RRA), AddressMode::IndexedIndirect, 8, 0},
    Instruction{_DEF(NOP), AddressMode::ZeroPage,        3, 0},
    Instruction{_DEF(ADC), AddressMode::ZeroPage,        3, 0},
    Instruction{_DEF(ROR), AddressMode::ZeroPage,        5, 0},
    Instruction{_DEF(RRA), AddressMode::ZeroPage,        5, 0},
    Instruction{_DEF(PLA), AddressMode::Implicit,        4, 0},
    Instruction{_DEF(ADC), AddressMode::Immediate,       2, 0},
    Instruction{_DEF(ROR), AddressMode::Implicit,        2, 0},
    Instruction{_DEF(ARR), AddressMode::Immediate,       2, 0},
    Instruction{_DEF(JMP), AddressMode::Indirect,        5, 0},
    Instruction{_DEF(ADC), AddressMode::Absolute,        4, 0},
    Instruction{_DEF(ROR), AddressMode::Absolute,        6, 0},
    Instruction{_DEF(RRA), AddressMode::Absolute,        6, 0},
    Instruction{_DEF(BVS), AddressMode::Relative,        2, 1},
    Instruction{_DEF(ADC), AddressMode::IndirectIndexed, 5, 1},
    Instruction{_DEF(KIL), AddressMode::Implicit,        0, 0},
    Instruction{_DEF(RRA), AddressMode::IndirectIndexed, 8, 0},
    Instruction{_DEF(NOP), AddressMode::ZeroPageX,       4, 0},
    Instruction{_DEF(ADC), AddressMode::ZeroPageX,       4, 0},
    Instruction{_DEF(ROR), AddressMode::ZeroPageX,       6, 0},
    Instruction{_DEF(RRA), AddressMode::ZeroPageX,       6, 0},
    Instruction{_DEF(SEI), AddressMode::Implicit,        2, 0},
    Instruction{_DEF(ADC), AddressMode::AbsoluteY,       4, 1},
    Instruction{_DEF(NOP), AddressMode::Implicit,        2, 0},
    Instruction{_DEF(RRA), AddressMode::AbsoluteY,       7, 0},
    Instruction{_DEF(NOP), AddressMode::AbsoluteX,       4, 1},
    Instruction{_DEF(ADC), AddressMode::AbsoluteX,       4, 1},
    Instruction{_DEF(ROR), AddressMode::AbsoluteX,       7, 0},
    Instruction{_DEF(RRA), AddressMode::AbsoluteX,       7, 0},
    Instruction{_DEF(NOP), AddressMode::Immediate,       2, 0},
    Instruction{_DEF(STA), AddressMode::IndexedIndirect, 6, 0},
    Instruction{_DEF(NOP), AddressMode::Immediate,       2, 0},
    Instruction{_DEF(SAX), AddressMode::IndexedIndirect, 6, 0},
    Instruction{_DEF(STY), AddressMode::ZeroPage,        3, 0},
    Instruction{_DEF(STA), AddressMode::ZeroPage,        3, 0},
    Instruction{_DEF(STX), AddressMode::ZeroPage,        3, 0},
    Instruction{_DEF(SAX), AddressMode::ZeroPage,        3, 0},
    Instruction{_DEF(DEY), AddressMode::Implicit,        2, 0},
    Instruction{_DEF(NOP), AddressMode::Immediate,       2, 0},
    Instruction{_DEF(TXA), AddressMode::Implicit,        2, 0},
    Instruction{_DEF(XAA), AddressMode::Immediate,       2, 0},
    Instruction{_DEF(STY), AddressMode::Absolute,        4, 0},
    Instruction{_DEF(STA), AddressMode::Absolute,        4, 0},
    Instruction{_DEF(STX), AddressMode::Absolute,        4, 0},
    Instruction{_DEF(SAX), AddressMode::Absolute,        4, 0},
    Instruction{_DEF(BCC), AddressMode::Relative,        2, 1},
    Instruction{_DEF(STA), AddressMode::IndirectIndexed, 6, 0},
    Instruction{_DEF(KIL), AddressMode::Implicit,        0, 0},
    Instruction{_DEF(AHX), AddressMode::IndirectIndexed, 6, 0},
    Instruction{_DEF(STY), AddressMode::ZeroPageX,       4, 0},
    Instruction{_DEF(STA), AddressMode::ZeroPageX,       4, 0},
    Instruction{_DEF(STX), AddressMode::ZeroPageY,       4, 0},
    Instruction{_DEF(SAX), AddressMode::ZeroPageY,       4, 0},
    Instruction{_DEF(TYA), AddressMode::Implicit,        2, 0},
    Instruction{_DEF(STA), AddressMode::AbsoluteY,       5, 0},
    Instruction{_DEF(TXS), AddressMode::Implicit,        2, 0},
    Instruction{_DEF(TAS), AddressMode::AbsoluteY,       5, 0},
    Instruction{_DEF(SHY), AddressMode::AbsoluteX,       5, 0},
    Instruction{_DEF(STA), AddressMode::AbsoluteX,       5, 0},
    Instruction{_DEF(SHX), AddressMode::AbsoluteY,       5, 0},
    Instruction{_DEF(AHX), AddressMode::AbsoluteY,       5, 0},
    Instruction{_DEF(LDY), AddressMode::Immediate,       2, 0},
    Instruction{_DEF(LDA), AddressMode::IndexedIndirect, 6, 0},
    Instruction{_DEF(LDX), AddressMode::Immediate,       2, 0},
    Instruction{_DEF(LAX), AddressMode::IndexedIndirect, 6, 0},
    Instruction{_DEF(LDY), AddressMode::ZeroPage,        3, 0},
    Instruction{_DEF(LDA), AddressMode::ZeroPage,        3, 0},
    Instruction{_DEF(LDX), AddressMode::ZeroPage,        3, 0},
    Instruction{_DEF(LAX), AddressMode::ZeroPage,        3, 0},
    Instruction{_DEF(TAY), AddressMode::Implicit,        2, 0},
    Instruction{_DEF(LDA), AddressMode::Immediate,       2, 0},
    Instruction{_DEF(TAX), AddressMode::Implicit,        2, 0},
    Instruction{_DEF(LAX), AddressMode::Immediate,       2, 0},
    Instruction{_DEF(LDY), AddressMode::Absolute,        4, 0},
    Instruction{_DEF(LDA), AddressMode::Absolute,        4, 0},
    Instruction{_DEF(LDX), AddressMode::Absolute,        4, 0},
    Instruction{_DEF(LAX), AddressMode::Absolute,        4, 0},
    Instruction{_DEF(BCS), AddressMode::Relative,        2, 1},
    Instruction{_DEF(LDA), AddressMode::IndirectIndexed, 5, 1},
    Instruction{_DEF(KIL), AddressMode::Implicit,        0, 0},
    Instruction{_DEF(LAX), AddressMode::IndirectIndexed, 5, 1},
    Instruction{_DEF(LDY), AddressMode::ZeroPageX,       4, 0},
    Instruction{_DEF(LDA), AddressMode::ZeroPageX,       4, 0},
    Instruction{_DEF(LDX), AddressMode::ZeroPageY,       4, 0},
    Instruction{_DEF(LAX), AddressMode::ZeroPageY,       4, 0},
    Instruction{_DEF(CLV), AddressMode::Implicit,        2, 0},
    Instruction{_DEF(LDA), AddressMode::AbsoluteY,       4, 1},
    Instruction{_DEF(TSX), AddressMode::Implicit,        2, 0},
    Instruction{_DEF(LAS), AddressMode::AbsoluteY,       4, 1},
    Instruction{_DEF(LDY), AddressMode::AbsoluteX,       4, 1},
    Instruction{_DEF(LDA), AddressMode::AbsoluteX,       4, 1},
    Instruction{_DEF(LDX), AddressMode::AbsoluteY,       4, 1},
    Instruction{_DEF(LAX), AddressMode::AbsoluteY,       4, 1},
    Instruction{_DEF(CPY), AddressMode::Immediate,       2, 0},
    Instruction{_DEF(CMP), AddressMode::IndexedIndirect, 6, 0},
    Instruction{_DEF(NOP), AddressMode::Immediate,       2, 0},
    Instruction{_DEF(DCP), AddressMode::IndexedIndirect, 8, 0},
    Instruction{_DEF(CPY), AddressMode::ZeroPage,        3, 0},
    Instruction{_DEF(CMP), AddressMode::ZeroPage,        3, 0},
    Instruction{_DEF(DEC), AddressMode::ZeroPage,        5, 0},
    Instruction{_DEF(DCP), AddressMode::ZeroPage,        5, 0},
    Instruction{_DEF(INY), AddressMode::Implicit,        2, 0},
    Instruction{_DEF(CMP), AddressMode::Immediate,       2, 0},
    Instruction{_DEF(DEX), AddressMode::Implicit,        2, 0},
    Instruction{_DEF(AXS), AddressMode::Immediate,       2, 0},
    Instruction{_DEF(CPY), AddressMode::Absolute,        4, 0},
    Instruction{_DEF(CMP), AddressMode::Absolute,        4, 0},
    Instruction{_DEF(DEC), AddressMode::Absolute,        6, 0},
    Instruction{_DEF(DCP), AddressMode::Absolute,        6, 0},
    Instruction{_DEF(BNE), AddressMode::Relative,        2, 1},
    Instruction{_DEF(CMP), AddressMode::IndirectIndexed, 5, 1},
    Instruction{_DEF(KIL), AddressMode::Implicit,        0, 0},
    Instruction{_DEF(DCP), AddressMode::IndirectIndexed, 8, 0},
    Instruction{_DEF(NOP), AddressMode::ZeroPageX,       4, 0},
    Instruction{_DEF(CMP), AddressMode::ZeroPageX,       4, 0},
    Instruction{_DEF(DEC), AddressMode::ZeroPageX,       6, 0},
    Instruction{_DEF(DCP), AddressMode::ZeroPageX,       6, 0},
    Instruction{_DEF(CLD), AddressMode::Implicit,        2, 0},
    Instruction{_DEF(CMP), AddressMode::AbsoluteY,       4, 1},
    Instruction{_DEF(NOP), AddressMode::Implicit,        2, 0},
    Instruction{_DEF(DCP), AddressMode::AbsoluteY,       7, 0},
    Instruction{_DEF(NOP), AddressMode::AbsoluteX,       4, 1},
    Instruction{_DEF(CMP), AddressMode::AbsoluteX,       4, 1},
    Instruction{_DEF(DEC), AddressMode::AbsoluteX,       7, 0},
    Instruction{_DEF(DCP), AddressMode::AbsoluteX,       7, 0},
    Instruction{_DEF(CPX), AddressMode::Immediate,       2, 0},
    Instruction{_DEF(SBC), AddressMode::IndexedIndirect, 6, 0},
    Instruction{_DEF(NOP), AddressMode::Immediate,       2, 0},
    Instruction{_DEF(ISC), AddressMode::IndexedIndirect, 8, 0},
    Instruction{_DEF(CPX), AddressMode::ZeroPage,        3, 0},
    Instruction{_DEF(SBC), AddressMode::ZeroPage,        3, 0},
    Instruction{_DEF(INC), AddressMode::ZeroPage,        5, 0},
    Instruction{_DEF(ISC), AddressMode::ZeroPage,        5, 0},
    Instruction{_DEF(INX), AddressMode::Implicit,        2, 0},
    Instruction{_DEF(SBC), AddressMode::Immediate,       2, 0},
    Instruction{_DEF(NOP), AddressMode::Implicit,        2, 0},
    Instruction{_DEF(SBC), AddressMode::Immediate,       2, 0},
    Instruction{_DEF(CPX), AddressMode::Absolute,        4, 0},
    Instruction{_DEF(SBC), AddressMode::Absolute,        4, 0},
    Instruction{_DEF(INC), AddressMode::Absolute,        6, 0},
    Instruction{_DEF(ISC), AddressMode::Absolute,        6, 0},
    Instruction{_DEF(BEQ), AddressMode::Relative,        2, 1},
    Instruction{_DEF(SBC), AddressMode::IndirectIndexed, 5, 1},
    Instruction{_DEF(KIL), AddressMode::Implicit,        0, 0},
    Instruction{_DEF(ISC), AddressMode::IndirectIndexed, 8, 0},
    Instruction{_DEF(NOP), AddressMode::ZeroPageX,       4, 0},
    Instruction{_DEF(SBC), AddressMode::ZeroPageX,       4, 0},
    Instruction{_DEF(INC), AddressMode::ZeroPageX,       6, 0},
    Instruction{_DEF(ISC), AddressMode::ZeroPageX,       6, 0},
    Instruction{_DEF(SED), AddressMode::Implicit,        2, 0},
    Instruction{_DEF(SBC), AddressMode::AbsoluteY,       4, 1},
    Instruction{_DEF(NOP), AddressMode::Implicit,        2, 0},
    Instruction{_DEF(ISC), AddressMode::AbsoluteY,       7, 0},
    Instruction{_DEF(NOP), AddressMode::AbsoluteX,       4, 1},
    Instruction{_DEF(SBC), AddressMode::AbsoluteX,       4, 1},
    Instruction{_DEF(INC), AddressMode::AbsoluteX,       7, 0},
    Instruction{_DEF(ISC), AddressMode::AbsoluteX,       7, 0}
};