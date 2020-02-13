#pragma once

#include "emulation/common.h"
#include "gui/renderer.h"
#include "emulation/rom.h"

class Console {
private:
    void powerOn();
    void loadInstructions();
public:
/////////////////////////// State ///////////////////////////
    struct {
        u16_t pc; // program counter
        u8_t sp; // stack pointer
        u8_t a; // accumulator
        u8_t x; // index
        u8_t y; // index
        union {
            struct {
                u8_t c:1; // carry flag
                u8_t z:1; // zero flag
                u8_t i:1; // interrupt disable
                u8_t d:1; // decimal mode
                u8_t b:1; // break command
                u8_t:1; 
                u8_t v:1; // overflow flag
                u8_t n:1; // negative flag
            } bits;
            u8_t byte;
        } flags; // processor status
    } regs;

    MemType memory;
    MemType vram;
    array<u8_t, 256> sprram; // sprite ram

    InstructionSet instrucSet;
    u16_t cpuCycles, ppuCycles, apuCycles;

    union ScrollReg {
        struct {
            u8_t xScroll:5;
            u8_t yScroll:5;
            u8_t bit10:1;
            u8_t bit11:1;
            u8_t yOffset:3;
            u8_t:1;
        } bits;
        u16_t word;

        inline void incXScroll() {bits.bit10 ^= (++bits.xScroll == 0);}
        inline void incYScroll() {
            if (++bits.yScroll == 30) {
                bits.bit11 ^= 1;
                bits.yScroll = 0;
            }
        }
    } *scrollReg = (ScrollReg*)(memory.data() + PPU_SCRL_REG);

    union PPUControlReg {
        struct {
            // reg0
            u8_t nameTable:2; // name table 0, 1, 2 or 3
            u8_t ppuIncRate:1; // 0 -> 1 or 1 -> 32
            u8_t spritesPattTable:1; // pattern table 0 or 1
            u8_t bckgPattTable:1; // pattern table 0 or 1
            SpriteType spriteType:1;
            u8_t:1; // Changes PPU between master and slave modes. This is not used by the NES.
            bool nmiEnabled:1; // whether non-maskable-interrupt should occur upon V-Blank or not

            // reg1
            ColorMode colorMode:1;
            bool showLeftBckg:1; // whether to show background in the left 8 pixels or not
            bool showLeftSprites:1; // whether to show sprites in the left 8 pixels or not
            bool showBckg:1;  // whether to show whole background or not
            bool showSprites:1; // whether to show whole sprites or not
            u8_t color:3; // background colour in monochrome mode or colour intensity in colour mode
        } bits;
        u8_t reg0, reg1;

        inline u16_t getNameTableAddress() {return 0x2000 + bits.nameTable * 0x0400;}
        inline u8_t getPPUIncrementRate() {return bits.ppuIncRate ? /*vertical*/32 : /*horizontal*/1;}
    } *controlReg = (PPUControlReg*)(memory.data() + PPU_CTRL_REG0);

    union PPUStatusReg {
        struct {
            u8_t:1;
            u8_t:1;
            u8_t:1;
            u8_t:1;
            u8_t ignoreVramWrites:1;
            u8_t spritesMoreThan8:1;    
            u8_t sprite0Hit:1; // set when a non-transparent pixel of 
                                // sprite 0 overlaps a non-transparent background pixel
            u8_t vblank:1; // set when V-Blank is occurring
        } bits;
        u8_t byte;
    } *ppuStatusReg = (PPUStatusReg*)(memory.data() + PPU_STS_REG);

    // TODO listen for changes in reg
    union JoyPadReg {
        struct {
            u8_t data:1; // Reads data from joypad or causes joypad strobe when writing.
            u8_t:1;
            u8_t:1;
            u8_t zapperIsPointing:1;
            u8_t zapperIsTriggered:1;
            u8_t:3;
        } bits;
        u8_t byte;
    } *joypadReg0 = (JoyPadReg*)(memory.data() + 0x4016), 
    *joypadReg1 = (JoyPadReg*)(memory.data() + 0x4017);

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

    // temporary placeholder for values written in memory[VRAM_ADDR_REG1]
    struct VRamAddrReg1 {
        u16_t addr;

        // possible states, multiple ones could be combined
        static constexpr u8_t 
            EMPTY=0x00,  // addr is not yet initialized
            LSN=0x01, // leaset significant nibble (4 bits) has been loaded
            FULL=0x02, // most signifcant nibble (4 bits) has been loaded
            CAN_READ=0x04, 
            CAN_WRITE=0x08, 
            READ_BUFFERED=0x10; // you can only read colr-palette

        // bitset indicates the state of VRAM_ADDR_REG1 register
        u8_t state = EMPTY; 
    } vramAddrReg1; 

/////////////////////////// Methods ///////////////////////////

    /*addressing modes for 6502
    from Appendix E: http://www.nesdev.com/NESDoc.pdf
    */
    u16_t zeroPageAddress(const u8_t bb);
    u16_t indexedZeroPageAddress(const u8_t bb, const u8_t i);
    u16_t absoluteAddress(const u8_t bb, const u8_t cc);
    u16_t indexedAbsoluteAddress(const u8_t bb, const u8_t cc, const u8_t i);
    u16_t indirectAddress(const u8_t bb, const u8_t cc);
    u16_t indexedIndirectAddress(const u8_t bb, const u8_t i);
    u16_t indirectIndexedAddress(const u8_t bb, const u8_t i);

    // ram
    u8_t read(u16_t address);
    u16_t read16(u16_t address);
    u8_t fetch(); // read and increment pc

    void write(u16_t address, u8_t value);
    void write16(u16_t address, u16_t v);

    void push(u8_t v);
    void push16(u16_t v);

    u8_t pop();
    u16_t pop16();

    // vram
    u8_t readVRam(u16_t address);
    u16_t readVRam16(u16_t address);

    void writeVRam(u16_t address, u8_t v);
    void writeVRam16(u16_t address, u16_t v);

    u16_t getSpriteAddr(SpriteInfo* inf, SpriteType type);
    u8_t readPPUStatusRegister();

    // main
    int init(ROM* rom);
    void reset();

    // cycle
    void oneCPUCycle();
    void onePPUCycle(Renderer* renderer);
    void oneAPUCycle();

    // getAssembly returns array of the assembly representation of instructions in memory
    // from addr-n ... addr+n, with total size of (2*n+1) string
    // each line represents instruction (could be 1,2 or 3 bytes of memory)
    // unknown and out of range instructions are marked with "???"
    vector<string> getAssembly(const u16_t addr, const u16_t n);
};