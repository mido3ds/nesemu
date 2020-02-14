#pragma once

#include <fstream>
#include <vector>
#include <string>
#include <functional>
#include <thread>

#include "stdtype.h"

using namespace std;

struct Color {
    u8_t r, g, b;

    static Color fromPalatte(const u8_t palatte);
};

enum class AddressMode {
    Implicit, Accumulator, Immediate, ZeroPage,
    ZeroPageX, ZeroPageY, Relative, Absolute,
    AbsoluteX, AbsoluteY, Indirect, IndexedIndirect,
    IndirectIndexed
};

struct Region {
    u16_t start, end;

    constexpr bool contains(u16_t addr) const {return addr <= end && addr >= start;}  
    constexpr u16_t size() const {return (end + 1) - start;}
};

struct Mirror {
    Region source, dest;

    vector<u16_t> getAdresses(const u16_t address) const;
};

enum class SpriteType : u8_t {S8x8 = 0, S8x16 = 1};
enum class ColorMode : u8_t {Color = 0, Monochrome = 1};

struct SpriteInfo {
    u8_t y; // Y-coordinate of the top left of the sprite minus 1
    u8_t i; // Index number of the sprite in the pattern tables.

    union {
        struct {
            u8_t color:2; // Most significant two bits of the colour
            u8_t:3;
            u8_t pritority:1; // Indicates whether this sprite has priority over the background
            u8_t hFlip:1; // Indicates whether to flip the sprite horizontally
            u8_t vFlip:1; // Indicates whether to flip the sprite vertically}
        } bits;
        u8_t byte;
    } attr;
};

/*
 *  DCBA98 76543210
 *  ---------------
 *  0HRRRR CCCCPTTT
 *  |||||| |||||+++- T: Fine Y offset, the row number within a tile
 *  |||||| ||||+---- P: Bit plane (0: "lower"; 1: "upper")
 *  |||||| ++++----- C: Tile column
 *  ||++++---------- R: Tile row
 *  |+-------------- H: Half of sprite table (0: "left"; 1: "right")
 *  +--------------- 0: Pattern table is at $0000-$1FFF
 */
union PatternTablePointer {
    enum class BitPlane {LOWER, UPPER};
    enum class TableHalf {LEFT, RIGHT};

    struct {
        u8_t rowInTile:3;
        BitPlane bitPlane:1;
        u8_t tileCol:4;
        u8_t tileRow:4;
        TableHalf tableHalf:1;
        u8_t:3;
    } bits;
    const u16_t word = 0;
};

constexpr u8_t SPRITE_8x8_SIZE = 16;
constexpr u8_t SPRITE_8x16_SIZE = 2 * SPRITE_8x8_SIZE;
constexpr u32_t MEM_SIZE = 0xFFFF + 1;

typedef array<u8_t, MEM_SIZE> MemType;

// Video systems info 
constexpr struct VideoSystem {
    int cpuCycles; // in nanoseconds
    int fps;
    f32_t timePerFrame; // in milliseconds
    int scanlinesPerFrame;
    f32_t cpuCyclesPerScanline;
    struct {int width, height;} resolution;
} NTSC {559, 60, 16.67f, 262, 113.33f, {256, 224}},
PAL {601, 50, 20, 312, 106.56f, {256, 240}};

constexpr Color DEFAULT_COLOR = Color({0, 0, 0});

// memory regions
constexpr Region
    ZERO_PAGE {0x0000, 0x0100-1},
    STACK {0x0100, 0x0200-1},
    RAM {0x0200, 0x0800-1},

    IO_REGS0 {0x2000, 0x2008-1},
    IO_REGS1 {0x4000, 0x4020-1},

    EX_ROM {0x4020, 0x6000-1},
    SRAM {0x6000, 0x8000-1},
    PRG_ROM_LOW {0x8000, 0xC000-1},
    PRG_ROM_UP {0xC000, 0xFFFF};

// varm regions
constexpr Region 
    /* pattern tables */
    PATT_TBL0 {0x0000, 0x1000-1},
    PATT_TBL1 {0x1000, 0x2000-1},

    /* name tables */
    NAME_TBL0 {0x2000, 0x23C0-1},
    ATT_TBL0 {0x23C0, 0x2400-1},
    NAME_TBL1 {0x2400, 0x27C0-1},
    ATT_TBL1 {0x27C0, 0x2800-1},
    NAME_TBL2 {0x2800, 0x2BC0-1},
    ATT_TBL2 {0x2BC0, 0x2C00-1},
    NAME_TBL3 {0x2C00, 0x2FC0-1},
    ATT_TBL3 {0x2FC0, 0x3000-1},

    /* palettes */
    IMG_PLT {0x3F00, 0x3F10-1}, 
    SPR_PLT {0x3F10, 0x3F20-1};

constexpr array<Mirror, 2> MEM_MIRRORS {
    Mirror({{0x0000, 0x07FF}, {0x0800, 0x2000-1}}),
    Mirror({IO_REGS0, {0x2008, 0x4000-1}}),
};

constexpr array<Mirror, 3> VRAM_MIRRORS {
    Mirror({{0x2000, 0x2EFF}, {0x3000, 0x3F00-1}}),
    Mirror({{0x3F00, 0x3F1F}, {0x3F20, 0x4000-1}}),
    Mirror({{0x0000, 0x3FFF}, {0x4000, 0xFFFF}}),
};

// interrupt vector table
constexpr u16_t
    IRQ = 0xFFFE, 
    NMI = 0xFFFA, 
    RH = 0xFFFC;

// registers addresses
constexpr u16_t PPU_CTRL_REG0 = 0x2000,
                PPU_CTRL_REG1 = 0x2001, 
                PPU_STS_REG   = 0x2002,
                PPU_SCRL_REG  = 0x2006,

                SPRRAM_ADDR_REG = 0x2003,
                SPRRAM_IO_REG   = 0x2004,
                SPRITE_DMA_REG  = 0x4014,

                VRAM_ADDR_REG0 = 0x2005,
                VRAM_ADDR_REG1 = 0x2006,
                VRAM_IO_REG    = 0x2007,

                APU_PULSE1_CONTROL_REG                = 0x4000,
                APU_PULSE1_RAMP_CONTROL_REG           = 0x4001,
                APU_PULSE1_FINETUNE_REG               = 0x4002,
                APU_PULSE1_COARSETUNE_REG             = 0x4003,
                APU_PULSE2_CONTROL_REG                = 0x4004,
                APU_PULSE2_RAMP_CONTROL_REG           = 0x4005,
                APU_PULSE2_FINETUNE_REG               = 0x4006,
                APU_PULSE2_COARSETUNE_REG             = 0x4007,
                APU_TRIANGLE_CONTROL_REG1             = 0x4008,
                APU_TRIANGLE_CONTROL_REG2             = 0x4009,
                APU_TRIANGLE_FREQUENCY_REG1           = 0x400A,
                APU_TRIANGLE_FREQUENCY_REG2           = 0x400B,
                APU_NOISE_CONTROL_REG1                = 0x400C,
                APU_NOISE_FREQUENCY_REG1              = 0x400E,
                APU_NOISE_FREQUENCY_REG2              = 0x400F,
                APU_DELTA_MODULATION_CONTROL_REG      = 0x4010,
                APU_DELTA_MODULATION_DA_REG           = 0x4011,
                APU_DELTA_MODULATION_ADDRESS_REG      = 0x4012,
                APU_DELTA_MODULATION_DATA_LENGTH_REG  = 0x4013,
                APU_VERTICAL_CLOCK_SIGNAL_REG         = 0x4015;
