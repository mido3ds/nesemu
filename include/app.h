#pragma once

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include "sdttype.h"
#include "renderer.h"
#include "console.h"

class App {
protected:
    Console* dev = nullptr;
    sf::RenderWindow mainWind, debugWind;
    Renderer mainRenderer, debugRenderer;
    sf::Font mainFont;

    bool quit = false, pause = false, 
        debugging = true, showMem = true,
        stepping = true;

    u16_t memBeggining = 0;
    f64_t fps;

    void toggleDebugger();
    void renderMem();

    int debuggerTick();
    int mainTick();
public:
    int init(string title, Console* dev);

    int mainLoop();

    ~App();
};