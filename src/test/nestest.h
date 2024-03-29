#pragma once

#include <mu/utils.h>

struct TestLogLine {
    uint16_t pc;
    mu::StrView inst_bytes;
    mu::StrView inst_asm;
    uint8_t a, x, y, p, sp;
    struct { uint16_t x, y; } ppu; // TODO: ??
    uint64_t cycles;
};

using TestLogs = mu::Arr<TestLogLine, 8991>;
extern const TestLogs testLogs;
