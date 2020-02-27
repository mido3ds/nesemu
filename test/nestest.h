#pragma once

#include <string>
#include <array>

#include "stdtype.h"

using namespace std;

struct TestLogLine {
    u16_t pc;
    string inst_bytes;
    string inst_asm;
    u8_t a, x, y, p, sp;
    struct { u16_t x, y; } ppu; // TODO: ??
    u64_t cycles;
};

typedef array<TestLogLine, 8991> TestLogs;
extern const TestLogs testLogs;