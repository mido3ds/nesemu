#pragma once

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include "utils.h"
#include "gui/SFMLRenderer.h"
#include "gui/SFMLImageRenderer.h"
#include "emulation/Console.h"

struct App {
    int init(StrView title, Console* dev);

    int mainLoop();

    ~App();

    Console* dev;
    sf::RenderWindow mainWind, debugWind;
    SFMLRenderer memRenderer, debugRenderer;
    SFMLImageRenderer devRenderer;
    sf::Font mainFont;

    bool quit = false, pause = false,
        debugging = true, showMem = true,
        inDebugMode = true, doOneInstr = false;

    uint16_t memoryStart = 0;
    double fps;

    void toggleDebugger();
    void renderMem();

    void handleEvents(sf::RenderWindow& w);
    void onKeyPressed(sf::Keyboard::Key key);
    void onKeyReleased(sf::Keyboard::Key key);

    int debuggerTick();
    int mainTick();

    JoyPadInput getInput();
};
