/*
 * main.c
 * contains the main() entrypoint used in Maxoverload v1.61b2
 * refs:
 * http://libnds.devkitpro.org/group__sub__display__registers.html
 * http://dualis.1emu.net/dsti.html
 * http://problemkaputt.de/gbatek.htm
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

typedef uint8_t  8
typedef uint16_t 16
typedef uint32_t 32

//#define 0x803FFC
#define MAIN_MEMORY 0x2000000   // Main memory, for both ARM9 and ARM7
// The following definitions are specific to ARM9
#define DISPSTAT    0x4000004   // General LCD Status (Read/Write)
#define IPCSYNC     0x4000180   // IPC Synchronize Register (R/W)
#define IPCFIFOCNT  0x4000184   // IPC Fifo Control Register (R/W)
#define EXMEMCNT    0x4000204   // External Memory Control (R/W)
#define IME         0x4000208   // Interrupt Master Enable (R/W)
#define IE          0x4000210   // Interrupt Enable (R/W)
#define IF          0x4000214   // Interrupt Request Flags (R/W)
#define POWCNT1     0x4000304   // Graphics Power Control Register (R/W)
#define SIZEOFMODAT 0x10C       // reported size of MODEV!L.DAT file
#define SIZEOFMOLST 0x40740     // reported size of MODEV!L.LST file

// apologies for the unnamed subroutines, will fix that later
void sub_FFC(u32 r0, u32 r1);
void sub_12D0();
void sub_13E8(u32 a1, u32 a2); // not implemented
void sub_189C();
u32  sub_1D68(u32 a1); // not implemented
void sub_20A0();
void sub_52E0(u8 literal, u8* data);
u32  sub_5320(u32 *a1, const char* filename, char literal); // not implemented
u32  sub_5580(); // not implemented
u32  sub_5808(u32 a1, u8* a2, u32 a3, u32* a4); // not implemented
u32  sub_5C04(u32* addr);
void sub_5C20(u32* a1, u32 a2); // not implemented
u32  sub_5AEC(); // not implemented
u32  sub_7ED4(u32 a1, u32 a2); // not implemented
void sub_818C(u32 a1, char* a2, u32 a3); // not implemented
u32  sub_823C(u32 a1, u32 a2, u32 a3); // not implemented
u32  sub_859C(u8* a1, u8* a2);

int main(int argc, char* argv[], char* envp[])
{
    // Stack-based allocated variables, not yet analyzed
    // total is 0x44C bytes
    u32 v70;            // [sp+0h] [bp-470h]@3
    u8* v71[0xC];       // [sp+4h] [bp-46Ch]@116
    u8* v72[0x218];     // [sp+10h] [bp-460h]@118
    u8* v73[0x240];     // [sp+228h] [bp-248h]@3
    u32 counter, u8* index;

    *(u16*)IPCSYNC    = 0;
    *(u16*)IPCFIFOCNT = 0;
    *(u16*)EXMEMCNT   &= 0x4008;
    *(u16*)POWCNT1    = 0x8203;
    *(u32*)0x2064178  = 0;
    *(u16*)0x5000402  = 0x7FFF;
    *(u16*)0x50005FE  = 0x7FFF;
    sub_12D0();
    // setupVram(); is sub_10AC, which will be implemented in vram.h/c later
    // though, I theorize that's simply a libnds init function from years back
    for (index = (u8*)0x2012AB0; index != (u8*)0x2012ACF; index++)
        *index -= 0x10;
    sub_52E0(0, (u8*)0x20227E4);
    *(u32*)(MAIN_MEMORY+0xC) = 0x22222222;
    *(u32*)(MAIN_MEMORY+0x8) = 0;

    int v70 = sub_5320(&v73, "MODEV!L.DAT", 19);
    if (!v70)
    {
        sub_5580((int)&v73, (u32*)0x2064ACC, SIZEOFMODAT, &v70);
        *(u32*)(MAIN_MEMORY+0x8) = *(u32*)0x2064BCC & 1;
        sub_818C(0x2064BD8, (char *)0x2064ACC, SIZEOFMODAT);
    }
    v70 = sub_5320((int *)0x2064188, "MODEV!L.LST", 3);
    if (!v70)
    {
        if (*(u32*)0x2064194 == SIZEOFMOLST)
        {
            sub_5580(0x2064188, 0x2022A20, SIZEOFMOLST, &v70);
            *(u32*)0x20225D0 = 1;
        }
        else
        {
            *(u32*)0x20225D0 = 0;
            v70 = -1;
        }
        sub_5C04((u32*)0x2064188);
    }
    sub_823C((u32*)0x20643AC, 0, 0x3E8u);
    *(u32*)0x2063164 = 0;
    *(u32*)0x2064ABC = 0;
    *(u32*)0x2063170 = 0;
    if (*(u32*)0x20225D0 && *(u16*)0x20225D4 != 2)
    {
        *(u32*)0x2064180 = sub_189C();
        *(u32*)0x2063170 = *(u32*)0x2064AB4;
    }
    else
    {
        for (counter = 0; counter < 0x101D0; counter++)
            *(u32*)(0x2022A20 + counter) = 0;
        
        main_sub(0x20643AC);
        *(u32*)0x2064180 = *(u32*)0x2063164;
        *(u32*)0x2063170 = *(u32*)0x2064AB4;
        v70 = sub_5320((u32*)0x2064188, "MODEV!L.LST", 19);// 
        if (!v70)
        {
            sub_5C20((u32*)0x2064188, 0);
            sub_5808(0x2064188, (char *)0x2022A20, SIZEOFMOLST, &v70);
            sub_5C04((u32*)0x2064188);
        }
    }
    sub_20A0();
    *(u32*)0x2000008 = *(u32*)0x2064BCC & 1;

    // this is likely wrong. if you know about armv4/armv5 swi's, 
    // please talk to me, even if it means opening up an issue.
    u32 retval = sub_7ED4(0x6400, *(u32*)0x2064180);
    *(u32*)0x2022A14 = 0;
    ((void (__cdecl *)(int)) __asm { svc 0x50000 } )(retval);
    
    sub_13E8(*(u32*)0x2064AB4, *(u32*)0x2064180 - 1);
    if (*(u32*)0x2064180)
        writeBackgroundControlRegs();
    
    for (counter = 0; counter < 0x72F0; counter++)
        *(u8*)(0x201AAB4+counter) ^= counter;

    *(u32*)0x2064178 = 1;
    if (*(u16*)0x20225D4 == 8)
    {
        *(u32*)0x2064BD4 = 0;
        while (*(u16*)0x20225D4 == 8);  // semaphore, or infloop
    }

    //
    // a _lot_ more left to go here!
    //


    sub_FFC(0x23D0000, 0x72F0);
    sub_FFC(0x2000000, 0x200);
    *(u16*)IME        = 0;
    *(u32*)0x803FFC   = 0;
    *(u32*)IE         = 3;
    *(u32*)DISPSTAT   = -1;
    *(u16*)IF         = 0x18;
    *(u32*)0x23FFE24  = (u32*)0x23D0000;
    return 0;
}

// mcrCalls
void sub_FFC(u32 r0, u32 r1) {
    __asm {
        mov r12, #0
        add r1, r1, r0
        bic r0, r0, #0x1F

        loop:
            p15, 0, r12, c7, c10, 4
            p15, 0, r0, c7, c14, 1
            add r0, r0, #0x20
            cmp r0, r1
            blt loop
            @ bx lr
    }
}

// Called only by main(), assign more variables
void sub_12D0()
{
    *(u16*)IME        = 0;
    *(u32*)0x803FFC   = 0x2000AAC;      // assigned only here
    *(u32*)IE         = 3;
    *(u32*)IF         = -1;
    *(u16*)DISPSTAT   = 0x18;
    *(u16*)IME        = 1;
}

void sub_13E8(u32 a1, u32 a2)
{
}

// There is a loop of 200 - this perhaps allocates space for the NTR Header
void sub_189C() 
{ 
    u32 counter;

    for (index = 0; index < 200; index++) {
        u32 src = 0x2022A20 + (counter * 0x528);

        if (!sub_859C((u8*)src, (u8*)0x2064ACC) )
            *(u32*)0x2064AB4 = *(u32*)(src + 0x524);
        if (!*(u32*)(src))
            break;
    }

    return 200;
}

u32 sub_1D68(u32 a1)
{
    return 0;
}

// uses global variables only, doSomethingWithVars(), meaning unsure yet
void sub_20A0()
{
    u32 i, j;

    u32  length = *(u32*)0x2064180;
    u32 *dest   = (u32*)0x2064794;
    u32 *cond   = (u32*)0x2022A20;

    for (i = 0; i != length; i++)
    {
        for (j = 0; j != length; j++) {
            if (*(u32*)(cond + (0x528*j) + 0x524) == i)
                *(u32*)(dest + i) = j;
        }
    }
}

// Called only by main(), if cond's are met, assigns @data to *(u32*)0x20226EC
void sub_52E0(u8 literal, u8* data)
{
    if (literal) return 11;     // this never actually returns
    if (*(u32*)0x20226EC)
      *(u8*)0x20226EC = 0;
    if (data)
      *data = 0;
    *(u32*)0x20226EC = data;

    return 0;
}

// Called by many subroutines; is a.k.a. "fileOperations"
int sub_5320(int *a1, const char* filename, char literal)
{
    return 0;
}

u32 sub_5580()
{
    return 0;
}

u32 sub_5808(u32 a1, u8* a2, u32 a3, u32* a4)
{
    return 0;
}

u32 sub_5C04(u32* addr)
{
    u32 result = sub_5AEC(addr);
    if ((!result))
        *addr = result;
    return result;
}

void sub_5C20(u32* a1, u32 a2)
{
}

u32 sub_5AEC()
{
    return 0;
}

// called by main() only, series of bitwise operations
u32 sub_7ED4(u32 a1, u32 a2) 
{
    return 0;
}

void sub_818C(u32 a1, char* a2, u32 a3)
{
}

u32  sub_823C(u32 a1, u32 a2, u32 a3)
{
    return 0;
}

u32  sub_859C(u8* offsetA, u8* offsetB)
{
    // Incomplete, requires revision later

    // keep parameters unchanged
    u8* offsetA_copy = offsetA;
    u8* offsetB_copy = offsetB;

    if (((u32)offsetA | (u32)offsetB) & 3)
    {
        u8 valAtOffsetA = *offsetA;
        if (valAtOffsetA)
        {
            if (*offsetB != valAtOffsetA)
                return valAtOffsetA - *offsetB;
            u32 index = 0;
            while (1)
            {
                // comparing the two by the following byte in each
                valAtOffsetA = *(offsetA + index + 1);
                offsetB++;
                if (!valAtOffsetA)
                    break;
                if (*offsetB != valAtOffsetA)
                    return valAtOffsetA - *offsetB;
                index++:
            }
        }
        return valAtOffsetA - *offsetB;
    }
    if (*(u32*)offsetA != *(u32*)offsetB)
    {        
        // same code block, label repeat
        u8 valAtOffsetA = *offsetA;
        if (valAtOffsetA)
        {
            if (*offsetB != valAtOffsetA)
                return valAtOffsetA - *offsetB;
            u32 index = 0;
            while (1)
            {
                // comparing the two by the following byte in each
                valAtOffsetA = *(offsetA + index + 1);
                offsetB++;
                if (!valAtOffsetA)
                    break;
                if (*offsetB != valAtOffsetA)
                    return valAtOffsetA - *offsetB;
                index++:
            }
        }
        return valAtOffsetA - *offsetB;
    }
    if (!((*(u32*)offsetA - 0x1010101) & ~*(u32*)offsetA & 0x80808080))
    {
        v3 = a2;
        while (1)
        {
            v10 = *((u32*)offsetB + 1);
            offsetB 4;
            
            v11 = ~v10 & (v10 - 0x1010101) & 0x80FFFFFF;
            v12 = *(u32*)(offsetA + 4);
            offsetA += 4;
            v13 = v11 & 0xFF808080;
            if ( v10 != v12 )
               break;
            if ( v13 )
                return 0;
        }
            
        // same code block, label repeat
        u8 valAtOffsetA = *offsetA;
        if (valAtOffsetA)
        {
            if (*offsetB != valAtOffsetA)
                return valAtOffsetA - *offsetB;
            u32 index = 0;
            while (1)
            {
                // comparing the two by the following byte in each
                valAtOffsetA = *(offsetA + index + 1);
                offsetB++;
                if (!valAtOffsetA)
                    break;
                if (*offsetB != valAtOffsetA)
                    return valAtOffsetA - *offsetB;
                index++:
            }
        }
        return valAtOffsetA - *offsetB;
    }
    return 0;
}