#include <fstream>
#include <tuple>

#include "utils.h"
#include "emulation/ROM.h"
#include "emulation/instructions.h"

static uint32_t rom_get_prg_rom_size(const ROM& self) {
    return self.header.num_prgs*16*1024; // 16 KB
}

static uint32_t rom_get_chr_rom_size(const ROM& self) {
    return self.header.num_chrs*8*1024; // 8 KB
}

void rom_load(ROM& self, const Str& path) {
    INFO("reading rom from {}", path);

    auto file = file_content_str(path.c_str(), memory::tmp());
    uint8_t* buffer = (uint8_t*) file.data();

    // check header
    const unsigned char constants[] = {0x4E, 0x45, 0x53, 0x1A};
    if (file.size() < 16 || memcmp(buffer, constants, 4) != 0) {
        panic("no valid header");
    }

    // copy rest of header
	self.header = *(ROM::Header*) (buffer+4);
	static_assert(sizeof(self.header) == 16-4);

    // no trainer
    if (self.header.flags6.bits.has_trainer) {
        WARNING("emulator doesnt support trainers, ignoring trainer");
    }

    // cpy PRG
    uint8_t* prg_ptr = buffer+16;
    if (self.header.flags6.bits.has_trainer) {
        prg_ptr += 512;
    }

    auto prg_size = rom_get_prg_rom_size(self);
    if (prg_ptr+prg_size > buffer+file.size()) {
        panic("no PRG ROM");
    }

    self.prg.clear();
    self.prg.insert(self.prg.end(), prg_ptr, prg_ptr+prg_size);

    // cpy CHR
    uint8_t* chr_ptr = prg_ptr+prg_size;

    auto chr_size = rom_get_chr_rom_size(self);
    if (chr_ptr+chr_size > buffer+file.size()) {
        panic("no CHR ROM");
    }

    self.chr.clear();
    self.chr.insert(self.chr.end(), chr_ptr, chr_ptr+chr_size);

    // no playchoice
    if (self.header.flags7.bits.has_play_choice) {
        WARNING("emulator doesnt support PlayChoice, ignoring PlayChoice");
    }

    const bool valid_mmc0_rom = rom_get_mapper_number(self) == 0 &&
        (self.prg.size() % (16*1024) == 0) &&
        (self.chr.size() == (8*1024));
    if (!valid_mmc0_rom) {
        panic("only supports MMC0");
    }

    INFO("rom mapper num = {}", rom_get_mapper_number(self));
    INFO("iNES version = {}", self.header.flags7.bits.nes2format == 2? 2:1);
    INFO("rom num of PRG roms = {}", self.header.num_prgs);
    INFO("rom num of CHR roms = {}", self.header.num_chrs);
    if (!self.header.flags6.bits.ignore_mirroring_control) {
        if (self.header.flags6.bits.mirroring == 0){
            INFO("rom mirroring is horizontal");
        } else {
            INFO("rom mirroring is vertical");
        }
    } else {
        INFO("rom ignores mirroring");
    }
    INFO("done reading ROM");
}

bool rom_read(ROM& self, uint16_t addr, uint8_t& data) {
    if (self.prg.size() > 0 && PRG_REGION.contains(addr)) {
        data = self.prg[(addr - PRG_ROM_LOW.start) % self.prg.size()];

        return true;
    }

    return false;
}

bool rom_write(ROM& self, uint16_t addr, uint8_t data) {
    if (self.prg.size() > 0 && PRG_REGION.contains(addr)) {
        self.prg[(addr - PRG_ROM_LOW.start) % self.prg.size()] = data;

        return true;
    }

    return false;
}

Vec<Assembly> rom_disassemble(const ROM& rom, memory::Allocator* allocator) {
    Vec<Assembly> out (allocator);

    uint8_t const* mem = rom.prg.data();
    int size = rom.prg.size();

    uint16_t addr = (size == PRG_ROM_LOW.size())? PRG_ROM_UP.start : PRG_ROM_LOW.start;

    while (size > 0) {
        int consumedBytes = 1;
        Str instr(allocator);
        Str a(memory::tmp()), b(memory::tmp());

        auto name = instruction_set[mem[0]].name;
        if (name == "???") {
            str_push(instr, "{:02X} ?????", mem[0]);
        } else {
            str_push(instr, name);
        }

        if (size >= 2) { str_push(a, "{:02X}", mem[1]); }
        else           { a = "??"; }

        if (size >= 3) { str_push(b, "{:02X}", mem[1]); }
        else           { b = "??"; }

        switch (instruction_set[mem[0]].mode ) {
        case AddressMode::Implicit:
            break;
        case AddressMode::Accumulator:
            str_push(instr, " A");
            break;
        case AddressMode::Immediate:
            str_push(instr, " #{}", a);
            consumedBytes++;
            break;
        case AddressMode::ZeroPage:
            str_push(instr, " ${}", a);
            consumedBytes++;
            break;
        case AddressMode::ZeroPageX:
            str_push(instr, " ${}, X", a);
            consumedBytes++;
            break;
        case AddressMode::ZeroPageY:
            str_push(instr, " ${}, Y", a);
            consumedBytes++;
            break;
        case AddressMode::Relative:
            if (size >= 2) { a.clear(); str_push(a, "{:+}", int8_t(mem[1])); }
            else           { a = "??"; }

            str_push(instr, " {}", a);
            consumedBytes++;
            break;
        case AddressMode::Absolute:
            str_push(instr, " ${}{}", b, a);
            consumedBytes += 2;
            break;
        case AddressMode::AbsoluteX:
            str_push(instr, " ${}{}, X", b, a);
            consumedBytes += 2;
            break;
        case AddressMode::AbsoluteY:
            str_push(instr, " ${}{}, Y", b, a);
            consumedBytes += 2;
            break;
        case AddressMode::Indirect:
            str_push(instr, " (${}{})", b, a);
            consumedBytes += 2;
            break;
        case AddressMode::IndexedIndirect:
            str_push(instr, " (${}, X)", a);
            consumedBytes++;
            break;
        case AddressMode::IndirectIndexed:
            str_push(instr, " (${}, Y)", a);
            consumedBytes++;
            break;
        default: unreachable();
        }

        out.push_back(Assembly {
            .adr = addr,
            .instr = instr,
        });

        size -= consumedBytes;
        mem  += consumedBytes;
        addr += consumedBytes;
    }

    return std::move(out);
}
