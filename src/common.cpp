#include "common.h"
#include "utils.h"

RGBAColor RGBAColor::from_sys_palette(uint8_t index) {
    if (index > 0x3F) {
        ERROR("invalid index color(0x{:02x}), returning black", index);
        return {};
    }

    static constexpr Arr<RGBAColor, 0x3F + 1> sys_palette = {
        RGBAColor({0x75, 0x75, 0x75}),
        RGBAColor({0x27, 0x1B, 0x8F}),
        RGBAColor({0x00, 0x00, 0xAB}),
        RGBAColor({0x47, 0x00, 0x9F}),
        RGBAColor({0x8F, 0x00, 0x77}),
        RGBAColor({0xAB, 0x00, 0x13}),
        RGBAColor({0xA7, 0x00, 0x00}),
        RGBAColor({0x7F, 0x0B, 0x00}),
        RGBAColor({0x43, 0x2F, 0x00}),
        RGBAColor({0x00, 0x47, 0x00}),
        RGBAColor({0x00, 0x51, 0x00}),
        RGBAColor({0x00, 0x3F, 0x17}),
        RGBAColor({0x1B, 0x3F, 0x5F}),
        RGBAColor({0x00, 0x00, 0x00}),
        RGBAColor({0x00, 0x00, 0x00}),
        RGBAColor({0x00, 0x00, 0x00}),
        RGBAColor({0xBC, 0xBC, 0xBC}),
        RGBAColor({0x00, 0x73, 0xEF}),
        RGBAColor({0x23, 0x3B, 0xEF}),
        RGBAColor({0x83, 0x00, 0xF3}),
        RGBAColor({0xBF, 0x00, 0xBF}),
        RGBAColor({0xE7, 0x00, 0x5B}),
        RGBAColor({0xDB, 0x2B, 0x00}),
        RGBAColor({0xCB, 0x4F, 0x0F}),
        RGBAColor({0x8B, 0x73, 0x00}),
        RGBAColor({0x00, 0x97, 0x00}),
        RGBAColor({0x00, 0xAB, 0x00}),
        RGBAColor({0x00, 0x93, 0x3B}),
        RGBAColor({0x00, 0x83, 0x8B}),
        RGBAColor({0x00, 0x00, 0x00}),
        RGBAColor({0x00, 0x00, 0x00}),
        RGBAColor({0x00, 0x00, 0x00}),
        RGBAColor({0xFF, 0xFF, 0xFF}),
        RGBAColor({0x3F, 0xBF, 0xFF}),
        RGBAColor({0x5F, 0x97, 0xFF}),
        RGBAColor({0xA7, 0x8B, 0xFD}),
        RGBAColor({0xF7, 0x7B, 0xFF}),
        RGBAColor({0xFF, 0x77, 0xB7}),
        RGBAColor({0xFF, 0x77, 0x63}),
        RGBAColor({0xFF, 0x9B, 0x3B}),
        RGBAColor({0xF3, 0xBF, 0x3F}),
        RGBAColor({0x83, 0xD3, 0x13}),
        RGBAColor({0x4F, 0xDF, 0x4B}),
        RGBAColor({0x58, 0xF8, 0x98}),
        RGBAColor({0x00, 0xEB, 0xDB}),
        RGBAColor({0x00, 0x00, 0x00}),
        RGBAColor({0x00, 0x00, 0x00}),
        RGBAColor({0x00, 0x00, 0x00}),
        RGBAColor({0xFF, 0xFF, 0xFF}),
        RGBAColor({0xAB, 0xE7, 0xFF}),
        RGBAColor({0xC7, 0xD7, 0xFF}),
        RGBAColor({0xD7, 0xCB, 0xFF}),
        RGBAColor({0xFF, 0xC7, 0xFF}),
        RGBAColor({0xFF, 0xC7, 0xDB}),
        RGBAColor({0xFF, 0xBF, 0xB3}),
        RGBAColor({0xFF, 0xDB, 0xAB}),
        RGBAColor({0xFF, 0xE7, 0xA3}),
        RGBAColor({0xE3, 0xFF, 0xA3}),
        RGBAColor({0xAB, 0xF3, 0xBF}),
        RGBAColor({0xB3, 0xFF, 0xCF}),
        RGBAColor({0x9F, 0xFF, 0xF3}),
        RGBAColor({0x00, 0x00, 0x00}),
        RGBAColor({0x00, 0x00, 0x00}),
        RGBAColor({0x00, 0x00, 0x00})
    };

    return sys_palette[index];
}
