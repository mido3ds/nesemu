#pragma once

#include "common.h"

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
    Console* console;

    CPURegs regs;
    uint16_t cycles;

    // for instructions
    uint8_t arg_value;
    uint16_t arg_addr;
    AddressMode mode;

    bool cross_page_penalty;
};

CPU cpu_new(Console* console);
void cpu_reset(CPU& self);
void cpu_clock(CPU& self);

uint8_t cpu_read(CPU& self, uint16_t address);
uint16_t cpu_read16(CPU& self, uint16_t address);
uint8_t cpu_fetch(CPU& self); // read and increment pc

void cpu_write(CPU& self, uint16_t address, uint8_t data);
void cpu_write16(CPU& self, uint16_t address, uint16_t v);

void cpu_push(CPU& self, uint8_t v);
void cpu_push16(CPU& self, uint16_t v);

uint8_t cpu_pop(CPU& self);
uint16_t cpu_pop16(CPU& self);

void cpu_write_arg(CPU& self, uint8_t v);
void cpu_reprepare_jmp_arg(CPU& self);
