#include <imgui.h>
#include <imgui-SFML.h>
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

int run_tests(int argc, char** argv);

int main(int argc, char** argv) {
    if (argc == 1 || argv[1] == "--help"_str_lit) {
        fmt::print(stderr, "Usage: {} </path/to/rom | test [args to Carch2] | --help>\n", file_get_base_name(argv[0]));
        return 1;
    }

    if (argv[1] == "test"_str_lit) {
        return run_tests(argc-1, argv+1);
    }

    Console dev {};
    console_init(dev);
    console_load_rom(dev, argv[1]);

    bool should_quit = false, should_pause = false,
        show_debug_wind = false, show_mem = true,
        in_debug_mode = true, do_one_instr = false;

    uint16_t memory_start = 0;

    // main window
    auto title = str_tmpf("NESEMU - {}", argv[1]);
    sf::RenderWindow main_wind(
        sf::VideoMode(Config::main_wind.w, Config::main_wind.h), 
        title.c_str(),
        sf::Style::Titlebar|sf::Style::Close
    );
    defer(main_wind.close());
    main_wind.setPosition(sf::Vector2i(0,0));

    // imgui
    auto _imgui_ini_file_path = str_format("{}/{}", folder_config(memory::tmp()), "nesemu-imgui.ini");

    ImGui::SFML::Init(main_wind);
    defer(ImGui::SFML::Shutdown(main_wind));

    ImGui::StyleColorsDark();
    ImGui::GetIO().IniFilename = _imgui_ini_file_path.c_str();

    SFMLRenderer mem_renderer;
    mem_renderer.init(&main_wind, Config::main_wind);

    SFMLImageRenderer dev_renderer;
    dev_renderer.init(&main_wind, Config::resolution);

    // debug window
    sf::RenderWindow debug_wind(
        sf::VideoMode(Config::debug_wind.w, Config::debug_wind.h),
        "Debugger", 
        sf::Style::Titlebar|sf::Style::Close
    );
    defer(debug_wind.close());
    debug_wind.setVisible(show_debug_wind);
    auto mpos = main_wind.getPosition();
    debug_wind.setPosition(sf::Vector2i(mpos.x+Config::main_wind.w+5, mpos.y));

    ImGui::SFML::Init(debug_wind);
    defer(ImGui::SFML::Shutdown(debug_wind));

    SFMLRenderer debug_renderer;
    debug_renderer.init(&debug_wind, Config::debug_wind);

    // font
    sf::Font main_font;
    if (!main_font.loadFromFile(Config::font_path)) {
        ERROR("cant load main font");
        return 1;
    }

    sf::Clock delta_clock;
    while (!should_quit && main_wind.isOpen()) {
        memory::reset_tmp();

        // time
        const auto time_elapsed = delta_clock.restart();
        ImGui::SFML::Update(main_wind, time_elapsed);

        // handle events
        sf::Event event;
        while (main_wind.pollEvent(event) || debug_wind.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(main_wind, event);

            switch (event.type) {
            case sf::Event::Closed:
                should_quit = true;
                break;
            case sf::Event::KeyPressed:
                switch (event.key.code) {
                case Config::reset:
                    console_reset(dev);
                    break;
                case Config::exit:
                    should_quit = true;
                    break;
                case Config::pause:
                    should_pause = !should_pause;

                    if (should_pause) { INFO("pause"); }
                    else       { INFO("unpause"); }
                    break;
                case Config::debug: {
                    // toggle debuger
                    show_debug_wind = !show_debug_wind;
                    debug_wind.setVisible(show_debug_wind);
                    auto mpos = main_wind.getPosition();
                    debug_wind.setPosition(sf::Vector2i(mpos.x+Config::main_wind.w+5, mpos.y));
                    break;
                }
                case Config::show_mem:
                    show_mem = !show_mem;
                    break;
                case Config::toggle_stepping:
                    in_debug_mode = !in_debug_mode;
                    break;
                case Config::next_instr:
                    do_one_instr = true;
                    break;
                }
                break;
            }
        }

        ImGui::ShowDemoWindow();

        if (!should_pause) {
            console_input(dev, JoyPadInput {
                .a       = sf::Keyboard::isKeyPressed(Config::a),
                .b       = sf::Keyboard::isKeyPressed(Config::b),
                .select  = sf::Keyboard::isKeyPressed(Config::select),
                .start   = sf::Keyboard::isKeyPressed(Config::start),
                .up      = sf::Keyboard::isKeyPressed(Config::up),
                .down    = sf::Keyboard::isKeyPressed(Config::down),
                .left    = sf::Keyboard::isKeyPressed(Config::left),
                .right   = sf::Keyboard::isKeyPressed(Config::right)
            });

            if (!in_debug_mode || do_one_instr) {
                console_clock(dev, &dev_renderer);
            }
            do_one_instr = false;
        }

        // main window
        if (show_mem) {
            const auto keyb = sf::Keyboard::isKeyPressed;
            int scrolMultiplier = keyb(sf::Keyboard::LControl) || keyb(sf::Keyboard::RControl) ? MEM_HEIGHT/2:1;

            if (keyb(Config::scroll_mem_down)) {
                if ((memory_start+scrolMultiplier+MEM_HEIGHT)*MEM_WIDTH <= 0x0800) {
                    memory_start += scrolMultiplier;
                }
            } else if (keyb(Config::scroll_mem_up)) {
                if (memory_start >= scrolMultiplier) {
                    memory_start -= scrolMultiplier;
                }
            }

            mem_renderer.clear({0,0,0},0);

            for (int i = 0; i < MEM_WIDTH; i++) {
                int x = MEM_HPADDING+53+i*30;
                mem_renderer.text(str_tmpf("{:02X}", i), x, MEM_VPADDING, 1, 1, (Font*)&main_font, {255,255,255}, 0, 0);
            }

            for (int j = 0; j < MEM_HEIGHT; j++) {
                int y = MEM_VPADDING+(j+1)*(Config::font_size+1);
                mem_renderer.text(str_tmpf("{:04X}", (memory_start+j)*MEM_WIDTH), MEM_HPADDING, y, 1, 1, (Font*)&main_font, {255,255,255}, 0, 0);

                for (int i = 0; i < MEM_WIDTH; i++) {
                    int x = MEM_HPADDING+53+i*30;
                    uint8_t data;
                    if (dev.ram.read((memory_start+j)*MEM_WIDTH+i, data)) {
                        mem_renderer.text(str_tmpf("{:02X}", data), x, y, 1, 1, (Font*)&main_font, {255,255,0}, 0, 0);
                    }
                }
            }

            mem_renderer.show();
        } else {
            dev_renderer.show();
        }

        // debug window
        if (show_debug_wind) {
            debug_renderer.clear({0,0,0},0);

            const int h = Config::font_size+1,
                w = Config::debug_wind.w/2;
            int i = 0;

            // fps
            debug_renderer.text(str_tmpf("FPS: {}", 1000 / time_elapsed.asMilliseconds()), 10,(i)*h,1,1, (Font*)&main_font,{255,0,0}, 0, 0);

            // mem
            debug_renderer.text("MEM ", 10+w,(i)*h,1,1, (Font*)&main_font,{255,255,255}, 0, 0);
            if (show_mem) { debug_renderer.text("ON", 10+w+40,(i++)*h,1,1, (Font*)&main_font,{0,255,0}, 0, 0); }
            else { debug_renderer.text("OFF", 10+w+40,(i++)*h,1,1, (Font*)&main_font,{255,0,0}, 0, 0); }
            i++;

            // regs
            Color c{255,255,0};
            const auto regs = &dev.cpu.regs;
            debug_renderer.text(str_tmpf("SP: ${:02X}", regs->sp), 10,(i)*h,1,1, (Font*)&main_font,c, 0, 0);
            debug_renderer.text(str_tmpf("A: ${:02X}", regs->a), 10+w,(i++)*h,1,1, (Font*)(Font*)&main_font,c, 0, 0);

            debug_renderer.text(str_tmpf("X: ${:02X}", regs->x), 10,(i)*h,1,1, (Font*)&main_font,c, 0, 0);
            debug_renderer.text(str_tmpf("Y: ${:02X}", regs->y), 10+w,(i++)*h,1,1, (Font*)&main_font,c, 0, 0);
            i++;

            debug_renderer.text(str_tmpf("C: {}", regs->flags.bits.c), 10,(i)*h,1,1, (Font*)&main_font,c, 0, 0);
            debug_renderer.text(str_tmpf("Z: {}", regs->flags.bits.z), 10+w*2.0/3,(i)*h,1,1, (Font*)&main_font,c, 0, 0);
            debug_renderer.text(str_tmpf("I: {}", regs->flags.bits.i), 10+w*4.0/3,(i++)*h,1,1, (Font*)&main_font,c, 0, 0);

            debug_renderer.text(str_tmpf("D: {}", regs->flags.bits.d), 10,(i)*h,1,1, (Font*)&main_font,c, 0, 0);
            debug_renderer.text(str_tmpf("B: {}", regs->flags.bits.b), 10+w*2.0/3,(i)*h,1,1, (Font*)&main_font,c, 0, 0);
            debug_renderer.text(str_tmpf("V: {}", regs->flags.bits.v), 10+w*4.0/3,(i++)*h,1,1, (Font*)&main_font,c, 0, 0);

            debug_renderer.text(str_tmpf("N: {}", regs->flags.bits.n), 10,(i)*h,1,1, (Font*)&main_font,c, 0, 0);
            debug_renderer.text(str_tmpf("PC: ${:04X}", regs->pc), 10+w*2.0/3,(i++)*h,1,1, (Font*)&main_font,{255,0,0}, 0, 0);
            i++;

            // assembly
            const int n = 18;
            int j = 1;
            for (auto& s:  disassembler_get(dev.disassembler, regs->pc, n)) {
                c = Color {.r=255, .g=255, .b=255};
                if (j++ == n+1) {
                    c.b = 0;
                }

                if (s.size() >= 8 && s[7] == '$') {
                    c.g = c.b = 0;
                }

                debug_renderer.text(s, 10,(i++)*h,1,1, (Font*)&main_font,c, 0, 0);
            }

            debug_renderer.show();
        }
    }

    return 0;
}

/*
TODO:
- refactor
	- imgui
		- only one window
		- move all UI to it
    - merge ROM with MMC0
	- no dynamic dispatch
    - function_name(self) instead of self.function_name
	-? sfml -> SDL

- complete all nestest.nes
- support illegal NES instructions
- handle reset correctly (how?)
- handle NMI correctly (how?)
- handle IRQ correctly (how?)
- perform one PPU cycle
*/
