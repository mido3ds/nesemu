#include "emulation/PPU.h"
#include "Config.h"
#include "log.h"

int PPU::init(Bus* bus) {
    if (!bus) { return 1; }
    this->bus = bus;

    return 0;
}

void PPU::clock(IRenderer* renderer) {
    // TODO
    // for now we will draw random white-black pixels
    renderer->pixel(col++, row, rand()%10 == 0? Color{0,0,0}:Color{255,255,255}, 255);
    col %= Config::sys.resolution.width;

    if (col == 0) {
        row++;
        row %= Config::sys.resolution.height;
    }
}

void PPU::reset() {
    // TODO
    cycles = 0;
}

bool PPU::read(u16_t addr, u8_t& data) {
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

bool PPU::write(u16_t addr, u8_t data) {
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
