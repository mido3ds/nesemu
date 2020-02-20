#include "emulation/MMC.h"
#include "emulation/MMCs/MMC0.h"
#include "log.h"

shared_ptr<MMC> MMC::fromROM(ROM rom) {
    if (rom.getMapperNumber() == 0) {
        return make_shared<MMC0>();
    }

    ERROR("mapper %d not supported", rom.getMapperNumber());
    return nullptr;
}