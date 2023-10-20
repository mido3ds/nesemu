#pragma once

#include <mu/utils.h>

struct TestLogLine {
    uint16_t pc;
    StrView inst_bytes;
    StrView inst_asm;
    uint8_t a, x, y, p, sp;
    struct { uint16_t x, y; } ppu; // TODO: ??
    uint64_t cycles;
};

typedef Arr<TestLogLine, 8991> TestLogs;
extern const TestLogs testLogs;
