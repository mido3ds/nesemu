#include <array>
#include <functional>

#include "common.h"
#include "CPU.h"
#include "instructions.h"

void ADC(CPU& cpu) {
    auto v = cpu.arg_value;

    uint16_t result = cpu.regs.a + v + cpu.regs.flags.bits.c;

    cpu.regs.flags.bits.c = (uint16_t)result > UINT8_MAX;
    cpu.regs.flags.bits.v = (int16_t)result > INT8_MAX || (int16_t)result < INT8_MIN;

    cpu.regs.a = (uint8_t)result;

    cpu.regs.flags.bits.z = cpu.regs.a == 0;
    cpu.regs.flags.bits.n = cpu.regs.a >> 7;
}

void AHX(CPU& cpu) {
    mu::log_error("invalid/unsupported opcode was called");
}

void ALR(CPU& cpu) {
    mu::log_error("invalid/unsupported opcode was called");
}

void ANC(CPU& cpu) {
    mu::log_error("invalid/unsupported opcode was called");
}

void AND(CPU& cpu) {
    auto v = cpu.arg_value;

    cpu.regs.a = cpu.regs.a & v;
    cpu.regs.flags.bits.z = cpu.regs.a == 0;
    cpu.regs.flags.bits.n = cpu.regs.a >> 7;
}

void ARR(CPU& cpu) {
    mu::log_error("invalid/unsupported opcode was called");
}

void ASL(CPU& cpu) {
    auto v = cpu.arg_value;

    cpu.regs.flags.bits.c = v >> 7;
    v <<= 1;
    cpu.regs.flags.bits.z = v == 0; // TODO: not sure if Accumulator only or any value
    cpu.regs.flags.bits.n = v >> 7;

    cpu_write_arg(cpu, v);
}

void AXS(CPU& cpu) {
    mu::log_error("invalid/unsupported opcode was called");
}

void BCC(CPU& cpu) {
    if (!cpu.regs.flags.bits.c) {
        auto fetched = (int8_t)cpu.arg_value;
        cpu.regs.pc += fetched;
        cpu.cycles++;
    } else {
        cpu.cross_page_penalty = false;
    }
}

void BCS(CPU& cpu) {
    if (cpu.regs.flags.bits.c) {
        auto fetched = (int8_t)cpu.arg_value;
        cpu.regs.pc += fetched;
        cpu.cycles++;
    } else {
        cpu.cross_page_penalty = false;
    }
}

void BEQ(CPU& cpu) {
    if (cpu.regs.flags.bits.z) {
        auto fetched = (int8_t)cpu.arg_value;
        cpu.regs.pc += fetched;
        cpu.cycles++;
    } else {
        cpu.cross_page_penalty = false;
    }
}

void BIT(CPU& cpu) {
    auto v = cpu.arg_value;

    cpu.regs.flags.bits.z = (v & cpu.regs.a) == 0;
    cpu.regs.flags.bits.v = v >> 6;
    cpu.regs.flags.bits.n = v >> 7;
}

void BMI(CPU& cpu) {
    if (cpu.regs.flags.bits.n) {
        auto fetched = (int8_t)cpu.arg_value;
        cpu.regs.pc += fetched;
        cpu.cycles++;
    } else {
        cpu.cross_page_penalty = false;
    }
}

void BNE(CPU& cpu) {
    if (!cpu.regs.flags.bits.z) {
        auto fetched = (int8_t)cpu.arg_value;
        cpu.regs.pc += fetched;
        cpu.cycles++;
    } else {
        cpu.cross_page_penalty = false;
    }
}

void BPL(CPU& cpu) {
    if (!cpu.regs.flags.bits.n) {
        auto fetched = (int8_t)cpu.arg_value;
        cpu.regs.pc += fetched;
        cpu.cycles++;
    } else {
        cpu.cross_page_penalty = false;
    }
}

void BRK(CPU& cpu) {
    if (cpu.regs.flags.bits.i == 1) return;

    cpu_push(cpu, cpu.regs.pc);
    cpu_push(cpu, cpu.regs.flags.byte);
    cpu.regs.pc = cpu_read16(cpu, IRQ);
    cpu.regs.flags.bits.b = 1;
    cpu.regs.flags.bits.i = 1;
}

void BVC(CPU& cpu) {
    if (!cpu.regs.flags.bits.v) {
        auto fetched = (int8_t)cpu.arg_value;
        cpu.regs.pc += fetched;
        cpu.cycles++;
    } else {
        cpu.cross_page_penalty = false;
    }
}

void BVS(CPU& cpu) {
    if (cpu.regs.flags.bits.v) {
        auto fetched = (int8_t)cpu.arg_value;
        cpu.regs.pc += fetched;
        cpu.cycles++;
    } else {
        cpu.cross_page_penalty = false;
    }
}

void CLC(CPU& cpu) {
    cpu.regs.flags.bits.c = 0;
}

void CLD(CPU& cpu) {
    cpu.regs.flags.bits.d = 0;
}

void CLI(CPU& cpu) {
    cpu.regs.flags.bits.i = 0;
}

void CLV(CPU& cpu) {
    cpu.regs.flags.bits.v = 0;
}

void CMP(CPU& cpu) {
    auto v = cpu.arg_value;
    uint8_t result = cpu.regs.a - v;
    cpu.regs.flags.bits.c = result > 0;
    cpu.regs.flags.bits.z = result == 0;
    cpu.regs.flags.bits.n = result >> 7;
}

void CPX(CPU& cpu) {
    auto v = cpu.arg_value;
    uint8_t result = cpu.regs.x - v;
    cpu.regs.flags.bits.c = result > 0;
    cpu.regs.flags.bits.z = result == 0;
    cpu.regs.flags.bits.n = result >> 7;
}

void CPY(CPU& cpu) {
    auto v = cpu.arg_value;
    uint8_t result = cpu.regs.y - v;
    cpu.regs.flags.bits.c = result > 0;
    cpu.regs.flags.bits.z = result == 0;
    cpu.regs.flags.bits.n = result >> 7;
}

void DCP(CPU& cpu) {
    mu::log_error("invalid/unsupported opcode was called");
}

void DEC(CPU& cpu) {
    auto v = cpu.arg_value;
    v--;

    cpu.regs.flags.bits.z = v == 0;
    cpu.regs.flags.bits.n = v >> 7;

    cpu_write_arg(cpu, v);
}

void DEX(CPU& cpu) {
    cpu.regs.x--;
    cpu.regs.flags.bits.z = cpu.regs.x == 0;
    cpu.regs.flags.bits.n = cpu.regs.x >> 7;
}

void DEY(CPU& cpu) {
    cpu.regs.y--;
    cpu.regs.flags.bits.z = cpu.regs.y == 0;
    cpu.regs.flags.bits.n = cpu.regs.y >> 7;
}

void EOR(CPU& cpu) {
    auto v = cpu.arg_value;
    cpu.regs.a ^= v;

    cpu.regs.flags.bits.z = cpu.regs.a == 0;
    cpu.regs.flags.bits.n = cpu.regs.a >> 7;
}

void INC(CPU& cpu) {
    auto v = cpu.arg_value;
    v++;

    cpu.regs.flags.bits.z = v == 0;
    cpu.regs.flags.bits.n = v >> 7;

    cpu_write_arg(cpu, v);
}

void INX(CPU& cpu) {
    cpu.regs.x++;
    cpu.regs.flags.bits.z = cpu.regs.x == 0;
    cpu.regs.flags.bits.n = cpu.regs.x >> 7;
}

void INY(CPU& cpu) {
    cpu.regs.y++;
    cpu.regs.flags.bits.z = cpu.regs.y == 0;
    cpu.regs.flags.bits.n = cpu.regs.y >> 7;
}

void ISC(CPU& cpu) {
    mu::log_error("invalid/unsupported opcode was called");
}

void JMP(CPU& cpu) {
    cpu_reprepare_jmp_arg(cpu);
    cpu.regs.pc = cpu.arg_addr;
}

void JSR(CPU& cpu) {
    cpu_push16(cpu, cpu.regs.pc);
    cpu.regs.pc = cpu.arg_addr;
}

void KIL(CPU& cpu) {
    mu::log_error("KIL was called");
}

void LAS(CPU& cpu) {
    mu::log_error("invalid/unsupported opcode was called");
}

void LAX(CPU& cpu) {
    mu::log_error("invalid/unsupported opcode was called");
}

void LDA(CPU& cpu) {
    auto v = cpu.arg_value;
    cpu.regs.a = v;

    cpu.regs.flags.bits.z = cpu.regs.a == 0;
    cpu.regs.flags.bits.n = cpu.regs.a >> 7;
}

void LDX(CPU& cpu) {
    auto v = cpu.arg_value;
    cpu.regs.x = v;

    cpu.regs.flags.bits.z = cpu.regs.x == 0;
    cpu.regs.flags.bits.n = cpu.regs.x >> 7;
}

void LDY(CPU& cpu) {
    auto v = cpu.arg_value;
    cpu.regs.y = v;

    cpu.regs.flags.bits.z = cpu.regs.y == 0;
    cpu.regs.flags.bits.n = cpu.regs.y >> 7;
}

void LSR(CPU& cpu) {
    auto v = cpu.arg_value;
    cpu.regs.flags.bits.c = v & 1;

    v >>= 1;

    cpu.regs.flags.bits.z = v == 0;
    cpu.regs.flags.bits.n = 0;

    cpu_write_arg(cpu, v);
}

void NOP(CPU& cpu) {}

void ORA(CPU& cpu) {
    auto v = cpu.arg_value;
    cpu.regs.a |= v;

    cpu.regs.flags.bits.z = cpu.regs.a == 0;
    cpu.regs.flags.bits.n = cpu.regs.a >> 7;
}

void PHA(CPU& cpu) {
    cpu_push(cpu, cpu.regs.a);
}

void PHP(CPU& cpu) {
    cpu_push(cpu, cpu.regs.flags.byte);
}

void PLA(CPU& cpu) {
    cpu.regs.a = cpu_pop(cpu);
}

void PLP(CPU& cpu) {
    cpu.regs.flags.byte = cpu_pop(cpu);
}

void RLA(CPU& cpu) {
    mu::log_error("invalid/unsupported opcode was called");
}

void ROL(CPU& cpu) {
    auto v = cpu.arg_value;
    uint8_t oldCarry = cpu.regs.flags.bits.c;
    cpu.regs.flags.bits.c = v >> 7;

    v <<= 1;
    v |= oldCarry;

    cpu.regs.flags.bits.z = v == 0;
    cpu.regs.flags.bits.n = v >> 7;

    cpu_write_arg(cpu, v);
}

void ROR(CPU& cpu) {
    auto v = cpu.arg_value;
    uint8_t oldCarry = cpu.regs.flags.bits.c;
    cpu.regs.flags.bits.c = v & 1;

    v >>= 1;
    v |= oldCarry << 7;

    cpu.regs.flags.bits.z = v == 0;
    cpu.regs.flags.bits.n = oldCarry;

    cpu_write_arg(cpu, v);
}

void RRA(CPU& cpu) {
    mu::log_error("invalid/unsupported opcode was called");
}

void RTI(CPU& cpu) {
    cpu.regs.flags.byte = cpu_pop(cpu);
    cpu.regs.pc = cpu_pop16(cpu);
}

void RTS(CPU& cpu) {
    cpu.regs.pc = cpu_pop16(cpu) + 1;
}

void SAX(CPU& cpu) {
    mu::log_error("invalid/unsupported opcode was called");
}

void SBC(CPU& cpu) {
    auto v = cpu.arg_value;
    uint16_t result = cpu.regs.a - v - (~ cpu.regs.flags.bits.c);

    cpu.regs.flags.bits.c = (uint16_t)result > UINT8_MAX;
    cpu.regs.flags.bits.v = (int16_t)result > INT8_MAX || (int16_t)result < INT8_MAX;

    cpu.regs.a = (uint8_t)result;

    cpu.regs.flags.bits.z = cpu.regs.a == 0;
    cpu.regs.flags.bits.n = cpu.regs.a >> 7;
}

void SEC(CPU& cpu) {
    cpu.regs.flags.bits.c = 1;
}

void SED(CPU& cpu) {
    cpu.regs.flags.bits.d = 1;
}

void SEI(CPU& cpu) {
    cpu.regs.flags.bits.i = 1;
}

void SHX(CPU& cpu) {
    mu::log_error("invalid/unsupported opcode was called");
}

void SHY(CPU& cpu) {
    mu::log_error("invalid/unsupported opcode was called");
}

void SLO(CPU& cpu) {
    mu::log_error("invalid/unsupported opcode was called");
}

void SRE(CPU& cpu) {
    mu::log_error("invalid/unsupported opcode was called");
}

void STA(CPU& cpu) {
    cpu_write_arg(cpu, cpu.regs.a);
}

void STX(CPU& cpu) {
    cpu_write_arg(cpu, cpu.regs.x);
}

void STY(CPU& cpu) {
    cpu_write_arg(cpu, cpu.regs.y);
}

void TAS(CPU& cpu) {
    mu::log_error("invalid/unsupported opcode was called");
}

void TAX(CPU& cpu) {
    cpu.regs.x = cpu.regs.a;
    cpu.regs.flags.bits.z = cpu.regs.x == 0;
    cpu.regs.flags.bits.n = cpu.regs.x >> 7;
}

void TAY(CPU& cpu) {
    cpu.regs.y = cpu.regs.a;
    cpu.regs.flags.bits.z = cpu.regs.y == 0;
    cpu.regs.flags.bits.n = cpu.regs.y >> 7;
}

void TSX(CPU& cpu) {
    cpu.regs.x = cpu.regs.sp;
    cpu.regs.flags.bits.z = cpu.regs.x == 0;
    cpu.regs.flags.bits.n = cpu.regs.x >> 7;
}

void TXA(CPU& cpu) {
    cpu.regs.a = cpu.regs.x;
    cpu.regs.flags.bits.z = cpu.regs.x == 0;
    cpu.regs.flags.bits.n = cpu.regs.x >> 7;
}

void TXS(CPU& cpu) {
    cpu.regs.sp = cpu.regs.x;
    cpu.regs.flags.bits.z = cpu.regs.x == 0;
    cpu.regs.flags.bits.n = cpu.regs.x >> 7;
}

void TYA(CPU& cpu) {
    cpu.regs.a = cpu.regs.y;
    cpu.regs.flags.bits.z = cpu.regs.a == 0;
    cpu.regs.flags.bits.n = cpu.regs.a >> 7;
}

void XAA(CPU& cpu) {
    mu::log_error("invalid/unsupported opcode was called");
}

#define _DEF(x) x, #x
const InstructionSet instruction_set{
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
