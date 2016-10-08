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
//#define undef_var_local   0x2064178
//#define undef_var_local   0x2064BCC
//#define undef_palEngB_2   0x5000402
//#define undef_palEngB_1   0x50005FE

// apologies for the unnamed subroutines, will fix that later
void sub_12D0();

int main(int argc, char* argv[], char* envp[])
{
    // Stack-based allocated variables
    // TODO: I don't understand these in full, please fix
    int v70; // [sp+0h] [bp-470h]@3
    int v71; // [sp+4h] [bp-46Ch]@116
    unsigned int v72; // [sp+10h] [bp-460h]@118
    int v73; // [sp+228h] [bp-248h]@3
    u8* counter;

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
    for (counter = (u8*)0x2012AB0; counter != (u8*)0x2012ACF; counter++)
        *counter -= 0x10;
    sub_52E0(0, (u8*)0x20227E4);
    *(u32*)(MAIN_MEMORY+0xC) = 0x22222222;
    *(u32*)(MAIN_MEMORY+0x8) = 0;

    //
    // a _lot_ more left to go here!
    //

    return 0;
}


// Called only by main(), assign more variables
void sub_12D0()
{
    *(u16*)IME        = 0;
    *(u32*)0x803FFC   = 0x2000AAC;
    *(u32*)IE         = 3;
    *(u32*)DISPSTAT   = -1;
    *(u16*)IF         = 0x18;
    *(u16*)IME        = 1;
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
