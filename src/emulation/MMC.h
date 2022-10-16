#pragma once

#include <thread>
#include <memory>

#include "emulation/ROM.h"
#include "emulation/ICPUBusAttachable.h"
#include "emulation/IPPUBusAttachable.h"

struct MMC: public ICPUBusAttachable, public IPPUBusAttachable {
    ROM rom;

    static std::shared_ptr<MMC> fromROM(ROM rom);
};
