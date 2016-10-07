#include "arm7.h"
#include "arm7defs.h"

//
// Analyze the ARM7 section of the NDS image, filesize of image being @fsize
//
int analyzeArm7(u8* ndsData, u32 fsize, FILE* patFp)
{
    u32 arm7offset    = *(u32*)&ndsData[0x30];
    u32 arm7entryAddr = *(u32*)&ndsData[0x34];
    u32 arm7ramAddr   = *(u32*)&ndsData[0x38];
    u32 arm7size      = *(u32*)&ndsData[0x3C];

    printf("\nArm7 (patch v2.0)\n");
    printf("Rom Offset:\t%08X\n", arm7offset);
    printf("Entry Addr:\t%08X\n", arm7entryAddr);
    printf("Ram Addr:\t%08X\n", arm7ramAddr);
    printf("Rom Size:\t%08X\n", arm7size);

    u32 alignedA7size = (arm7size + 511) & 0xFFFFFE00;
    u8 *virtualNdsData = malloc(0x400000u);
    memset(virtualNdsData, 0xAE, 0x400000u);
    u8 *arm7data = &virtualNdsData[(arm7ramAddr - 0x2000000)];
    memcpy(arm7data, &ndsData[arm7offset], alignedA7size);

    // Find the relocation signature
    u32 relocationStart = getOffsetA7(arm7data, alignedA7size,
        relocateStartSignature, 4, 1);
    if (!relocationStart) {
        printf("Relocation start now found\n");
        free(virtualNdsData); return 0;
    }

    // Validate the relocation signature
    u32 forwardedRelocStartAddr = relocationStart + arm7ramAddr + 4;
    if (!*(u32*)&virtualNdsData[forwardedRelocStartAddr - 0x2000000])
        forwardedRelocStartAddr += 4;
    u32 vAddrOfRelocSrc =
        *(u32*)&virtualNdsData[forwardedRelocStartAddr - 0x2000000 + 8];
    // sanity checks
    u32 relocationCheck1 =
        *(u32*)&virtualNdsData[forwardedRelocStartAddr - 0x2000000 + 0xC];
    u32 relocationCheck2 =
        *(u32*)&virtualNdsData[forwardedRelocStartAddr - 0x2000000 + 0x10];
    if ( vAddrOfRelocSrc != relocationCheck1
      || vAddrOfRelocSrc != relocationCheck2) {
        printf("Error in relocation checking\n");
        free(virtualNdsData); return 0;
    }

    // Get the remaining details regarding relocation
    u32 valueAtRelocStart =
        *(u32*)&virtualNdsData[forwardedRelocStartAddr - 0x2000000];
    u32 relocDestAtSharedMem =
        *(u32*)&virtualNdsData[valueAtRelocStart - 0x2000000];
    if (relocDestAtSharedMem != 0x37F8000) { // shared memory in RAM
        // Try again
        vAddrOfRelocSrc +=
            *(u32*)&virtualNdsData[valueAtRelocStart - 0x2000000 + 4];
        relocDestAtSharedMem =
            *(u32*)&virtualNdsData[valueAtRelocStart - 0x2000000 + 0xC];
        if (relocDestAtSharedMem != 0x37F8000) {
            printf("Error in finding shared memory relocation area\n");
            free(virtualNdsData); return 0;
        }
    }
    printf("Relocation src:\t%08X\n", vAddrOfRelocSrc);
    printf("Relocation dst:\t%08X\n", relocDestAtSharedMem);

    // I'm... not sure what to call the rest of this yet :/
    // define 0x18000 as a magic constant somehow
    u8* allocMem = (u8*)malloc(0x18000u);
    memset(allocMem, 0xAEu, 0x18000u);
    memcpy(allocMem, &virtualNdsData[vAddrOfRelocSrc - 0x2000000],
      4 * ((arm7ramAddr - vAddrOfRelocSrc + 0x18000) >> 2)
        + (((u8)arm7ramAddr - (u8)vAddrOfRelocSrc) & 3));

    // Find the card read
    u32 cardReadEndAddr =
        getOffsetA7(allocMem, 0x18000, a7cardReadSignature, 8, 1);
    if (!cardReadEndAddr) {
        printf("[Error!] Card read addr not found\n");
        free(allocMem); free(virtualNdsData); return 0;
    }


    // nonsense variable names below
    u32 someWramAddr = *(u32*)&allocMem[cardReadEndAddr - 4];
    u32 addrOfSomething_13C =
         getOffsetA7(&allocMem[cardReadEndAddr],
             0x18000 - cardReadEndAddr, &someWramAddr, 4u, 1);

    if (!addrOfSomething_13C)
    {
        printf("[Error!] ___ addr not found\n");
        free(allocMem); free(virtualNdsData); return 0;
    }
    u32 addrOfSomething_7F1C = addrOfSomething_13C + cardReadEndAddr + 4;

    u32 returned_A0_with_MKDS =
        getOffsetA7(&allocMem[addrOfSomething_7F1C], 0x100,
            (void*)a7something1Signature, 8, 1);
    if (!returned_A0_with_MKDS) {
        printf("[Error!]...\n");
        free(allocMem); free(virtualNdsData); return 0;
    }

    u32 addrOfSomething_85C0 =
        getOffsetA7(allocMem, 0x18000,
            (void*)a7something2Signature, 8u, 1);
    if ( !addrOfSomething_85C0 )
    {
        printf("[Error!] ...\n");
        free(allocMem); free(virtualNdsData); return 0;
    }

    u32 anotherLocinA7WRAM = *(u32*)&allocMem[addrOfSomething_85C0 - 4];

    u32 amal_8CBC = returned_A0_with_MKDS + addrOfSomething_7F1C;

    // These aren't used anywhere else. Legitimately unused variables?
    u32 a7wram_loc1_dword_424510 = someWramAddr;
    u32 a7wram_loc2_dword_424508 = anotherLocinA7WRAM - 4;
    u32 a7wram_loc3_dword_42450C = anotherLocinA7WRAM + 12;

    // no, no idea what this is yet
    // and no idea how to cleanly fix this warning yet.
    // but it should be (in MKDS), 0x7F54
    u8* aFinalLocation =
        (u8*)(amal_8CBC
        + 4 * (*(u32*)&allocMem[amal_8CBC + 0x38] & 0xFFFFFF)
        + 0x48
        + 4 * (*(u32*)(&allocMem[
                4 * (*(u32*)&allocMem[amal_8CBC + 0x38] & 0xFFFFFF) + 0x48
                ] + amal_8CBC) | 0xFF000000
              )
        + 8);

    u8* buffer = &aFinalLocation[vAddrOfRelocSrc];
    u32 four = 4; u8 vals[4] = {0x90,0x03,0x00,0x00};
    fwrite(&buffer, sizeof(u8), 4, patFp);
    fwrite((void*)vals, sizeof(u8), 4, patFp);
    fwrite(unk_4244DC, sizeof(u8), 0x390, patFp);

    u32 currentOffset =
        amal_8CBC + 4 * (*(u32*)&allocMem[amal_8CBC + 0x14] & 0xFFFFFF) + 0x1C;
    u32 v26 = vAddrOfRelocSrc + currentOffset;
    u32 somea7instr = generateA7Instr(currentOffset, *(u32*)&aFinalLocation);
    printf("Eeprom protect:\t\t%08X (%08X)\n",
        relocDestAtSharedMem + currentOffset, v26);
    fwrite(&v26, sizeof(u8), 4, patFp);
    fwrite(&four, sizeof(u8), 4, patFp);
    fwrite(&somea7instr, sizeof(u8), 4, patFp);

    currentOffset =
        amal_8CBC + 4 * (*(u32*)&allocMem[amal_8CBC + 0x18] & 0xFFFFFF) + 0x20;
    v26 = vAddrOfRelocSrc + currentOffset;
    aFinalLocation+=0x4;
    somea7instr = generateA7Instr(currentOffset, *(u32*)&aFinalLocation);
    printf("Card id:\t\t%08X (%08X)\n",
        relocDestAtSharedMem + currentOffset, v26);
    fwrite(&v26, sizeof(u8), 4, patFp);
    fwrite(&four, sizeof(u8), 4, patFp);
    fwrite(&somea7instr, sizeof(u8), 4, patFp);

    currentOffset =
        amal_8CBC + 4 * (*(u32*)&allocMem[amal_8CBC + 0x1C] & 0xFFFFFF) + 0x34;
    v26 = vAddrOfRelocSrc + currentOffset;
    aFinalLocation+=0x6;
    somea7instr = generateA7Instr(currentOffset, *(u32*)&aFinalLocation);
    printf("Card read:\t\t%08X (%08X)\n",
        relocDestAtSharedMem + currentOffset, v26);
    fwrite(&v26, sizeof(u8), 4, patFp);
    fwrite(&four, sizeof(u8), 4, patFp);
    fwrite(&somea7instr, sizeof(u8), 4, patFp);

    currentOffset =
        amal_8CBC + 4 * (*(u32*)&allocMem[amal_8CBC + 0x24] & 0xFFFFFF) + 0x3C;
    v26 = vAddrOfRelocSrc + currentOffset;
    aFinalLocation+=0x2;
    somea7instr = generateA7Instr(currentOffset, *(u32*)&aFinalLocation);
    printf("Eeprom read:\t\t%08X (%08X)\n",
        relocDestAtSharedMem + currentOffset, v26);
    fwrite(&v26, sizeof(u8), 4, patFp);
    fwrite(&four, sizeof(u8), 4, patFp);
    fwrite(&somea7instr, sizeof(u8), 4, patFp);

    currentOffset =
        amal_8CBC + 4 * (*(u32*)&allocMem[amal_8CBC + 0x28] & 0xFFFFFF) + 0x40;
    v26 = vAddrOfRelocSrc + currentOffset;
    aFinalLocation+=0x4;
    somea7instr = generateA7Instr(currentOffset, *(u32*)&aFinalLocation);
    printf("Eeprom page write:\t%08X (%08X)\n",
        relocDestAtSharedMem + currentOffset, v26);
    fwrite(&v26, sizeof(u8), 4, patFp);
    fwrite(&four, sizeof(u8), 4, patFp);
    fwrite(&somea7instr, sizeof(u8), 4, patFp);

    currentOffset =
        amal_8CBC + 4 * (*(u32*)&allocMem[amal_8CBC + 0x2C] & 0xFFFFFF) + 0x44;
    v26 = vAddrOfRelocSrc + currentOffset;
    aFinalLocation+=0x4;
    somea7instr = generateA7Instr(currentOffset, *(u32*)&aFinalLocation);
    printf("Eeprom page prog:\t%08X (%08X)\n",
        relocDestAtSharedMem + currentOffset, v26);
    fwrite(&v26, sizeof(u8), 4, patFp);
    fwrite(&four, sizeof(u8), 4, patFp);
    fwrite(&somea7instr, sizeof(u8), 4, patFp);

    currentOffset =
        amal_8CBC + 4 * (*(u32*)&allocMem[amal_8CBC + 0x30] & 0xFFFFFF) + 0x48;
    v26 = vAddrOfRelocSrc + currentOffset;
    aFinalLocation+=0x4;
    somea7instr = generateA7Instr(currentOffset, *(u32*)&aFinalLocation);
    printf("Eeprom verify:\t\t%08X (%08X)\n",
        relocDestAtSharedMem + currentOffset, v26);
    fwrite(&v26, sizeof(u8), 4, patFp);
    fwrite(&four, sizeof(u8), 4, patFp);
    fwrite(&somea7instr, sizeof(u8), 4, patFp);

    currentOffset =
        amal_8CBC + 4 * (*(u32*)&allocMem[amal_8CBC + 0x38] & 0xFFFFFF) + 0x48;
    v26 = vAddrOfRelocSrc + currentOffset;
    aFinalLocation+=0x4;
    somea7instr = generateA7Instr(currentOffset, *(u32*)&aFinalLocation);
    printf("Eeprom page erase:\t%08X (%08X)\n",
        relocDestAtSharedMem + currentOffset, v26);
    fwrite(&v26, sizeof(u8), 4, patFp);
    fwrite(&four, sizeof(u8), 4, patFp);
    fwrite(&somea7instr, sizeof(u8), 4, patFp);

    printf("Arm7 patched!\n");

    free(allocMem);
    free(virtualNdsData);
    return 1;
}

//
// In the event of the above function failing, tries again differently
//
int retryAnalyzeArm7(u8* ndsData, u32 fsize, FILE* patFp)
{
    u32 arm7offset    = *(u32*)&ndsData[0x30];
    u32 arm7entryAddr = *(u32*)&ndsData[0x34];
    u32 arm7ramAddr   = *(u32*)&ndsData[0x38];
    u32 arm7size      = *(u32*)&ndsData[0x3C];

    printf("\nArm7 (patch v1.0)\n");
    printf("Rom Offset:\t%08X\n", arm7offset);
    printf("Entry Addr:\t%08X\n", arm7entryAddr);
    printf("Ram Addr:\t%08X\n", arm7ramAddr);
    printf("Rom Size:\t%08X\n", arm7size);

    u32 alignedA7size = (arm7size + 511) & 0xFFFFFE00;
    u8 *virtualNdsData = malloc(0x400000u);
    memset(virtualNdsData, 0xAE, 0x400000u);
    u8 *arm7data = &virtualNdsData[(arm7ramAddr - 0x2000000)];
    memcpy(arm7data, &ndsData[arm7offset], alignedA7size);

    // Find the relocation signature
    u32 relocationStart = getOffsetA7(arm7data, alignedA7size,
        relocateStartSignature, 4, 1);
    if (!relocationStart) {
        printf("Relocation start now found\n");
        free(virtualNdsData); return 0;
    }

    // Validate the relocation signature
    u32 forwardedRelocStartAddr = relocationStart + arm7ramAddr + 4;
    if (!*(u32*)&virtualNdsData[forwardedRelocStartAddr - 0x2000000])
        forwardedRelocStartAddr += 4;
    u32 vAddrOfRelocSrc =
        *(u32*)&virtualNdsData[forwardedRelocStartAddr - 0x2000000 + 8];
    // sanity checks
    u32 relocationCheck1 =
        *(u32*)&virtualNdsData[forwardedRelocStartAddr - 0x2000000 + 0xC];
    u32 relocationCheck2 =
        *(u32*)&virtualNdsData[forwardedRelocStartAddr - 0x2000000 + 0x10];
    if ( vAddrOfRelocSrc != relocationCheck1
      || vAddrOfRelocSrc != relocationCheck2) {
        printf("Error in relocation checking\n");
        free(virtualNdsData); return 0;
    }

    // Get the remaining details regarding relocation
    u32 valueAtRelocStart =
        *(u32*)&virtualNdsData[forwardedRelocStartAddr - 0x2000000];
    u32 relocDestAtSharedMem =
        *(u32*)&virtualNdsData[valueAtRelocStart - 0x2000000];
    if (relocDestAtSharedMem != 0x37F8000) { // shared memory in RAM
        // Try again
        vAddrOfRelocSrc +=
            *(u32*)&virtualNdsData[valueAtRelocStart - 0x2000000 + 4];
        relocDestAtSharedMem =
            *(u32*)&virtualNdsData[valueAtRelocStart - 0x2000000 + 0xC];
        if (relocDestAtSharedMem != 0x37F8000) {
            printf("Error in finding shared memory relocation area\n");
            free(virtualNdsData); return 0;
        }
    }
    printf("Relocation src:\t%08X\n", vAddrOfRelocSrc);
    printf("Relocation dst:\t%08X\n", relocDestAtSharedMem);

    // I'm... not sure what to call the rest of this yet :/
    // define 0x18000 as a magic constant somehow
    u8* allocMem = (u8*)malloc(0x18000u);
    memset(allocMem, 0xAEu, 0x18000u);
    memcpy(allocMem, &virtualNdsData[vAddrOfRelocSrc - 0x2000000],
      4 * ((arm7ramAddr - vAddrOfRelocSrc + 0x18000) >> 2)
        + (((u8)arm7ramAddr - (u8)vAddrOfRelocSrc) & 3));

    // Find the card read
    u32 cardReadEndAddr =
        getOffsetA7(allocMem, 0x18000, a7cardReadSignature, 8, 1);
    if (!cardReadEndAddr) {
        printf("[Error!] Card read addr not found\n");
        free(allocMem); free(virtualNdsData); return 0;
    }


    // nonsense variable names below
    u32 someWramAddr = *(u32*)&allocMem[cardReadEndAddr - 4];
    u32 addrOfSomething_13C =
         getOffsetA7(&allocMem[cardReadEndAddr],
             0x18000 - cardReadEndAddr, &someWramAddr, 4u, 1);

    if (!addrOfSomething_13C)
    {
        printf("[Error!] ___ addr not found\n");
        free(allocMem); free(virtualNdsData); return 0;
    }
    u32 addrOfSomething_7F1C = addrOfSomething_13C + cardReadEndAddr + 4;

    //
    // Here is where the differences in the retry begin
    //

    u32 specificWramAddr = *(u32*)&allocMem[addrOfSomething_7F1C + 0x10];
    // if out of specific ram range...
    if (specificWramAddr < 0x37F8000 || specificWramAddr > 0x380FFFF) {
        addrOfSomething_7F1C +=
            getOffsetA7(&allocMem[addrOfSomething_7F1C],
              0x18000 - addrOfSomething_7F1C, &someWramAddr, 4, 1) + 4;
        specificWramAddr = *(u32*)&allocMem[addrOfSomething_7F1C + 0x10];
    }


    u32 someAddr_799C = getOffsetA7(allocMem, 0x18000, a7something2Signature,
        8, 1);
    if (!someAddr_799C) {
        printf("[Error!] ___ someOffset not found\n");
        free(virtualNdsData); free(allocMem); return 0;
    }

    // begin writes
    // TODO: if I'm going to be modifying these things in the future,
    // best to memcpy() in the future.
    u8* hardcodedArm7 = unk_4240C0;
    u32 dword_4240FC = *(u32*)&allocMem[someAddr_799C - 4];
    u32 dword_4240F4 = someWramAddr;
    u32 dword_4240F0 = dword_4240FC + 0x10;
    u32 dword_4240F8 = dword_4240FC - 0x04;
    *(u32*)&hardcodedArm7[0x30] = dword_4240F0;
    *(u32*)&hardcodedArm7[0x34] = dword_4240F4;
    *(u32*)&hardcodedArm7[0x38] = dword_4240F8;
    *(u32*)&hardcodedArm7[0x3C] = dword_4240FC;

    u32 Buffer = vAddrOfRelocSrc + specificWramAddr - relocDestAtSharedMem;

    u32 four = 4; u8 vals[4] = {0x1C,0x04,0x00,0x00};
    fwrite(&Buffer, sizeof(u8), 4, patFp);
    fwrite(vals, sizeof(u8), 4, patFp);
    fwrite(hardcodedArm7, sizeof(u8), 0x41C, patFp);


    u32 current = vAddrOfRelocSrc + addrOfSomething_7F1C + 0x10;
    u32 other = specificWramAddr + 0x1C;
    printf("Eeprom page erase:\t%08X (%08X)\n",
        addrOfSomething_7F1C + relocDestAtSharedMem + 0x10, current);
    fwrite(&current, sizeof(u8), 4, patFp);
    fwrite(&four, sizeof(u8), 4, patFp);
    fwrite(&other, sizeof(u8), 4, patFp);

    current = vAddrOfRelocSrc + addrOfSomething_7F1C + 0x2C;
    other = specificWramAddr + 0x18;
    printf("Eeprom verify:\t\t%08X (%08X)\n",
        addrOfSomething_7F1C + relocDestAtSharedMem + 0x2C, current);
    fwrite(&current, sizeof(u8), 4, patFp);
    fwrite(&four, sizeof(u8), 4, patFp);
    fwrite(&other, sizeof(u8), 4, patFp);

    current = vAddrOfRelocSrc + addrOfSomething_7F1C + 0x48;
    other = specificWramAddr + 0x10;
    printf("Eeprom page write:\t%08X (%08X)\n",
        addrOfSomething_7F1C + relocDestAtSharedMem + 0x48, current);
    fwrite(&current, sizeof(u8), 4, patFp);
    fwrite(&four, sizeof(u8), 4, patFp);
    fwrite(&other, sizeof(u8), 4, patFp);

    current = vAddrOfRelocSrc + addrOfSomething_7F1C + 0x64;
    other = specificWramAddr + 0x14;
    printf("Eeprom page prog:\t%08X (%08X)\n",
        addrOfSomething_7F1C + relocDestAtSharedMem + 0x64, current);
    fwrite(&current, sizeof(u8), 4, patFp);
    fwrite(&four, sizeof(u8), 4, patFp);
    fwrite(&other, sizeof(u8), 4, patFp);

    current = vAddrOfRelocSrc + addrOfSomething_7F1C + 0x80;
    other = specificWramAddr + 0xC;
    printf("Eeprom read:\t\t%08X (%08X)\n",
        addrOfSomething_7F1C + relocDestAtSharedMem + 0x80, current);
    fwrite(&current, sizeof(u8), 4, patFp);
    fwrite(&four, sizeof(u8), 4, patFp);
    fwrite(&other, sizeof(u8), 4, patFp);

    current = vAddrOfRelocSrc + addrOfSomething_7F1C + 0xA0;
    other = specificWramAddr + 0x8;
    printf("Card read:\t\t%08X (%08X)\n",
        addrOfSomething_7F1C + relocDestAtSharedMem + 0xA0, current);
    fwrite(&current, sizeof(u8), 4, patFp);
    fwrite(&four, sizeof(u8), 4, patFp);
    fwrite(&other, sizeof(u8), 4, patFp);

    // different @other for card id
    current = vAddrOfRelocSrc + addrOfSomething_7F1C + 0xAC;
    other = generateA7Instr(addrOfSomething_7F1C + relocDestAtSharedMem + 0xAC,
        specificWramAddr + 4);
    printf("Card id:\t\t%08X (%08X)\n",
        addrOfSomething_7F1C + relocDestAtSharedMem + 0xAC, current);
    fwrite(&current, sizeof(u8), 4, patFp);
    fwrite(&four, sizeof(u8), 4, patFp);
    fwrite(&other, sizeof(u8), 4, patFp);

    u32 anotherWramAddr = *(u32*)&allocMem[addrOfSomething_7F1C + 0xD0];
    if (anotherWramAddr > 0x37F7FFF && anotherWramAddr < 0x3810000) {
        current = vAddrOfRelocSrc + addrOfSomething_7F1C + 0xD0;
        other = specificWramAddr + 0x20;
        printf("???:\t\t\t%08X (%08X)\n",
            addrOfSomething_7F1C + relocDestAtSharedMem + 0xD0, current);
        fwrite(&current, sizeof(u8), 4, patFp);
        fwrite(&four, sizeof(u8), 4, patFp);
        fwrite(&other, sizeof(u8), 4, patFp);
    }

    printf("Arm7 patched!\n");

    free(allocMem);
    free(virtualNdsData);
    return 1;
}

u32 generateA7Instr(int arg1, int arg2) {
    return (((u32)(arg2 - arg1 - 8) >> 2) & 0xFFFFFF) | 0xEB000000;
}


//
// Look in @data for @find and return the position of it.
//
u32 getOffsetA7(u8* data, int size, void* find, int sizeofFind, int specifier)
{
    u32 result = 0;

    // Go backwards
    if (specifier == -1) {
        // Simply scan through @data
        for (result = 0; result > -size; result--)
        {
            // If @find is found, break and return the decremented offset
            if (!memcmp((void*)(data - sizeofFind + result), find, sizeofFind))
                return result;
        }
    }

    // Go forwards
    if (specifier == 1) {
        // Simply scan through @data
        for (result = 0; result < size - sizeofFind; result++)
        {
            // If @find is found, break and return the incremented offset
            if (!memcmp((void*)(data + result), find, sizeofFind))
                return result;
        }
    }

    // If scanning is finished and end was not reached, return no offset.
    return 0;
}
