#include <ostream>

#include "catch.hpp"
#include "nestest.h"

#include "emulation/Console.h"

static string hex8(u8_t v) {
    char buffer[10] = {0};
    sprintf(buffer, "%02X", v);
    return buffer;
}

static string hex16(u16_t v) {
    char buffer[10] = {0};
    sprintf(buffer, "%04X", v);
    return buffer;
}

ostream& operator<<(ostream& os, const TestLogLine& line) {
    os << "{pc: " << hex16(line.pc) <<
        ", inst_bytes: " << line.inst_bytes <<
        ", inst_asm: " << line.inst_asm <<
        ", a: " << hex8(line.a) <<
        ", x: " << hex8(line.x)  <<
        ", y: " << hex8(line.y)  <<
        ", p: " << hex8(line.p)  <<
        ", sp: " << hex8(line.sp) <<
        ", cycles: " << line.cycles <<
        ", ppu: {x: " << int(line.ppu.x) <<
        ", y: " << int(line.ppu.y) << "}}";
}

struct { u16_t x, y; } ppu; // TODO: ??

TEST_CASE("nestest") {
    Console dev;
    REQUIRE(dev.init("nestest.nes") == 0);

    auto cpu = dev.getCPU();
    auto memory = dev.getRAM();
    auto regs = cpu->getRegs();

    regs->pc = 0xC000;
    u64_t cycles = 7;

    for (auto& line: testLogs) {
        CAPTURE(line);
        REQUIRE(line.pc == regs->pc);
        // REQUIRE(line.inst_bytes == ???) TODO
        // REQUIRE(line.inst_asm == ???) TODO
        REQUIRE(line.a == regs->a);
        REQUIRE(line.x == regs->x);
        REQUIRE(line.y == regs->y);
        REQUIRE(line.sp == regs->sp);
        REQUIRE(line.p == regs->flags.byte);
        // REQUIRE(line.ppu == ???) TODO
        REQUIRE(line.cycles == cycles);

        cpu->clock();
        cycles += cpu->getCycles();
    }
}