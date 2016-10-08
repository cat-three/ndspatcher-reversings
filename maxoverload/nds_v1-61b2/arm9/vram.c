/*
 * Collection of decompilation of certain subroutines.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "defs.h"

typedef uint8_t  u8
typedef uint32_t u32
typedef uint16_t u16

//
// sub_10AC
//
u16* setupVram()
{
    int v0; // r7@1
    int v1; // r5@2
    u16 *result; // r0@3
    int i, j, k;

    // sub_1028
    __asm { SVC 0x50000 }; // inlined sub_FF4
    for (i = 0; i < 0x400, i += 4) {
        *(u16*)(OAM_ENGINE_B + i) = 0x200;
    }
    // end
    // sub_DA4
    for (i = 0; i < 0x40000; i += 2) {
        *(u16*)(0x201270C + i) = 0;
        *(u16*)(0x2012714 + i) = 0;
    }
    // end
    *(u8*)VRAMCNT_A = 0x81u;
    *(u32*)DISPCNT_L = 0x10901;
    *(u16*)BG0CNT = 0x87;
    *(u16*)BG3CNT = 0x4184;
    *(u32*)BG0HOFS = 0;
    *(u32*)BG3HOFS = 0;
    *(u16*)REG_BG0CNT_SUB = 0x87;
    *(u16*)REG_BG2CNT_SUB = 0x1C13;
    *(u32*)DISPCNT_SUB_L = 0x6F628E9; //&dword_111A4[0xDB] == &0x6F628E9 (bkwds)
    *(u8*)VRAMCNT_C = 0x84u;
    *(u8*)VRAMCNT_D = 0x84u;
    *(u32*)REG_BG0HOFS_SUB = 0;
    *(u32*)REG_BG1HOFS_SUB = 0;
    // sub_FF4
    __asm { SVC 0x50000 };
    // end
    sub_7BCC()
    *(u16*)PALETTE_ENGINE_B = 0x6540;
    *(u16*)0x50005FE = 0x7FFC;
    // sub_2DCC ; or modifyVram(0xFF);
    u32 byteForVram;
    for (i = 0; i < 0x300; i += 8) {
        for (j = 8; j; j--) {
            for (k = 0; k < 8; k++) {
                u32 v9 = (1 << (7 - k)) & 0xFF;
                u32 data = *(u8*)(0x2012AD2 + i*8 + (8-j));
                if (k & 1) {
                    if (data & v9)
                        byteForVram |= 0xFF << 8;
                    else
                        byteForVram = (u8)byteForVram;
                    *(u16*)0x6204000 = byteForVram;
                } else if (data & v9) {
                    byteForVram = 0xFF;
                } else {
                    byteForVram = 0;
                }
            }
        }
    }
    // end
    // sub_D74 ; or reverseMemcpy(0x2012720, 0x5000420, 0x10);
    for (i = 0; i < 0x10; i++) {
        *(u16*)(0x5000420 + i*2) = *(u16*)(0x2012720 + i*2);
    }
    // end
    // sub_E5C ; (0, 0x15, (u16*)0x20127C0, 0x10000, 0, 1, 1);
    u32 var1 = 2 * (0x10000 / 2);
    u32 var2 = 0x15 * 0x10;
    u16* writeData = (u16*)0x20127C0;
    for (i = 0; i < var2; i++) {
        if ( a7 )
          *(u16*)(0x2012714 + var1) = *writeData;
        var1 += 2;
        writeData++;
    }
    // end
    // sub_EF8 ; (0, (u16*)0x2012740, 0xD000, 0, 0, 1, 0x80008, 0xC000D, 1);
    u16* arg2 = (u16*)0x2012740;
    u32 v11 = (0x80008 >> 0x10) & 0xFFFF;
    u16* v12 = (u16*)((u16)0x80008 * v11);
    u32 v13 = ((0xC000D >> 0x10) & 0xFFFF) + (0xD000 / 2) + (0x20 * 0xD);

    v14 = a2;
    v15 = 0;
    v16 = 0;
    arg2 = 0;
    for (i = 0; i != v12; i++) {
        *(u16*)(0x2012714 + (v13 + 0x20 * v16 + v15) * 2) = (*v14) | (1 << 12);
        if ( v11 == ++v15 )
        {
          ++v16;
          v15 = 0;
        }
        arg2 = (u16*)((u8*)arg2 + 1);
        ++v14;
    }
    // end
    *(u16*)0x5000402 = 0x6D80; // (u32)&dword_6D80; is addr, but value=0x40001A1
    *(u16*)0x5000404 = 0x5D00;
    // sub_E5C ; (0x16, 9, (_WORD *)0x20125A0, 0x10000, 0, 1, 1);
    u32 var1 = 2 * ((0x10000 / 2) + (0x16 * 0x10));
    u32 var2 = 0x9 * 0x10;
    u16* writeData = (u16*)0x20125A0;
    for (i = 0; i < var2; i++) {
        *(u16*)(0x2012714 + var1) = *writeData;
        var1 += 2;
        writeData++;
    }
    // end
    for (i = 0; i < 0x20; i += 4) {
        for (j = 0; j < 0x20; j += 4) {
            //sub_EF8 ; (0x16, (u16*)0x20126C0, 0xE000, 0, 0, 1,
            //    0x40004, i | (j << 0x16), 0);
            u32 arg1 = 0x16;
            u16 *arg2 = 0x20126C0;
            u32 v11 = (0x40004 >> 16) & 0xFFFF;
            u16 *v12 = (u16*)(0x4 * v11);
            u32 v13 = ((i | (j << 0x16) >> 0x10) & 0xFFFF) + (0xE000 / 2) +
                0x20 * (u16)(i | (j << 0x16));

            v14 = arg2;
            v15 = 0;
            v16 = 0;
            arg2 = 0;
            do
            {
              *(u16*)(2 * (v13 + 0x20 * v16 + v15) + 0x2012714) = (*v14 + 0x16);
              if ( v11 == ++v15 )
              {
                ++v16;
                v15 = 0;
              }
              arg2 = (u16*)((u8*)arg2 + 1);
              ++v14;
            }
            while ( v12 != arg2 );
            // end
        }
    }
    return result;
}


//
// The following three are called by sub_10AC
// called "setupVram" in my labels
// and that is called by __main_maybe (sub_2110)
//
u32 sub_7BCC()
{
    __asm { SVC 0x50000 };

    sub_7B54((u16*)0x6004000, (u16*)0x2013574, 0x39A0);
    sub_7B54((u16*)BG_VRAM_ENGINE_A, (u16*)0x2012F74, 0x300);
    sub_7B54((u16*)PALETTE_ENGINE_A, (u16*)0x201A8B4, 0x100);
    sub_7B54((u16*)PALETTE_ENGINE_B, (u16*)0x201A8B4, 0x100);
    sub_7B9C((u16*)0x6000800, 0, 0x400);

    u16* v0 = (u16*)0x2012DE4;
    u16* v1 = (u16*)0x60008C6;
    u16* retval;
    do {
        retval = sub_7B54(v1, v0, 0xA);
        v1 += 16;
        v0 += 10;
    } while (v1 != (u16*)0x6000B46);
    return retval;
}
void sub_7B54(u32* retval, u16* a2, u16 a3)
{
    u32 v3;
    u16 v4;
    if (!a3) return retval;
    v3 = 0;
    do {
        v4 = *a2;
        a2++;
        *retval = v4;
        ++retval;
        ++v3;
    } while (a3 != v3);
    return retval;
}
void sub_VB9C(u16* retval, u16 a2, u32 a3)
{
    u32 v3 = 0;
    if (!a3) return retval;
    do {
      *retval = a2;
      ++retval;
      ++v3;
    } while (a3 != v3);
    return result;
}

////
////
////
////
