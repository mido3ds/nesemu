#include "catch2/catch.hpp"

#include "console.h"

TEST_CASE("memory-access") {
    Console dev;
    REQUIRE(dev.init() == 0);

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
            REQUIRE(dev.memory[oldsp+STACK.end] == 0);
            dev.push(i);
            REQUIRE(dev.regs.sp == oldsp-1);
            REQUIRE(dev.memory[oldsp+STACK.end] == i);
        }
    }

    SECTION("pop-increases") {
        for (int i = 0; i < 0xFF; i++) {
            dev.memory[STACK.end+i] = i;
        }

        dev.regs.sp = 0;

        for (int i = 0; i < 0xFF; i++) {
            auto oldsp = dev.regs.sp;

            REQUIRE(dev.pop() == i);
            
            REQUIRE(dev.regs.sp == oldsp+1);
            REQUIRE(dev.memory[oldsp+STACK.end] == i);
        }
    }
}

TEST_CASE("branch") {
    Console dev;
    REQUIRE(dev.init() == 0);

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

        REQUIRE(dev.oneCPUCycle() == 0);
        
        REQUIRE(dev.regs.pc == oldpc+addr+2);
        REQUIRE(oldflags == dev.regs.flags.byte);
        REQUIRE(dev.cpuCycles == dev.instrucSet[BCC].cpuCycles+1);
    }

    SECTION("no-BCC") {
        const auto BCC = 0x90;
        dev.regs.flags.bits.c = 1;
        auto oldflags = dev.regs.flags.byte;

        dev.regs.pc = oldpc;
        dev.memory[dev.regs.pc] = BCC;
        dev.memory[dev.regs.pc+1] = addr;

        REQUIRE(dev.oneCPUCycle() == 0);
        
        REQUIRE(dev.regs.pc == oldpc+2);
        REQUIRE(oldflags == dev.regs.flags.byte);
        REQUIRE(dev.cpuCycles == dev.instrucSet[BCC].cpuCycles);
    }

    SECTION("BCC-cross-page") {
        const auto BCC = 0x90;
        auto oldflags = dev.regs.flags.byte;
        oldpc = 0x00FF -1 -1;
        addr = 0x01;

        dev.regs.pc = oldpc;
        dev.memory[dev.regs.pc] = BCC;
        dev.memory[dev.regs.pc+1] = addr;

        REQUIRE(dev.oneCPUCycle() == 0);
        
        REQUIRE(dev.regs.pc == oldpc+addr+2);
        REQUIRE(oldflags == dev.regs.flags.byte);
        REQUIRE(dev.cpuCycles == dev.instrucSet[BCC].cpuCycles+1+1); // added penalty
    }

    SECTION("BEQ") {
        const auto BEQ = 0xF0;
        dev.regs.flags.bits.z = 1;
        auto oldflags = dev.regs.flags.byte;

        dev.regs.pc = oldpc;
        dev.memory[dev.regs.pc] = BEQ;
        dev.memory[dev.regs.pc+1] = addr;

        REQUIRE(dev.oneCPUCycle() == 0);
        
        REQUIRE(dev.regs.pc == oldpc+addr+2);
        REQUIRE(oldflags == dev.regs.flags.byte);
        REQUIRE(dev.cpuCycles == dev.instrucSet[BEQ].cpuCycles+1);
    }

    SECTION("no-BEQ") {
        const auto BEQ = 0xF0;
        dev.regs.flags.bits.z = 0;
        auto oldflags = dev.regs.flags.byte;

        dev.regs.pc = oldpc;
        dev.memory[dev.regs.pc] = BEQ;
        dev.memory[dev.regs.pc+1] = addr;

        REQUIRE(dev.oneCPUCycle() == 0);
        
        REQUIRE(dev.regs.pc == oldpc+2);
        REQUIRE(oldflags == dev.regs.flags.byte);
        REQUIRE(dev.cpuCycles == dev.instrucSet[BEQ].cpuCycles);
    }

    SECTION("BEQ") {
        const auto BEQ = 0xF0;
        dev.regs.flags.bits.z = 1;
        auto oldflags = dev.regs.flags.byte;

        dev.regs.pc = oldpc;
        dev.memory[dev.regs.pc] = BEQ;
        dev.memory[dev.regs.pc+1] = addr;

        REQUIRE(dev.oneCPUCycle() == 0);
        
        REQUIRE(dev.regs.pc == oldpc+addr+2);
        REQUIRE(oldflags == dev.regs.flags.byte);
        REQUIRE(dev.cpuCycles == dev.instrucSet[BEQ].cpuCycles+1);
    }

    SECTION("no-BEQ") {
        const auto BEQ = 0xF0;
        dev.regs.flags.bits.z = 0;
        auto oldflags = dev.regs.flags.byte;

        dev.regs.pc = oldpc;
        dev.memory[dev.regs.pc] = BEQ;
        dev.memory[dev.regs.pc+1] = addr;

        REQUIRE(dev.oneCPUCycle() == 0);
        
        REQUIRE(dev.regs.pc == oldpc+2);
        REQUIRE(oldflags == dev.regs.flags.byte);
        REQUIRE(dev.cpuCycles == dev.instrucSet[BEQ].cpuCycles);
    }
}

TEST_CASE("immediate-instructs") {
    Console dev;
    REQUIRE(dev.init() == 0);

    memset(&dev.regs, 0, sizeof dev.regs);
    dev.memory.fill(0);
    dev.cpuCycles = 0;
    dev.regs.pc = 0;

    SECTION("AND") {
        dev.regs.a = 0b01010101;
        dev.memory[0] = 0x29;
        dev.memory[1] = 0b00010001;
        REQUIRE(dev.oneCPUCycle() == 0);
        REQUIRE(dev.regs.a == (0b01010101 & 0b00010001));
        REQUIRE(dev.regs.flags.bits.z == 0);
    }

    SECTION("AND-zero-a") {
        dev.regs.a = 0b01010101;
        dev.memory[0] = 0x29;
        dev.memory[1] = 0b00000000;
        REQUIRE(dev.oneCPUCycle() == 0);
        REQUIRE(dev.regs.a == (0b01010101 & 0b00000000));
        REQUIRE(dev.regs.flags.bits.z == 1);
    }
}