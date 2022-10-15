#pragma once

#include <string>
#include <array>



using namespace std;

struct TestLogLine {
    uint16_t pc;
    string inst_bytes;
    string inst_asm;
    uint8_t a, x, y, p, sp;
    struct { uint16_t x, y; } ppu; // TODO: ??
    uint64_t cycles;
};

typedef array<TestLogLine, 8991> TestLogs;
extern const TestLogs testLogs;
