#include <bitset>

#include <imgui.h>
#include <imgui-SFML.h>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include "Console.h"

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

namespace Config {
    constexpr sf::Keyboard::Key
        // window
        pause = sf::Keyboard::P,
        exit = sf::Keyboard::Escape,

        // console
        reset = sf::Keyboard::R,
        up = sf::Keyboard::Up,
        down = sf::Keyboard::Down,
        left = sf::Keyboard::Left,
        right = sf::Keyboard::Right,
        a = sf::Keyboard::A,
        b = sf::Keyboard::S,
        start = sf::Keyboard::Return,
        select = sf::Keyboard::Tab,

        // debugging
        next_instr = sf::Keyboard::F10,
        debug = sf::Keyboard::D,
        show_mem = sf::Keyboard::M,
        toggle_stepping = sf::Keyboard::F5,
        scroll_mem_down = sf::Keyboard::J,
        scroll_mem_up = sf::Keyboard::K;
}

struct World {
    mu::Str rom_path;
    sf::RenderWindow window;
    mu::Str imgui_ini_file_path;

    bool should_pause;
    bool do_one_instr;

    mu::Timer loop_timer;
    double frame_time_secs;

    Console console;
    sf::Sprite console_sprite;
    sf::Texture console_texture;
    mu::Timer console_render_timer;

    sf::Texture tile_texture;
    sf::Sprite tile_sprite;
};

namespace sys {
    void window_init(World& world) {
        auto title = mu::str_format("NESEMU - {}", world.rom_path);
        world.window.create(
            sf::VideoMode(Config::window.w * Config::view_scale, Config::window.h * Config::view_scale),
            title.c_str(),
            sf::Style::Titlebar|sf::Style::Close
        );
        world.window.setPosition(sf::Vector2i(0,0));
    }

    void imgui_init(World& world) {
        ImGui::SFML::Init(world.window);

        ImGui::StyleColorsDark();

        world.imgui_ini_file_path = mu::str_format("{}/{}", mu::folder_config(mu::memory::tmp()), "nesemu-imgui.ini");
        ImGui::GetIO().IniFilename = world.imgui_ini_file_path.c_str();

        ImGui::GetIO().FontGlobalScale = Config::view_scale;
    }

    void imgui_free(World& world) {
        ImGui::SFML::Shutdown(world.window);
    }

    void imgui_rendering_begin(World& world) {
        ImGui::SFML::Update(world.window, sf::seconds((float)world.frame_time_secs));
    }

    void imgui_rendering_end(World& world) {
        ImGui::SFML::Render(world.window);
    }

    void imgui_memory_window(World& world) {
        if (ImGui::Begin("Memory")) {
            if (ImGui::BeginTabBar("memory_tab_bar")) {
                constexpr uint32_t width = 16;
                constexpr auto TABLE_FLAGS = ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter;

                if (ImGui::BeginTabItem("RAM")) {
                    const auto height = int(world.console.ram.size() / width);
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
                    const auto height = int(prg.size() / width);
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
                    const auto height = int(chr.size() / width);
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
                    static auto table_half = PatternTablePointer::TableHalf::LEFT;
                    static int row = 0, col = 0;

                    enum class ColorType { BG, SPRITE };
                    static ColorType color_type = ColorType::BG;
                    static int palette_index = 0; // 0,1,2,3

                    // get tile
                    uint8_t tile[8][8] = {0};
                    RGBAColor colored_tile[8][8] = {0};
                    for (int j = 0; j < 8; j++) {
                        PatternTablePointer p {
                            .bits = {
                                .row_in_tile = (uint8_t) j,
                                .tile_col = (uint8_t) col,
                                .tile_row = (uint8_t) row,
                                .table_half = table_half,
                            }
                        };
                        p.bits.bit_plane = PatternTablePointer::BitPlane::LOWER;
                        auto l = std::bitset<8>(world.console.rom.chr[p.word]);
                        p.bits.bit_plane = PatternTablePointer::BitPlane::UPPER;
                        auto h = std::bitset<8>(world.console.rom.chr[p.word]);

                        for (int i = 0; i < 8; i++) {
                            tile[j][i] = h[7-i] << 1 | l[7-i];

                            const Palette& palette = color_type == ColorType::BG ?
                                world.console.ppu.bg_palettes[palette_index] : world.console.ppu.sprite_palettes[palette_index];
                            colored_tile[j][i] = color_from_palette(palette.index[tile[j][i]]);
                        }
                    }

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

                    ImGui::SameLine();

                    // tile as image
                    world.tile_texture.update((const sf::Uint8*)colored_tile);
                    world.tile_sprite.setScale(15 * Config::view_scale, 15 * Config::view_scale);
                    ImGui::Image(world.tile_sprite);

                    // configs
                    ImGui::SliderInt("Col", &col, 0, 15, "%d", ImGuiSliderFlags_AlwaysClamp);
                    ImGui::SliderInt("Row", &row, 0, 15, "%d", ImGuiSliderFlags_AlwaysClamp);
                    MyImGui::EnumsCombo("Half", &table_half, {
                        {PatternTablePointer::TableHalf::LEFT, "LEFT"},
                        {PatternTablePointer::TableHalf::RIGHT, "RIGHT"},
                    });
                    ImGui::NewLine();
                    MyImGui::EnumsCombo("Color Type", &color_type, {
                        {ColorType::BG, "BG"},
                        {ColorType::SPRITE, "SPRITE"},
                    });
                    ImGui::SliderInt("Palette Index", &palette_index, 0, 3, "%d", ImGuiSliderFlags_AlwaysClamp);

                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Palettes")) {
                    constexpr auto PALETTE_POPUP_NAME = "nes-palette-color-picker";
                    static uint8_t* popup_clr_index_ptr = nullptr;

                    if (ImGui::BeginPopup(PALETTE_POPUP_NAME)) {
                        ImGui::Text("Palette");
                        for (uint8_t i = 0; i < NES_PALETTE.size(); i++) {
                            ImGui::PushID(i);
                            if ((i % 16) != 0) {
                                ImGui::SameLine(0.0f, ImGui::GetStyle().ItemSpacing.y);
                            }

                            ImVec4 colorvec {
                                NES_PALETTE[i].r / float(0xFF),
                                NES_PALETTE[i].g / float(0xFF),
                                NES_PALETTE[i].b / float(0xFF),
                                NES_PALETTE[i].a / float(0xFF),
                            };

                            ImGuiColorEditFlags palette_button_flags = ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoPicker;
                            if (ImGui::ColorButton(mu::str_tmpf("0x{:02X}##palette", i).c_str(), colorvec, palette_button_flags, ImVec2(20, 20))) {
                                *popup_clr_index_ptr = i;
                                ImGui::CloseCurrentPopup();
                            }

                            ImGui::PopID();
                        }
                        ImGui::EndPopup();
                    }

                    const auto MyImGui_ColorButton = [](mu::StrView id, uint8_t* index) {
                        ImVec4 color {
                            NES_PALETTE[*index].r / float(0xFF),
                            NES_PALETTE[*index].g / float(0xFF),
                            NES_PALETTE[*index].b / float(0xFF),
                            NES_PALETTE[*index].a / float(0xFF),
                        };
                        auto desc_id = mu::str_tmpf("0x{:02X}##color-picker:{}", *index, id).c_str();
                        if (ImGui::ColorButton(desc_id, color, 0)) {
                            popup_clr_index_ptr = index;
                            ImGui::OpenPopup(PALETTE_POPUP_NAME);
                        }
                    };

                    ImGui::Text("Universal Background");
                    ImGui::SameLine();
                    MyImGui_ColorButton("ub", &world.console.ppu.universal_bg_index);

                    ImGui::Separator();

                    ImGui::Text("Background");
                    for (int i = 0; i < 4; i++) {
                        MyImGui_ColorButton(mu::str_tmpf("bg{}0", i), &world.console.ppu.bg_palettes[i].index[0]); ImGui::SameLine();
                        MyImGui_ColorButton(mu::str_tmpf("bg{}1", i), &world.console.ppu.bg_palettes[i].index[1]); ImGui::SameLine();
                        MyImGui_ColorButton(mu::str_tmpf("bg{}2", i), &world.console.ppu.bg_palettes[i].index[2]); ImGui::SameLine();
                        MyImGui_ColorButton(mu::str_tmpf("bg{}3", i), &world.console.ppu.bg_palettes[i].index[3]);
                    }

                    ImGui::Separator();

                    ImGui::Text("Sprite");
                    for (int i = 0; i < 4; i++) {
                        MyImGui_ColorButton(mu::str_tmpf("sp{}0", i), &world.console.ppu.sprite_palettes[i].index[0]); ImGui::SameLine();
                        MyImGui_ColorButton(mu::str_tmpf("sp{}1", i), &world.console.ppu.sprite_palettes[i].index[1]); ImGui::SameLine();
                        MyImGui_ColorButton(mu::str_tmpf("sp{}2", i), &world.console.ppu.sprite_palettes[i].index[2]); ImGui::SameLine();
                        MyImGui_ColorButton(mu::str_tmpf("sp{}3", i), &world.console.ppu.sprite_palettes[i].index[3]);
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
            auto time_millis = std::max(mu::timer_elapsed(world.loop_timer), uint64_t(1));
            auto fps = uint64_t(1000.0 / time_millis);
            ImGui::Text(mu::str_tmpf("FPS: {}", fps).c_str());

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

                    ImGuiListClipper clipper(int(world.console.assembly.size()));
                    while (clipper.Step()) {
                        for (int j = clipper.DisplayStart; j < clipper.DisplayEnd; j++) {
                            ImGui::TableNextRow();

                            if (world.console.assembly[j].adr == world.console.cpu.regs.pc) {
                                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImVec4{0.3f, 0.3f, 0.7f, 0.65f}));
                            }

                            if (ImGui::TableSetColumnIndex(0)) {
                                ImGui::Text(mu::str_tmpf("${:04X}", world.console.assembly[j].adr).c_str());
                            }
                            if (ImGui::TableSetColumnIndex(1)) {
                                ImGui::Text(world.console.assembly[j].instr.c_str());
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
        world.window.setView(world.window.getDefaultView());
        world.window.clear();
    }

    void wnd_rendering_end(World& world) {
        world.window.display();
    }

    void events_collect(World& world) {
        sf::Event event;
        while (world.window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(world.window, event);

            switch (event.type) {
            case sf::Event::Closed:
                world.window.close();
                break;
            case sf::Event::KeyPressed:
                switch (event.key.code) {
                case Config::exit:
                    world.window.close();
                    break;
                }
                break;
            }
        }
    }

    void clock_update(World& world) {
        world.frame_time_secs = std::max(mu::timer_elapsed(world.loop_timer) / 1000.0, 0.001);
        world.loop_timer = mu::timer_new();
    }

    void console_init(World& world) {
        console_init(world.console, world.rom_path);

        world.should_pause = true;
        world.do_one_instr = false;

        if (world.console_texture.create(Config::resolution.w, Config::resolution.h) == false) {
            mu::panic("texture.create failed");
        }
        world.console_sprite.setTexture(world.console_texture);

        if (world.tile_texture.create(8, 8) == false) {
            mu::panic("failed to create texture");
        }
        world.tile_sprite.setTexture(world.tile_texture);
    }

    void console_update(World& world) {
        if (!world.should_pause || world.do_one_instr) {
            // console_input(world.console, JoyPadInput {
            //     .a       = sf::Keyboard::isKeyPressed(Config::a),
            //     .b       = sf::Keyboard::isKeyPressed(Config::b),
            //     .select  = sf::Keyboard::isKeyPressed(Config::select),
            //     .start   = sf::Keyboard::isKeyPressed(Config::start),
            //     .up      = sf::Keyboard::isKeyPressed(Config::up),
            //     .down    = sf::Keyboard::isKeyPressed(Config::down),
            //     .left    = sf::Keyboard::isKeyPressed(Config::left),
            //     .right   = sf::Keyboard::isKeyPressed(Config::right)
            // });

            console_clock(world.console);

            if (mu::timer_elapsed(world.console_render_timer) >= Config::sys.millis_per_frame) {
                world.console_render_timer = mu::timer_new();
                ppu_render(world.console.ppu, world.console.screen_buf);
            }
        }
        world.do_one_instr = !world.should_pause;
    }

    void console_render_screen(World& world) {
        world.console_texture.update((const sf::Uint8*) world.console.screen_buf.pixels.data());
        world.window.setView(sf::View(sf::FloatRect(0, 0, (float)world.console.screen_buf.w, (float)world.console.screen_buf.h)));
        world.window.draw(world.console_sprite);
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

    sys::clock_update(world);

    while (world.window.isOpen()) {
        sys::clock_update(world);
        sys::events_collect(world);
        sys::console_update(world);

        sys::wnd_rendering_begin(world); {
            sys::console_render_screen(world);

            sys::imgui_rendering_begin(world); {
                sys::imgui_memory_window(world);
                sys::imgui_viewer_window(world);
                sys::imgui_debug_window(world);

                ImGui::ShowDemoWindow();
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
- remove vram
- whole pattern table (left and right)
- nametable
    - what are registers?
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
