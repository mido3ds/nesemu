#include <cstdio>
#include <functional>
#include <cstdint>
#include <string>
#include <cstdarg>
using namespace std;

#include "Date.h"

void logInfo(string format, ...) {
    format = string("[INFO][") + string(GMTDateTime()) + "]: " + format + '\n';

    va_list args;
    va_start (args, format);
    vfprintf (stdout, format.c_str(), args);
    va_end (args);
}

void logWarning(string format, ...) {
    format = string("[WARNING][") + string(GMTDateTime()) + "]: " + format + '\n';

    va_list args;
    va_start (args, format);
    vfprintf (stdout, format.c_str(), args);
    va_end (args);
}

void logError(string format, ...) {
    format = string("[ERROR][") + string(GMTDateTime()) + "]: " + format + '\n';

    va_list args;
    va_start (args, format);
    vfprintf (stderr, format.c_str(), args);
    va_end (args);
}

struct Registers {
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
            uint8_t __unused__:1; 
            uint8_t v:1; // overflow flag
            uint8_t n:1; // negative flag
        } bits;
        uint8_t byte;
    } p; // processor status
};

class Device {
private:
    uint8_t memory[0x10000+1];
public:
};

int main(int argc, char const *argv[])
{
    logInfo("in main");
    logWarning("this is number %i", 5);
    logError("wow what happened %s", "is bad");
    return 0;
}
