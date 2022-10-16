#if 0 // TODO enable this when it's fast enough to compile this file, or read it
#include <ostream>

#include <catch2/catch.hpp>
#include "nestest.h"

#include "emulation/Console.h"

static string hex8(uint8_t v) {
    char buffer[10] = {0};
    sprintf(buffer, "%02X", v);
    return buffer;
}

static string hex16(uint16_t v) {
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
	return os;
}

struct { uint16_t x, y; } ppu; // TODO: ??

TEST_CASE("nestest") {
    Console dev {};
    console_init(dev);
    console_load_rom(ASSETS_DIR "/nestest.nes");

    auto& regs = dev.cpu.regs;

    regs.pc = 0xC000;
    uint64_t cycles = 7;

    for (auto& line: testLogs) {
        CAPTURE(line);
        REQUIRE(line.pc == regs.pc);
        // REQUIRE(line.inst_bytes == ???) TODO
        // REQUIRE(line.inst_asm == ???) TODO
        REQUIRE(line.a == regs.a);
        REQUIRE(line.x == regs.x);
        REQUIRE(line.y == regs.y);
        REQUIRE(line.sp == regs.sp);
        REQUIRE(line.p == regs.flags.byte);
        // REQUIRE(line.ppu == ???) TODO
        REQUIRE(line.cycles == cycles);

        dev.cpu.clock();
        cycles += dev.cpu.cycles;
    }
}
#endif