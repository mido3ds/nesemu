#include "catch2/catch.hpp"

#include "console.h"

TEST_CASE("hello-test") {
    Console dev;
    REQUIRE(dev.init() == 0);
    REQUIRE(dev.instrucSet[0x69].name == "ADC");
    REQUIRE(dev.instrucSet[0x69].mode == AddressMode::Immediate);
    REQUIRE(dev.instrucSet[0x69].cycles == 2);
}