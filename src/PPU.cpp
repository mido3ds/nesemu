#include "Console.h"

PPU ppu_new(Console* console) {
    return PPU {
        .console = console,
    };
}

void ppu_clock(PPU& self) {
    // TODO
    // for now we will draw random white-black pixels
    auto& buf = self.console->screen_buf;
    for (size_t y = 0; y < buf.h; y++) {
        for (size_t x = 0; x < buf.w; x++) {
            screenbuf_put(buf, x, y, rand()%10 == 0? RGBAColor{}:RGBAColor{255,255,255,255});
        }
    }
}

void ppu_reset(PPU& self) {
    // TODO
    self.cycles = 0;
}

bool ppu_read(PPU& self, uint16_t addr, uint8_t& data) {
    // TODO
    switch (addr) {
    case 0x0000: // Control
        break;
    case 0x0001: // Mask
        break;
    case 0x0002: // Status
        break;
    case 0x0003: // OAM Address
        break;
    case 0x0004: // OAM Data
        break;
    case 0x0005: // Scroll
        break;
    case 0x0006: // PPU Address
        break;
    case 0x0007: // PPU Data
        break;
    }
    return false;
}

bool ppu_write(PPU& self, uint16_t addr, uint8_t data) {
    // TODO
    switch (addr) {
    case 0x0000: // Control
        break;
    case 0x0001: // Mask
        break;
    case 0x0002: // Status
        break;
    case 0x0003: // OAM Address
        break;
    case 0x0004: // OAM Data
        break;
    case 0x0005: // Scroll
        break;
    case 0x0006: // PPU Address
        break;
    case 0x0007: // PPU Data
        break;
    }
    return false;
}
