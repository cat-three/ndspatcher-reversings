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
void sub_189C(); // not implemented
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
    *(u32*)DISPSTAT   = -1;
    *(u16*)IF         = 0x18;
    *(u16*)IME        = 1;
}

void sub_13E8(u32 a1, u32 a2)
{
}

void sub_189C() 
{ 
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
    // v3 = data?;
    // filename_ = (u8*)filename;
    // literal_ = literal;
    // *data? = 0;
    // v5 = literal & 0x1F;
    // result = modifyStruct(&filename_, &v19, literal & 0x1E);
    // if ( result )
    // return result;
    // v21 = &v17;
    // v22 = &v16;
    // result = messesWithFilename((int)&v18, filename_);
    // if (!(literal_ & 0x1C))
    // {
    //     if ( result )
    //         return result;
    //     v8 = v20;
    //     if ( !v20 || *(u8*)(v20 + 11) & 0x10 )
    //         return 4;
    //     if ( !(v5 & 2) || !(*(u8*)(v20 + 11) & 1) )
    //         goto LABEL_23;
    //     return 7;
    // }
    // if ( !result )
    // {
    //     if ( v5 & 4 )
    //         return 8;
    //     v8 = v20;
    //     if ( v20 && !(*(u8*)(v20 + 11) & 0x11) )
    //     {
    //         if ( !(v5 & 8) )
    //             goto LABEL_23;
    //         v9 = *(u8*)(v20 + 20) | (*(u8*)(v20 + 21) << 8);
    //         v10 = *(u8*)(v20 + 26) | (*(u8*)(v20 + 27) << 8);
    //         *(u8*)(v20 + 20) = 0;
    //         *(u8*)(v8 + 21) = 0;
    //         *(u8*)(v8 + 26) = 0;
    //         *(u8*)(v8 + 27) = 0;
    //         *(u8*)(v8 + 28) = 0;
    //         *(u8*)(v8 + 29) = 0;
    //         *(u8*)(v8 + 30) = 0;
    //         *(u8*)(v8 + 31) = 0;
    //         *(u8*)(v19 + 4) = 1;
    //         v11 = *(u32*)(v19 + 44);
    //         v12 = v10 | (v9 << 16);
    //         if ( v10 | (v9 << 16) )
    //         {
    //             result = sub_3A58(v19, v10 | (v9 << 16));
    //             if ( result )
    //                 return result;
    //             *(u32*)(v19 + 12) = v12 - 1;
    //         }
    //         result = utilize_utilizeRetFunctions1(v19, v11);
    //         if ( !result )
    //             goto LABEL_17;
    //         return result;
    //     }
    //     return 7;
    // }
    // if ( result == 4 )
    // {
    //     result = sub_44C0((int)&v18);
    //     if ( !result )
    //     {
    //         v7 = v5 | 8;
    //         v5 = (v5 | 8) & 0xFF;
    //         v8 = v20;
    //         if ( v7 & 8 )
    //         {
    //         LABEL_17:
    //             *(u8*)(v8 + 0xB) = 0;
    //             &(0x34C630C3) = return0x34C630C3();
    //             *(u8*)(v8 + 0xE) = &(0x34C630C3);
    //             *(u8*)(v8 + 0xF) = &(0x34C630C3) << 16 >> 0x18;
    //             *(u8*)(v8 + 0x10) = &(0x34C630C3) >> 0x10;
    //             *(u8*)(v8 + 0x11) = BYTE3(&(0x34C630C3));
    //             *(u8*)(v19 + 4) = 1;
    //             LOBYTE(v5) = v5 | 0x20;
    //         }
    //         LABEL_23:
    //         v14 = v19;
    //         v3[7] = *(u32*)(v19 + 44);
    //         v3[8] = v20;
    //         *((u8*)v3 + 6) = v5;
    //         v3[4] = *(u8*)(v8 + 26) | (*(u8*)(v8 + 27) << 8) | ((*(u8*)(v8 + 20) | (*(u8*)(v8 + 21) << 8)) << 16);
    //         v3[3] = *(u8*)(v8 + 28) | (*(u8*)(v8 + 30) << 16) | (*(u8*)(v8 + 31) << 24) | (*(u8*)(v8 + 29) << 8);
    //         v3[2] = 0;
    //         *((u8*)v3 + 7) = -1;
    //         v3[6] = 0;
    //         *v3 = v14;
    //         *((u16*)v3 + 2) = *(u16*)(v14 + 6);
    //         return 0;
    //     }
    // }
    // return result;
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