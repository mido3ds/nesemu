#pragma once

#include <SFML/Window.hpp>

#include "common.h"

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

    constexpr VideoSystem sys = NTSC;

    struct Rect {int w, h;};
    constexpr Rect resolution{NTSC.resolution.width, NTSC.resolution.height};

    constexpr Rect main_wind{resolution.w * 3, resolution.h * 3};
    constexpr Rect debug_wind{230, main_wind.h};

    constexpr auto font_path = ASSETS_DIR "/zig.ttf";
    constexpr int font_size = 13;
}
