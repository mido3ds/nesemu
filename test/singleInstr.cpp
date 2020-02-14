#include <cstring>

#include "catch.hpp"

#include "emulation/console.h"
#include "emulation/instructions.h"

TEST_CASE("memory-access") {
    Console dev;
    dev.init(nullptr);

    memset(&dev.regs, 0, sizeof dev.regs);
    dev.memory.fill(0);

    SECTION("basic-read8") {
        for (auto i = 0; i < MEM_SIZE; i++) {
            dev.memory[i] = i;
            REQUIRE(dev.read(i) == dev.memory[i]);
        }
    }

    SECTION("basic-read16") {
        for (auto i = 0; i < MEM_SIZE-1; i++) {
            dev.memory[i] = i;
            REQUIRE(dev.read16(i) == (dev.memory[i] | (dev.memory[i+1] << 8)));
        }
    }

    SECTION("fetch-increments") {
        REQUIRE(dev.regs.pc == 0);

        for (auto i = 0; i < MEM_SIZE-1; i++) {
            dev.memory[i] = i;

            auto oldPC = dev.regs.pc;
            REQUIRE(dev.fetch() == dev.memory[i]);
            REQUIRE(dev.regs.pc == oldPC+1);
        }

        REQUIRE(dev.fetch() == dev.memory[MEM_SIZE-1]);
        REQUIRE(dev.regs.pc == 0);
    }

    SECTION("basic-write") {
        for (auto i = 0; i < MEM_SIZE; i++) {
            dev.write(i, i);
            REQUIRE(dev.memory[i] == u8_t(i));
        }
    }

    SECTION("basic-write16") {
        for (auto i = 0; i < MEM_SIZE-1; i++) {
            CAPTURE(i);
            dev.write16(i, i);
            REQUIRE(dev.memory[i] == u8_t(i & 255));
            REQUIRE(dev.memory[i+1] == u8_t((i >> 8) & 255));
        }
    }

    SECTION("push-decreases") {
        dev.regs.sp = 0xFF;

        for (int i = 0; i < 0xFF; i++) {
            auto oldsp = dev.regs.sp;
            REQUIRE(dev.memory[oldsp+STACK.start] == 0);
            dev.push(i);
            REQUIRE(dev.regs.sp == oldsp-1);
            REQUIRE(dev.memory[oldsp+STACK.start] == i);
        }
    }

    SECTION("pop-increases") {
        dev.regs.sp = 0;
        for (int i = 0xFF; i >= 0; i--) {
            dev.push(i);
        }
        REQUIRE(dev.regs.sp == 0);

        for (int i = 0; i < 0xFF; i++) {
            auto oldsp = dev.regs.sp;

            REQUIRE(dev.pop() == i);

            REQUIRE(dev.regs.sp == oldsp+1);
            REQUIRE(dev.memory[dev.regs.sp+STACK.start] == i);
        }
    }
}

TEST_CASE("branch") {
    Console dev;
    dev.init(nullptr);

    memset(&dev.regs, 0, sizeof dev.regs);
    dev.memory.fill(0);
    dev.cpuCycles = 0;

    auto oldpc = 123;
    auto addr = 100;

    SECTION("BCC") {
        const auto BCC = 0x90;
        auto oldflags = dev.regs.flags.byte;

        dev.regs.pc = oldpc;
        dev.memory[dev.regs.pc] = BCC;
        dev.memory[dev.regs.pc+1] = addr;

        dev.oneCPUCycle();
        
        REQUIRE(dev.regs.pc == oldpc+addr+2);
        REQUIRE(oldflags == dev.regs.flags.byte);
        REQUIRE(dev.cpuCycles == instructionSet[BCC].cpuCycles+1);
    }

    SECTION("no-BCC") {
        const auto BCC = 0x90;
        dev.regs.flags.bits.c = 1;
        auto oldflags = dev.regs.flags.byte;

        dev.regs.pc = oldpc;
        dev.memory[dev.regs.pc] = BCC;
        dev.memory[dev.regs.pc+1] = addr;

        dev.oneCPUCycle();
        
        REQUIRE(dev.regs.pc == oldpc+2);
        REQUIRE(oldflags == dev.regs.flags.byte);
        REQUIRE(dev.cpuCycles == instructionSet[BCC].cpuCycles);
    }

    SECTION("BCC-cross-page") {
        const auto BCC = 0x90;
        auto oldflags = dev.regs.flags.byte;
        oldpc = 0x00FF -1 -1;
        addr = 0x01;

        dev.regs.pc = oldpc;
        dev.memory[dev.regs.pc] = BCC;
        dev.memory[dev.regs.pc+1] = addr;

        dev.oneCPUCycle();
        
        REQUIRE(dev.regs.pc == oldpc+addr+2);
        REQUIRE(oldflags == dev.regs.flags.byte);
        REQUIRE(dev.cpuCycles == instructionSet[BCC].cpuCycles+1+1); // added penalty
    }

    SECTION("BEQ") {
        const auto BEQ = 0xF0;
        dev.regs.flags.bits.z = 1;
        auto oldflags = dev.regs.flags.byte;

        dev.regs.pc = oldpc;
        dev.memory[dev.regs.pc] = BEQ;
        dev.memory[dev.regs.pc+1] = addr;

        dev.oneCPUCycle();
        
        REQUIRE(dev.regs.pc == oldpc+addr+2);
        REQUIRE(oldflags == dev.regs.flags.byte);
        REQUIRE(dev.cpuCycles == instructionSet[BEQ].cpuCycles+1);
    }

    SECTION("no-BEQ") {
        const auto BEQ = 0xF0;
        dev.regs.flags.bits.z = 0;
        auto oldflags = dev.regs.flags.byte;

        dev.regs.pc = oldpc;
        dev.memory[dev.regs.pc] = BEQ;
        dev.memory[dev.regs.pc+1] = addr;

        dev.oneCPUCycle();
        
        REQUIRE(dev.regs.pc == oldpc+2);
        REQUIRE(oldflags == dev.regs.flags.byte);
        REQUIRE(dev.cpuCycles == instructionSet[BEQ].cpuCycles);
    }

    SECTION("BEQ") {
        const auto BEQ = 0xF0;
        dev.regs.flags.bits.z = 1;
        auto oldflags = dev.regs.flags.byte;

        dev.regs.pc = oldpc;
        dev.memory[dev.regs.pc] = BEQ;
        dev.memory[dev.regs.pc+1] = addr;

        dev.oneCPUCycle();
        
        REQUIRE(dev.regs.pc == oldpc+addr+2);
        REQUIRE(oldflags == dev.regs.flags.byte);
        REQUIRE(dev.cpuCycles == instructionSet[BEQ].cpuCycles+1);
    }

    SECTION("no-BEQ") {
        const auto BEQ = 0xF0;
        dev.regs.flags.bits.z = 0;
        auto oldflags = dev.regs.flags.byte;

        dev.regs.pc = oldpc;
        dev.memory[dev.regs.pc] = BEQ;
        dev.memory[dev.regs.pc+1] = addr;

        dev.oneCPUCycle();
        
        REQUIRE(dev.regs.pc == oldpc+2);
        REQUIRE(oldflags == dev.regs.flags.byte);
        REQUIRE(dev.cpuCycles == instructionSet[BEQ].cpuCycles);
    }
}

TEST_CASE("immediate-instructs") {
    Console dev;
    dev.init(nullptr);

    memset(&dev.regs, 0, sizeof dev.regs);
    dev.memory.fill(0);
    dev.cpuCycles = 0;
    dev.regs.pc = 0;

    SECTION("AND") {
        dev.regs.a = 0b01010101;
        dev.memory[0] = 0x29;
        dev.memory[1] = 0b00010001;
        dev.oneCPUCycle();
        REQUIRE(dev.regs.a == (0b01010101 & 0b00010001));
        REQUIRE(dev.regs.flags.bits.z == 0);
    }

    SECTION("AND-zero-a") {
        dev.regs.a = 0b01010101;
        dev.memory[0] = 0x29;
        dev.memory[1] = 0b00000000;
        dev.oneCPUCycle();
        REQUIRE(dev.regs.a == (0b01010101 & 0b00000000));
        REQUIRE(dev.regs.flags.bits.z == 1);
    }

    SECTION("ADC") {
        dev.regs.a = 3;
        dev.regs.flags.bits.c = 1;
        dev.memory[0] = 0x69;
        dev.memory[1] = 99;
        dev.oneCPUCycle();
        REQUIRE(dev.regs.a == (3 + 99 + 1));
        REQUIRE(dev.regs.flags.bits.c == 0);
    }

    SECTION("ADC-unsigned-overflow") {
        dev.regs.a = 0xFF;
        dev.memory[0] = 0x69;
        dev.memory[1] = 1;
        dev.oneCPUCycle();
        REQUIRE(dev.regs.a == 0);
        REQUIRE(dev.regs.flags.bits.c == 1);
    }

    SECTION("ADC-signed-overflow") {
        dev.regs.a = 4;
        dev.memory[0] = 0x69;
        dev.memory[1] = -10;
        dev.oneCPUCycle();
        REQUIRE(dev.regs.a == u8_t(4-10));
        REQUIRE(dev.regs.flags.bits.c == 0);
        REQUIRE(dev.regs.flags.bits.v == 1);
    }
}

TEST_CASE("implied-instructs") {
    Console dev;
    dev.init(nullptr);

    memset(&dev.regs, 0, sizeof dev.regs);
    dev.memory.fill(0);
    dev.cpuCycles = 0;
    dev.regs.pc = 0;

    SECTION("PHP") {
        dev.regs.flags.byte = 0xF5;
        dev.memory[0] = 0x08;
        dev.oneCPUCycle();
        REQUIRE(dev.regs.flags.byte == 0xF5);
        REQUIRE(dev.pop() == 0xF5);
    }
}

TEST_CASE("jmp-bug") {
    Console dev;
    dev.init(nullptr);

    memset(&dev.regs, 0, sizeof dev.regs);
    dev.memory.fill(0);
    dev.cpuCycles = 0;

    SECTION("absolute-bug") {
        dev.regs.pc = 0x00FE;
        dev.memory[0x00FE] = 0x4C;
        dev.memory[0x00FF] = 0x11;
        dev.memory[0x0000] = 0xF5;

        dev.oneCPUCycle();
        REQUIRE(dev.regs.pc == 0xF511);
    }

    SECTION("indirect-bug") {
        dev.regs.pc = 0x00FE;
        dev.memory[0x00FE] = 0x6C;
        dev.memory[0x00FF] = 0x11;
        dev.memory[0x0000] = 0xF5;

        dev.memory[0xF511] = 0x13;
        dev.memory[0xF512] = 0x0f;

        dev.oneCPUCycle();
        REQUIRE(dev.regs.pc == 0x0F13);
    }

    SECTION("absolute-no-bug") {
        dev.regs.pc = 0x00FD;
        dev.memory[0x00FD] = 0x4C;
        dev.memory[0x00FE] = 0x11;
        dev.memory[0x00FF] = 0xF5;

        dev.oneCPUCycle();
        REQUIRE(dev.regs.pc == 0xF511);
    }

    SECTION("indirect-no-bug") {
        dev.regs.pc = 0x00FD;
        dev.memory[0x00FD] = 0x6C;
        dev.memory[0x00FE] = 0x11;
        dev.memory[0x00FF] = 0xF5;

        dev.memory[0xF511] = 0x13;
        dev.memory[0xF512] = 0x0f;

        dev.oneCPUCycle();
        REQUIRE(dev.regs.pc == 0x0F13);
    }
}