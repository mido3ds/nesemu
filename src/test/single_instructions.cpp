#include <catch2/catch.hpp>

#include "Console.h"

static uint8_t mem_read(RAM& ram, uint16_t a) {
    uint8_t data;
    ram_read(ram, a, data);
    return data;
}

static uint16_t mem_read16(RAM& ram, uint16_t a) {
    uint8_t up, low;

    ram_read(ram, a, up);
    ram_read(ram, a, low);

    return low | up << 8;
}

static void mem_write(RAM& ram, uint16_t a, uint8_t data) {
    ram_write(ram, a, data);
}

static void mem_write16(RAM& ram, uint16_t a, uint16_t data) {
    ram_write(ram, a, data);
    ram_write(ram, a+1, data >> 8);
}

TEST_CASE("branch") {
    Console dev {};
    console_init(dev);

    auto& regs = dev.cpu.regs;

    memset(&regs, 0, sizeof(CPURegs));

    auto oldpc = 123;
    auto addr = 100;

    SECTION("BCC") {
        const auto BCC = 0x90;
        auto oldflags = regs.flags.byte;

        regs.pc = oldpc;
        mem_write(dev.ram, regs.pc, BCC);
        mem_write(dev.ram, regs.pc+1, addr);

        console_clock(dev);

        REQUIRE(regs.pc == oldpc+addr+2);
        REQUIRE(oldflags == regs.flags.byte);
        REQUIRE(dev.cpu.cycles == instruction_set[BCC].cycles+1);
    }

    SECTION("no-BCC") {
        const auto BCC = 0x90;
        regs.flags.bits.c = 1;
        auto oldflags = regs.flags.byte;

        regs.pc = oldpc;
        mem_write(dev.ram, regs.pc, BCC);
        mem_write(dev.ram, regs.pc+1, addr);

        console_clock(dev);

        REQUIRE(regs.pc == oldpc+2);
        REQUIRE(oldflags == regs.flags.byte);
        REQUIRE(dev.cpu.cycles == instruction_set[BCC].cycles);
    }

    SECTION("BCC-cross-page") {
        const auto BCC = 0x90;
        auto oldflags = regs.flags.byte;
        oldpc = 0x00FF -1 -1;
        addr = 0x01;

        regs.pc = oldpc;
        mem_write(dev.ram, regs.pc, BCC);
        mem_write(dev.ram, regs.pc+1, addr);

        console_clock(dev);

        REQUIRE(regs.pc == oldpc+addr+2);
        REQUIRE(oldflags == regs.flags.byte);
        REQUIRE(dev.cpu.cycles == instruction_set[BCC].cycles+1+1); // added penalty
    }

    SECTION("BEQ") {
        const auto BEQ = 0xF0;
        regs.flags.bits.z = 1;
        auto oldflags = regs.flags.byte;

        regs.pc = oldpc;
        mem_write(dev.ram, regs.pc, BEQ);
        mem_write(dev.ram, regs.pc+1, addr);

        console_clock(dev);

        REQUIRE(regs.pc == oldpc+addr+2);
        REQUIRE(oldflags == regs.flags.byte);
        REQUIRE(dev.cpu.cycles == instruction_set[BEQ].cycles+1);
    }

    SECTION("no-BEQ") {
        const auto BEQ = 0xF0;
        regs.flags.bits.z = 0;
        auto oldflags = regs.flags.byte;

        regs.pc = oldpc;
        mem_write(dev.ram, regs.pc, BEQ);
        mem_write(dev.ram, regs.pc+1, addr);

        console_clock(dev);

        REQUIRE(regs.pc == oldpc+2);
        REQUIRE(oldflags == regs.flags.byte);
        REQUIRE(dev.cpu.cycles == instruction_set[BEQ].cycles);
    }

    SECTION("BEQ") {
        const auto BEQ = 0xF0;
        regs.flags.bits.z = 1;
        auto oldflags = regs.flags.byte;

        regs.pc = oldpc;
        mem_write(dev.ram, regs.pc, BEQ);
        mem_write(dev.ram, regs.pc+1, addr);

        console_clock(dev);

        REQUIRE(regs.pc == oldpc+addr+2);
        REQUIRE(oldflags == regs.flags.byte);
        REQUIRE(dev.cpu.cycles == instruction_set[BEQ].cycles+1);
    }

    SECTION("no-BEQ") {
        const auto BEQ = 0xF0;
        regs.flags.bits.z = 0;
        auto oldflags = regs.flags.byte;

        regs.pc = oldpc;
        mem_write(dev.ram, regs.pc, BEQ);
        mem_write(dev.ram, regs.pc+1, addr);

        console_clock(dev);

        REQUIRE(regs.pc == oldpc+2);
        REQUIRE(oldflags == regs.flags.byte);
        REQUIRE(dev.cpu.cycles == instruction_set[BEQ].cycles);
    }
}

TEST_CASE("immediate-instructs") {
    Console dev {};
    console_init(dev);

    auto& regs = dev.cpu.regs;

    memset(&regs, 0, sizeof(CPURegs));
    regs.pc = 0;

    SECTION("AND") {
        regs.a = 0b01010101;
        mem_write(dev.ram, 0, 0x29);
        mem_write(dev.ram, 1, 0b00010001);
        console_clock(dev);
        REQUIRE(regs.a == (0b01010101 & 0b00010001));
        REQUIRE(regs.flags.bits.z == 0);
    }

    SECTION("AND-zero-a") {
        regs.a = 0b01010101;
        mem_write(dev.ram, 0, 0x29);
        mem_write(dev.ram, 1, 0b00000000);
        console_clock(dev);
        REQUIRE(regs.a == (0b01010101 & 0b00000000));
        REQUIRE(regs.flags.bits.z == 1);
    }

    SECTION("ADC") {
        regs.a = 3;
        regs.flags.bits.c = 1;
        mem_write(dev.ram, 0, 0x69);
        mem_write(dev.ram, 1, 99);
        console_clock(dev);
        REQUIRE(regs.a == (3 + 99 + 1));
        REQUIRE(regs.flags.bits.c == 0);
    }

    SECTION("ADC-unsigned-overflow") {
        regs.a = 0xFF;
        mem_write(dev.ram, 0, 0x69);
        mem_write(dev.ram, 1, 1);
        console_clock(dev);
        REQUIRE(regs.a == 0);
        REQUIRE(regs.flags.bits.c == 1);
    }

    SECTION("ADC-signed-overflow") {
        regs.a = 4;
        mem_write(dev.ram, 0, 0x69);
        mem_write(dev.ram, 1, -10);
        console_clock(dev);
        REQUIRE(regs.a == uint8_t(4-10));
        REQUIRE(regs.flags.bits.c == 0);
        REQUIRE(regs.flags.bits.v == 1);
    }
}

TEST_CASE("implied-instructs") {
    Console dev {};
    console_init(dev);

    auto& regs = dev.cpu.regs;

    memset(&regs, 0, sizeof(CPURegs));
    regs.pc = 0;

    SECTION("PHP") {
        regs.flags.byte = 0xF5;
        mem_write(dev.ram, 0, 0x08);
        console_clock(dev);
        REQUIRE(regs.flags.byte == 0xF5);
        REQUIRE(mem_read(dev.ram, STACK.start | (regs.sp+1)) == 0xF5);
    }
}

TEST_CASE("jmp-bug") {
    Console dev {};
    console_init(dev);

    auto& regs = dev.cpu.regs;

    memset(&regs, 0, sizeof(CPURegs));

    SECTION("absolute-bug") {
        regs.pc = 0x00FE;
        mem_write(dev.ram, 0x00FE, 0x4C);
        mem_write(dev.ram, 0x00FF, 0x11);
        mem_write(dev.ram, 0x0000, 0xF5);

        console_clock(dev);
        REQUIRE(regs.pc == 0xF511);
    }

    SECTION("indirect-bug") {
        regs.pc = 0x00FE;
        mem_write(dev.ram, 0x00FE, 0x6C);
        mem_write(dev.ram, 0x00FF, 0x11);
        mem_write(dev.ram, 0x0000, 0x15);

        mem_write(dev.ram, 0x1511, 0x13);
        mem_write(dev.ram, 0x1512, 0x0F);

        console_clock(dev);
        REQUIRE(regs.pc == 0x0F13);
    }

    SECTION("absolute-no-bug") {
        regs.pc = 0x00FD;
        mem_write(dev.ram, 0x00FD, 0x4C);
        mem_write(dev.ram, 0x00FE, 0x11);
        mem_write(dev.ram, 0x00FF, 0xF5);

        console_clock(dev);
        REQUIRE(regs.pc == 0xF511);
    }

    SECTION("indirect-no-bug") {
        regs.pc = 0x00FD;
        mem_write(dev.ram, 0x00FD, 0x6C);
        mem_write(dev.ram, 0x00FE, 0x11);
        mem_write(dev.ram, 0x00FF, 0x15);

        mem_write(dev.ram, 0x1511, 0x13);
        mem_write(dev.ram, 0x1512, 0x0F);

        console_clock(dev);
        REQUIRE(regs.pc == 0x0F13);
    }
}
