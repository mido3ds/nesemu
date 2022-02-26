#include <string>

#include "gui/App.h"
#include "emulation/Console.h"

int main(int argc, char** argv) {
    int err;

    if (argc != 2) {
        printf("Usage: NesEmu /path/to/rom\n");
        return 1;
    }

    string title = "NESEMU - " + string(argv[1]);

    Console dev;
    if (err = dev.init(argv[1])) {
        return err;
    }

    App app;
    err = app.init(title, &dev);
    if (err != 0) {
        return err;
    }

    return app.mainLoop();
}

/*
TODO:
- refactor
	- move test here
	- imgui
		- include
		- use with sfml
		- only one window
		- move all UI to it
	- sfml -> SDL
	- mn
	- no smart ptrs
	- no dynamic dispatch
	- panic instead of handle errors
	- remove stdtype.h
	- remove log.h

- complete all nestest.nes
- support illegal NES instructions
- handle reset correctly (how?)
- handle NMI correctly (how?)
- handle IRQ correctly (how?)
- perform one PPU cycle
*/
