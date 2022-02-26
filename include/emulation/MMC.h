#pragma once

#include <thread>

#include "emulation/ROM.h"
#include "emulation/ICPUBusAttachable.h"
#include "emulation/IPPUBusAttachable.h"

class MMC: public ICPUBusAttachable, public IPPUBusAttachable {
public:
    static shared_ptr<MMC> fromROM(ROM rom);

protected:
    ROM rom;
};
