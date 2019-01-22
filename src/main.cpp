#include <cstdio>
#include <functional>
#include <cstdint>
#include <string>
#include <cstdarg>
using namespace std;

#include "Date.h"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

void logInfo(string format, ...) {
    format = string(ANSI_COLOR_BLUE "[INFO]" ANSI_COLOR_RESET) + string(GMTDateTime()) + ": " + format + '\n';

    va_list args;
    va_start (args, format);
    vfprintf (stdout, format.c_str(), args);
    va_end (args);
}

void logWarning(string format, ...) {
    format = string(ANSI_COLOR_MAGENTA "[WARNING]" ANSI_COLOR_RESET) + string(GMTDateTime()) + ": " + format + '\n';

    va_list args;
    va_start (args, format);
    vfprintf (stdout, format.c_str(), args);
    va_end (args);
}

void logError(string format, ...) {
    format = string(ANSI_COLOR_RED "[ERROR]" ANSI_COLOR_RESET) + string(GMTDateTime()) + ": " + format + '\n';

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
    
    return 0;
}
