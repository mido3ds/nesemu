/* 
    Mahmoud Adas, 2019
    NES-6502 Emulator
*/
#include <string>

#include "gui/app.h"
#include "emulation/console.h"

int main(int argc, char** argv) {
    int err;

    if (argc != 2) {
        printf("Usage: NesEmu /path/to/rom\n");
        return 1;
    }

    string title = "NESEMU - " + string(argv[1]);

    ROM rom;
    err = rom.fromFile(argv[1]);
    if (err != 0) {
        return err;
    }

    Console dev;
    err = dev.init(&rom);
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
