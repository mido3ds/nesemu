#include <fstream>
#include <tuple>

#include "ROM.h"
#include "instructions.h"

static uint32_t rom_get_prg_rom_size(const ROM& self) {
    return self.header.num_prgs*16*1024; // 16 KB
}

static uint32_t rom_get_chr_rom_size(const ROM& self) {
    return self.header.num_chrs*8*1024; // 8 KB
}

inline static mu::Vec<uint8_t>
_file_content_bin(const char* path, mu::memory::Allocator* allocator = mu::memory::default_allocator()) {
    auto file = fopen(path, "rb");
    if (file == nullptr) {
        mu::panic("failed to open file '{}' for reading", path);
    }
    mu_defer(fclose(file));

    if (fseek(file, 0, SEEK_END)) {
        mu::panic("failed to seek in file '{}' for reading", path);
    }
    const auto expected_size = ftell(file);
    if (fseek(file, 0, SEEK_SET)) {
        mu::panic("failed to seek back in file '{}' for reading", path);
    }

    mu::Vec<uint8_t> content(expected_size, 0, allocator);
    const auto read_size = fread(content.data(), 1, expected_size, file);
    if (read_size != expected_size) {
        mu::panic("failed to read file '{}', expected to read {} bytes but got {} bytes", path, expected_size, read_size);
    }

    return content;
}

void rom_from_ines_file(ROM& self, const mu::Str& ines_path) {
    auto file = _file_content_bin(ines_path.c_str(), mu::memory::tmp());
    uint8_t* buffer = (uint8_t*) file.data();

    // copy header
    if (file.size() < sizeof(self.header)) {
        mu::panic("too small iNES file");
    }
	self.header = *(INESFileHeader*) buffer;

    constexpr uint8_t ines_magic[] = {0x4E, 0x45, 0x53, 0x1A}; // ("NES" + MS/DOS EOF)
    if (memcmp(self.header._magic, ines_magic, 4) != 0) {
        mu::panic("no iNES header");
    }

    // copy PRG
    uint8_t* prg_ptr = buffer+sizeof(self.header);
    if (self.header.flags6.bits.has_trainer) {
        mu::log_warning("emulator doesnt support trainers, ignoring trainer");
        prg_ptr += 512;
    }

    auto prg_size = rom_get_prg_rom_size(self);
    if (prg_ptr+prg_size > buffer+file.size()) {
        mu::panic("no PRG ROM");
    }

    self.prg.clear();
    self.prg.insert(self.prg.end(), prg_ptr, prg_ptr+prg_size);

    // copy CHR
    uint8_t* chr_ptr = prg_ptr+prg_size;

    auto chr_size = rom_get_chr_rom_size(self);
    if (chr_ptr+chr_size > buffer+file.size()) {
        mu::panic("no CHR ROM");
    }

    self.chr.clear();
    self.chr.insert(self.chr.end(), chr_ptr, chr_ptr+chr_size);

    const bool valid_mmc0_rom = rom_get_mapper_number(self) == 0 &&
        (self.prg.size() % (16*1024) == 0) &&
        (self.chr.size() == (8*1024));
    if (!valid_mmc0_rom) {
        mu::panic("only supports MMC0");
    }

    if (self.header.flags7.bits.has_play_choice) {
        mu::log_warning("emulator doesnt support PlayChoice, ignoring PlayChoice");
    }
    mu::log_debug("rom mapper num = {}", rom_get_mapper_number(self));
    mu::log_debug("iNES version = {}", self.header.flags7.bits.nes2format == 2? 2:1);
    mu::log_debug("rom num of PRG roms = {}", self.header.num_prgs);
    mu::log_debug("rom num of CHR roms = {}", self.header.num_chrs);
    if (!self.header.flags6.bits.ignore_mirroring_control) {
        if (self.header.flags6.bits.mirroring == 0){
            mu::log_debug("rom mirroring is horizontal");
        } else {
            mu::log_debug("rom mirroring is vertical");
        }
    } else {
        mu::log_warning("rom ignores mirroring");
    }
    mu::log_debug("loaded rom from {}", ines_path);
}

bool rom_read(ROM& self, uint16_t addr, uint8_t& data) {
    if (self.prg.size() > 0 && region_contains(PRG_REGION, addr)) {
        data = self.prg[(addr - PRG_ROM_LOW.start) % self.prg.size()];

        return true;
    }

    return false;
}

bool rom_write(ROM& self, uint16_t addr, uint8_t data) {
    if (self.prg.size() > 0 && region_contains(PRG_REGION, addr)) {
        self.prg[(addr - PRG_ROM_LOW.start) % self.prg.size()] = data;

        return true;
    }

    return false;
}

mu::Vec<Assembly> rom_disassemble(const ROM& rom, mu::memory::Allocator* allocator) {
    mu::Vec<Assembly> out(allocator);

    uint8_t const* mem = rom.prg.data();
    int size = (int) rom.prg.size();

    uint16_t addr = (size == region_size(PRG_ROM_LOW)) ? PRG_ROM_UP.start : PRG_ROM_LOW.start;

    while (size > 0) {
        int consumedBytes = 1;
        mu::Str instr(allocator);
        mu::Str a(mu::memory::tmp()), b(mu::memory::tmp());

        auto name = instruction_set[mem[0]].name;
        if (name == "???") {
            mu::str_push(instr, "{:02X} ?????", mem[0]);
        } else {
            mu::str_push(instr, name);
        }

        if (size >= 2) { mu::str_push(a, "{:02X}", mem[1]); }
        else           { a = "??"; }

        if (size >= 3) { mu::str_push(b, "{:02X}", mem[1]); }
        else           { b = "??"; }

        switch (instruction_set[mem[0]].mode ) {
        case AddressMode::Implicit:
            break;
        case AddressMode::Accumulator:
            mu::str_push(instr, " A");
            break;
        case AddressMode::Immediate:
            mu::str_push(instr, " #{}", a);
            consumedBytes++;
            break;
        case AddressMode::ZeroPage:
            mu::str_push(instr, " ${}", a);
            consumedBytes++;
            break;
        case AddressMode::ZeroPageX:
            mu::str_push(instr, " ${}, X", a);
            consumedBytes++;
            break;
        case AddressMode::ZeroPageY:
            mu::str_push(instr, " ${}, Y", a);
            consumedBytes++;
            break;
        case AddressMode::Relative:
            if (size >= 2) { a.clear(); mu::str_push(a, "{:+}", int8_t(mem[1])); }
            else           { a = "??"; }

            mu::str_push(instr, " {}", a);
            consumedBytes++;
            break;
        case AddressMode::Absolute:
            mu::str_push(instr, " ${}{}", b, a);
            consumedBytes += 2;
            break;
        case AddressMode::AbsoluteX:
            mu::str_push(instr, " ${}{}, X", b, a);
            consumedBytes += 2;
            break;
        case AddressMode::AbsoluteY:
            mu::str_push(instr, " ${}{}, Y", b, a);
            consumedBytes += 2;
            break;
        case AddressMode::Indirect:
            mu::str_push(instr, " (${}{})", b, a);
            consumedBytes += 2;
            break;
        case AddressMode::IndexedIndirect:
            mu::str_push(instr, " (${}, X)", a);
            consumedBytes++;
            break;
        case AddressMode::IndirectIndexed:
            mu::str_push(instr, " (${}, Y)", a);
            consumedBytes++;
            break;
        default: mu_unreachable();
        }

        out.push_back(Assembly {
            .adr = addr,
            .instr = instr,
        });

        size -= consumedBytes;
        mem  += consumedBytes;
        addr += consumedBytes;
    }

    return out;
}
