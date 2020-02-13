#include <chrono>
#include <sstream>
#include <unistd.h>

#include "app.h"
#include "logger.h"
#include "config.h"

#define MEM_WIDTH 16
#define MEM_HEIGHT 46
#define MEM_HPADDING 5
#define MEM_VPADDING 5

int App::init(string title, Console* dev) {
    int err;

    this->dev = dev;
    if (dev == nullptr) {
        ERROR("dev null");
        return 1;
    }

    // main window
    mainWind.create(sf::VideoMode(Config::mainWind.w, Config::mainWind.h), 
        title, sf::Style::Titlebar|sf::Style::Close);
    mainWind.setPosition(sf::Vector2i(0,0));

    err = mainRenderer.init(&mainWind, Config::resolution, Config::mainWind);
    if (err != 0) {
        return err;
    }

    // debug window
    debugWind.create(sf::VideoMode(Config::debugWind.w, Config::debugWind.h), 
        "Debugger", sf::Style::Titlebar|sf::Style::Close);
    debugWind.setVisible(debugging);
    auto mpos = mainWind.getPosition();
    debugWind.setPosition(sf::Vector2i(mpos.x+Config::mainWind.w+5, mpos.y));

    err = debugRenderer.init(&debugWind, Config::resolution, Config::debugWind);
    if (err != 0) {
        return err;
    }

    // font
    if (!mainFont.loadFromFile(Config::fontPath)) {
        ERROR("cant load main font");
        return 1;
    }

    INFO("initialized app");
    return 0;
}

App::~App() {
    if (debugWind.isOpen()) {
        debugWind.close();
    }

    if (mainWind.isOpen()) {
        mainWind.close();
    }
}

void App::toggleDebugger() {
    debugging = !debugging;
    debugWind.setVisible(debugging);
    auto mpos = mainWind.getPosition();
    debugWind.setPosition(sf::Vector2i(mpos.x+Config::mainWind.w+5, mpos.y));
}

static string hex8(u8_t v) {
    char buffer[10] = {0};
    sprintf(buffer, "%02X", v);
    return buffer;
}

static string hex16(u16_t v) {
    char buffer[10] = {0};
    sprintf(buffer, "%04X", v);
    return buffer;
}

void App::handleEvents(sf::RenderWindow& w) {
    sf::Event event;
    while (w.pollEvent(event)) {
        switch (event.type) {
        case sf::Event::Closed:
            quit = true;
            break;
        case sf::Event::KeyPressed:
            onKeyPressed(event.key.code);
            break;
        case sf::Event::KeyReleased:
            onKeyReleased(event.key.code);
            break;
        }
    }
}

void App::onKeyPressed(sf::Keyboard::Key key) {
    switch (key) {
    case Config::reset:
        dev->reset();
        break;
    case Config::exit:
        quit = true;
        break;
    case Config::pause:
        pause = !pause;

        if (pause) { INFO("pause"); }
        else       { INFO("unpause"); }
        break;
    case Config::debug:
        toggleDebugger();
        break;
    case Config::showMem:
        showMem = !showMem;
        break;
    case Config::toggleStepping:
        inDebugMode = !inDebugMode;
        break;
    case Config::nextInstr:
        doOneInstr = true;
        break;
    }
}

void App::onKeyReleased(sf::Keyboard::Key key) {}

int App::debuggerTick() {
    if (!debugging) { return 0; }

    handleEvents(debugWind);

    debugRenderer.clear({0,0,0},0);

    const int h = Config::fontSize+1, 
        w = Config::debugWind.w/2;
    int i = 0;

    // fps
    debugRenderer.text("FPS: "+to_string(int(fps)), 10,(i)*h,1,1, &mainFont,{255,0,0}, 0, 0);

    // mem
    debugRenderer.text("MEM ", 10+w,(i)*h,1,1, &mainFont,{255,255,255}, 0, 0);
    if (showMem) { debugRenderer.text("ON", 10+w+40,(i++)*h,1,1, &mainFont,{0,255,0}, 0, 0); }
    else { debugRenderer.text("OFF", 10+w+40,(i++)*h,1,1, &mainFont,{255,0,0}, 0, 0); }
    i++;

    // regs
    Color c{255,255,0};
    debugRenderer.text("SP: $" + hex8(dev->regs.sp), 10,(i)*h,1,1, &mainFont,c, 0, 0);
    debugRenderer.text("A: $" + hex8(dev->regs.a), 10+w,(i++)*h,1,1, &mainFont,c, 0, 0);

    debugRenderer.text("X: $" + hex8(dev->regs.x), 10,(i)*h,1,1, &mainFont,c, 0, 0);
    debugRenderer.text("Y: $" + hex8(dev->regs.y), 10+w,(i++)*h,1,1, &mainFont,c, 0, 0);
    i++;

    debugRenderer.text("C: " + to_string(dev->regs.flags.bits.c), 10,(i)*h,1,1, &mainFont,c, 0, 0);
    debugRenderer.text("Z: " + to_string(dev->regs.flags.bits.z), 10+w*2.0/3,(i)*h,1,1, &mainFont,c, 0, 0);
    debugRenderer.text("I: " + to_string(dev->regs.flags.bits.i), 10+w*4.0/3,(i++)*h,1,1, &mainFont,c, 0, 0);

    debugRenderer.text("D: " + to_string(dev->regs.flags.bits.d), 10,(i)*h,1,1, &mainFont,c, 0, 0);
    debugRenderer.text("B: " + to_string(dev->regs.flags.bits.b), 10+w*2.0/3,(i)*h,1,1, &mainFont,c, 0, 0);
    debugRenderer.text("V: " + to_string(dev->regs.flags.bits.v), 10+w*4.0/3,(i++)*h,1,1, &mainFont,c, 0, 0);

    debugRenderer.text("N: " + to_string(dev->regs.flags.bits.n), 10,(i++)*h,1,1, &mainFont,c, 0, 0);
    i++;
    
    // assembly
    const int n = 18;
    int j = 1;
    for (auto& s: dev->getAssembly(dev->regs.pc, n)) {
        c = {r:255, g:255, b:255};
        if (j++ == n+1) {
            c.b=0;
        }

        if (s.size() >= 8 && s[7] == '$') {
            c.g = c.b = 0;
        }

        debugRenderer.text(s, 10,(i++)*h,1,1, &mainFont,c, 0, 0);
    }

    debugRenderer.show();
    
    return 0;
}

void App::renderMem() {
    mainRenderer.clear({0,0,0},0);

    for (int i = 0; i < MEM_WIDTH; i++) {
        int x = MEM_HPADDING+53+i*30;
        mainRenderer.text(hex8(i), x, MEM_VPADDING, 1, 1, &mainFont, {255,255,255}, 0, 0);
    }

    for (int j = 0; j < MEM_HEIGHT; j++) {
        int y = MEM_VPADDING+(j+1)*(Config::fontSize+1);
        mainRenderer.text(hex16((memBeggining+j)*MEM_WIDTH), MEM_HPADDING, y, 1, 1, &mainFont, {255,255,255}, 0, 0);

        for (int i = 0; i < MEM_WIDTH; i++) {
            int x = MEM_HPADDING+53+i*30;
            mainRenderer.text(hex8(dev->memory[(memBeggining+j)*MEM_WIDTH+i]), x, y, 1, 1, &mainFont, {255,255,0}, 0, 0);
        }
    }

    mainRenderer.show();
}

int App::mainTick() {
    int err;

    handleEvents(mainWind);

    if (pause) { return 0; }

    const auto keyb = sf::Keyboard::isKeyPressed;
    dev->joypad0.up      = keyb(Config::up);
    dev->joypad0.down    = keyb(Config::down);
    dev->joypad0.left    = keyb(Config::left);
    dev->joypad0.right   = keyb(Config::right);
    dev->joypad0.a       = keyb(Config::a);
    dev->joypad0.b       = keyb(Config::b);
    dev->joypad0.start   = keyb(Config::start);
    dev->joypad0.select  = keyb(Config::select);

    if (!inDebugMode || doOneInstr) {
        dev->oneCPUCycle();
        dev->onePPUCycle(&mainRenderer);
        dev->oneAPUCycle();
    }
    doOneInstr = false;

    if (showMem) { 
        int scrolMultiplier = keyb(sf::Keyboard::LControl) || keyb(sf::Keyboard::RControl) ? MEM_HEIGHT:1;

        if (keyb(Config::scrollMemDown)) {
            if ((memBeggining+scrolMultiplier+MEM_HEIGHT)*MEM_WIDTH < MEM_SIZE) {
                memBeggining += scrolMultiplier;
            }
        } else if (keyb(Config::scrollMemUp)) {
            if (memBeggining >= scrolMultiplier) {
                memBeggining -= scrolMultiplier;
            }
        }

        renderMem(); 
    }

    return 0;
}

int App::mainLoop() {
    using namespace chrono;

    INFO("start executing");
    int err;

    while (!quit && mainWind.isOpen()) {
        auto t1 = high_resolution_clock::now();

        if ((err = mainTick()) != 0) {
            return err;
        }

        if ((err = debuggerTick()) != 0) {
            return err;
        }
        
        usleep(1); // guard window from overdrawing

        auto t2 = high_resolution_clock::now();
        auto duration = duration_cast<chrono::milliseconds>(t2 - t1).count();

        fps = 1000.0/duration;
    }

    return 0;
}