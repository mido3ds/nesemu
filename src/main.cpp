#include <bitset>
#include <imgui.h>
#include <imgui-SFML.h>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include "utils.h"
#include "Console.h"
#include "Image.h"

#define MEM_WIDTH 16
#define MEM_HEIGHT 46
#define MEM_HPADDING 5
#define MEM_VPADDING 5

int run_tests(int argc, char** argv);

namespace MyImGui {
    template<typename T>
    void EnumsCombo(const char* label, T* p_enum, const std::initializer_list<std::pair<T, const char*>>& enums) {
            int var_i = -1;
            const char* preview = "- Invalid Value -";
            for (const auto& [type, type_str] : enums) {
                    var_i++;
                    if (type == *p_enum) {
                            preview = type_str;
                            break;
                    }
            }

            if (ImGui::BeginCombo(label, preview)) {
                    for (const auto& [type, type_str] : enums) {
                            if (ImGui::Selectable(type_str,  type == *p_enum)) {
                                    *p_enum = type;
                            }
                    }

                    ImGui::EndCombo();
            }
    }
}

int main(int argc, char** argv) {
    if (argc == 1 || argv[1] == "--help"_str_lit) {
        fmt::print(stderr, "Usage: {} </path/to/rom | test [args to Catch2] | --help>\n", file_get_base_name(argv[0]));
        return 1;
    }

    if (argv[1] == "test"_str_lit) {
        return run_tests(argc-1, argv+1);
    }

    Console dev {};
    console_init(dev, argv[1]);
    const auto assembly = rom_disassemble(dev.rom);

    // window
    auto title = str_format("NESEMU - {}", argv[1]);
    sf::RenderWindow main_wind(
        sf::VideoMode(Config::main_wind.w, Config::main_wind.h),
        title.c_str(),
        sf::Style::Titlebar|sf::Style::Close
    );
    main_wind.setPosition(sf::Vector2i(0,0));

    Image image = image_new();

    // imgui
    auto _imgui_ini_file_path = str_format("{}/{}", folder_config(memory::tmp()), "nesemu-imgui.ini");

    ImGui::SFML::Init(main_wind);
    defer(ImGui::SFML::Shutdown(main_wind));

    ImGui::StyleColorsDark();
    ImGui::GetIO().IniFilename = _imgui_ini_file_path.c_str();

    bool should_pause = true;
    bool do_one_instr = false;
    sf::Clock delta_clock;

    sf::Texture tile_texture;
    if (tile_texture.create(8, 8) == false) {
        panic("failed to create texture");
    }
    sf::Sprite tile_sprite(tile_texture);

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

            console_clock(dev, &image);
        }
        do_one_instr = !should_pause;

        if (ImGui::Begin("Memory")) {
            if (ImGui::BeginTabBar("memory_tab_bar")) {
                constexpr uint32_t width = 16;
                constexpr auto TABLE_FLAGS = ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter;

                if (ImGui::BeginTabItem("RAM")) {
                    const uint32_t height = dev.ram.size() / width;
                    if (ImGui::BeginTable("ram_table", width+1, TABLE_FLAGS)) {
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
                    if (ImGui::BeginTable("ram_table", width+1, TABLE_FLAGS)) {
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
                    if (ImGui::BeginTable("ram_table", width+1, TABLE_FLAGS)) {
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

        if (ImGui::Begin("Viewer")) {
            if (ImGui::BeginTabBar("viewer_tab_bar")) {
                if (ImGui::BeginTabItem("Pattern Table")) {
                    // get tile
                    static int row = 0, col = 0;
                    static auto table_half = PatternTablePointer::TableHalf::LEFT;
                    PatternTablePointer p {
                        bits: {
                            row_in_tile: 0,
                            bit_plane: PatternTablePointer::BitPlane::LOWER,
                            tile_col: col,
                            tile_row: row,
                            table_half: table_half,
                        }
                    };
                    uint8_t tile[8][8] = {0};
                    for (int j = 0; j < 8; j++) {
                        p.bits.row_in_tile = j;
                        p.bits.bit_plane = PatternTablePointer::BitPlane::LOWER;
                        auto l = std::bitset<8>(dev.rom.chr[p.word]);
                        p.bits.bit_plane = PatternTablePointer::BitPlane::UPPER;
                        auto h = std::bitset<8>(dev.rom.chr[p.word]);

                        for (int i = 0; i < 8; i++) {
                            tile[j][i] = h[7-i] << 1 | l[7-i];
                        }
                    }

                    // render tile
                    RGBAColor tile_monochrome[8][8] = {0};
                    for (int j = 0; j < 8; j++) {
                        for (int i = 0; i< 8; i++) {
                            RGBAColor color {};
                            switch (tile[j][i]) {
                            case 3: color = {255, 255, 255, 255}; break;
                            case 2: color = {150, 150, 150, 255}; break;
                            case 1: color = {50, 50, 50, 255}; break;
                            default: color = {255, 0, 255, 255}; break;
                            }
                            tile_monochrome[j][i] = color;
                        }
                    }
                    tile_texture.update((const sf::Uint8*)tile_monochrome);
                    tile_sprite.setScale(15, 15);
                    ImGui::Image(tile_sprite);

                    ImGui::SameLine();

                    // tile as a table
                    constexpr auto TABLE_FLAGS = ImGuiTableFlags_NoHostExtendX | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersOuter;
                    if (ImGui::BeginTable("pattern_table", 8, TABLE_FLAGS)) {
                        for (int j = 0; j < 8; j++) {
                            ImGui::TableNextRow();

                            for (int i = 0; i < 8; i++) {
                                ImGui::TableSetColumnIndex(i);

                                const int val = tile[j][i];
                                if (val == 0) {
                                    ImGui::Text(".");
                                } else {
                                    ImVec4 color;
                                    switch (val) {
                                    case 3: color = {1.0f, 1.0f, 1.0f, 1.0f}; break;
                                    case 2: color = {67.0f/255.0f, 145.0f/255.0f, 170.0f/255.0f, 1.0f}; break;
                                    case 1: color = {131.0f/255.0f, 162.0f/255.0f, 173.0f/255.0f, 1.0f}; break;
                                    default: unreachable();
                                    }
                                    ImGui::TextColored(color, str_tmpf("{}", val).c_str());
                                }
                            }
                        }

                        ImGui::EndTable();
                    }

                    ImGui::SliderInt("Col", &col, 0, 15, "%d", ImGuiSliderFlags_AlwaysClamp);
                    ImGui::SliderInt("Row", &row, 0, 15, "%d", ImGuiSliderFlags_AlwaysClamp);
                    MyImGui::EnumsCombo("Half", &table_half, {
                        {PatternTablePointer::TableHalf::LEFT, "LEFT"},
                        {PatternTablePointer::TableHalf::RIGHT, "RIGHT"},
                    });

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
                constexpr auto TABLE_FLAGS = ImGuiTableFlags_ScrollY | ImGuiTableFlags_ScrollX | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersInnerV;
                if (ImGui::BeginTable("assembly_table", 2, TABLE_FLAGS)) {
                    ImGui::TableSetupScrollFreeze(0, 1);
                    ImGui::TableSetupColumn("Adr");
                    ImGui::TableSetupColumn("Inst");
                    ImGui::TableHeadersRow();

                    ImGuiListClipper clipper(assembly.size());
                    while (clipper.Step()) {
                        for (int j = clipper.DisplayStart; j < clipper.DisplayEnd; j++) {
                            ImGui::TableNextRow();

                            if (assembly[j].adr == dev.cpu.regs.pc) {
                                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImVec4{0.3f, 0.3f, 0.7f, 0.65f}));
                            }

                            if (ImGui::TableSetColumnIndex(0)) {
                                ImGui::Text(str_tmpf("${:04X}", assembly[j].adr).c_str());
                            }
                            if (ImGui::TableSetColumnIndex(1)) {
                                ImGui::Text(assembly[j].instr.c_str());
                            }
                        }
                    }
                    clipper.End();

                    ImGui::EndTable();
                }

                ImGui::TreePop();
            }
        }
        ImGui::End();

        ImGui::ShowDemoWindow();

        image_show(image, main_wind);
        ImGui::SFML::Render(main_wind);

        main_wind.display();
    }

    return 0;
}

/*
TODO:
- pattern table
    - show current color pallete
    - render with color (select pallete)
- nametable
    - render one frame without color
    - render one frame with color from one pallete
    - select pallete from attribute table
- sprites
    - render one sprite without color
    - select color from pallete
- complete all nestest.nes
- support illegal NES instructions
- handle reset correctly (how?)
- handle NMI correctly (how?)
- handle IRQ correctly (how?)
- perform one PPU cycle
*/
