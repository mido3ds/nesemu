#include <sstream>
#include <tuple>
#include <map>
#include <cstring>
#include <iterator>

#include "emulation/Disassembler.h"
#include "emulation/instructions.h"
#include "log.h"

void Disassembler::init(const vector<uint8_t>& data, uint16_t addr) {
    uint8_t const* mem = data.data();
    int size = data.size();

    while (size > 0) {
        string instr;
        int consumedBytes;
        tie(instr, consumedBytes) = dissasmble(mem, size);

        assembly[addr] = instr;

        size -= consumedBytes;
        mem  += consumedBytes;
        addr += consumedBytes;
    }
}

static string addAddress(uint16_t addr, string s) {
    char adrPart[10];
    sprintf(adrPart, "$%04X", addr);
    return string(adrPart) + ": " + s;
}

vector<string> Disassembler::get(const uint16_t addr, const uint16_t n) const {
    vector<string> ret(2*n+1, "$????: ???");

    auto tmpItr = assembly.lower_bound(addr);
    auto fItr = tmpItr; // forward itr, addr : end
    auto rItr = make_reverse_iterator(tmpItr); // reverse itr, addr-1 : rend

    // addr
    if (fItr != assembly.end()) {
        if (fItr->first == addr) {
            ret[n] = addAddress(fItr->first, fItr->second);
            fItr++;
        } else {
            ret[n] = addAddress(addr, "???");
        }
    }

    // [addr+1..addr+n]
    for (auto i = n+1; fItr != assembly.end() && i < ret.size(); fItr++, i++) {
        ret[i] = addAddress(fItr->first, fItr->second);
    }

    // [addr-n..addr-1]
    for (auto i = n-1; rItr != assembly.rend() && i >= 0; rItr++, i--) {
        ret[i] = addAddress(rItr->first, rItr->second);
    }

    return ret;
}

tuple<string, int> Disassembler::dissasmble(uint8_t const* mem, uint32_t size) {
    if (mem == nullptr || size == 0) { return make_tuple("???", 0); }

    int bytes = 1;
    ostringstream ss;
    char a[5] = {0}, b[5] = {0};

    auto name = instructionSet[mem[0]].name;
    if (name == "???") {
        char tmp[10];
        sprintf(tmp, "$%02X", mem[0]);
        ss << tmp << " ?????";
    } else {
        ss << name;
    }

    if (size >= 2) { sprintf(a, "%02X", mem[1]); }
    else           { strcpy(a, "??"); }

    if (size >= 3) { sprintf(b, "%02X", mem[2]); }
    else           { strcpy(b, "??"); }

    switch (instructionSet[mem[0]].mode ) {
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
    default: ERROR("unknown address mode");
    }

    return make_tuple(ss.str(), bytes);
}
