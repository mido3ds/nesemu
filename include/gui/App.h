#pragma once

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include "stdtype.h"
#include "gui/SFMLRenderer.h"
#include "gui/SFMLImageRenderer.h"
#include "emulation/Console.h"

class App {
public:
    int init(string title, Console* dev);

    int mainLoop();

    ~App();

private:
    Console* dev;
    sf::RenderWindow mainWind, debugWind;
    SFMLRenderer memRenderer, debugRenderer;
    SFMLImageRenderer devRenderer;
    sf::Font mainFont;

    bool quit = false, pause = false, 
        debugging = true, showMem = true,
        inDebugMode = true, doOneInstr = false;

    u16_t memBeggining = 0;
    f64_t fps;

    void toggleDebugger();
    void renderMem();

    void handleEvents(sf::RenderWindow& w);
    void onKeyPressed(sf::Keyboard::Key key);
    void onKeyReleased(sf::Keyboard::Key key);

    int debuggerTick();
    int mainTick();

    JoyPadInput getInput();
};