#pragma once

#include "utils.h"

using RAM = Arr<uint8_t, 0x07FF+1>;

bool ram_read(RAM& self, uint16_t addr, uint8_t& data);
bool ram_write(RAM& self, uint16_t addr, uint8_t data);
