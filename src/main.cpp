#include <string>

#include "gui/App.h"
#include "emulation/Console.h"

int testMain(int argc, char** argv);

int main(int argc, char** argv) {
    if (argc == 1) {
        printf("Usage: NesEmu /path/to/rom | test\n");
        return 1;
    }

    if (string(argv[1]) == "test") {
        return testMain(argc-1, argv+1);
    }

    const auto title = "NESEMU - " + string(argv[1]);

    Console dev;
    if (auto err = dev.init(argv[1])) {
        return err;
    }

    App app;
    if (auto err = app.init(title, &dev)) {
        return err;
    }

    return app.mainLoop();
}

/*
TODO:
- refactor
	- declare files by name in cmake
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
	- remove log.h

- complete all nestest.nes
- support illegal NES instructions
- handle reset correctly (how?)
- handle NMI correctly (how?)
- handle IRQ correctly (how?)
- perform one PPU cycle
*/
