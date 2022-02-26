#pragma once

#include <SFML/Window.hpp>

#include "emulation/common.h"

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
        nextInstr = sf::Keyboard::F10,
        debug = sf::Keyboard::D, 
        showMem = sf::Keyboard::M,
        toggleStepping = sf::Keyboard::F5,
        scrollMemDown = sf::Keyboard::J,
        scrollMemUp = sf::Keyboard::K;

    constexpr VideoSystem sys = NTSC;

    struct Rect {int w, h;};
    constexpr Rect resolution{NTSC.resolution.width, NTSC.resolution.height};
    
    constexpr Rect mainWind{resolution.w * 3, resolution.h * 3}; 
    constexpr Rect debugWind{230, mainWind.h};

    constexpr auto fontPath = "zig.ttf";
    constexpr int fontSize = 13;
}