#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include "utils.h"
#include "emulation/Console.h"
#include "gui/SFMLRenderer.h"
#include "gui/SFMLImageRenderer.h"

#define MEM_WIDTH 16
#define MEM_HEIGHT 46
#define MEM_HPADDING 5
#define MEM_VPADDING 5

int testMain(int argc, char** argv);

int main(int argc, char** argv) {
    if (argc == 1 || argv[1] == "--help"_str_lit) {
        fmt::print(stderr, "Usage: {} </path/to/rom | test [args to Carch2] | --help>\n", file_get_base_name(argv[0]));
        return 1;
    }

    if (argv[1] == "test"_str_lit) {
        return testMain(argc-1, argv+1);
    }

    Console dev;
    dev.init(argv[1]);

    sf::RenderWindow mainWind, debugWind;
    SFMLRenderer memRenderer, debugRenderer;
    SFMLImageRenderer devRenderer;
    sf::Font mainFont;

    bool shouldQuit = false, shouldPause = false,
        showDebugWind = true, showMem = true,
        inDebugMode = true, doOneInstr = false;

    uint16_t memoryStart = 0;
    double fps = 0;

    // main window
    auto title = str_tmpf("NESEMU - {}", argv[1]);
    mainWind.create(sf::VideoMode(Config::mainWind.w, Config::mainWind.h), 
        title.c_str(), sf::Style::Titlebar|sf::Style::Close);
    mainWind.setPosition(sf::Vector2i(0,0));

    memRenderer.init(&mainWind, Config::mainWind);
    devRenderer.init(&mainWind, Config::resolution);

    // debug window
    debugWind.create(sf::VideoMode(Config::debugWind.w, Config::debugWind.h),
        "Debugger", sf::Style::Titlebar|sf::Style::Close);
    debugWind.setVisible(showDebugWind);
    auto mpos = mainWind.getPosition();
    debugWind.setPosition(sf::Vector2i(mpos.x+Config::mainWind.w+5, mpos.y));

    debugRenderer.init(&debugWind, Config::debugWind);

    // font
    if (!mainFont.loadFromFile(Config::fontPath)) {
        ERROR("cant load main font");
        return 1;
    }

    while (!shouldQuit && mainWind.isOpen()) {
        memory::reset_tmp();

        // fps
        auto t1 = std::chrono::high_resolution_clock::now();
        defer({
            auto t2 = std::chrono::high_resolution_clock::now();
            auto duration = duration_cast<std::chrono::milliseconds>(t2 - t1).count();
            fps = 1000.0/duration;
        });

        // handle events
        sf::Event event;
        while (mainWind.pollEvent(event) || debugWind.pollEvent(event)) {
            switch (event.type) {
            case sf::Event::Closed:
                shouldQuit = true;
                break;
            case sf::Event::KeyPressed:
                switch (event.key.code) {
                case Config::reset:
                    dev.reset();
                    break;
                case Config::exit:
                    shouldQuit = true;
                    break;
                case Config::pause:
                    shouldPause = !shouldPause;

                    if (shouldPause) { INFO("pause"); }
                    else       { INFO("unpause"); }
                    break;
                case Config::debug: {
                    // toggle debuger
                    showDebugWind = !showDebugWind;
                    debugWind.setVisible(showDebugWind);
                    auto mpos = mainWind.getPosition();
                    debugWind.setPosition(sf::Vector2i(mpos.x+Config::mainWind.w+5, mpos.y));
                    break;
                }
                case Config::showMem:
                    showMem = !showMem;
                    break;
                case Config::toggleStepping:
                    inDebugMode = !inDebugMode;
                    break;
                case Config::nextInstr:
                    doOneInstr = true;
                    break;
                }
                break;
            }
        }

        if (!shouldPause) {
            dev.input(JoyPadInput {
                .a       = sf::Keyboard::isKeyPressed(Config::a),
                .b       = sf::Keyboard::isKeyPressed(Config::b),
                .select  = sf::Keyboard::isKeyPressed(Config::select),
                .start   = sf::Keyboard::isKeyPressed(Config::start),
                .up      = sf::Keyboard::isKeyPressed(Config::up),
                .down    = sf::Keyboard::isKeyPressed(Config::down),
                .left    = sf::Keyboard::isKeyPressed(Config::left),
                .right   = sf::Keyboard::isKeyPressed(Config::right)
            });

            if (!inDebugMode || doOneInstr) {
                dev.clock(&devRenderer);
            }
            doOneInstr = false;
        }

        // main window
        if (showMem) {
            const auto keyb = sf::Keyboard::isKeyPressed;
            int scrolMultiplier = keyb(sf::Keyboard::LControl) || keyb(sf::Keyboard::RControl) ? MEM_HEIGHT/2:1;

            if (keyb(Config::scrollMemDown)) {
                if ((memoryStart+scrolMultiplier+MEM_HEIGHT)*MEM_WIDTH <= 0x0800) {
                    memoryStart += scrolMultiplier;
                }
            } else if (keyb(Config::scrollMemUp)) {
                if (memoryStart >= scrolMultiplier) {
                    memoryStart -= scrolMultiplier;
                }
            }

            memRenderer.clear({0,0,0},0);

            for (int i = 0; i < MEM_WIDTH; i++) {
                int x = MEM_HPADDING+53+i*30;
                memRenderer.text(str_tmpf("{:02X}", i), x, MEM_VPADDING, 1, 1, (Font*)&mainFont, {255,255,255}, 0, 0);
            }

            for (int j = 0; j < MEM_HEIGHT; j++) {
                int y = MEM_VPADDING+(j+1)*(Config::fontSize+1);
                memRenderer.text(str_tmpf("{:04X}", (memoryStart+j)*MEM_WIDTH), MEM_HPADDING, y, 1, 1, (Font*)&mainFont, {255,255,255}, 0, 0);

                for (int i = 0; i < MEM_WIDTH; i++) {
                    int x = MEM_HPADDING+53+i*30;
                    uint8_t data;
                    if (dev.ram.read((memoryStart+j)*MEM_WIDTH+i, data)) {
                        memRenderer.text(str_tmpf("{:02X}", data), x, y, 1, 1, (Font*)&mainFont, {255,255,0}, 0, 0);
                    }
                }
            }

            memRenderer.show();
        } else {
            devRenderer.show();
        }

        // debug window
        if (showDebugWind) {
            debugRenderer.clear({0,0,0},0);

            const int h = Config::fontSize+1,
                w = Config::debugWind.w/2;
            int i = 0;

            // fps
            debugRenderer.text(str_tmpf("FPS: {}", int(fps)), 10,(i)*h,1,1, (Font*)&mainFont,{255,0,0}, 0, 0);

            // mem
            debugRenderer.text("MEM ", 10+w,(i)*h,1,1, (Font*)&mainFont,{255,255,255}, 0, 0);
            if (showMem) { debugRenderer.text("ON", 10+w+40,(i++)*h,1,1, (Font*)&mainFont,{0,255,0}, 0, 0); }
            else { debugRenderer.text("OFF", 10+w+40,(i++)*h,1,1, (Font*)&mainFont,{255,0,0}, 0, 0); }
            i++;

            // regs
            Color c{255,255,0};
            const auto regs = &dev.cpu.regs;
            debugRenderer.text(str_tmpf("SP: ${:02X}", regs->sp), 10,(i)*h,1,1, (Font*)&mainFont,c, 0, 0);
            debugRenderer.text(str_tmpf("A: ${:02X}", regs->a), 10+w,(i++)*h,1,1, (Font*)(Font*)&mainFont,c, 0, 0);

            debugRenderer.text(str_tmpf("X: ${:02X}", regs->x), 10,(i)*h,1,1, (Font*)&mainFont,c, 0, 0);
            debugRenderer.text(str_tmpf("Y: ${:02X}", regs->y), 10+w,(i++)*h,1,1, (Font*)&mainFont,c, 0, 0);
            i++;

            debugRenderer.text(str_tmpf("C: {}", regs->flags.bits.c), 10,(i)*h,1,1, (Font*)&mainFont,c, 0, 0);
            debugRenderer.text(str_tmpf("Z: {}", regs->flags.bits.z), 10+w*2.0/3,(i)*h,1,1, (Font*)&mainFont,c, 0, 0);
            debugRenderer.text(str_tmpf("I: {}", regs->flags.bits.i), 10+w*4.0/3,(i++)*h,1,1, (Font*)&mainFont,c, 0, 0);

            debugRenderer.text(str_tmpf("D: {}", regs->flags.bits.d), 10,(i)*h,1,1, (Font*)&mainFont,c, 0, 0);
            debugRenderer.text(str_tmpf("B: {}", regs->flags.bits.b), 10+w*2.0/3,(i)*h,1,1, (Font*)&mainFont,c, 0, 0);
            debugRenderer.text(str_tmpf("V: {}", regs->flags.bits.v), 10+w*4.0/3,(i++)*h,1,1, (Font*)&mainFont,c, 0, 0);

            debugRenderer.text(str_tmpf("N: {}", regs->flags.bits.n), 10,(i)*h,1,1, (Font*)&mainFont,c, 0, 0);
            debugRenderer.text(str_tmpf("PC: ${:04X}", regs->pc), 10+w*2.0/3,(i++)*h,1,1, (Font*)&mainFont,{255,0,0}, 0, 0);
            i++;

            // assembly
            const int n = 18;
            int j = 1;
            for (auto& s: dev.disassembler.get(regs->pc, n)) {
                c = Color {.r=255, .g=255, .b=255};
                if (j++ == n+1) {
                    c.b = 0;
                }

                if (s.size() >= 8 && s[7] == '$') {
                    c.g = c.b = 0;
                }

                debugRenderer.text(s, 10,(i++)*h,1,1, (Font*)&mainFont,c, 0, 0);
            }

            debugRenderer.show();
        }
    }

    return 0;
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
	- no dynamic dispatch

- complete all nestest.nes
- support illegal NES instructions
- handle reset correctly (how?)
- handle NMI correctly (how?)
- handle IRQ correctly (how?)
- perform one PPU cycle
*/
