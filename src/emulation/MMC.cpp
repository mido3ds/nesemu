#include "emulation/MMC.h"
#include "emulation/MMCs/MMC0.h"
#include "log.h"

shared_ptr<MMC> MMC::fromROM(ROM rom) {
    if (MMC0::valid(rom)) {
        auto mmc0 = make_shared<MMC0>();
        mmc0->rom = rom;
        return mmc0;
    }

    ERROR("mapper %d not supported", rom.getMapperNumber());
    return nullptr;
}