#include <chrono>
#include <sstream>

#include "utils.h"
#include "gui/App.h"
#include "Config.h"

#define MEM_WIDTH 16
#define MEM_HEIGHT 46
#define MEM_HPADDING 5
#define MEM_VPADDING 5

int App::init(StrView title, Console* dev) {
    int err;

    this->dev = dev;
    if (!dev) {
        ERROR("dev is null");
        return 1;
    }

    // main window
    mainWind.create(sf::VideoMode(Config::mainWind.w, Config::mainWind.h),
        sf::String(title.begin()), sf::Style::Titlebar|sf::Style::Close);
    mainWind.setPosition(sf::Vector2i(0,0));

    err = memRenderer.init(&mainWind, Config::mainWind);
    if (err != 0) {
        return err;
    }

    err = devRenderer.init(&mainWind, Config::resolution);
    if (err != 0) {
        return err;
    }

    // debug window
    debugWind.create(sf::VideoMode(Config::debugWind.w, Config::debugWind.h),
        "Debugger", sf::Style::Titlebar|sf::Style::Close);
    debugWind.setVisible(debugging);
    auto mpos = mainWind.getPosition();
    debugWind.setPosition(sf::Vector2i(mpos.x+Config::mainWind.w+5, mpos.y));

    err = debugRenderer.init(&debugWind, Config::debugWind);
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
    debugRenderer.text(str_tmpf("FPS: {}", int(fps)), 10,(i)*h,1,1, (Font*)&mainFont,{255,0,0}, 0, 0);

    // mem
    debugRenderer.text("MEM ", 10+w,(i)*h,1,1, (Font*)&mainFont,{255,255,255}, 0, 0);
    if (showMem) { debugRenderer.text("ON", 10+w+40,(i++)*h,1,1, (Font*)&mainFont,{0,255,0}, 0, 0); }
    else { debugRenderer.text("OFF", 10+w+40,(i++)*h,1,1, (Font*)&mainFont,{255,0,0}, 0, 0); }
    i++;

    // regs
    Color c{255,255,0};
    const auto regs = &dev->cpu.regs;
    debugRenderer.text(str_tmpf("SP: ${:02X}", regs->sp), 10,(i)*h,1,1, (Font*)&mainFont,c, 0, 0);
    debugRenderer.text(str_tmpf("A: ${:02X}", regs->a), 10+w,(i++)*h,1,1, (Font*)(Font*)&mainFont,c, 0, 0);

    debugRenderer.text(str_tmpf("X: ${:02X}", regs->x), 10,(i)*h,1,1, (Font*)&mainFont,c, 0, 0);
    debugRenderer.text(str_tmpf("Y: ${:02X}", regs->y), 10+w,(i++)*h,1,1, (Font*)&mainFont,c, 0, 0);
    i++;

    debugRenderer.text(str_tmpf("C: {}", regs->flags.bits.c), 10,(i)*h,1,1, (Font*)&mainFont,c, 0, 0);
    debugRenderer.text(str_tmpf("Z: {}", regs->flags.bits.z), 10+w*2.0/3,(i)*h,1,1, (Font*)&mainFont,c, 0, 0);
    debugRenderer.text(str_tmpf("I: {}", regs->flags.bits.i), 10+w*4.0/3,(i++)*h,1,1, (Font*)&mainFont,c, 0, 0);

    debugRenderer.text(str_tmpf("D: {}", regs->flags.bits.d), 10,(i)*h,1,1, (Font*)&mainFont,c, 0, 0);
    debugRenderer.text(str_tmpf("B: {}", regs->flags.bits.b), 10+w*2.0/3,(i)*h,1,1, (Font*)&mainFont,c, 0, 0);
    debugRenderer.text(str_tmpf("V: {}", regs->flags.bits.v), 10+w*4.0/3,(i++)*h,1,1, (Font*)&mainFont,c, 0, 0);

    debugRenderer.text(str_tmpf("N: {}", regs->flags.bits.n), 10,(i)*h,1,1, (Font*)&mainFont,c, 0, 0);
    debugRenderer.text(str_tmpf("PC: ${:04X}", regs->pc), 10+w*2.0/3,(i++)*h,1,1, (Font*)&mainFont,{255,0,0}, 0, 0);
    i++;

    // assembly
    const int n = 18;
    int j = 1;
    for (auto& s: dev->disassembler.get(regs->pc, n)) {
        c = {.r=255, .g=255, .b=255};
        if (j++ == n+1) {
            c.b=0;
        }

        if (s.size() >= 8 && s[7] == '$') {
            c.g = c.b = 0;
        }

        debugRenderer.text(Str(s), 10,(i++)*h,1,1, (Font*)&mainFont,c, 0, 0);
    }

    debugRenderer.show();

    return 0;
}

void App::renderMem() {
    memRenderer.clear({0,0,0},0);

    for (int i = 0; i < MEM_WIDTH; i++) {
        int x = MEM_HPADDING+53+i*30;
        memRenderer.text(str_tmpf("{:02X}", i), x, MEM_VPADDING, 1, 1, (Font*)&mainFont, {255,255,255}, 0, 0);
    }

    for (int j = 0; j < MEM_HEIGHT; j++) {
        int y = MEM_VPADDING+(j+1)*(Config::fontSize+1);
        memRenderer.text(str_tmpf("{:04X}", (memoryStart+j)*MEM_WIDTH), MEM_HPADDING, y, 1, 1, (Font*)&mainFont, {255,255,255}, 0, 0);

        for (int i = 0; i < MEM_WIDTH; i++) {
            int x = MEM_HPADDING+53+i*30;
            uint8_t data;
            if (dev->ram->read((memoryStart+j)*MEM_WIDTH+i, data)) {
                memRenderer.text(str_tmpf("{:02X}", data), x, y, 1, 1, (Font*)&mainFont, {255,255,0}, 0, 0);
            }
        }
    }

    memRenderer.show();
}

JoyPadInput App::getInput() {
    return JoyPadInput{
        .a       = sf::Keyboard::isKeyPressed(Config::a),
        .b       = sf::Keyboard::isKeyPressed(Config::b),
        .select  = sf::Keyboard::isKeyPressed(Config::select),
        .start   = sf::Keyboard::isKeyPressed(Config::start),
        .up      = sf::Keyboard::isKeyPressed(Config::up),
        .down    = sf::Keyboard::isKeyPressed(Config::down),
        .left    = sf::Keyboard::isKeyPressed(Config::left),
        .right   = sf::Keyboard::isKeyPressed(Config::right)
    };
}

int App::mainTick() {
    handleEvents(mainWind);
    if (pause) { return 0; }

    dev->input(getInput());

    if (!inDebugMode || doOneInstr) {
        dev->clock(&devRenderer);
    }
    doOneInstr = false;

    if (showMem) {
        const auto keyb = sf::Keyboard::isKeyPressed;
        int scrolMultiplier = keyb(sf::Keyboard::LControl) || keyb(sf::Keyboard::RControl) ? MEM_HEIGHT/2:1;

        if (keyb(Config::scrollMemDown)) {
            if ((memoryStart+scrolMultiplier+MEM_HEIGHT)*MEM_WIDTH <= 0x0800) {
                memoryStart += scrolMultiplier;
            }
        } else if (keyb(Config::scrollMemUp)) {
            if (memoryStart >= scrolMultiplier) {
                memoryStart -= scrolMultiplier;
            }
        }

        renderMem();
    } else {
        devRenderer.show();
    }

    return 0;
}

int App::mainLoop() {
    INFO("start executing");
    int err;

    while (!quit && mainWind.isOpen()) {
        memory::reset_tmp();

        auto t1 = std::chrono::high_resolution_clock::now();

        if ((err = mainTick())) { return err; }
        if ((err = debuggerTick())) { return err; }

        auto t2 = std::chrono::high_resolution_clock::now();
        auto duration = duration_cast<std::chrono::milliseconds>(t2 - t1).count();

        fps = 1000.0/duration;
    }

    return 0;
}
