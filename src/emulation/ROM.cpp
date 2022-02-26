#include <fstream>
#include <tuple>

#include "emulation/ROM.h"
#include "log.h"

tuple<u8_t*, size_t> readBinaryFile(string path) {
    ifstream file(path, ios::in|ios::binary|ios::ate);
    if (!file.is_open()) return make_tuple(nullptr, 0);

    size_t size = (size_t)file.tellg();
    auto buffer = new char[size];

    file.seekg(0, ios::beg);
    file.read(buffer, size);
    file.close();

    return make_tuple((u8_t*) buffer, size);
}

int ROM::init(string path) {
    size_t size = 0;
    u8_t* buffer = nullptr;

    INFO("reading rom from %s", path.c_str());

    tie(buffer, size) = readBinaryFile(path);
    if (size == 0 || buffer == nullptr) {
        ERROR("couldn't load rom from path %s", path.c_str());
        return 1;
    }

    // check header
    const unsigned char constants[] = {0x4E, 0x45, 0x53, 0x1A};
    if (size < 16 || memcmp(buffer, constants, 4) != 0) {
        ERROR("no valid header");
        return 1;
    }

    // copy rest of header
	header = *(Header*) (buffer+4);
	static_assert(sizeof(header) == 16-4);

    // no trainer
    if (header.flags6.bits.hasTrainer) {
        WARNING("emulator doesnt support trainers, ignoring trainer");
    }

    // cpy PRG
    u8_t* prgPtr = buffer+16;
    if (header.flags6.bits.hasTrainer) {
        prgPtr += 512;
    }

    auto prgSize = getPRGRomSize();
    if (prgPtr+prgSize > buffer+size) {
        ERROR("no PRG ROM");
        return 1;
    }

    prg.clear();
    prg.insert(prg.end(), prgPtr, prgPtr+prgSize);

    // cpy CHR
    u8_t* chrPtr = prgPtr+prgSize;

    auto chrSize = getCHRRomSize();
    if (chrPtr+chrSize > buffer+size) {
        ERROR("no CHR ROM");
        return 1;
    }

    chr.clear();
    chr.insert(chr.end(), chrPtr, chrPtr+chrSize);

    // no playchoice
    if (header.flags7.bits.hasPlayChoice) {
        ERROR("emulator doesnt support PlayChoice");
        WARNING("ignoring PlayChoice");
    }

    delete buffer;

    INFO("rom mapper num = %d", getMapperNumber());
    INFO("iNES version = %d", header.flags7.bits.nes2format == 2? 2:1);
    INFO("rom num of PRG roms = %d", header.numPRGs);
    INFO("rom num of CHR roms = %d", header.numCHRs);
    if (!header.flags6.bits.ignoreMirroringControl) {
        if (header.flags6.bits.mirroring == 0){
            INFO("rom mirroring is horizontal");
        } else {
            INFO("rom mirroring is vertical");
        }
    } else {
        INFO("rom ignores mirroring");
    }
    INFO("done reading ROM");
    return 0;
}

u16_t ROM::getMapperNumber() const {
    return header.flags6.bits.lowerMapperNum | header.flags7.bits.upperMapperNum << 8;
}

u32_t ROM::getPRGRomSize() const {
    return header.numPRGs*16*1024; // 16 KB
}

u32_t ROM::getCHRRomSize() const {
    return header.numCHRs*8*1024; // 8 KB
}
