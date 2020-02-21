#pragma once

#include <thread>

#include "emulation/ROM.h"
#include "emulation/BusAttachable.h"

class MMC: public BusAttachable {
public:
    static shared_ptr<MMC> fromROM(ROM rom);

protected:
    ROM rom;
};