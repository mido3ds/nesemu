#include <fstream>
#include <tuple>

#include "rom.h"
#include "logger.h"

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

int ROM::fromFile(string path) {
    size_t size = 0;
    u8_t* buffer = nullptr;
    
    tie(buffer, size) = readBinaryFile(path);
    if (size == 0 || buffer == nullptr) {  
        logError("couldn't load rom from path %s", path.c_str());
        return 1;
    }

    // check header
    const unsigned char constants[] = {0x4E, 0x45, 0x53, 0x1A};
    if (size < 16 || memcmp(buffer, constants, 4) != 0) {
        logError("no valid header");
        return 1;
    }

    // copy rest of header
    memcpy(&header, buffer+4, 16-4);

    // no trainer

    // cpy PRG 
    u8_t* prgPtr = buffer+16;
    if (header.flags6.bits.hasTrainer) { 
        prgPtr += 512; 
    }

    auto prgSize = getPRGRomSize();
    if (prgPtr+prgSize > buffer+size) {
        logError("no PRG ROM");
        return 1;
    }

    if (!prgData) { prgData = new u8_t[prgSize]; }
    memcpy(prgData, prgPtr, prgSize);

    // cpy CHR
    u8_t* chrPtr = prgPtr+prgSize;

    auto chrSize = getCHRRomSize();
    if (chrPtr+chrSize > buffer+size) {
        logError("no CHR ROM");
        return 1;
    }
    
    if (!chrData) { chrData = new u8_t[chrSize]; }
    memcpy(chrData, chrPtr, chrSize);

    // no playchoice

    delete buffer;
    return 0;
}

int ROM::loadIntoMemory(u8_t* memory) {
    logWarning("TODO: load rom");
    return 0;
}

u16_t ROM::getMapperNumber() {
    return header.flags6.bits.lowerMapperNum | header.flags7.bits.upperMapperNum << 8;
}

u32_t ROM::getPRGRomSize() {
    return header.prgSize*16384;
}

u32_t ROM::getCHRRomSize() {
    return header.chrSize*8192;
}

ROM::~ROM() {
    if (chrData) {
        delete chrData;
    }

    if (prgData) {
        delete prgData;
    }
}