#pragma once

#include <thread>

#include "emulation/Bus.h"
#include "emulation/common.h"

struct CPURegs {
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
};

struct CPU {
    int init(Bus* bus);

    void reset();
    void clock();

    CPURegs regs;

    uint16_t cycles = 0;
    Bus* bus;

    /*addressing modes for 6502
    from Appendix E: http://www.nesdev.com/NESDoc.pdf
    */
    uint16_t zeroPageAddress(const uint8_t bb);
    uint16_t indexedZeroPageAddress(const uint8_t bb, const uint8_t i);
    uint16_t absoluteAddress(const uint8_t bb, const uint8_t cc);
    uint16_t indexedAbsoluteAddress(const uint8_t bb, const uint8_t cc, const uint8_t i);
    uint16_t indirectAddress(const uint8_t bb, const uint8_t cc);
    uint16_t indexedIndirectAddress(const uint8_t bb, const uint8_t i);
    uint16_t indirectIndexedAddress(const uint8_t bb, const uint8_t i);

    // ram
    uint8_t read(uint16_t address);
    uint16_t read16(uint16_t address);
    uint8_t fetch(); // read and increment pc

    void write(uint16_t address, uint8_t value);
    void write16(uint16_t address, uint16_t v);

    void push(uint8_t v);
    void push16(uint16_t v);

    uint8_t pop();
    uint16_t pop16();

    // for instructions
    uint8_t getArgValue();
    uint16_t getArgAddr();
    void writeArg(uint8_t v); uint8_t argValue; uint16_t argAddr; AddressMode mode;
    void prepareArg(AddressMode mode);
    void reprepareJMPArg();
    bool crossPagePenalty;
};
