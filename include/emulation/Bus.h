#pragma once

#include <thread>
#include <vector>

using namespace std;

#include "emulation/BusAttachable.h"

class Bus {
public:
    int attach(shared_ptr<BusAttachable> attachment);

    void reset();

    bool read(u16_t addr, u8_t& data);
    bool read16(u16_t addr, u16_t& data);

    bool write(u16_t addr, u8_t data);
    bool write16(u16_t addr, u16_t data);

private:
    vector<shared_ptr<BusAttachable>> attachments;
};