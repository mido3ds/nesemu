/* 
    Mahmoud Adas, 2019
    NES-6502 Emulator
*/

// TODO: cycles++ if page crossed
/* TODO: decide whether to emulate this bug or not:
    An original 6502 has does not correctly fetch the target address 
    if the indirect vector falls on a page boundary 
    (e.g. $xxFF where xx is any value from $00 to $FF). 
    In this case fetches the LSB from $xxFF as expected but takes the MSB from $xx00. 
    This is fixed in some later chips like the 65SC02 so for compatibility 
    always ensure the indirect vector is not at the end of the page.
*/

#include <sstream>

#include "window.h"
#include "console.h"
#include "config.h"
#include "logger.h"
#include "disassembler.h"

int main(int argc, char const *argv[]) {
    if (argc == 1) {
        printf("Usage: NesEmu /path/to/rom [/path/to/config->yaml]\n");
        return 1;
    }

    Config config;
    if (argc >= 3) {
        config = Config::fromFile(argv[2]);
    }

    ostringstream ss;
    ss << "NESEMU - " << argv[1] << " [" << config.windowSize.w << "x" << config.windowSize.h << "]";

    Window w(ss.str(), &config);

    Console dev;
    if (!dev.powerOn() || !dev.setROM(argv[1])) {
        return 1;
    }

    while (w.loop(&dev)) {
        if (argc == 3) {
            config = Config::fromFile(argv[2]);
        }
        dev.reset();
    }
}
