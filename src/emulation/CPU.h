#pragma once

#include <thread>

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

struct Console;

struct CPU {
    CPURegs regs;
    uint16_t cycles = 0;
    Console* console;

    /*addressing modes for 6502
    from Appendix E: http://www.nesdev.com/NESDoc.pdf
    */
    uint16_t zero_page_address(const uint8_t bb);
    uint16_t indexed_zero_page_address(const uint8_t bb, const uint8_t i);
    uint16_t absolute_address(const uint8_t bb, const uint8_t cc);
    uint16_t indexed_absolute_address(const uint8_t bb, const uint8_t cc, const uint8_t i);
    uint16_t indirect_address(const uint8_t bb, const uint8_t cc);
    uint16_t indexed_indirect_address(const uint8_t bb, const uint8_t i);
    uint16_t indirect_indexed_address(const uint8_t bb, const uint8_t i);

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
    uint8_t arg_value;
    uint16_t arg_addr;
    AddressMode mode;

    void write_arg(uint8_t v); 
    void prepare_arg(AddressMode mode);
    void reprepare_jmp_arg();
    bool cross_page_penalty;
};

CPU cpu_new(Console* console);
void cpu_reset(CPU& self);
void cpu_clock(CPU& self);
