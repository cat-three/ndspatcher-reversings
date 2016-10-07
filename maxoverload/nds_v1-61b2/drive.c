/*
 * Collection of decompilation of certain subroutines.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef uint8_t  u8
typedef uint32_t u32
typedef uint16_t u16
typedef int8_t   s8
typedef int32_t  s32
typedef int16_t  s16

u32 sub_4EB4(u32* filename, u32* data, u8 literal))
u32 sub_4D04(u32 sturct, u8* filename)
u32 sub_3C24(u32 someStruct, u32 literal)

///////////////////////////////////////////////////////////////////
//
//  Below are just notes
//
char v7 = [sp+0x204] [bp-0x14]@2  or, sp,#0x218+var_14,  var_14
char v8 = [sp+0x4] [bp-0x214]@2   or, sp,#0x218+var_214, var_214

someStruct...
offsets probablesize  purpose
0x4     4             used for assignment; another struct?
0x8     4             modif. by checkForDrive
0xC     4             modif. by sub_3C24
0x10    4             modif. by sub_3C24
0x14    4             address for... another struct?, ret 5 if null
0x18    4             modif. by checkForDrive
0x1C    4             modif. by checkForDrive

structProperty...
0xB     1             ??
0x14    1
0x15    1
0x1A    1
0x1B    1

//
//  End "notes"
//
///////////////////////////////////////////////////////////////////

// sizeof() must be 0x44
typedef struct structA {
    // four bytes here
    u32 a;  // offset 4h
    u32 b;  // offset 8h
    u32 c;  // offset Ch
    u32 d;  // offset 10h
    u32 e;  // offset 14h
    u32 f;  // offset 18h
    u32 g;  // offset 1Ch
    // missing things here?
}

///////////////////////////////////////////////////////////////////
var_20643AC
///////////////////////////////////////////////////////////////////

//
// this is called only once, and the return value is what decides
// whether "could not find drive" is displayed on the screen or not
// specifically, a non-zero return value => "FAILED TO FIND DRIVE"

// @someStruct: stack-alloc 0x44-size (i.e., char[0x44], or someStruct)
// @arg2: =0x20643AC (a variable elsewhere)
//
// sub_5F1C
int checkForDrive(u32 someStruct, u32 arg2 = 0x20643AC)
{
    // spData is for the 0x210 bytes allocated by SP
    // first four bytes = 0x20643AC (which is an address or var, itself)
    u8 spData[0x210];
    *(u32*)spData = arg2;

    if (!sub_4EB4(&spData, (u32*)(someStruct + 4), 0))
    {
      *(u32*)(someStruct + 0x18) = &v7; // sp+#0x204
      *(u32*)(someStruct + 0x1C) = &v8; // sp+#0x4
      if (!sub_4D04(someStruct, spData))
      {
        u32 structProperty = *(u32*)(someStruct + 0x14);
        if (structProperty)
        {
          if (!(*(u8*)(structProperty + 0xB) & 0x10))
          {
            *(u8*)someStruct = *(u16*)(*(u32*)(someStruct + 4) + 6);
            return 5;
          }
          *(u32*)(someStruct + 8) =   (*(u8*)(structProperty + 0x1A) << 0)  |
                                      (*(u8*)(structProperty + 0x1B) << 8)  |
                                      (*(u8*)(structProperty + 0x14) << 16) |
                                      (*(u8*)(structProperty + 0x15) << 24);
        }
        else
        {
          u32 v5;
          if ( **(u8**)(someStruct + 4) == 3 )
            v5 = *(u32*)(*(u32*)(someStruct + 4) + 0x24);
          else
            v5 = 0;
          *(u32*)(someStruct + 8) = v5;
        }
        result = sub_3C24(someStruct, 0);
        *(u8*)someStruct = *(u16*)(*(u32*)(someStruct + 4) + 6);
        return result;
      }
      if ( result == 4 )
        result = 5;
    }
    return result;
}

// sub_4EB4
// @buffer: the allocation of size 0x210, first four bytes = 0x20643AC. R0 = SP.
// @structProperty: offset 4h on the allocation of size 0x44
// @literal: 0, in the scope of checkForDrive();
u32 sub_4EB4(u32* buffer, u32* structProperty, u8 literal))
{
    u32 spData = *buffer;
    u8 buf_char0 = (*(u8*)*buffer - 0x30) & 0xFF;
    u8 buf_char1 = *(u8*)(spData + 1);
    if (buf_char0 <= 0x09 && buf_char1 == 0x3A)
    {
        buffer += 2;
        if (buf_char0)
          return 11;
    }
    u32 var1 = *(u32*)0x20226EC;
    *structProperty = *(u32*)0x20226EC;
    if (!var1)
        return 12;
    if (*(u8*)var1)
    {
        u8 v8 = utilizeRetfunctions_2(*(u8*)(var1 + 1));
        if ( !(v8 & 1) )
        {
            if (!literal || !(v8 & 4))
                return 0;
            return 10;
        }
    }
    *(u8*)var1 = 0;
    *(u8*)(var1 + 1) = 0;
    u8 v9 = utilizeRetfunctions_1(0);
    if (v9 & 1)
        return 3;
    if (literal && (v9 & 4))
        return 10;
    s32 v10 = sub_4E10(var1, 0);
    if ( v10 == 1 )
    {
        if (!*(u8*)(var1 + 0x1F2))
            return 13;
        u32 v11 = (*(u8*)(var1 + 0x1F6) << 0)   |
                  (*(u8*)(var1 + 0x1F7) << 8)   |
                  (*(u8*)(var1 + 0x1F8) << 16)  |
                  (*(u8*)(var1 + 0x1F9) << 24);
        v10 = sub_4E10(var1, v11);
    }
    else
    {
      v11 = 0;
    }
    if ( v10 == 3 )
      return 1;
    if ( v10 || (*(u8*)(var1 + 0x3B) | (*(u8*)(var1 + 0x3C) << 8)) != 0x200 )
      return 13;
    v12 = (*(u8*)(var1 + 0x46) << 0) |
          (*(u8*)(var1 + 0x47) << 8);
    if ( !v12 )
      v12 = *(u8*)(var1 + 0x54) |
            (*(u8*)(var1 + 0x55) << 8) |
            (*(u8*)(var1 + 0x56) << 16) |
            (*(u8*)(var1 + 0x57) << 24) | ;
    *(u32*)(var1 + 0x18) = v12;
    *(u8*)(var1 + 3) = *(u8*)(var1 + 0x40);
    v13 = *(u8*)(var1 + 3) * v12;
    *(u32*)(var1 + 0x20) =  (*(u8*)(var1 + 0x3E) |
                            (*(u8*)(var1 + 0x3F) << 8)) + v11;
    *(u8*)(var1 + 2) = *(u8*)(var1 + 0x3D);
    *(u16*)(var1 + 8) = *(u8*)(var1 + 0x41) |
                        (u16)(*(u8*)(var1 + 0x42) << 8);
    v14 = (*(u8*)(var1 + 0x43)) |
          (*(u8*)(var1 + 0x44) << 8);
    if ( !v14 )
      v14 =   (*(u8*)(var1 + 0x50)) |
              (*(u8*)(var1 + 0x51) << 8) |
              (*(u8*)(var1 + 0x52) << 16) |
              (*(u8*)(var1 + 0x53) << 24);
    v15 = sub_7DC0(
            v14
          - (*(u8*)(var1 + 0x3E) | (*(u8*)(var1 + 0x3F) << 8))
          - v13
          - ((u32)*(u16*)(var1 + 8) >> 4),
            *(u8*)(var1 + 2))
        + 2;
    *(u32*)(var1 + 0x1C) = v15;
    if ( v15 > 0xFF6 )
    {
      if ( v15 > 0xFFF6 )
      {
        v16 = 3;
        goto LABEL_35;
      }
      v16 = 2;
    }
    else
    {
      v16 = 1;
    }
    if ( v16 != 3 )
    {
      *(u32*)(var1 + 0x24) = v13 + *(u32*)(var1 + 0x20);
      goto LABEL_36;
    }
    LABEL_35:
    *(u32*)(var1 + 0x24) =  (*(u8*)(var1 + 0x5C) << 0)  |
                            (*(u8*)(var1 + 0x5D) << 8)  |
                            (*(u8*)(var1 + 0x5E) << 16) |
                            (*(u8*)(var1 + 0x5F) << 24);
    LABEL_36:
    *(u32*)(var1 + 0x28) = v13
                          + *(u32*)(var1 + 0x20)
                          + ((u32)*(u16*)(var1 + 8) >> 4);
    *(u32*)(var1 + 0x10) = -1;
    *(u8*)(var1 + 4) = 0;
    if ( v16 == 3 )
    {
        v17 = (*(u8*)(var1 + 0x60) |
              (*(u8*)(var1 + 0x61) << 8)) + v11;
        *(u32*)(var1 + 0x14) = v17;
        *(u8*)(var1 + 0xA) = 0;
        if (!utilizeRetfunctions_3(*(u8*)(var1 + 1), var1 + 0x30, v17, 1)
          && (*(u8*)(var1 + 0x22E) | (*(u8*)(var1 + 0x22F) << 8)) == 0xAA55
          && (*(u8*)(var1 + 0x30) |
              (*(u8*)(var1 + 0x32) << 16) |
              (*(u8*)(var1 + 0x33) << 24) |
              (*(u8*)(var1 + 0x31) << 8)) == 0x41615252
          && (*(u8*)(var1 + 0x214) |
              (*(u8*)(var1 + 0x216) << 16) |
              (*(u8*)(var1 + 0x217) << 24) |
              (*(u8*)(var1 + 0x215) << 8)) == 0x61417272)
        {
            *(u32*)(var1 + 0xC) =   (*(u8*)(var1 + 0x21C) << 0) |
                                    (*(u8*)(var1 + 0x21D) << 8) |
                                    (*(u8*)(var1 + 0x21E) << 16) |
                                    (*(u8*)(var1 + 0x21F) << 24);
            *(u32*)(var1 + 0x10) =  (*(u8*)(var1 + 0x218) << 0) |
                                    (*(u8*)(var1 + 0x219) << 16) |
                                    (*(u8*)(var1 + 0x21A) << 8) |
                                    (*(u8*)(var1 + 0x21B) << 24);
        }
    }
    *(u32*)(var1 + 0x2C) = 0;
    *(u8*)var1 = v16;
    v18 = (v20226F0 + 1) & 0xFFFF;
    ++v20226F0;
    *(u16*)(var1 + 6) = v18;
    return 0;
}



// or, sub_messesWithFilename
u32 sub_4D04(u32 sturct, u8* filename)
{
    return 0;
}



u32 sub_3C24(u32 someStruct, u32 literal)
{
    u32 truncatedLiteral = (u16)literal;
    *(u16*)(someStruct + 2) = literal;
    if (*(u32*)(someStruct + 8) == 1)
      return 2;
    u32 structProperty = *(u32*)(someStruct + 4);
    if (*(u32*)(someStruct + 8) >= *(u32*)(structProperty + 0x1C))
      return 2;
    if (!*(u32*)(someStruct + 8))
    {
      if (*(u16*)(structProperty + 8) > truncatedLiteral)
      {
        *(u32*)(someStruct + 0x10) = *(u32*)(structProperty + 0x24) +
                                      (literal >> 4);
        *(u32*)(someStruct + 0x14) = *(u32*)(someStruct + 4) + 0x30 +
                                      0x20 * (truncatedLiteral & 0xF);
        return 0;
      }
      return 2;
    }
    u32 modifStructProp = 0x10 * *(u8*)(structProperty + 2);
    if (truncatedLiteral < modifStructProp)
    {
      *(u32*)(someStruct + 0xC) = *(u32*)(someStruct + 8);
      *(u32*)(someStruct + 0x10) = validateStructProperty(
                                      *(u32*)(someStruct + 4),
                                      *(u32*)(someStruct + 8)) +
                                          (truncatedLiteral >> 4);
      *(u32*)(someStruct + 0x14) = *(u32*)(someStruct + 4) +
                                      0x30 + 0x20 * (truncatedLiteral & 0xF);
      return 0;
    }
    while (1)
    {
      u32 retvalA = sub_3764(*(u32*)(someStruct + 4), *(u32*)(someStruct + 8));
      *(u32*)(someStruct + 8) = retvalA;
      if (retvalA == -1)
        return 1;
      if (retvalA <= 1 || retvalA >= *(u32*)(*(u32*)(someStruct + 4) + 0x1C))
        return 2;
      truncatedLiteral = (truncatedLiteral - modifStructProp) & 0xFFFF;
      if (modifStructProp > truncatedLiteral)
          *(u32*)(someStruct + 0xC) = *(u32*)(someStruct + 8);
          *(u32*)(someStruct + 0x10) = validateStructProperty(
                                        *(u32*)(someStruct + 4),
                                        *(u32*)(someStruct + 8)) +
                                            (truncatedLiteral >> 4);
          *(u32*)(someStruct + 0x14) = *(u32*)(someStruct + 4) +
                                        0x30 + 0x20 * (v4 & 0xF);
          return 0;
    }
}
