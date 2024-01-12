#include <bitset>
#include <imgui.h>
#include <imgui-SFML.h>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include "Console.h"
#include "Image.h"

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

    bool InputByte(const char* label, uint8_t* v, uint8_t step = 1, uint8_t step_fast = 100, ImGuiInputTextFlags flags = ImGuiInputTextFlags_CharsHexadecimal) {
        // Hexadecimal input provided as a convenience but the flag name is awkward. Typically you'd use InputText() to parse your own data, if you want to handle prefixes.
        const char* format = (flags & ImGuiInputTextFlags_CharsHexadecimal) ? "0x%02X" : "%d";
        return ImGui::InputScalar(label, ImGuiDataType_U8, (void*)v, (void*)(step > 0 ? &step : NULL), (void*)(step_fast > 0 ? &step_fast : NULL), format, flags);
    }
}

struct World {
    mu::Str rom_path;
    sf::RenderWindow main_wind;
    mu::Str imgui_ini_file_path;

    bool should_pause;
    bool do_one_instr;

    sf::Clock delta_clock;
    sf::Time frame_time;

    Console console;
    mu::Vec<Assembly> assembly;

    Image console_image;
    sf::Texture tile_texture;
    sf::Sprite tile_sprite;
};

namespace sys {
    void window_init(World& world) {
        auto title = mu::str_format("NESEMU - {}", world.rom_path);
        world.main_wind.create(
            sf::VideoMode(Config::main_wind.w * Config::view_scale, Config::main_wind.h * Config::view_scale),
            title.c_str(),
            sf::Style::Titlebar|sf::Style::Close
        );
        world.main_wind.setPosition(sf::Vector2i(0,0));
    }

    void imgui_init(World& world) {
        ImGui::SFML::Init(world.main_wind);

        ImGui::StyleColorsDark();

        world.imgui_ini_file_path = mu::str_format("{}/{}", mu::folder_config(mu::memory::tmp()), "nesemu-imgui.ini");
        ImGui::GetIO().IniFilename = world.imgui_ini_file_path.c_str();

        ImGui::GetIO().FontGlobalScale = Config::view_scale;
    }

    void imgui_free(World& world) {
        ImGui::SFML::Shutdown(world.main_wind);
    }

    void imgui_rendering_begin(World& world) {
        ImGui::SFML::Update(world.main_wind, world.frame_time);
    }

    void imgui_rendering_end(World& world) {
        ImGui::SFML::Render(world.main_wind);
    }

    void imgui_memory_window(World& world) {
        if (ImGui::Begin("Memory")) {
            if (ImGui::BeginTabBar("memory_tab_bar")) {
                constexpr uint32_t width = 16;
                constexpr auto TABLE_FLAGS = ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter;

                if (ImGui::BeginTabItem("RAM")) {
                    const uint32_t height = world.console.ram.size() / width;
                    if (ImGui::BeginTable("ram_table", width+1, TABLE_FLAGS)) {
                        ImGui::TableSetupScrollFreeze(1, 1);
                        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_NoHeaderLabel);
                        for (int i = 0; i < width; i++) {
                            ImGui::TableSetupColumn(mu::str_tmpf("{:02X}", i).c_str());
                        }
                        ImGui::TableHeadersRow();

                        ImGuiListClipper clipper(height);
                        while (clipper.Step()) {
                            for (int j = clipper.DisplayStart; j < clipper.DisplayEnd; j++) {
                                ImGui::TableNextRow();
                                ImGui::TableSetColumnIndex(0);
                                ImGui::Text(mu::str_tmpf("{:04X}", j).c_str());

                                for (int i = 0; i < width; i++) {
                                    ImGui::TableSetColumnIndex(i+1);
                                    ImGui::TextColored(ImVec4 {1.0f, 1.0f, 0, 1.0f}, mu::str_tmpf("{:02X}", world.console.ram[j*width+i]).c_str());
                                }
                            }
                        }
                        clipper.End();

                        ImGui::EndTable();
                    }

                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("PRG")) {
                    const auto& prg = world.console.rom.prg;
                    const uint32_t height = prg.size() / width;
                    if (ImGui::BeginTable("ram_table", width+1, TABLE_FLAGS)) {
                        ImGui::TableSetupScrollFreeze(1, 1);
                        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_NoHeaderLabel);
                        for (int i = 0; i < width; i++) {
                            ImGui::TableSetupColumn(mu::str_tmpf("{:02X}", i).c_str());
                        }
                        ImGui::TableHeadersRow();

                        ImGuiListClipper clipper(height);
                        while (clipper.Step()) {
                            for (int j = clipper.DisplayStart; j < clipper.DisplayEnd; j++) {
                                ImGui::TableNextRow();
                                ImGui::TableSetColumnIndex(0);
                                ImGui::Text(mu::str_tmpf("{:04X}", j).c_str());

                                for (int i = 0; i < width; i++) {
                                    ImGui::TableSetColumnIndex(i+1);
                                    ImGui::TextColored(ImVec4 {1.0f, 1.0f, 0, 1.0f}, mu::str_tmpf("{:02X}", prg[j*width+i]).c_str());
                                }
                            }
                        }
                        clipper.End();

                        ImGui::EndTable();
                    }

                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("CHR")) {
                    const auto& chr = world.console.rom.chr;
                    const uint32_t height = chr.size() / width;
                    if (ImGui::BeginTable("ram_table", width+1, TABLE_FLAGS)) {
                        ImGui::TableSetupScrollFreeze(1, 1);
                        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_NoHeaderLabel);
                        for (int i = 0; i < width; i++) {
                            ImGui::TableSetupColumn(mu::str_tmpf("{:02X}", i).c_str());
                        }
                        ImGui::TableHeadersRow();

                        ImGuiListClipper clipper(height);
                        while (clipper.Step()) {
                            for (int j = clipper.DisplayStart; j < clipper.DisplayEnd; j++) {
                                ImGui::TableNextRow();
                                ImGui::TableSetColumnIndex(0);
                                ImGui::Text(mu::str_tmpf("{:04X}", j).c_str());

                                for (int i = 0; i < width; i++) {
                                    ImGui::TableSetColumnIndex(i+1);
                                    ImGui::TextColored(ImVec4 {1.0f, 1.0f, 0, 1.0f}, mu::str_tmpf("{:02X}", chr[j*width+i]).c_str());
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
    }

    void imgui_viewer_window(World& world) {
        if (ImGui::Begin("Viewer")) {
            if (ImGui::BeginTabBar("viewer_tab_bar")) {
                if (ImGui::BeginTabItem("Pattern Table")) {
                    // get tile
                    static int row = 0, col = 0, palette_index = 0 /*0-4*/;
                    enum class ColorType { BG, SPRITE };
                    static ColorType color_type = ColorType::BG;
                    static auto table_half = PatternTablePointer::TableHalf::LEFT;
                    PatternTablePointer p {
                        .bits = {
                            .row_in_tile = 0,
                            .bit_plane = PatternTablePointer::BitPlane::LOWER,
                            .tile_col = (uint8_t) col,
                            .tile_row = (uint8_t) row,
                            .table_half = table_half,
                        }
                    };
                    uint8_t tile[8][8] = {0};
                    for (int j = 0; j < 8; j++) {
                        p.bits.row_in_tile = j;
                        p.bits.bit_plane = PatternTablePointer::BitPlane::LOWER;
                        auto l = std::bitset<8>(world.console.rom.chr[p.word]);
                        p.bits.bit_plane = PatternTablePointer::BitPlane::UPPER;
                        auto h = std::bitset<8>(world.console.rom.chr[p.word]);

                        for (int i = 0; i < 8; i++) {
                            tile[j][i] = palette_index << 2 | h[7-i] << 1 | l[7-i];
                        }
                    }

                    // render tile
                    RGBAColor tile_monochrome[8][8] = {0};
                    for (int j = 0; j < 8; j++) {
                        for (int i = 0; i< 8; i++) {
                            tile_monochrome[j][i] = color_type == ColorType::BG ?
                                ppu_get_bg_color(world.console.ppu, tile[j][i]) : ppu_get_sprite_color(world.console.ppu, tile[j][i]);
                        }
                    }
                    world.tile_texture.update((const sf::Uint8*)tile_monochrome);
                    world.tile_sprite.setScale(15 * Config::view_scale, 15 * Config::view_scale);
                    ImGui::Image(world.tile_sprite);

                    ImGui::SameLine();

                    // tile as a table
                    constexpr auto TABLE_FLAGS = ImGuiTableFlags_NoHostExtendX | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersOuter;
                    if (ImGui::BeginTable("pattern_table", 8, TABLE_FLAGS)) {
                        for (int j = 0; j < 8; j++) {
                            ImGui::TableNextRow();

                            for (int i = 0; i < 8; i++) {
                                ImGui::TableSetColumnIndex(i);

                                const int val = tile[j][i] & 0b11;
                                if (val == 0) {
                                    ImGui::Text(".");
                                } else {
                                    ImVec4 color;
                                    switch (val) {
                                    case 3: color = {1.0f, 1.0f, 1.0f, 1.0f}; break;
                                    case 2: color = {67.0f/255.0f, 145.0f/255.0f, 170.0f/255.0f, 1.0f}; break;
                                    case 1: color = {131.0f/255.0f, 162.0f/255.0f, 173.0f/255.0f, 1.0f}; break;
                                    default: mu_unreachable();
                                    }
                                    ImGui::TextColored(color, mu::str_tmpf("{}", val).c_str());
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
                    ImGui::SliderInt("Palette Index", &palette_index, 0, 3, "%d", ImGuiSliderFlags_AlwaysClamp);
                    MyImGui::EnumsCombo("Color Type", &color_type, {
                        {ColorType::BG, "BG"},
                        {ColorType::SPRITE, "SPRITE"},
                    });

                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Palettes")) {
                    MyImGui::InputByte("Universal Background", &world.console.ppu.universal_bg_index);
                    ImGui::Separator();
                    for (int i = 0; i < 4; i++) {
                        ImGui::Text(mu::str_tmpf("Background {}", i).c_str());
                        for (int j = 0; j < 4; j++) {
                            MyImGui::InputByte(mu::str_tmpf("{}##bg{}", j, i).c_str(), &world.console.ppu.bg_palette[i].index[j]);
                        }
                    }
                    ImGui::Separator();
                    for (int i = 0; i < 4; i++) {
                        ImGui::Text(mu::str_tmpf("Sprite {}", i).c_str());
                        for (int j = 0; j < 4; j++) {
                            MyImGui::InputByte(mu::str_tmpf("{}##sp{}", j, i).c_str(), &world.console.ppu.sprite_palette[i].index[j]);
                        }
                    }

                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }
        }
        ImGui::End();
    }

    void imgui_debug_window(World& world) {
        if (ImGui::Begin("Debug")) {
            // fps
            auto time_millis = world.frame_time.asMilliseconds();
            ImGui::Text(mu::str_tmpf("FPS: {}", time_millis == 0? 0 : 1000 / time_millis).c_str());

            if (world.should_pause) {
                if (ImGui::Button("Resume")) {
                    world.should_pause = false;
                }
            } else {
                if (ImGui::Button("Pause")) {
                    world.should_pause = true;
                }
            }

            ImGui::SameLine();

            ImGui::BeginDisabled(!world.should_pause);
            world.do_one_instr = ImGui::Button("Step");
            ImGui::EndDisabled();

            ImGui::SameLine();

            if (ImGui::Button("Reset")) {
                console_reset(world.console);
            }

            if (ImGui::TreeNodeEx("Regs", ImGuiTreeNodeFlags_DefaultOpen)) {
                const auto& regs = world.console.cpu.regs;
                ImGui::Text(mu::str_tmpf("PC: ${:04X}", regs.pc).c_str());

                ImGui::Text(mu::str_tmpf("SP: ${:02X}", regs.sp).c_str());
                ImGui::Text(mu::str_tmpf("A: ${:02X}", regs.a).c_str());

                ImGui::Text(mu::str_tmpf("X: ${:02X}", regs.x).c_str());
                ImGui::Text(mu::str_tmpf("Y: ${:02X}", regs.y).c_str());

                ImGui::Text(mu::str_tmpf("C: {}", regs.flags.bits.c).c_str());
                ImGui::Text(mu::str_tmpf("Z: {}", regs.flags.bits.z).c_str());
                ImGui::Text(mu::str_tmpf("I: {}", regs.flags.bits.i).c_str());

                ImGui::Text(mu::str_tmpf("D: {}", regs.flags.bits.d).c_str());
                ImGui::Text(mu::str_tmpf("B: {}", regs.flags.bits.b).c_str());
                ImGui::Text(mu::str_tmpf("V: {}", regs.flags.bits.v).c_str());

                ImGui::Text(mu::str_tmpf("N: {}", regs.flags.bits.n).c_str());

                ImGui::TreePop();
            }

            if (ImGui::TreeNodeEx("Instructions", ImGuiTreeNodeFlags_DefaultOpen)) {
                constexpr auto TABLE_FLAGS = ImGuiTableFlags_ScrollY | ImGuiTableFlags_ScrollX | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersInnerV;
                if (ImGui::BeginTable("assembly_table", 2, TABLE_FLAGS)) {
                    ImGui::TableSetupScrollFreeze(0, 1);
                    ImGui::TableSetupColumn("Adr");
                    ImGui::TableSetupColumn("Inst");
                    ImGui::TableHeadersRow();

                    ImGuiListClipper clipper(world.assembly.size());
                    while (clipper.Step()) {
                        for (int j = clipper.DisplayStart; j < clipper.DisplayEnd; j++) {
                            ImGui::TableNextRow();

                            if (world.assembly[j].adr == world.console.cpu.regs.pc) {
                                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImVec4{0.3f, 0.3f, 0.7f, 0.65f}));
                            }

                            if (ImGui::TableSetColumnIndex(0)) {
                                ImGui::Text(mu::str_tmpf("${:04X}", world.assembly[j].adr).c_str());
                            }
                            if (ImGui::TableSetColumnIndex(1)) {
                                ImGui::Text(world.assembly[j].instr.c_str());
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
    }

    void wnd_rendering_begin(World& world) {
        world.main_wind.clear();
    }

    void wnd_rendering_end(World& world) {
        world.main_wind.display();
    }

    void wnd_render_images(World& world) {
        image_render(world.console_image, world.main_wind);
    }

    void events_collect(World& world) {
        sf::Event event;
        while (world.main_wind.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(world.main_wind, event);

            switch (event.type) {
            case sf::Event::Closed:
                world.main_wind.close();
                break;
            case sf::Event::KeyPressed:
                switch (event.key.code) {
                case Config::exit:
                    world.main_wind.close();
                    break;
                }
                break;
            }
        }
    }

    void clock_update(World& world) {
        world.frame_time = world.delta_clock.restart();
    }

    void console_init(World& world) {
        console_init(world.console, world.rom_path);

        world.assembly = rom_disassemble(world.console.rom);

        world.should_pause = true;
        world.do_one_instr = false;

        world.console_image = image_new();

        if (world.tile_texture.create(8, 8) == false) {
            mu::panic("failed to create texture");
        }
        world.tile_sprite.setTexture(world.tile_texture);
    }

    void console_update(World& world) {
        if (!world.should_pause || world.do_one_instr) {
            console_input(world.console, JoyPadInput {
                .a       = sf::Keyboard::isKeyPressed(Config::a),
                .b       = sf::Keyboard::isKeyPressed(Config::b),
                .select  = sf::Keyboard::isKeyPressed(Config::select),
                .start   = sf::Keyboard::isKeyPressed(Config::start),
                .up      = sf::Keyboard::isKeyPressed(Config::up),
                .down    = sf::Keyboard::isKeyPressed(Config::down),
                .left    = sf::Keyboard::isKeyPressed(Config::left),
                .right   = sf::Keyboard::isKeyPressed(Config::right)
            });

            console_clock(world.console, &world.console_image);
        }
        world.do_one_instr = !world.should_pause;
    }
}

int main(int argc, char** argv) {
    if (argc > 1 && argv[1] == mu::StrView("--help")) {
        fmt::print(stderr, "Usage: {} </path/to/rom | --test [args to Catch2] | --help>\n", mu::file_get_base_name(argv[0]));
        return 1;
    }

    if (argc > 1 && argv[1] == mu::StrView("--test")) {
        return run_tests(argc-1, argv+1);
    }

    World world {
        .rom_path = argc > 1 ? argv[1] : ASSETS_DIR "/nestest.nes",
    };

    sys::window_init(world);

    sys::imgui_init(world);
    mu_defer(sys::imgui_free(world));

    sys::console_init(world);

    while (world.main_wind.isOpen()) {
        sys::clock_update(world);
        sys::events_collect(world);
        sys::console_update(world);

        sys::wnd_rendering_begin(world); {
            sys::wnd_render_images(world);

            sys::imgui_rendering_begin(world); {
                sys::imgui_memory_window(world);
                sys::imgui_viewer_window(world);
                sys::imgui_debug_window(world);
            }
            sys::imgui_rendering_end(world);
        }
        sys::wnd_rendering_end(world);

        mu::memory::reset_tmp();
    }

    return 0;
}

/*
TODO:
- pallette:
    - show/edit current color palette as colors
- nametable
    - render one frame without color
    - render one frame with color from one palette
    - select palette from attribute table
- sprites
    - render one sprite without color
    - select color from palette
- complete all nestest.nes
- support illegal NES instructions
- handle reset correctly (how?)
- handle NMI correctly (how?)
- handle IRQ correctly (how?)
- perform one PPU cycle
*/
