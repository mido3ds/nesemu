#pragma once

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include "stdtype.h"
#include "gui/renderer.h"
#include "emulation/console.h"

class App {
protected:
    Console* dev = nullptr;
    sf::RenderWindow mainWind, debugWind;
    Renderer mainRenderer, debugRenderer;
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
public:
    int init(string title, Console* dev);

    int mainLoop();

    ~App();
};