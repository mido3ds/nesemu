#pragma once

#include <thread>

#include "stdtype.h"
#include "emulation/Bus.h"

struct CPURegs {
    u16_t pc; // program counter
    u8_t sp; // stack pointer
    u8_t a; // accumulator
    u8_t x; // index
    u8_t y; // index
    union {
        struct {
            u8_t c:1; // carry flag
            u8_t z:1; // zero flag
            u8_t i:1; // interrupt disable
            u8_t d:1; // decimal mode
            u8_t b:1; // break command
            u8_t:1; 
            u8_t v:1; // overflow flag
            u8_t n:1; // negative flag
        } bits;
        u8_t byte;
    } flags; // processor status
};

class CPU {
public: 
    void init(Bus* bus);

    void reset();
    void clock();

    CPURegs getRegs();
    u16_t getCycles();

private:
    CPURegs regs;

    u16_t cycles = 0;
    Bus* bus;

    /*addressing modes for 6502
    from Appendix E: http://www.nesdev.com/NESDoc.pdf
    */
    u16_t zeroPageAddress(const u8_t bb);
    u16_t indexedZeroPageAddress(const u8_t bb, const u8_t i);
    u16_t absoluteAddress(const u8_t bb, const u8_t cc);
    u16_t indexedAbsoluteAddress(const u8_t bb, const u8_t cc, const u8_t i);
    u16_t indirectAddress(const u8_t bb, const u8_t cc);
    u16_t indexedIndirectAddress(const u8_t bb, const u8_t i);
    u16_t indirectIndexedAddress(const u8_t bb, const u8_t i);

    // ram
    u8_t read(u16_t address);
    u16_t read16(u16_t address);
    u8_t fetch(); // read and increment pc

    void write(u16_t address, u8_t value);
    void write16(u16_t address, u16_t v);

    void push(u8_t v);
    void push16(u16_t v);

    u8_t pop();
    u16_t pop16();

    // for instructions
    u8_t getArgValue();
    u16_t getArgAddr();
    void writeArg(u8_t v); u8_t argValue; u16_t argAddr; AddressMode mode;
    void prepareArg(AddressMode mode);
    void reprepareJMPArg();
    void noCrossPage(); bool cpp;

    // instructions
    friend void ADC(CPU&);
    friend void AHX(CPU&);
    friend void ALR(CPU&);
    friend void ANC(CPU&);
    friend void AND(CPU&);
    friend void ARR(CPU&);
    friend void ASL(CPU&);
    friend void AXS(CPU&);
    friend void BCC(CPU&);
    friend void BCS(CPU&);
    friend void BEQ(CPU&);
    friend void BIT(CPU&);
    friend void BMI(CPU&);
    friend void BNE(CPU&);
    friend void BPL(CPU&);
    friend void BRK(CPU&);
    friend void BVC(CPU&);
    friend void BVS(CPU&);
    friend void CLC(CPU&);
    friend void CLD(CPU&);
    friend void CLI(CPU&);
    friend void CLV(CPU&);
    friend void CMP(CPU&);
    friend void CPX(CPU&);
    friend void CPY(CPU&);
    friend void DCP(CPU&);
    friend void DEC(CPU&);
    friend void DEX(CPU&);
    friend void DEY(CPU&);
    friend void EOR(CPU&);
    friend void INC(CPU&);
    friend void INX(CPU&);
    friend void INY(CPU&);
    friend void ISC(CPU&);
    friend void JMP(CPU&);
    friend void JSR(CPU&);
    friend void KIL(CPU&);
    friend void LAS(CPU&);
    friend void LAX(CPU&);
    friend void LDA(CPU&);
    friend void LDX(CPU&);
    friend void LDY(CPU&);
    friend void LSR(CPU&);
    friend void NOP(CPU&);
    friend void ORA(CPU&);
    friend void PHA(CPU&);
    friend void PHP(CPU&);
    friend void PLA(CPU&);
    friend void PLP(CPU&);
    friend void RLA(CPU&);
    friend void ROL(CPU&);
    friend void ROR(CPU&);
    friend void RRA(CPU&);
    friend void RTI(CPU&);
    friend void RTS(CPU&);
    friend void SAX(CPU&);
    friend void SBC(CPU&);
    friend void SEC(CPU&);
    friend void SED(CPU&);
    friend void SEI(CPU&);
    friend void SHX(CPU&);
    friend void SHY(CPU&);
    friend void SLO(CPU&);
    friend void SRE(CPU&);
    friend void STA(CPU&);
    friend void STX(CPU&);
    friend void STY(CPU&);
    friend void TAS(CPU&);
    friend void TAX(CPU&);
    friend void TAY(CPU&);
    friend void TSX(CPU&);
    friend void TXA(CPU&);
    friend void TXS(CPU&);
    friend void TYA(CPU&);
    friend void XAA(CPU&);
};