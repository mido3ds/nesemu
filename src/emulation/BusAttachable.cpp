#include "emulation/BusAttachable.h"

void BusAttachable::reset() {}

bool BusAttachable::read(u16_t addr, u8_t& data) { return false; }

bool BusAttachable::write(u16_t addr, u8_t data) { return false; }

bool BusAttachable::ppuRead(u16_t addr, u8_t& data) { return false; }

bool BusAttachable::ppuWrite(u16_t addr, u8_t data) { return false; }