/* 
    Mahmoud Adas, 2019
    NES-6502 Emulator
*/
#include <string>

#include "app.h"
#include "console.h"

int main(int argc, char** argv) {
    int err;

    if (argc != 2) {
        printf("Usage: NesEmu /path/to/rom\n");
        return 1;
    }

    string title = "NESEMU - " + string(argv[1]);

    Console dev;
    err = dev.init();
    if (err != 0) {
        return err;
    }

    err = dev.powerOn();
    if (err != 0) {
        return err;
    }

    err = dev.loadROM(ROM::fromFile(argv[1]));
    if (err != 0) {
        return err;
    }
    
    App app;
    err = app.init(title, &dev);
    if (err != 0) {
        return err;
    }
    
    return app.mainLoop();
}
