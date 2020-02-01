#pragma once

#include "common.h"
#include "renderer.h"

class Console {
protected:

    struct {
        uint16_t pc; // program counter
        uint8_t sp; // stack pointer
        uint8_t a; // accumulator
        uint8_t x; // index
        uint8_t y; // index
        union {
            struct {
                uint8_t c:1; // carry flag
                uint8_t z:1; // zero flag
                uint8_t i:1; // interrupt disable
                uint8_t d:1; // decimal mode
                uint8_t b:1; // break command
                uint8_t:1; 
                uint8_t v:1; // overflow flag
                uint8_t n:1; // negative flag
            } bits;
            uint8_t byte;
        } flags; // processor status
    } regs;

    ROM rom;
    array<uint8_t, MEM_SIZE> memory;
    array<uint8_t, MEM_SIZE> vram;
    array<uint8_t, 256> sprram; // sprite ram
    InstructionSet instrucSet;

    union ScrollReg {
        struct {
            uint8_t xScroll:5;
            uint8_t yScroll:5;
            uint8_t bit10:1;
            uint8_t bit11:1;
            uint8_t yOffset:3;
            uint8_t:1;
        } bits;
        uint16_t word;

        void incXScroll() {bits.bit10 ^= (++bits.xScroll == 0);}
        void incYScroll() {
            if (++bits.yScroll == 30) {
                bits.bit11 ^= 1;
                bits.yScroll = 0;
            }
        }
    } *scrollReg = (ScrollReg*)(memory.data() + PPU_SCRL_REG);

    union PPUControlReg {
        struct {
            // reg0
            uint8_t nameTable:2; // name table 0, 1, 2 or 3
            uint8_t ppuIncRate:1; // 0 -> 1 or 1 -> 32
            uint8_t spritesPattTable:1; // pattern table 0 or 1
            uint8_t bckgPattTable:1; // pattern table 0 or 1
            SpriteType spriteType:1;
            uint8_t:1; // Changes PPU between master and slave modes. This is not used by the NES.
            bool nmiEnabled:1; // whether non-maskable-interrupt should occur upon V-Blank or not

            // reg1
            ColorMode colorMode:1;
            bool showLeftBckg:1; // whether to show background in the left 8 pixels or not
            bool showLeftSprites:1; // whether to show sprites in the left 8 pixels or not
            bool showBckg:1;  // whether to show whole background or not
            bool showSprites:1; // whether to show whole sprites or not
            uint8_t color:3; // background colour in monochrome mode or colour intensity in colour mode
        } bits;
        uint8_t reg0, reg1;

        uint16_t getNameTableAddress() {return 0x2000 + bits.nameTable * 0x0400;}
        uint8_t getPPUIncrementRate() {return bits.ppuIncRate ? /*vertical*/32 : /*horizontal*/1;}
    } *controlReg = (PPUControlReg*)(memory.data() + PPU_CTRL_REG0);

    union PPUStatusReg {
        struct {
            uint8_t:1;
            uint8_t:1;
            uint8_t:1;
            uint8_t:1;
            uint8_t ignoreVramWrites:1;
            uint8_t spritesMoreThan8:1;    
            uint8_t sprite0Hit:1; // set when a non-transparent pixel of 
                                // sprite 0 overlaps a non-transparent background pixel
            uint8_t vblank:1; // set when V-Blank is occurring
        } bits;
        uint8_t byte;
    } *statusReg = (PPUStatusReg*)(memory.data() + PPU_STS_REG);

    uint8_t& sprramAddrReg                    = memory[0x2003];
    uint8_t& sprramIOReg                      = memory[0x2004];

    uint8_t& vramAddrReg0                     = memory[0x2005];
    uint8_t& vramAddrReg1                     = memory[0x2006];
    uint8_t& vramIOReg                        = memory[0x2007];

    uint8_t& apuPulse1ControlReg              = memory[0x4000];
    uint8_t& apuPulse1RampControlReg          = memory[0x4001];
    uint8_t& apuPulse1FineTuneReg             = memory[0x4002];
    uint8_t& apuPulse1CoarseTuneReg           = memory[0x4003];
    uint8_t& apuPulse2ControlReg              = memory[0x4004];
    uint8_t& apuPulse2RampControlReg          = memory[0x4005];
    uint8_t& apuPulse2FineTuneReg             = memory[0x4006];
    uint8_t& apuPulse2CoarseTuneReg           = memory[0x4007];
    uint8_t& apuTriangleControlReg1           = memory[0x4008];
    uint8_t& apuTriangleControlReg2           = memory[0x4009];
    uint8_t& apuTriangleFrequencyReg1         = memory[0x400A];
    uint8_t& apuTriangleFrequencyReg2         = memory[0x400B];
    uint8_t& apuNoiseControlReg1              = memory[0x400C];
    uint8_t& apuNoiseFrequencyReg1            = memory[0x400E];
    uint8_t& apuNoiseFrequencyReg2            = memory[0x400F];
    uint8_t& apuDeltaModulationControlReg     = memory[0x4010];
    uint8_t& apuDeltaModulationDAReg          = memory[0x4011];
    uint8_t& apuDeltaModulationAddressReg     = memory[0x4012];
    uint8_t& apuDeltaModulationDataLengthReg  = memory[0x4013];
    uint8_t& apuVerticalClockSignalReg        = memory[0x4015];

    uint8_t& spriteDMAReg                     = memory[0x4014];

    // TODO listen for changes in reg
    struct JoyPadReg {
        union {
            uint8_t data:1; // Reads data from joypad or causes joypad strobe when writing.
            uint8_t:1;
            uint8_t:1;
            uint8_t zapperIsPointing:1;
            uint8_t zapperIsTriggered:1;
            uint8_t:3;
        } bits;
        uint8_t byte;
    } *joypadReg0 = (JoyPadReg*)(memory.data() + 0x4016), 
    *joypadReg1 = (JoyPadReg*)(memory.data() + 0x4017);

    uint16_t cycles;

    // temporary placeholder for values written in memory[0x2006]
    struct X2006Reg {
        uint16_t addr;

        // possible states, multiple ones could be combined
        static constexpr uint8_t 
            EMPTY=0,  // addr is not yet initialized
            LSN=0b1, // leaset significant nibble (4 bits) has been loaded
            FULL=0b10, // most signifcant nibble (4 bits) has been loaded
            CAN_READ=0b100, CAN_WRITE=0b1000, 
            READ_BUFFERED=0b10000; // you can only read colr-palette

        // bitset indicates the state of 0x2006 register
        uint8_t state = EMPTY; 
    } temp0x2006; 

public:

    Console();

    struct JoyPad {
        bool a; 
        bool b;
        bool select;
        bool start;
        bool up;
        bool down;
        bool left;
        bool right;
    } joypad0, joypad1;

    /*addressing modes for 6502
    from Appendix E: http://www.nesdev.com/NESDoc.pdf
    */
    inline uint16_t zeroPageAddress(const uint8_t bb) {
        return bb;
    }

    // indexed
    inline uint16_t indexedZeroPageAddress(const uint8_t bb, const uint8_t i) {
        return (bb+i) % 0xFF;
    }

    inline uint16_t absoluteAddress(const uint8_t bb, const uint8_t cc) {
        return cc << 8 | bb;
    }

    // indexed
    inline uint16_t indexedAbsoluteAddress(const uint8_t bb, const uint8_t cc, const uint8_t i) {
        return absoluteAddress(bb, cc) + i;
    }

    inline uint16_t indirectAddress(const uint8_t bb, const uint8_t cc) {
        uint16_t ccbb = absoluteAddress(bb, cc);
        return absoluteAddress(read(ccbb), read(ccbb+1));
    }

    inline uint16_t indexedIndirectAddress(const uint8_t bb, const uint8_t i) {
       return absoluteAddress(read(bb+i), read(bb+i+1));
    }

    inline uint16_t indirectIndexedAddress(const uint8_t bb, const uint8_t i) {
        return absoluteAddress(read(bb), read(bb+1)) + i;
    }

    bool setROM(string romPath);

    template<typename T>
    T _read(uint16_t address);

    inline uint8_t read(uint16_t address) { return _read<uint8_t>(address); }
    inline uint16_t read16(uint16_t address) { return _read<uint16_t>(address); }

    inline uint8_t  fetch() { return _read<uint8_t>(regs.pc++); }
    inline uint16_t fetch16() { return _read<uint16_t>(regs.pc++); }

    template<typename T>
    void _write(uint16_t address, T value);

    inline void write(uint16_t address, uint8_t v) { _write<uint8_t>(address, v); }
    inline void write16(uint16_t address, uint16_t v) { _write<uint16_t>(address, v); }

    template<typename T>
    void _push(T v);

    inline void push(uint8_t v) { _push<uint8_t>(v); }
    inline void push16(uint16_t v) { _push<uint16_t>(v); }

    template<typename T>
    T _pop();

    inline uint8_t pop() { return _pop<uint8_t>(); }
    inline uint16_t pop16() { return _pop<uint16_t>(); }

    template<typename T>
    T _readVRam(uint16_t address);

    inline uint8_t readVRam(uint16_t address) { return _readVRam<uint8_t>(address); }
    inline uint16_t readVRam16(uint16_t address) { return _readVRam<uint16_t>(address); }

    template<typename T>
    void _writeVRam(uint16_t address, T value);

    inline void writeVRam(uint16_t address, uint8_t v) { _writeVRam<uint8_t>(address, v); }
    inline void writeVRam16(uint16_t address, uint16_t v) { _writeVRam<uint16_t>(address, v); }

    uint16_t getSpriteAddr(SpriteInfo* inf, SpriteType type);

    uint8_t readPPUStatusRegister();

    void reset();

    bool powerOn();

    // https://wiki.nesdev.com/w/index.php/CPU_power_up_state#At_power-up
    bool _powerOnCPU();

    // https://wiki.nesdev.com/w/index.php/PPU_power_up_state
    bool _powerOnPPU();

    void oneCPUCycle();

    void onePPUCycle(Renderer const* renderer);

    void oneAPUCycle();
};