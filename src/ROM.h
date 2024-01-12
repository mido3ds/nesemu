#pragma once

#include "common.h"

struct INESFileHeader {
    uint8_t _magic[4];

    uint8_t num_prgs;
    uint8_t num_chrs;

    // 76543210
    // ||||||||
    // |||||||+- Mirroring: 0: horizontal (vertical arrangement) (CIRAM A10 = PPU A11)
    // |||||||              1: vertical (horizontal arrangement) (CIRAM A10 = PPU A10)
    // ||||||+-- 1: Cartridge contains battery-backed PRG RAM ($6000-7FFF) or other persistent memory
    // |||||+--- 1: 512-byte trainer at $7000-$71FF (stored before PRG data)
    // ||||+---- 1: Ignore mirroring control or above mirroring bit; instead provide four-screen VRAM
    // ++++----- Lower nybble of mapper number
    union {
        struct {
            uint8_t mirroring:1;
            bool has_battery_backed_prgram:1;
            bool has_trainer:1;
            bool ignore_mirroring_control:1;
            uint8_t lower_mapper_num:4;
        } bits;
        uint8_t byte;
    } flags6;

    // 76543210
    // ||||||||
    // |||||||+- VS Unisystem
    // ||||||+-- PlayChoice-10 (8KB of Hint Screen data stored after CHR data)
    // ||||++--- If equal to 2, flags 8-15 are in NES 2.0 format
    // ++++----- Upper nybble of mapper number
    union {
        struct {
            uint8_t vs_unisystem:1;
            bool has_play_choice:1;
            uint8_t nes2format:2;
            uint8_t upper_mapper_num:4;
        } bits;
        uint8_t byte;
    } flags7;

    // Flags 8, 9 and 10 are not supported
    uint8_t _padding[8];
};

static_assert(sizeof(INESFileHeader) == 16);

struct ROM {
    INESFileHeader header;
    mu::Vec<uint8_t> prg, chr;
};

void rom_from_ines_file(ROM& self, const mu::Str& ines_path);

inline uint16_t rom_get_mapper_number(const ROM& self) {
    return self.header.flags6.bits.lower_mapper_num | self.header.flags7.bits.upper_mapper_num << 8;
}

bool rom_read(ROM& self, uint16_t addr, uint8_t& data);
bool rom_write(ROM& self, uint16_t addr, uint8_t data);

struct Assembly {
    uint16_t adr;
    mu::Str instr;
};

mu::Vec<Assembly> rom_disassemble(const ROM& self, mu::memory::Allocator* allocator = mu::memory::default_allocator());
