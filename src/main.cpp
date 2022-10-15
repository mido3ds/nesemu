#include "utils.h"
#include "gui/App.h"
#include "emulation/Console.h"

int testMain(int argc, char** argv);

int main(int argc, char** argv) {
    if (argc == 1) {
        fmt::print(stderr, "Usage: NesEmu /path/to/rom | test\n");
        return 1;
    }

    if (argv[1] == "test"_str_lit) {
        return testMain(argc-1, argv+1);
    }

    Console dev;
    if (auto err = dev.init(argv[1])) {
        return err;
    }

    App app;
    if (auto err = app.init(str_tmpf("NESEMU - {}", argv[1]).c_str(), &dev)) {
        return err;
    }

    return app.mainLoop();
}

/*
TODO:
- refactor
	- imgui
		- include
		- use with sfml
		- only one window
		- move all UI to it
	- sfml -> SDL
	- my utils
	- no smart ptrs
	- no dynamic dispatch
	- panic instead of handle errors

- complete all nestest.nes
- support illegal NES instructions
- handle reset correctly (how?)
- handle NMI correctly (how?)
- handle IRQ correctly (how?)
- perform one PPU cycle
*/
