#include <cstdio>
#include <functional>
#include <cstdint>
using namespace std;

struct Registers {
    uint16_t pc; // program coutner
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
