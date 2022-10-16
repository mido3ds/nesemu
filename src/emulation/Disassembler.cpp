#include <sstream>
#include <tuple>
#include <map>
#include <cstring>
#include <iterator>

#include "utils.h"
#include "emulation/Disassembler.h"
#include "emulation/instructions.h"

void Disassembler::init(const Vec<uint8_t>& data, uint16_t addr) {
    uint8_t const* mem = data.data();
    int size = data.size();

    while (size > 0) {
        auto [instr, consumedBytes] = dissasmble(mem, size);

        assembly[addr] = instr;

        size -= consumedBytes;
        mem  += consumedBytes;
        addr += consumedBytes;
    }
}

static Str addAddress(uint16_t addr, StrView s) {
    return str_format("${:04X}: {}", addr, s);
}

Vec<Str> Disassembler::get(const uint16_t addr, const uint16_t n) const {
    Vec<Str> ret(2*n+1, "$????: ???");

    auto tmpItr = assembly.lower_bound(addr);
    auto fItr = tmpItr; // forward itr, addr : end
    auto rItr = make_reverse_iterator(tmpItr); // reverse itr, addr-1 : rend

    // addr
    if (fItr != assembly.end()) {
        if (fItr->first == addr) {
            ret[n] = str_format("${:04X}: {}", fItr->first, fItr->second);
            fItr++;
        } else {
            ret[n] = str_format("${:04X}: ???", addr);
        }
    }

    // [addr+1..addr+n]
    for (auto i = n+1; fItr != assembly.end() && i < ret.size(); fItr++, i++) {
        ret[i] = str_format("${:04X}: {}", fItr->first, fItr->second);
    }

    // [addr-n..addr-1]
    for (auto i = n-1; rItr != assembly.rend() && i >= 0; rItr++, i--) {
        ret[i] = str_format("${:04X}: {}", rItr->first, rItr->second);
    }

    return ret;
}

std::tuple<Str, int> Disassembler::dissasmble(uint8_t const* mem, uint32_t size) {
    if (mem == nullptr || size == 0) { return {"???", 0}; }

    int bytes = 1;
    Str out;
    Str a(memory::tmp()), b(memory::tmp());

    auto name = instructionSet[mem[0]].name;
    if (name == "???") {
        str_push(out, "{:02X} ?????", mem[0]);
    } else {
        str_push(out, name);
    }

    if (size >= 2) { str_push(a, "{:02X}", mem[1]); }
    else           { a = "??"; }

    if (size >= 3) { str_push(b, "{:02X}", mem[1]); }
    else           { b = "??"; }

    switch (instructionSet[mem[0]].mode ) {
    case AddressMode::Implicit:
        break;
    case AddressMode::Accumulator:
        str_push(out, " A");
        break;
    case AddressMode::Immediate:
        str_push(out, " #{}", a);
        bytes++;
        break;
    case AddressMode::ZeroPage:
        str_push(out, " ${}", a);
        bytes++;
        break;
    case AddressMode::ZeroPageX:
        str_push(out, " ${}, X", a);
        bytes++;
        break;
    case AddressMode::ZeroPageY:
        str_push(out, " ${}, Y", a);
        bytes++;
        break;
    case AddressMode::Relative:
        if (size >= 2) { a.clear(); str_push(a, "{:+}", int8_t(mem[1])); }
        else           { a = "??"; }

        str_push(out, " {}", a);
        bytes++;
        break;
    case AddressMode::Absolute:
        str_push(out, " ${}{}", b, a);
        bytes += 2;
        break;
    case AddressMode::AbsoluteX:
        str_push(out, " ${}{}, X", b, a);
        bytes += 2;
        break;
    case AddressMode::AbsoluteY:
        str_push(out, " ${}{}, Y", b, a);
        bytes += 2;
        break;
    case AddressMode::Indirect:
        str_push(out, " (${}{})", b, a);
        bytes += 2;
        break;
    case AddressMode::IndexedIndirect:
        str_push(out, " (${}, X)", a);
        bytes++;
        break;
    case AddressMode::IndirectIndexed:
        str_push(out, " (${}, Y)", a);
        bytes++;
        break;
    default: unreachable();
    }

    return {out, bytes};
}
