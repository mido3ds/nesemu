// #pragma once

// #include "emulation/common.h"
// #include "gui/SFMLRenderer.h"
// #include "emulation/rom.h"
// // TODO: move ppu/apu/dma things when added
// class Console_old {
// private:
//     void powerOn();
// public:
// /////////////////////////// State ///////////////////////////
//     struct {
//         uint16_t pc; // program counter
//         uint8_t sp; // stack pointer
//         uint8_t a; // accumulator
//         uint8_t x; // index
//         uint8_t y; // index
//         union {
//             struct {
//                 uint8_t c:1; // carry flag
//                 uint8_t z:1; // zero flag
//                 uint8_t i:1; // interrupt disable
//                 uint8_t d:1; // decimal mode
//                 uint8_t b:1; // break command
//                 uint8_t:1;
//                 uint8_t v:1; // overflow flag
//                 uint8_t n:1; // negative flag
//             } bits;
//             uint8_t byte;
//         } flags; // processor status
//     } regs;

//     MemType memory;
//     MemType vram;
//     array<uint8_t, 256> sprram; // sprite ram

//     uint16_t cpuCycles, ppuCycles, apuCycles;

//     union ScrollReg {
//         struct {
//             uint8_t xScroll:5;
//             uint8_t yScroll:5;
//             uint8_t bit10:1;
//             uint8_t bit11:1;
//             uint8_t yOffset:3;
//             uint8_t:1;
//         } bits;
//         uint16_t word;

//         inline void incXScroll() {bits.bit10 ^= (++bits.xScroll == 0);}
//         inline void incYScroll() {
//             if (++bits.yScroll == 30) {
//                 bits.bit11 ^= 1;
//                 bits.yScroll = 0;
//             }
//         }
//     } *scrollReg = (ScrollReg*)(memory.data() + PPU_SCRL_REG);

//     union PPUControlReg {
//         struct {
//             // reg0
//             uint8_t nameTable:2; // name table 0, 1, 2 or 3
//             uint8_t ppuIncRate:1; // 0 -> 1 or 1 -> 32
//             uint8_t spritesPattTable:1; // pattern table 0 or 1
//             uint8_t bckgPattTable:1; // pattern table 0 or 1
//             SpriteType spriteType:1;
//             uint8_t:1; // Changes PPU between master and slave modes. This is not used by the NES.
//             bool nmiEnabled:1; // whether non-maskable-interrupt should occur upon V-Blank or not

//             // reg1
//             ColorMode colorMode:1;
//             bool showLeftBckg:1; // whether to show background in the left 8 pixels or not
//             bool showLeftSprites:1; // whether to show sprites in the left 8 pixels or not
//             bool showBckg:1;  // whether to show whole background or not
//             bool showSprites:1; // whether to show whole sprites or not
//             uint8_t color:3; // background colour in monochrome mode or colour intensity in colour mode
//         } bits;
//         uint8_t reg0, reg1;

//         inline uint16_t getNameTableAddress() {return 0x2000 + bits.nameTable * 0x0400;}
//         inline uint8_t getPPUIncrementRate() {return bits.ppuIncRate ? /*vertical*/32 : /*horizontal*/1;}
//     } *controlReg = (PPUControlReg*)(memory.data() + PPU_CTRL_REG0);

//     union PPUStatusReg {
//         struct {
//             uint8_t:1;
//             uint8_t:1;
//             uint8_t:1;
//             uint8_t:1;
//             uint8_t ignoreVramWrites:1;
//             uint8_t spritesMoreThan8:1;
//             uint8_t sprite0Hit:1; // set when a non-transparent pixel of
//                                 // sprite 0 overlaps a non-transparent background pixel
//             uint8_t vblank:1; // set when V-Blank is occurring
//         } bits;
//         uint8_t byte;
//     } *ppuStatusReg = (PPUStatusReg*)(memory.data() + PPU_STS_REG);

//     // TODO listen for changes in reg
//     union JoyPadReg {
//         struct {
//             uint8_t data:1; // Reads data from joypad or causes joypad strobe when writing.
//             uint8_t:1;
//             uint8_t:1;
//             uint8_t zapperIsPointing:1;
//             uint8_t zapperIsTriggered:1;
//             uint8_t:3;
//         } bits;
//         uint8_t byte;
//     } *joypadReg0 = (JoyPadReg*)(memory.data() + 0x4016),
//     *joypadReg1 = (JoyPadReg*)(memory.data() + 0x4017);

//     struct JoyPad {
//         bool a;
//         bool b;
//         bool select;
//         bool start;
//         bool up;
//         bool down;
//         bool left;
//         bool right;
//     } joypad0, joypad1;

//     // temporary placeholder for values written in memory[VRAM_ADDR_REG1]
//     struct VRamAddrReg1 {
//         uint16_t addr;

//         // possible states, multiple ones could be combined
//         static constexpr uint8_t
//             EMPTY=0x00,  // addr is not yet initialized
//             LSN=0x01, // leaset significant nibble (4 bits) has been loaded
//             FULL=0x02, // most signifcant nibble (4 bits) has been loaded
//             CAN_READ=0x04,
//             CAN_WRITE=0x08,
//             READ_BUFFERED=0x10; // you can only read colr-palette

//         // bitset indicates the state of VRAM_ADDR_REG1 register
//         uint8_t state = EMPTY;
//     } vramAddrReg1;

// /////////////////////////// Methods ///////////////////////////

//     /*addressing modes for 6502
//     from Appendix E: http://www.nesdev.com/NESDoc.pdf
//     */
//     uint16_t zeroPageAddress(const uint8_t bb);
//     uint16_t indexedZeroPageAddress(const uint8_t bb, const uint8_t i);
//     uint16_t absoluteAddress(const uint8_t bb, const uint8_t cc);
//     uint16_t indexedAbsoluteAddress(const uint8_t bb, const uint8_t cc, const uint8_t i);
//     uint16_t indirectAddress(const uint8_t bb, const uint8_t cc);
//     uint16_t indexedIndirectAddress(const uint8_t bb, const uint8_t i);
//     uint16_t indirectIndexedAddress(const uint8_t bb, const uint8_t i);

//     // ram
//     uint8_t read(uint16_t address);
//     uint16_t read16(uint16_t address);
//     uint8_t fetch(); // read and increment pc

//     void write(uint16_t address, uint8_t value);
//     void write16(uint16_t address, uint16_t v);

//     void push(uint8_t v);
//     void push16(uint16_t v);

//     uint8_t pop();
//     uint16_t pop16();

//     // vram
//     uint8_t readVRam(uint16_t address);
//     uint16_t readVRam16(uint16_t address);

//     void writeVRam(uint16_t address, uint8_t v);
//     void writeVRam16(uint16_t address, uint16_t v);

//     uint16_t getSpriteAddr(SpriteInfo* inf, SpriteType type);
//     uint8_t readPPUStatusRegister();

//     // main
//     int init(ROM* rom);
//     void reset();

//     // cycle
//     void oneCPUCycle();
//     void onePPUCycle(SFMLRenderer* renderer);
//     void oneAPUCycle();

//     // instruction
//     uint8_t getArgValue();
//     uint16_t getArgAddr();
//     void writeArg(uint8_t v); uint8_t argValue; uint16_t argAddr; AddressMode mode;
//     void prepareArg(AddressMode mode);
//     void reprepareJMPArg();
//     void noCrossPage(); bool cpp;

//     // getAssembly returns array of the assembly representation of instructions in memory
//     // from addr-n ... addr+n, with total size of (2*n+1) string
//     // each line represents instruction (could be 1,2 or 3 bytes of memory)
//     // unknown and out of range instructions are marked with "???"
//     vector<string> getAssembly(const uint16_t addr, const uint16_t n);
// };
