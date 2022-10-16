#include "emulation/MMC.h"
#include "emulation/MMCs/MMC0.h"
#include "utils.h"

std::shared_ptr<MMC> MMC::fromROM(ROM rom) {
    if (MMC0::valid(rom)) {
        auto mmc0 = std::make_shared<MMC0>();
        mmc0->rom = rom;
        return mmc0;
    }

    ERROR("mapper {} not supported", rom.getMapperNumber());
    return nullptr;
}
