// VRAM Stuff, refs:
// http://libnds.devkitpro.org/group__sub__display__registers.html
// http://dualis.1emu.net/dsti.html
// http://problemkaputt.de/gbatek.htm

#define PALETTE_ENGINE_A  0x5000000
#define PALETTE_ENGINE_B  0x5000400
#define BG_VRAM_ENGINE_A  0x6000000
#define BG_VRAM_ENGINE_B  0x6200000
#define OBJ_VRAM_ENGINE_A 0x6400000
#define OBJ_VRAM_ENGINE_B 0x6600000
#define OAM_ENGINE_A      0x7000000
#define OAM_ENGINE_B      0x7000400
#define OAM_ENGINE_B_END  0x7000800

#define undef_VRAM_var0   0x6000800
#define MAIN_BG_somebank  0x6004000
#define undef_VRAM_var2   0x60008C6
#define undef_VRAM_var3   0x6000B46
#define underf            0x6204000
#define undef_palEngB_1   0x50005FE
#define undef_palEngB_2   0x5000402
#define undef_palEngB_3   0x5000404

#define VRAMCNT_A         0x4000240 // VRAM Control A
#define VRAMCNT_B         0x4000241 // VRAM Control B
#define VRAMCNT_C         0x4000242 // VRAM Control C
#define VRAMCNT_D         0x4000243 // VRAM Control D
#define DISPCNT_L         0x4000000 // Display Control, lower 16 bits
#define DISPCNT_H         0x4000002 // Display Control, upper 16 bits
#define DISPCNT_SUB_L     0x4001000 // Sub Display Control, lower 16 bits
#define DISPCNT_SUB_H     0x4001002 // Sub Display Control, upper 16 bits
#define BG0CNT            0x4000008 // BG 0 Control
#define BG3CNT            0x400000E // BG 3 Control
#define BG0HOFS           0x4000010 // BG0 X-Offset
#define BG3HOFS           0x400001C // BG3 X-Offset

#define REG_BG0CNT_SUB    0x4001008 // or, BGCTRL_SUB, BACKGROUND_SUB
#define REG_BG2CNT_SUB    0x400100C
#define REG_BG0HOFS_SUB   0x4001010
#define REG_BG1HOFS_SUB   0x4001014

// // sub: setupVramStuff
// var_2012720
// var_2012740
// var_20127C0
// var_20125A0
// var_20126C0
//
// // sub: clearMemAt0x201270CAnd...
// var_201270C
// var_2012714
//
// // sub_7BCC
// var_2013574
// var_2012F74
// var_201A8B4
// var_2012DE4
//
// // sub_2DCC *modifyVram
// var_2012AD2
// var_2012DD2 (this may just be the end of the above, though)

// GBA side things
#define SOMETHING   0x8000000
#define SOMETHING   0x8020000
#define SOMETHING   0x8040000
#define SOMETHING   0x8060000
#define SOMETHING   0x8080000
#define SOMETHING   0x80A0000
#define SOMETHING   0x80C0000
#define SOMETHING   0x80E0000

#define SOMETHING   0x8000000
