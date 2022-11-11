#include <fstream>
#include <tuple>

#include "utils.h"
#include "emulation/ROM.h"

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
