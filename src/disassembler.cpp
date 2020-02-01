#include <tuple>
#include <sstream>
#include <cstdio>
#include <cstring>

#include "disassembler.h"
#include "logger.h"

static tuple<string, int> oneInstr(uint8_t* mem, uint32_t size, const InstructionSet& instset) {
    if (mem == nullptr || size == 0) { return make_tuple("???", 0); }

    int bytes = 1;
    ostringstream ss;
    char a[4], b[4];

    ss << instset[mem[0]].name;

    if (size >= 2) { sprintf(a, "%02X", mem[1]); }
    else           { strcpy(a, "??"); }

    if (size >= 3) { sprintf(b, "%02X", mem[2]); }
    else           { strcpy(b, "??"); }

    switch (instset[mem[0]].mode ) {
    case AddressMode::Implicit:
        break;
    case AddressMode::Accumulator:
        ss << " A";
        break;
    case AddressMode::Immediate:
        ss << " #" << a;
        bytes++;
        break;
    case AddressMode::ZeroPage:
        ss << " $" << a;
        bytes++;
        break;
    case AddressMode::ZeroPageX:
        ss << " $" << a << ", X";
        bytes++;
        break;
    case AddressMode::ZeroPageY:
        ss << " $" << a << ", Y";
        bytes++;
        break;
    case AddressMode::Relative:
        if (size >= 2) { sprintf(a, "%+d", int8_t(mem[1])); }
        else           { strcpy(a, "??"); }

        ss << " " << a;
        bytes++;
        break;
    case AddressMode::Absolute:
        ss << " $" << b << a;
        bytes += 2;
        break;
    case AddressMode::AbsoluteX:
        ss << " $" << b << a << ", X";
        bytes += 2;
        break;
    case AddressMode::AbsoluteY:
        ss << " $" << b << a << ", Y";
        bytes += 2;
        break;
    case AddressMode::Indirect:
        ss << " ($" << b << a << ")";
        bytes += 2;
        break;
    case AddressMode::IndexedIndirect:
        ss << " ($" << a << ", X)";
        bytes++;
        break;
    case AddressMode::IndirectIndexed:
        ss << " ($" << a << "), Y";
        bytes++;
        break;
    default: logError("unknown address mode");
    }

    return make_tuple(ss.str(), bytes);
}

vector<string> disassemble(uint8_t* mem, uint32_t size, const InstructionSet& instset) {
    auto ret = vector<string>();
    string instr;
    int consumedBytes;

    while (size > 0) {
        tie(instr, consumedBytes) = oneInstr(mem, size, instset);

        ret.push_back(instr);

        size -= consumedBytes;
        mem += consumedBytes;
    }

    return ret;
}