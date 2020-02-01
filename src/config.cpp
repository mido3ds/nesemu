#include <unordered_map>

#include "config.h"
#include "logger.h"

static SDL_Scancode strToCode(string name) {
    static const unordered_map<string, SDL_Scancode> table {
        {"up", SDL_SCANCODE_UP},
        {"down", SDL_SCANCODE_DOWN},
        {"right", SDL_SCANCODE_RIGHT},
        {"left", SDL_SCANCODE_LEFT},
        {"esc", SDL_SCANCODE_ESCAPE},
        {"enter", SDL_SCANCODE_RETURN},
        {"tab", SDL_SCANCODE_TAB},
        {"lctrl", SDL_SCANCODE_LCTRL},
        {"rctrl", SDL_SCANCODE_RCTRL},
        {"lalt", SDL_SCANCODE_LALT},
        {"ralt", SDL_SCANCODE_RALT},
        {"=", SDL_SCANCODE_EQUALS},
        {"+", SDL_SCANCODE_EQUALS},
        {"backspace", SDL_SCANCODE_BACKSPACE},
        {"space", SDL_SCANCODE_SPACE},
        {"f1", SDL_SCANCODE_F1},
        {"f2", SDL_SCANCODE_F2},
        {"f3", SDL_SCANCODE_F3},
        {"f4", SDL_SCANCODE_F4},
        {"f5", SDL_SCANCODE_F5},
        {"f6", SDL_SCANCODE_F6},
        {"f7", SDL_SCANCODE_F7},
        {"f8", SDL_SCANCODE_F8},
        {"f9", SDL_SCANCODE_F9},
        {"f10", SDL_SCANCODE_F10},
        {"f11", SDL_SCANCODE_F11},
        {"f12", SDL_SCANCODE_F12},
    };

    if (table.find(name) != table.end()) {
        return table.at(name);
    } 

    if (name.size() == 1) {
        if (isalpha(name[0])) {
            return SDL_Scancode(tolower(name[0]) - 'a' + SDL_SCANCODE_A);
        } 

        if (isdigit(name[0])) {
            return SDL_Scancode(name[0] - '0' + SDL_SCANCODE_0);
        }
    }

    logError("[stringToScancode] cant find a scancode for %s", name.c_str());
    return SDL_SCANCODE_UNKNOWN;
}

Config Config::fromFile(string path) {
    logInfo("loading config");

    Config c;
    YAML::Node yaml = YAML::LoadFile(path);

    if (yaml["controls"]["keyboard"]) {
        auto key = yaml["controls"]["keyboard"];

        c.up = key["up"] ? strToCode(key["up"].as<string>()) : c.up;
        c.down = key["down"] ? strToCode(key["down"].as<string>()) : c.down;
        c.left = key["left"] ? strToCode(key["left"].as<string>()) : c.left;
        c.right = key["right"] ? strToCode(key["right"].as<string>()) : c.right;
        c.a = key["a"] ? strToCode(key["a"].as<string>()) : c.a;
        c.b = key["b"] ? strToCode(key["b"].as<string>()) : c.b;
        c.pause = key["pause"] ? strToCode(key["pause"].as<string>()) : c.pause;
        c.exit = key["exit"] ? strToCode(key["exit"].as<string>()) : c.exit;
        c.reset = key["reset"] ? strToCode(key["reset"].as<string>()) : c.reset;
        c.start = key["start"] ? strToCode(key["start"].as<string>()) : c.start;
        c.select = key["select"] ? strToCode(key["select"].as<string>()) : c.select;
    }

    if (yaml["video-system"]) {
        c.sys = yaml["video-system"].as<string>() == "pal" ? PAL:NTSC;
    }

    c.windowSize.w = yaml["window"]["width"] ? yaml["window"]["width"].as<int>() : c.windowSize.w;
    c.windowSize.h = yaml["window"]["height"] ? yaml["window"]["height"].as<int>() : c.windowSize.h;

    return c;
}