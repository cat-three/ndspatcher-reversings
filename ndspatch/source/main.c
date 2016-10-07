// ndspatcher2!main.c
// _catcatcat, 2016; no license yet

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "consts.h"
#include "structs.h"
#include "file.h"

// Function Prototypes
uint32_t getArm9decompression(uint32_t arm9offset);
uint32_t reindexNdsFile(struct NDSData ndsData, uint32_t decompressionOffset,
    uint32_t adjustedArm9Offset2);
uint32_t getArm9instrAddr(uint8_t* arm9code, uint32_t arm9off,
    uint32_t arm9size, uint32_t* signature, uint32_t sizeofSig,
    const char* nameOfSignature);
uint32_t writeDsGbaFile(const char* filenameWithoutExt,
    struct NDSData nds_data);

// Global Vars
uint8_t* virtualNdsData = NULL;

int main(int argc, char* argv[])
{
    int32_t ignoreSave = 0;

    // Initialization
    printf("ndspatcher2 v0.1 by _catcatcat\n");
    if (argc < 2) {
        // TODO: Display much more detail
        printf("Usage: %s <in.nds>\n", *argv);
        return 1;
    }
    if (argc > 2) {
        int i;
        for (i = 0; i < argc; i++) {
            if (!strcmp(argv[i], "-nosave")) {
                ignoreSave = 1;
            }
        }
    }

    // TODO: remove this for later
    printf("for now, using -nosave parameter anyways\n");
    ignoreSave = 1;
    //

    struct NDSData nds_data = initNDSData(argv[1]);
    if (!*(nds_data.ntrHeaderData)) {
        printf("Failed to open \"%s\" for reading.\n", argv[1]);
        return 1;
    }
    uint32_t filenameLength = strlen(argv[1]);
    char filenameWithoutExt[256];
    strncpy(filenameWithoutExt, argv[1], filenameLength - 1);
    filenameWithoutExt[filenameLength - 4] = '\0';

    uint32_t arm9offset     = *(uint32_t*)&nds_data.ntrHeaderData[0x20];
    uint32_t arm9entryAddr  = *(uint32_t*)&nds_data.ntrHeaderData[0x24];
    uint32_t arm9ramAddr    = *(uint32_t*)&nds_data.ntrHeaderData[0x28];
    uint32_t arm9size       = *(uint32_t*)&nds_data.ntrHeaderData[0x2C];
    uint32_t* adjustedArm9Offset =
        (uint32_t*)&arm9entryAddr[nds_data.arm9code - arm9ramAddr];


    uint32_t counter;
    while (*adjustedArm9Offset != 0xE5910014) {   // ldr r0, [r1, #20] ; 0x14
        counter++;
        adjustedArm9Offset++; // inc 4
        if (counter >= 0x100) {
            printf("Couldn't not find instruction\
             'ldr r0, [r1, #0x14]', aborting\n");
            freeNDSData(nds_data);
            return 1;
        }
    }
    uint32_t a9instrLocation = arm9entryAddr + 4 * counter;
    if (counter == 0x100) {
        printf("Couldn't not find instruction\
         'ldr r0, [r1, #0x14]', aborting\n");
        freeNDSData(nds_data);
        return 1;
    }

    // The original program seems to memcpy into unallocated windows
    // memory, and for this reason, requires admin rights; which is...
    // pretty unacceptable. @virtualNdsData will be a proper fix, I hope.
    virtualNdsData = malloc(0x00400000);
    memset(virtualNdsData, 0, 0x00400000);

    memcpy(&virtualNdsData[arm9ramAddr - 0x1BF41A0],
        nds_data.arm9code, arm9size);

    uint32_t adjustedArm9Offset2 =
        *(uint32_t*)&virtualNdsData[
            (*(uint32_t*)&virtualNdsData[a9instrLocation - 0x1BF41A4] & 0xFFF)
            + a9instrLocation - 0x1BF419C]
         + 0x14;

    uint32_t decompressionOffset = getArm9decompression(adjustedArm9Offset2);
    if (decompressionOffset)
        reindexNdsFile(nds_data, decompressionOffset, adjustedArm9Offset2);

    uint32_t addrOf_ReadRom = getArm9instrAddr(
        nds_data.arm9code, arm9offset, arm9size, (uint32_t*)readRomSignature,
        10, "CARDi_ReadRom");
    if (addrOf_ReadRom == -1) {
        printf("aborting\n");
        free(virtualNdsData); freeNDSData(nds_data); return 1;
    }
    *(uint32_t*)&nds_data.arm9code[addrOf_ReadRom] = 0xEA00000F;

    uint32_t addrOf_ReadCard = getArm9instrAddr(
        nds_data.arm9code, arm9offset, arm9size, (uint32_t*)readCardSignature,
        4, "CARDi_ReadCard");
    if (addrOf_ReadCard == -1) {
        printf("aborting\n");
        free(virtualNdsData); freeNDSData(nds_data); return 1;
    }

    //*(uint8_t*)&hardcodedReadCardPatch[0xD0/4] +=
    //    3 + *(uint8_t*)&nds_data.arm9code[addrOf_ReadCard + 220];
    uint32_t* physAddrOf_ReadCard =
        (uint32_t*)&nds_data.arm9code[addrOf_ReadCard];
    counter = 0;
    do
      *physAddrOf_ReadCard++ = hardcodedReadCardPatch[counter++];
    while ( counter < 0x3D );
    physAddrOf_ReadCard[1] = 0x4000204;
    physAddrOf_ReadCard[2] = 0x400C;
    printf("Patched\n");

    if (!ignoreSave) {
        //addrOf_ProgramBackupSyncCore = getArm9instrAddr(
        //    nds_data.arm9code, arm9offset, arm9size, (uint32_t*)unk_40935C,
        //    7, "CARDi_ProgramBackupSyncCore");
        //if (addrOf_ProgramBackupSyncCore == -1) {
        //    printf("skipping\n");
        //}
        // ...
    }

    printf("Writing patched NDS...");
    if (!writeDsGbaFile(filenameWithoutExt, nds_data)) {
        printf("failed to open '%s' for writing\n", filenameWithoutExt);
        free(virtualNdsData); freeNDSData(nds_data); return 1;
    } else {
        printf("All done\n");
    }
    free(virtualNdsData);
    freeNDSData(nds_data);
    return 0;
}



//
// Check for compression in the arm9 section, if it exists, decompress
// it and return the offset of where the decompression is written to in memory.
//
uint32_t getArm9decompression(uint32_t arm9offset)
{
    int addrOfDecompression = 0;
    uint32_t compressBase = *(uint32_t*)&virtualNdsData[arm9offset - 0x1BF41A0];
    printf("compressBase = %08X\n", compressBase);
    // ends up being 205C0AC.
    printf("Checking if arm9 is compressed...");
    if (compressBase)
    {
        printf("Yes, decompressing...");

        // was v3, is 2098FD8 (SM64)
        uint32_t startOff = compressBase +
           *(uint32_t*)&virtualNdsData[compressBase - 0x1BF41A4];

        // was v4, is 205C0A1 (SM64)
        uint32_t otherOff = compressBase -
          (*(uint32_t*)&virtualNdsData[compressBase - 0x1BF41A8] >> 24);
        printf("otherOff = %08X\n", otherOff);

        // was v14, is 2008000 (SM64)
        uint32_t decompressionStart = compressBase -
          (*(uint32_t*)&virtualNdsData[compressBase - 0x1BF41A8] & 0xFFFFFF);
        printf("decompressionStart = %08X\n", decompressionStart);

        // was v15, is 2098FD8
        uint32_t decompressionEnd = compressBase +
          *(uint32_t*)&virtualNdsData[compressBase - 0x1BF41A4];
        printf("decompressionEnd = %08X\n", decompressionEnd);

        if (otherOff > decompressionStart)
        {
               // FE40BE60
               // this is likely more random data...
               // this is -0x2000000 tho so
            uint8_t* writeAddr = &virtualNdsData[0xFE000000];
          //   do
          //   {
          //       uint8_t v6 = writeAddr[--otherOff];
           //
          //       uint8_t counter;
          //       for (counter = 8; counter != 1; counter--)
          //       {
          //           if ( v6 >= 0 )
          //           {
          //               writeAddr[--startOff] = writeAddr[--otherOff];
          //           }
          //           else
          //           {
          //               v7 = (uint8_t)writeAddr[otherOff];
          //               LOBYTE(v9) = writeAddr[--otherOff];
          //               BYTE1(v9) = v7;
          //               LOWORD(v9) = (v9 & 0xFFF) + 2;
          //               if (v7 + 0x20 >= 0)
          //               {
          //                   v9 &= 0xFFFF;
          //                   uint32_t miniCounter =
          //                       (uint32_t)(uint16_t)(v7 + 0x30) >> 4;
          //                   do
          //                   {
          //                       v11 = *(uint8_t*)(v9 + v3-- - 0x1BF41A0);
           //
          //                       writeAddr[startOff] = v11;
          //                       miniCounter--;
          //                   }
          //                   while (miniCounter);
          //               }
          //           }
          //           v16 *= 2;
          //       }
          //  }
          //  while ( otherOff > decompressionStart );
        }
        printf("Done\n");
        addrOfDecompression = decompressionEnd - 0x2000000;
    }
    else
    {
        printf("No\n");
        addrOfDecompression = 0;
    }
    return addrOfDecompression;
}



//
// Reindex the NDS file
//
uint32_t reindexNdsFile(struct NDSData ndsData, uint32_t decompressionOffset,
    uint32_t adjustedArm9Offset2)
{
    printf("Reindexing NDS...");
    // uint32_t* arm9romOffset   = (uint32_t*)&ndsData.ntrHeaderData[0x20];
    // uint32_t* arm9entryAddr   = (uint32_t*)&ndsData.ntrHeaderData[0x24];
    // uint32_t* arm9ramAddr     = (uint32_t*)&ndsData.ntrHeaderData[0x28];
    // uint32_t* arm9size        = (uint32_t*)&ndsData.ntrHeaderData[0x2C];
    // uint32_t* arm7romOffset   = (uint32_t*)&ndsData.ntrHeaderData[0x30];
    // uint32_t* arm7entryAddr   = (uint32_t*)&ndsData.ntrHeaderData[0x34];
    // uint32_t* arm7ramAddr     = (uint32_t*)&ndsData.ntrHeaderData[0x38];
    // uint32_t* arm7size        = (uint32_t*)&ndsData.ntrHeaderData[0x3C];
    // uint32_t* fntOffset       = (uint32_t*)&ndsData.ntrHeaderData[0x40];
    // uint32_t* fntSize         = (uint32_t*)&ndsData.ntrHeaderData[0x44];
    // uint32_t* fatOffset       = (uint32_t*)&ndsData.ntrHeaderData[0x48];
    // uint32_t* fatSize         = (uint32_t*)&ndsData.ntrHeaderData[0x4C];
    // uint32_t* a9overlayOffset = (uint32_t*)&ndsData.ntrHeaderData[0x50];
    // uint32_t* a9overlaySize   = (uint32_t*)&ndsData.ntrHeaderData[0x54];
    // uint32_t* a7overlayOffset = (uint32_t*)&ndsData.ntrHeaderData[0x58];
    // uint32_t* a7overlaySize   = (uint32_t*)&ndsData.ntrHeaderData[0x5C];
    // uint32_t* iconTitleOffset = (uint32_t*)&ndsData.ntrHeaderData[0x68];
    // uint32_t* totalRomSize    = (uint32_t*)&ndsData.ntrHeaderData[0x80];
    //

    // uint32_t v3 = adjustedArm9Offset2 - 0x2000000;
    // virtualNdsData[v3] = 0;
    // byte_40BE61[v3] = 0;
    // byte_40BE62[v3] = 0;
    // byte_40BE63[v3] = 0;
    // free(ndsData.arm9code);
    // decompressionData = malloc(decompressionOffset);
    //
    // *ndsData.arm9code = decompressionData;
    // memcpy(decompressionData, (const void *)(*arm9ramAddr - 0x1BF41A0),
    //     decompressionOffset);
    //
    // v7 = (*arm9romOffset + decompressionOffset + 0x1FF) & 0xFFFFFE00;
    // *arm9size = decompressionOffset;
    // if (*a9overlaySize)
    // {
    //     *a9overlayOffset = v7;
    //     v7 = (*arm9overlaySize + v7 + 0x1FF) & 0xFFFFFE00;
    // }
    //
    // *arm7romOffset = v7;
    // v9 = *arm7size + v7 + 0x1FF;
    //
    // v11 = v9 & 0xFFFFFE00;
    // if ( *arm7overlaySize )
    // {
    //     *a7overlayOffset = v11;
    //     v11 = (*arm7overlaySize + v11 + 0x1FF) & 0xFFFFFE00;
    // }
    // *fntOffset = v11;
    // v13 = *fntSize + v11 + 0x1FF;
    //
    // v13 &= 0xFFFFFE00;
    // *fatOffset = v13;
    // v15 = (fatSize + v13 + 0x1FF) & 0xFFFFFE00;
    // *(uin32_t*)(ndsData + 0x68) = v15;
    // v16 = (v15 + 0xA3F) & 0xFFFFFE00;
    // v17 = 0;
    // if ( fatSize & 0xFFFFFFF8 )
    // {
    //     do
    //     {
    //         v19 = *fatOffset + 8 * v17 + 4);
    //         v20 = *(uint32_t*)(fatOffset + 8 * v17);
    //         v21 = (v16 + 0x1FF) & 0xFFFFFE00;
    //         *(uint32_t*)(fatOffset + 8 * v17) = v21;
    //         v16 = v19 - v20 + v21;
    //         *(uint32_t*)(*ndsData.fileAllocationTableRef + 8 * v17++ + 4) = v16;
    //     }
    //     while ( v17 < *fatSize >> 3 );
    // }
    //
    // *totalRomSize = v16;
    // v22 = 0xFFFFu;
    // arm9codePlus2 = ndsData.arm9code + 2;
    // counterAt0x1000 = 0x1000;
    // do
    // {
    //     v25 = *(_BYTE *)(arm9codePlus2 - 2);
    //     arm9codePlus2 += 4;
    //     v26 =
    //         word_409100[(uint8_t)(v22 ^ v25)] ^ HIBYTE(v22);
    //     v27 =
    //         word_409100[(uint8_t)(v26 ^ *(uint8_t*)(arm9codePlus2 - 5))] ^ HIBYTE(v26);
    //     v28 =
    //         word_409100[(uint8_t)(v27 ^ *(uint8_t*)(arm9codePlus2 - 4))] ^ HIBYTE(v27);
    //     v22 =
    //         word_409100[(uint8_t)(v28 ^ *(uint8_t*)(arm9codePlus2 - 3))] ^ HIBYTE(v28);
    //     --counterAt0x1000;
    // }
    // while ( counterAt0x1000 );
    //
    // // Secure area checksum, CRC-16
    // *(uint16_t*)(ndsData + 0x6C) = v22;
    //
    // v29 = 0xFFFFu;
    // v30 = ndsData + 1;
    // counterAt0x46 = 0x46;
    // do
    // {
    //     v32 = *(uint8_t*)(v30 - 1);
    //     v30 += 5;
    //     v33 =
    //         word_409100[(uint8_t)(v29 ^ v32)] ^ HIBYTE(v29);
    //     v34 =
    //         word_409100[(uint8_t)(v33 ^ *(uint8_t*)(v30 - 5))] ^ HIBYTE(v33);
    //     v35 =
    //         word_409100[(uint8_t)(v34 ^ *(uint8_t*)(v30 - 4))] ^ HIBYTE(v34);
    //     v36 =
    //         word_409100[(uint8_t)(v35 ^ *(uint8_t*)(v30 - 3))] ^ HIBYTE(v35);
    //     v29 =
    //         word_409100[(uint8_t)(v36 ^ *(uint8_t*)(v30 - 2))] ^ HIBYTE(v36);
    //     --counterAt0x46;
    // }
    // while ( counterAt0x46 );
    //
    // // Header checksum, CRC-16
    // *(uint16_t*)(ndsData + 0x15E) = v29;
    return printf("Done\n");
}



//
// Search @arm9size bytes in @arm9offset to find @signature
//
uint32_t getArm9instrAddr(uint8_t* arm9code, uint32_t arm9off,
    uint32_t arm9size, uint32_t* signature, uint32_t sizeofSig,
    const char* nameOfSignature)
{
    printf("Looking for %s signature...\n", nameOfSignature);

    uint32_t signatureOffset = -1;
    uint32_t counter = 0;
    uint32_t instancesOfSignature = 0;
    uint32_t result = 0;
    uint32_t a9index = 0;

    if (arm9size == 0) {
        printf("  Not found, ");
        return -1;
    }

    do
    {
        uint32_t a9instr = signature[counter];
        if (!a9instr || *(uint32_t*)&arm9code[a9index] == a9instr)
        {
            counter++;
            if (counter == sizeofSig) {
                signatureOffset = (a9index + 4) - sizeof(uint32_t) * sizeofSig;
                instancesOfSignature++;
            }
        }
        if (counter == sizeofSig)
            counter = 0;

        a9index += sizeof(uint32_t);
    } while (a9index < arm9size);

    if (signatureOffset == -1) {
        printf("  Not found, ");
        return -1;
    }

    if ( instancesOfSignature <= 1 ) {
      printf("  Found at offset 0x%08x\n", signatureOffset);
      result = signatureOffset;
    } else
    {
      printf("Found more than one (%d), ", instancesOfSignature);
      result = -1;
    }

    return result;
}

//
// Save changes into a new file with a .ds.gba file extension
//
uint32_t writeDsGbaFile(const char* filenameWithoutExt,
    struct NDSData nds_data)
{
    // the + 8 is for ".nds" and ".gba"
    uint8_t filenameStrlen = strlen(filenameWithoutExt);
    char* newFilename = malloc(sizeof(char) * filenameStrlen + 8);
    strcpy(newFilename, (const char *)filenameWithoutExt);
    strcpy(&newFilename[filenameStrlen + 0], ".nds");
    strcpy(&newFilename[filenameStrlen + 4], ".gba");

    // If .ds.gba could not be created, exit
    FILE* fp = fopen(newFilename, "wb");
    if (!fp) {
        return 0;
    }

    memcpy(&gbaHeader[0xA0], nds_data.ntrHeaderData, 0xC);
    fwrite(gbaHeader, 1, 0x200, fp);

    uint32_t ndsOffset = 0x200;
    fwrite(nds_data.ntrHeaderData, 1, 0x200, fp);
    fseek(fp,        ndsOffset + *(uint32_t*)&nds_data.ntrHeaderData[0x20], 0);
    fwrite(nds_data.arm9code, 1, *(uint32_t*)&nds_data.ntrHeaderData[0x2C], fp);
    fseek(fp,        ndsOffset + *(uint32_t*)&nds_data.ntrHeaderData[0x30], 0);
    fwrite(nds_data.arm7code, 1, *(uint32_t*)&nds_data.ntrHeaderData[0x3C], fp);

    fseek(fp,        ndsOffset + *(uint32_t*)&nds_data.ntrHeaderData[0x40], 0);
    fwrite(nds_data.fileNameTable, 1,
          *(uint32_t*)&nds_data.ntrHeaderData[0x44], fp);
    fseek(fp,        ndsOffset + *(uint32_t*)&nds_data.ntrHeaderData[0x48], 0);
    fwrite(nds_data.fileAllocationTableRef, 1,
          *(uint32_t*)&nds_data.ntrHeaderData[0x4C], fp);

    uint32_t a9overlayOffset = *(uint32_t*)&nds_data.ntrHeaderData[0x50];
    if (a9overlayOffset)
    {
        fseek(fp, ndsOffset + a9overlayOffset, 0);
        fwrite(nds_data.arm9overlay, 1u,
            *(uint32_t*)&nds_data.ntrHeaderData[0x54], fp);
    }
    uint32_t a7overlayOffset = *(uint32_t*)&nds_data.ntrHeaderData[0x58];
    if (a7overlayOffset)
    {
        fseek(fp, ndsOffset + a7overlayOffset, 0);
        fwrite(nds_data.arm7overlay, 1u,
            *(uint32_t*)&nds_data.ntrHeaderData[0x5C], fp);
    }
    uint32_t iconTitleOffset = *(uint32_t*)&nds_data.ntrHeaderData[0x68];
    if (iconTitleOffset)
    {
        fseek(fp, ndsOffset + iconTitleOffset, 0);
        fwrite(nds_data.ntrIconTitleData, 1u, 0x840u, fp);
    }

    printf("\n");

    uint32_t counter = 0;
    uint32_t fileAllocationTableSize= *(uint32_t*)&nds_data.ntrHeaderData[0x4C];
    if (fileAllocationTableSize > 0xFF)
    {
        do
        {
            uint32_t fatStart =
                *(uint32_t*)&nds_data.fileAllocationTableRef[8*counter];
            uint32_t fatEnd =
                *(uint32_t*)&nds_data.fileAllocationTableRef[(8*counter)+4];
            uint32_t fatSize    = fatEnd - fatStart;

            fseek(fp, ndsOffset + fatStart, 0);
            fwrite(nds_data.fileAllocationTableData[counter], 1, fatSize, fp);

            ++counter;
        }
        while (counter < (fileAllocationTableSize / 8));
    }

    fclose(fp);
    free(newFilename);
    return 1;
}
