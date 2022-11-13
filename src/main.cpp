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
    console_init(dev, argv[1]);

    // window
    auto title = str_format("NESEMU - {}", argv[1]);
    sf::RenderWindow main_wind(
        sf::VideoMode(Config::main_wind.w, Config::main_wind.h), 
        title.c_str(),
        sf::Style::Titlebar|sf::Style::Close
    );
    main_wind.setPosition(sf::Vector2i(0,0));

    SFMLImageRenderer dev_renderer;
    dev_renderer.init(&main_wind, Config::resolution);

    // imgui
    auto _imgui_ini_file_path = str_format("{}/{}", folder_config(memory::tmp()), "nesemu-imgui.ini");

    ImGui::SFML::Init(main_wind);
    defer(ImGui::SFML::Shutdown(main_wind));

    ImGui::StyleColorsDark();
    ImGui::GetIO().IniFilename = _imgui_ini_file_path.c_str();

    bool should_pause = true;
    bool do_one_instr = false;
    sf::Clock delta_clock;

    while (main_wind.isOpen()) {
        memory::reset_tmp();
        main_wind.clear();

        // handle events
        sf::Event event;
        while (main_wind.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(main_wind, event);

            switch (event.type) {
            case sf::Event::Closed:
                main_wind.close();
                break;
            case sf::Event::KeyPressed:
                switch (event.key.code) {
                case Config::exit:
                    main_wind.close();
                    break;
                }
                break;
            }
        }

        // time
        const auto elapsed_time = delta_clock.restart();
        ImGui::SFML::Update(main_wind, elapsed_time);

        if (!should_pause || do_one_instr) {
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

            console_clock(dev, &dev_renderer);
        }
        do_one_instr = !should_pause;

        if (ImGui::Begin("Memory")) {
            if (ImGui::BeginTabBar("memory_tab_bar")) {
                constexpr uint32_t width = 16;
                constexpr auto TABLE_FLAGS = ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter;

                if (ImGui::BeginTabItem("RAM")) {
                    const uint32_t height = dev.ram.size() / width;
                    if (ImGui::BeginTable("ram_table", width+1, TABLE_FLAGS))
                    {
                        ImGui::TableSetupScrollFreeze(1, 1);
                        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_NoHeaderLabel);
                        for (int i = 0; i < width; i++) {
                            ImGui::TableSetupColumn(str_tmpf("{:02X}", i).c_str());
                        }
                        ImGui::TableHeadersRow();

                        ImGuiListClipper clipper(height);
                        while (clipper.Step()) {
                            for (int j = clipper.DisplayStart; j < clipper.DisplayEnd; j++) {
                                ImGui::TableNextRow();
                                ImGui::TableSetColumnIndex(0);
                                ImGui::Text(str_tmpf("{:04X}", j).c_str());

                                for (int i = 0; i < width; i++) {
                                    ImGui::TableSetColumnIndex(i+1);
                                    ImGui::TextColored(ImVec4 {1.0f, 1.0f, 0, 1.0f}, str_tmpf("{:02X}", dev.ram[j*width+i]).c_str());
                                }
                            }
                        }
                        clipper.End();

                        ImGui::EndTable();
                    }

                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("PRG")) {
                    const auto& prg = dev.rom.prg;
                    const uint32_t height = prg.size() / width;
                    if (ImGui::BeginTable("ram_table", width+1, TABLE_FLAGS))
                    {
                        ImGui::TableSetupScrollFreeze(1, 1);
                        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_NoHeaderLabel);
                        for (int i = 0; i < width; i++) {
                            ImGui::TableSetupColumn(str_tmpf("{:02X}", i).c_str());
                        }
                        ImGui::TableHeadersRow();

                        ImGuiListClipper clipper(height);
                        while (clipper.Step()) {
                            for (int j = clipper.DisplayStart; j < clipper.DisplayEnd; j++) {
                                ImGui::TableNextRow();
                                ImGui::TableSetColumnIndex(0);
                                ImGui::Text(str_tmpf("{:04X}", j).c_str());

                                for (int i = 0; i < width; i++) {
                                    ImGui::TableSetColumnIndex(i+1);
                                    ImGui::TextColored(ImVec4 {1.0f, 1.0f, 0, 1.0f}, str_tmpf("{:02X}", prg[j*width+i]).c_str());
                                }
                            }
                        }
                        clipper.End();

                        ImGui::EndTable();
                    }

                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("CHR")) {
                    const auto& chr = dev.rom.chr;
                    const uint32_t height = chr.size() / width;
                    if (ImGui::BeginTable("ram_table", width+1, TABLE_FLAGS))
                    {
                        ImGui::TableSetupScrollFreeze(1, 1);
                        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_NoHeaderLabel);
                        for (int i = 0; i < width; i++) {
                            ImGui::TableSetupColumn(str_tmpf("{:02X}", i).c_str());
                        }
                        ImGui::TableHeadersRow();

                        ImGuiListClipper clipper(height);
                        while (clipper.Step()) {
                            for (int j = clipper.DisplayStart; j < clipper.DisplayEnd; j++) {
                                ImGui::TableNextRow();
                                ImGui::TableSetColumnIndex(0);
                                ImGui::Text(str_tmpf("{:04X}", j).c_str());

                                for (int i = 0; i < width; i++) {
                                    ImGui::TableSetColumnIndex(i+1);
                                    ImGui::TextColored(ImVec4 {1.0f, 1.0f, 0, 1.0f}, str_tmpf("{:02X}", chr[j*width+i]).c_str());
                                }
                            }
                        }
                        clipper.End();

                        ImGui::EndTable();
                    }

                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }
        }
        ImGui::End();

        if (ImGui::Begin("Debug")) {
            // fps
            auto time_millis = elapsed_time.asMilliseconds();
            ImGui::Text(str_tmpf("FPS: {}", time_millis == 0? 0 : 1000 / time_millis).c_str());

            if (should_pause) {
                if (ImGui::Button("Resume")) {
                    should_pause = false;
                }
            } else {
                if (ImGui::Button("Pause")) {
                    should_pause = true;
                }
            }

            ImGui::SameLine();

            ImGui::BeginDisabled(!should_pause);
            do_one_instr = ImGui::Button("Step");
            ImGui::EndDisabled();

            ImGui::SameLine();

            if (ImGui::Button("Reset")) {
                console_reset(dev);
            }

            if (ImGui::TreeNodeEx("Regs", ImGuiTreeNodeFlags_DefaultOpen)) {
                const auto& regs = dev.cpu.regs;
                ImGui::Text(str_tmpf("PC: ${:04X}", regs.pc).c_str());

                ImGui::Text(str_tmpf("SP: ${:02X}", regs.sp).c_str());
                ImGui::Text(str_tmpf("A: ${:02X}", regs.a).c_str());

                ImGui::Text(str_tmpf("X: ${:02X}", regs.x).c_str());
                ImGui::Text(str_tmpf("Y: ${:02X}", regs.y).c_str());

                ImGui::Text(str_tmpf("C: {}", regs.flags.bits.c).c_str());
                ImGui::Text(str_tmpf("Z: {}", regs.flags.bits.z).c_str());
                ImGui::Text(str_tmpf("I: {}", regs.flags.bits.i).c_str());

                ImGui::Text(str_tmpf("D: {}", regs.flags.bits.d).c_str());
                ImGui::Text(str_tmpf("B: {}", regs.flags.bits.b).c_str());
                ImGui::Text(str_tmpf("V: {}", regs.flags.bits.v).c_str());

                ImGui::Text(str_tmpf("N: {}", regs.flags.bits.n).c_str());

                ImGui::TreePop();
            }

            if (ImGui::TreeNodeEx("Instructions", ImGuiTreeNodeFlags_DefaultOpen)) {
                const int n = 18;
                const auto assembly_lines = disassembler_get(dev.disassembler, dev.cpu.regs.pc, n);
                for (size_t i = 0; i < assembly_lines.size(); i++) {
                    if (i == n+1) {
                        ImGui::TextColored(ImVec4 {1.0f, 0, 0, 1.0f}, assembly_lines[i].c_str());
                    } else {
                        ImGui::Text(assembly_lines[i].c_str());
                    }
                }

                ImGui::TreePop();
            }
        }
        ImGui::End();

        ImGui::ShowDemoWindow();

        dev_renderer.show();
        ImGui::SFML::Render(main_wind);

        main_wind.display();
    }

    return 0;
}

/*
TODO:
- refactor
    - function_name(self) instead of self.function_name
	-? no dynamic dispatch in renderers
	-? sfml -> SDL

- complete all nestest.nes
- support illegal NES instructions
- handle reset correctly (how?)
- handle NMI correctly (how?)
- handle IRQ correctly (how?)
- perform one PPU cycle
*/
