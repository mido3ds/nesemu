#pragma once

#include <mu/utils.h>

struct RGBAColor {
    uint8_t r, g, b, a;
};

static constexpr mu::Arr<RGBAColor, 0x3F + 1> NES_PALETTE = {
    RGBAColor({0x75, 0x75, 0x75, 0xFF}),
    RGBAColor({0x27, 0x1B, 0x8F, 0xFF}),
    RGBAColor({0x00, 0x00, 0xAB, 0xFF}),
    RGBAColor({0x47, 0x00, 0x9F, 0xFF}),
    RGBAColor({0x8F, 0x00, 0x77, 0xFF}),
    RGBAColor({0xAB, 0x00, 0x13, 0xFF}),
    RGBAColor({0xA7, 0x00, 0x00, 0xFF}),
    RGBAColor({0x7F, 0x0B, 0x00, 0xFF}),
    RGBAColor({0x43, 0x2F, 0x00, 0xFF}),
    RGBAColor({0x00, 0x47, 0x00, 0xFF}),
    RGBAColor({0x00, 0x51, 0x00, 0xFF}),
    RGBAColor({0x00, 0x3F, 0x17, 0xFF}),
    RGBAColor({0x1B, 0x3F, 0x5F, 0xFF}),
    RGBAColor({0x00, 0x00, 0x00, 0xFF}),
    RGBAColor({0x00, 0x00, 0x00, 0xFF}),
    RGBAColor({0x00, 0x00, 0x00, 0xFF}),
    RGBAColor({0xBC, 0xBC, 0xBC, 0xFF}),
    RGBAColor({0x00, 0x73, 0xEF, 0xFF}),
    RGBAColor({0x23, 0x3B, 0xEF, 0xFF}),
    RGBAColor({0x83, 0x00, 0xF3, 0xFF}),
    RGBAColor({0xBF, 0x00, 0xBF, 0xFF}),
    RGBAColor({0xE7, 0x00, 0x5B, 0xFF}),
    RGBAColor({0xDB, 0x2B, 0x00, 0xFF}),
    RGBAColor({0xCB, 0x4F, 0x0F, 0xFF}),
    RGBAColor({0x8B, 0x73, 0x00, 0xFF}),
    RGBAColor({0x00, 0x97, 0x00, 0xFF}),
    RGBAColor({0x00, 0xAB, 0x00, 0xFF}),
    RGBAColor({0x00, 0x93, 0x3B, 0xFF}),
    RGBAColor({0x00, 0x83, 0x8B, 0xFF}),
    RGBAColor({0x00, 0x00, 0x00, 0xFF}),
    RGBAColor({0x00, 0x00, 0x00, 0xFF}),
    RGBAColor({0x00, 0x00, 0x00, 0xFF}),
    RGBAColor({0xFF, 0xFF, 0xFF, 0xFF}),
    RGBAColor({0x3F, 0xBF, 0xFF, 0xFF}),
    RGBAColor({0x5F, 0x97, 0xFF, 0xFF}),
    RGBAColor({0xA7, 0x8B, 0xFD, 0xFF}),
    RGBAColor({0xF7, 0x7B, 0xFF, 0xFF}),
    RGBAColor({0xFF, 0x77, 0xB7, 0xFF}),
    RGBAColor({0xFF, 0x77, 0x63, 0xFF}),
    RGBAColor({0xFF, 0x9B, 0x3B, 0xFF}),
    RGBAColor({0xF3, 0xBF, 0x3F, 0xFF}),
    RGBAColor({0x83, 0xD3, 0x13, 0xFF}),
    RGBAColor({0x4F, 0xDF, 0x4B, 0xFF}),
    RGBAColor({0x58, 0xF8, 0x98, 0xFF}),
    RGBAColor({0x00, 0xEB, 0xDB, 0xFF}),
    RGBAColor({0x00, 0x00, 0x00, 0xFF}),
    RGBAColor({0x00, 0x00, 0x00, 0xFF}),
    RGBAColor({0x00, 0x00, 0x00, 0xFF}),
    RGBAColor({0xFF, 0xFF, 0xFF, 0xFF}),
    RGBAColor({0xAB, 0xE7, 0xFF, 0xFF}),
    RGBAColor({0xC7, 0xD7, 0xFF, 0xFF}),
    RGBAColor({0xD7, 0xCB, 0xFF, 0xFF}),
    RGBAColor({0xFF, 0xC7, 0xFF, 0xFF}),
    RGBAColor({0xFF, 0xC7, 0xDB, 0xFF}),
    RGBAColor({0xFF, 0xBF, 0xB3, 0xFF}),
    RGBAColor({0xFF, 0xDB, 0xAB, 0xFF}),
    RGBAColor({0xFF, 0xE7, 0xA3, 0xFF}),
    RGBAColor({0xE3, 0xFF, 0xA3, 0xFF}),
    RGBAColor({0xAB, 0xF3, 0xBF, 0xFF}),
    RGBAColor({0xB3, 0xFF, 0xCF, 0xFF}),
    RGBAColor({0x9F, 0xFF, 0xF3, 0xFF}),
    RGBAColor({0x00, 0x00, 0x00, 0xFF}),
    RGBAColor({0x00, 0x00, 0x00, 0xFF}),
    RGBAColor({0x00, 0x00, 0x00, 0xFF})
};

inline static RGBAColor
color_from_palette(uint8_t index) {
    if (index >= NES_PALETTE.size()) {
        mu::log_error("invalid index color(0x{:02x}), returning black", index);
        return RGBAColor{0};
    }
    return NES_PALETTE[index];
}

enum class AddressMode {
    Implicit, Accumulator, Immediate, ZeroPage,
    ZeroPageX, ZeroPageY, Relative, Absolute,
    AbsoluteX, AbsoluteY, Indirect, IndexedIndirect,
    IndirectIndexed
};

struct Region {
    uint16_t start, end;
};

inline static bool
region_contains(Region self, uint16_t addr) {
    return addr <= self.end && addr >= self.start;
}

inline static uint16_t
region_size(Region self) {
    return (self.end + 1) - self.start;
}

// struct Mirror {
//     Region source, dest;

//     mu::Vec<uint16_t> get_address(const uint16_t address) const;
// };

// enum class SpriteType : uint8_t {S8x8 = 0, S8x16 = 1};
// enum class ColorMode : uint8_t {Color = 0, Monochrome = 1};

// struct SpriteInfo {
//     uint8_t y; // Y-coordinate of the top left of the sprite minus 1
//     uint8_t i; // Index number of the sprite in the pattern tables.

//     union {
//         struct {
//             uint8_t color:2; // Most significant two bits of the colour
//             uint8_t:3;
//             uint8_t pritority:1; // Indicates whether this sprite has priority over the background
//             uint8_t hFlip:1; // Indicates whether to flip the sprite horizontally
//             uint8_t vFlip:1; // Indicates whether to flip the sprite vertically}
//         } bits;
//         uint8_t byte;
//     } attr;
// };

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
    enum class BitPlane: uint8_t {LOWER, UPPER};
    enum class TableHalf: uint8_t {LEFT, RIGHT};

    struct {
        uint8_t row_in_tile:3;
        BitPlane bit_plane:1;
        uint8_t tile_col:4;
        uint8_t tile_row:4;
        TableHalf table_half:1;
        uint8_t:3;
    } bits;
    uint16_t word;
};

static_assert(sizeof(PatternTablePointer) == sizeof(uint16_t));

// constexpr uint8_t SPRITE_8x8_SIZE = 16;
// constexpr uint8_t SPRITE_8x16_SIZE = 2 * SPRITE_8x8_SIZE;
// constexpr uint32_t MEM_SIZE = 0xFFFF + 1;

// using MemType = mu::Arr<uint8_t, MEM_SIZE>;

// Video systems info
constexpr struct VideoSystem {
    int cpu_cycles; // in nanoseconds
    int fps;
    float time_per_frame; // in milliseconds
    int scanlines_per_frame;
    float cpu_cycles_per_scanline;
    struct {int width, height;} resolution;
} NTSC {559, 60, 16.67f, 262, 113.33f, {256, 224}},
PAL {601, 50, 20, 312, 106.56f, {256, 240}};

// memory regions
constexpr Region
    ZERO_PAGE {0x0000, 0x0100-1},
    STACK {0x0100, 0x0200-1},
    RRAM {0x0200, 0x0800-1}, // real ram
    RAM_REGION {0x0000, 0x1FFF}, // all ram, including mirrored parts

    IO_REGS0 {0x2000, 0x2008-1},
    IO_REGS1 {0x4000, 0x4020-1},

    EX_ROM {0x4020, 0x6000-1},
    SRAM {0x6000, 0x8000-1},
    PRG_ROM_LOW {0x8000, 0xC000-1},
    PRG_ROM_UP {0xC000, 0xFFFF},
    PRG_REGION {0x8000, 0xFFFF}; // all prg, with two parts

// vram regions
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

// constexpr mu::Arr<Mirror, 2> MEM_MIRRORS {
//     Mirror({{0x0000, 0x07FF}, {0x0800, 0x2000-1}}),
//     Mirror({IO_REGS0, {0x2008, 0x4000-1}}),
// };

// constexpr mu::Arr<Mirror, 3> VRAM_MIRRORS {
//     Mirror({{0x2000, 0x2EFF}, {0x3000, 0x3F00-1}}),
//     Mirror({{0x3F00, 0x3F1F}, {0x3F20, 0x4000-1}}),
//     Mirror({{0x0000, 0x3FFF}, {0x4000, 0xFFFF}}),
// };

// interrupt mu::Vec table
constexpr uint16_t
    IRQ = 0xFFFE,
    NMI = 0xFFFA,
    RH = 0xFFFC;

// registers addresses
constexpr uint16_t PPU_CTRL_REG0 = 0x2000,
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

namespace Config {
    constexpr VideoSystem sys = PAL;

    struct Rect {int w, h;};
    constexpr Rect resolution{sys.resolution.width, sys.resolution.height};

    constexpr Rect window{resolution.w * 3, resolution.h * 3};

    // scale if use retina display, because it has very big resolution
    // very hacky solution, if another system has big resolution it would fail :D
    #ifdef OS_MACOS
    constexpr int view_scale = 2;
    #else
    constexpr int view_scale = 1;
    #endif
}

struct CPURegs {
    uint16_t pc; // program counter
    uint8_t sp; // stack pointer
    uint8_t a; // accumulator
    uint8_t x; // index
    uint8_t y; // index
    union {
        struct {
            uint8_t c:1; // carry flag
            uint8_t z:1; // zero flag
            uint8_t i:1; // interrupt disable
            uint8_t d:1; // decimal mode
            uint8_t b:1; // break command
            uint8_t:1;
            uint8_t v:1; // overflow flag
            uint8_t n:1; // negative flag
        } bits;
        uint8_t byte;
    } flags; // processor status
};

static_assert(sizeof(CPURegs::flags) == sizeof(uint8_t));

struct Console;

struct CPU {
    Console* console;

    CPURegs regs;
    uint16_t cycles;

    // for instructions
    uint8_t arg_value;
    uint16_t arg_addr;
    AddressMode mode;

    bool cross_page_penalty;
};

CPU cpu_new(Console* console);
void cpu_reset(CPU& self);
void cpu_clock(CPU& self);

uint8_t cpu_read(CPU& self, uint16_t address);
uint16_t cpu_read16(CPU& self, uint16_t address);
uint8_t cpu_fetch(CPU& self); // read and increment pc

void cpu_write(CPU& self, uint16_t address, uint8_t data);
void cpu_write16(CPU& self, uint16_t address, uint16_t v);

void cpu_push(CPU& self, uint8_t v);
void cpu_push16(CPU& self, uint16_t v);

uint8_t cpu_pop(CPU& self);
uint16_t cpu_pop16(CPU& self);

void cpu_write_arg(CPU& self, uint8_t v);
void cpu_reprepare_jmp_arg(CPU& self);

struct Instruction {
    std::function<void(CPU&)> exec;
    mu::StrView name;
    AddressMode mode;
    uint16_t cycles;
    bool cross_page_penalty;
};

using InstructionSet = mu::Arr<Instruction, 0xFF+1>;
extern const InstructionSet instruction_set;

struct Assembly {
    uint16_t adr;
    mu::Str instr;
};

mu::Vec<Assembly>
bytecodes_disassemble(const mu::Vec<uint8_t>& bytecodes, mu::memory::Allocator* allocator = mu::memory::default_allocator());

struct ScreenBuf {
    size_t w, h;
    mu::Vec<RGBAColor> pixels;
};

inline static ScreenBuf
screenbuf_new(size_t w, size_t h) {
    return ScreenBuf {
        .w = w,
        .h = h,
        .pixels = mu::Vec<RGBAColor>(w*h, RGBAColor{0}),
    };
}

inline static void
screenbuf_put(ScreenBuf& self, size_t xw, size_t yh, RGBAColor color) {
    mu_assert(xw < self.w && yh < self.h);
    self.pixels.at(xw + yh * self.w) = color;
}

struct Palette {
    uint8_t index[4];
};

struct PPU {
    Console* console;
    uint16_t cycles = 0, row = 0, col = 0;

    // vram
    uint8_t universal_bg_index; // $3F00
    Palette bg_palettes[4],      // $3F01 - $3F0F
            sprite_palettes[4];  // $3F11 - $3F1F
};

PPU ppu_new(Console* console);
void ppu_clock(PPU& self);
void ppu_reset(PPU& self);
bool ppu_read(PPU& self, uint16_t addr, uint8_t& data);
bool ppu_write(PPU& self, uint16_t addr, uint8_t data);

using RAM = mu::Arr<uint8_t, 0x07FF+1>;

bool ram_read(RAM& self, uint16_t addr, uint8_t& data);
bool ram_write(RAM& self, uint16_t addr, uint8_t data);

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
    mu::Vec<uint8_t> prg; // program: instructions
    mu::Vec<uint8_t> chr; // characters: sprites/graphics
};

void rom_from_ines_file(ROM& self, const mu::Str& ines_path);

inline uint16_t rom_get_mapper_number(const ROM& self) {
    return self.header.flags6.bits.lower_mapper_num | self.header.flags7.bits.upper_mapper_num << 8;
}

bool rom_read(ROM& self, uint16_t addr, uint8_t& data);
bool rom_write(ROM& self, uint16_t addr, uint8_t data);

struct Console {
    uint64_t cycles;

    CPU cpu;
    PPU ppu;
    RAM ram;
    ROM rom;

    ScreenBuf screen_buf;
    mu::Vec<Assembly> assembly;
};

void console_init(Console& self, const mu::Str& rom_path = "");
void console_reset(Console& self);
void console_clock(Console& self);

struct JoyPadInput {
    bool a;
    bool b;
    bool select;
    bool start;
    bool up;
    bool down;
    bool left;
    bool right;
};

void console_input(Console& self, JoyPadInput joypad);
