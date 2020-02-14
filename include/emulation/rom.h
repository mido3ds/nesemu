#pragma once

#include <string>

#include "stdtype.h"
#include "emulation/common.h"

using namespace std;

class ROM {
private:
    struct {
        u8_t numPRGs = 0;
        u8_t numCHRs = 0;

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
                u8_t mirroring:1;
                bool hasBatteryBackedPRGRAM:1;
                bool hasTrainer:1;
                bool ignoreMirroringControl:1;
                u8_t lowerMapperNum:4;
            } bits;
            u8_t byte;
        } flags6;

        // 76543210
        // ||||||||
        // |||||||+- VS Unisystem
        // ||||||+-- PlayChoice-10 (8KB of Hint Screen data stored after CHR data)
        // ||||++--- If equal to 2, flags 8-15 are in NES 2.0 format
        // ++++----- Upper nybble of mapper number
        union {
            struct {
                u8_t vsUnisystem:1;
                bool hasPlayChoice:1;
                u8_t nes2format:2;
                u8_t upperMapperNum:4;
            } bits;
            u8_t byte;
        } flags7;

        // Flags 8, 9 and 10 are not supported
        u8_t:8;
        u8_t:8;
        u8_t:8;
    } header;

    u8_t *prgData = nullptr, 
        *chrData = nullptr;
public:
    int fromFile(string path);

    int copyToMemory(MemType* memory);

    u16_t getMapperNumber();

    u32_t getPRGRomSize();
    u32_t getCHRRomSize();

    ~ROM();
};