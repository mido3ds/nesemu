#include "Console.h"

#include <bitset>

void console_init(Console& self, const mu::Str& rom_path) {
    self = {};

    if (!rom_path.empty()) {
        rom_from_ines_file(self.rom, rom_path);
        self.assembly = bytecodes_disassemble(self.rom.prg);
    }

    self.cpu = cpu_new(&self);

    self.screen_buf = screenbuf_new(Config::resolution.w, Config::resolution.h);
}

void console_reset(Console& self) {
    // ppu_reset(self.ppu);
    cpu_reset(self.cpu);
    self.cycles = 0;
}

void console_clock(Console& self) {
    // ppu_clock(self.ppu);

    // because ppu is 3x faster than cpu
    if (self.cycles % 3 == 0) {
        cpu_clock(self.cpu);
    }

    self.cycles++;
}

// void console_input(Console& self, JoyPadInput joypad) {
//     // TODO
// }

mu::Vec<uint8_t>
console_get_tile_as_indices(
    const Console& self,
    PatternTablePointer::TableHalf table_half, int row, int col,
    mu::memory::Allocator* allocator
) {
    mu::Vec<uint8_t> out(8*8, allocator);
    for (int j = 0; j < 8; j++) {
        PatternTablePointer p {
            .bits = {
                .row_in_tile = (uint8_t) j,
                .tile_col = (uint8_t) col,
                .tile_row = (uint8_t) row,
                .table_half = table_half,
            }
        };
        p.bits.bit_plane = PatternTablePointer::BitPlane::LOWER;
        auto l = std::bitset<8>(self.rom.chr[p.word]);
        p.bits.bit_plane = PatternTablePointer::BitPlane::UPPER;
        auto h = std::bitset<8>(self.rom.chr[p.word]);

        for (int i = 0; i < 8; i++) {
            out[j*8+i] = h[7-i] << 1 | l[7-i];
        }
    }
    return out;
}

mu::Vec<RGBAColor>
console_get_tile_as_pixels(
    const Console& self,
    const mu::Vec<uint8_t>& tile_as_indices,
    PaletteType palette_type, int palette_index,
    mu::memory::Allocator* allocator
) {
    mu::Vec<RGBAColor> out(8*8, allocator);
    const Palette& palette = palette_type == PaletteType::BG ?
        self.ppu.bg_palettes[palette_index] : self.ppu.sprite_palettes[palette_index];

    for (int j = 0; j < 8; j++) {
        for (int i = 0; i < 8; i++) {
            out[j*8+i] =  color_from_palette(palette.index[tile_as_indices[j*8+i]]);
        }
    }
    return out;
}

// 2d array to 2d array
// i,j in dst
// template<typename T>
// void _copy_rect(mu::Vec<T>& dst, size_t i, size_t j, size_t dWidth, const mu::Vec<T>& src, size_t sWidth) {
//     size_t dHeight = dst.size() / dWidth;
//     size_t sHeight = src.size() / sWidth;

//     size_t maxI = std::min(dWidth, i+sWidth);
//     size_t maxJ = std::min(dHeight, j+sHeight);

//     for (size_t dR = j; dR < maxJ; dR++) {
//         for (size_t dC = i; dC < maxI; dC++) {
//             size_t sR = dR - j, sC = dC - i;
//             dst.at(dR*dWidth+dC) = src.at(sR*sWidth+sC);
//         }
//     }
// }
