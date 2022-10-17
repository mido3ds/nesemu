#include <fstream>
#include <tuple>

#include "utils.h"
#include "emulation/ROM.h"

void ROM::load(StrView path) {
    INFO("reading rom from {}", path);

    auto file = file_content_str(path.begin(), memory::tmp());
    uint8_t* buffer = (uint8_t*) file.data();

    // check header
    const unsigned char constants[] = {0x4E, 0x45, 0x53, 0x1A};
    if (file.size() < 16 || memcmp(buffer, constants, 4) != 0) {
        panic("no valid header");
    }

    // copy rest of header
	header = *(Header*) (buffer+4);
	static_assert(sizeof(header) == 16-4);

    // no trainer
    if (header.flags6.bits.has_trainer) {
        WARNING("emulator doesnt support trainers, ignoring trainer");
    }

    // cpy PRG
    uint8_t* prgPtr = buffer+16;
    if (header.flags6.bits.has_trainer) {
        prgPtr += 512;
    }

    auto prgSize = get_prg_rom_size();
    if (prgPtr+prgSize > buffer+file.size()) {
        panic("no PRG ROM");
    }

    prg.clear();
    prg.insert(prg.end(), prgPtr, prgPtr+prgSize);

    // cpy CHR
    uint8_t* chrPtr = prgPtr+prgSize;

    auto chrSize = get_chr_rom_size();
    if (chrPtr+chrSize > buffer+file.size()) {
        panic("no CHR ROM");
    }

    chr.clear();
    chr.insert(chr.end(), chrPtr, chrPtr+chrSize);

    // no playchoice
    if (header.flags7.bits.has_play_choice) {
        WARNING("emulator doesnt support PlayChoice, ignoring PlayChoice");
    }

    INFO("rom mapper num = {}", get_mapper_number());
    INFO("iNES version = {}", header.flags7.bits.nes2format == 2? 2:1);
    INFO("rom num of PRG roms = {}", header.num_prgs);
    INFO("rom num of CHR roms = {}", header.num_chrs);
    if (!header.flags6.bits.ignore_mirroring_control) {
        if (header.flags6.bits.mirroring == 0){
            INFO("rom mirroring is horizontal");
        } else {
            INFO("rom mirroring is vertical");
        }
    } else {
        INFO("rom ignores mirroring");
    }
    INFO("done reading ROM");
}

uint16_t ROM::get_mapper_number() const {
    return header.flags6.bits.lower_mapper_num | header.flags7.bits.upper_mapper_num << 8;
}

uint32_t ROM::get_prg_rom_size() const {
    return header.num_prgs*16*1024; // 16 KB
}

uint32_t ROM::get_chr_rom_size() const {
    return header.num_chrs*8*1024; // 8 KB
}
