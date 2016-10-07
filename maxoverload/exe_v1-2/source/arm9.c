#include "arm9.h"
#include "arm9defs.h"

//
// Analyze the ARM9 section of the NDS image, filesize of image being @fsize
//
int analyzeArm9(u8* ndsData, u32 fsize, FILE* patFp)
{
    u32 arm9offset    = *(u32*)&ndsData[0x20];
    u32 arm9entryAddr = *(u32*)&ndsData[0x24];
    u32 arm9ramAddr   = *(u32*)&ndsData[0x28];
    u32 arm9size      = *(u32*)&ndsData[0x2C];

    printf("\nArm9\n");
    printf("Rom Offset:\t%08X\n", arm9offset);
    printf("Entry Addr:\t%08X\n", arm9entryAddr);
    printf("Ram Addr:\t%08X\n", arm9ramAddr);
    printf("Rom Size:\t%08X\n", arm9size);

    u32 alignedA9size = (arm9size + 511) & 0xFFFFFE00;
    u8 *virtualNdsData = malloc(0x400000u);
    memset(virtualNdsData, 0xAE, 0x400000u);
    u8 *arm9data = &virtualNdsData[(arm9ramAddr - 0x2000000)];
    memcpy(arm9data, &ndsData[arm9offset], alignedA9size);

    // Find compression, decompress it, and save locations of each
    u32 decompressionStart, decompressionEnd, decompressionSize;
    u32 compressionOffset =
        getOffsetA9(arm9data, alignedA9size,
            (void*)compressionSignature, 8, 1);
    if (!compressionOffset) {
        printf("Compression not found\n");
        free(virtualNdsData); return 0;;
    }
    u32 vAddrOfCompression = compressionOffset + arm9ramAddr - 8;
    u32 addrOfCompression = *(u32*)&
        virtualNdsData[vAddrOfCompression - 0x2000000];
    if (addrOfCompression) {
        if (*(u32*)&virtualNdsData[addrOfCompression-0x2000000] != 0xDEC00621) {
            printf("Compression sanity error\n");
            free(virtualNdsData); return 0;;
        }
        decompress(virtualNdsData, addrOfCompression,
            &decompressionStart, &decompressionEnd);
        printf("Compressed:\t%08X %08X\n",decompressionStart, decompressionEnd);
        decompressionSize = decompressionEnd - decompressionStart;
    } else {
        decompressionSize = alignedA9size;
    }

    // Find the card read
    u32 cardReadEndOffset =     // should result in 5AA64
        getOffsetA9(&virtualNdsData[arm9ramAddr-0x2000000], decompressionSize,
              (void*)a9cardReadSignature, 8, 1);
    if (!cardReadEndOffset) {
        printf("Card read end not found\n");
        free(virtualNdsData); return 0;;
    }
    u8 *vAddrCardReadEnd = &virtualNdsData[cardReadEndOffset+arm9ramAddr];
    u32 cardReadStartOffset =   // should result in FFFFFF0A (-F6?)
        getOffsetA9((void*)(vAddrCardReadEnd - 0x2000000), 0x200,
              (void*)cardReadStartSignature, 2, -1);
    if (!cardReadStartOffset) {
        printf("Card read start not found\n");
        free(virtualNdsData); return 0;;
    }
    u32 vAddrOfCardRead = cardReadEndOffset + cardReadStartOffset +
        arm9ramAddr - 2;
    printf("Card read:\t%08X\n", vAddrOfCardRead);

    // Find the card ID
    u32 vAddrOfCardId = 0;
    u32 cardIdAddr = getOffsetA9(&virtualNdsData[vAddrOfCardRead-0x2000000],
        0x100, (void*)a9cardIdSignature, 8, -1);
    if (cardIdAddr)
    {
        u32 cardIdStart = getOffsetA9(
            &virtualNdsData[cardIdAddr+vAddrOfCardRead-0x2000000], 0x100,
            (void*)cardIdStartSignature, 2, -1);
        if (cardIdStart) {
            vAddrOfCardId = cardIdAddr + cardIdStart + vAddrOfCardRead - 2;
            printf("Card id:\t%08X\n", vAddrOfCardId);
        } else {
            printf("Card id start not found\n");
        }
    } else {
        printf("Card id not found\n");
    }

    // Find the BHI ARM9 instruction within the Card Read function
    u32 addrOfBHI = getOffsetA9(
        &virtualNdsData[vAddrOfCardRead-0x2000000], 0x300,
        (void*)a9instructionBHI, 4, -1);
    if (!addrOfBHI) {
        printf("BHI not found!\n");
        free(virtualNdsData); return 0;;
    }
    u32 vAddrOfBHI = vAddrOfCardRead + addrOfBHI;

    // Series of write operations if there is compression in the nds image
    u32 zero = 0, four = 4, sixteen = 0x10;

    if (addrOfCompression) {
        fwrite(&decompressionStart, sizeof(u8), 4, patFp);
        fwrite(&decompressionSize, sizeof(u8), 4, patFp);

        fwrite(&virtualNdsData[decompressionStart-0x2000000],
            sizeof(u8), decompressionSize, patFp);
        fwrite(&vAddrOfCompression, sizeof(u8), 4, patFp);

        fwrite(&four, sizeof(u8), 4, patFp);
        fwrite(&zero, sizeof(u8), 4, patFp);
    }

    // ldr*_off is the last byte of ldr instructions.
    // `ldr r0, [r9, #0x20]` ; so, 0x20
    u8 ldr0x20_off =
        *(u32*)&virtualNdsData[vAddrOfCardRead-0x2000000 + 0x44] & 0xFF;
    // `ldr r8, [r9, #0x1C]` ; so, 0x1C
    u8 ldr0x1C_off =
        *(u32*)&virtualNdsData[vAddrOfCardRead-0x2000000 + 0x38];
    // `ldr r0, [r9, #0x18]` ; so, 0x18
    u8 ldr0x18_off =
        *(u32*)&virtualNdsData[vAddrOfCardRead-0x2000000 + 0x28];

    u32 sizeofHardcodedArm9 = 0;
    u32* hardcodedArm9;

    // The Maxoverload suite can use some turbo load functionality
    // disable it for now
    // TODO: there is also saver functionality; haven't found that used yet
    u8 turboLoadEnabled = 0;
    if ( turboLoadEnabled )
    {
        hardcodedArm9 = (u32*)&unk_424A2C;
        sizeofHardcodedArm9 = 0x1AC;
    } else {
        if (vAddrOfCardId || arm9ramAddr == 0x2004000) {
            hardcodedArm9 = (u32*)&unk_42486C;
            sizeofHardcodedArm9 = 0x1C0;
            if (vAddrOfCardId)
                arm9ramAddrMod = vAddrOfCardRead + 0x200;
            else
                arm9ramAddrMod =
                    arm9ramAddr != 0x2004000 ? 0x23FEC00 : 0x2003E00;
        } else {
            hardcodedArm9 = (u32*)&unk_42486C;
            sizeofHardcodedArm9 = 0x1C0;
            arm9ramAddrMod = arm9ramAddr != 0x2004000 ? 0x23FEC00 : 0x2003E00;
        }

        hardcodedArm9[3] = arm9ramAddrMod;
    }

    // Modify .nds-specific values in the hardcoded arm9 instructions
    hardcodedArm9[4] = *(u32*)(vAddrCardReadEnd-0x2000004)+ldr0x18_off;
    hardcodedArm9[5] = *(u32*)(vAddrCardReadEnd-0x2000004)+ldr0x1C_off;
    hardcodedArm9[6] = *(u32*)(vAddrCardReadEnd-0x2000004)+ldr0x20_off;

    // Write the remaining necessary properties to patch file
    fwrite(&vAddrOfCardRead, sizeof(u8), 4, patFp);
    fwrite(&sizeofHardcodedArm9, sizeof(u8), 4, patFp);
    fwrite(hardcodedArm9, sizeof(u8), sizeofHardcodedArm9, patFp);
    if (vAddrOfCardId) {
        fwrite(&vAddrOfCardId, sizeof(u8), 4, patFp);
        fwrite(&sixteen, sizeof(u8), 4, patFp);
        fwrite(&unk_424BD8, sizeof(u8), 0x10, patFp);
    }
    fwrite(&vAddrOfBHI, sizeof(u8), 4, patFp);
    fwrite(&four, sizeof(u8), 4, patFp);
    fwrite(&unk_424BE8, sizeof(u8), 4, patFp);

    printf("Arm9 patched!\n");
    free(virtualNdsData);
    return 1;
}

//
// Look in @data for @find and return the position of it.
//
u32 getOffsetA9(u8* data, int size, void* find, u32 sizeofFind, int specifier)
{
    s32 result = 0;

    // Go backwards
    if (specifier == -1) {
        // Simply scan through @data
        u8* comparison = (data-sizeofFind);
        result = -sizeofFind;
        s32 offset = 0;
        // @result, @offset, exist due to a reasoning in the original patcher
        // I can't entirely explain; this is required for proper offsets
        for (result = -sizeofFind; result >= -size; result--)
        {
            // If @find is found, break and return the decremented offset
            if (!memcmp(comparison + offset, find, sizeofFind))
                return result;
            offset--;
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

//
// Find the start and end of the compression in @data based on @base.
// TODO: I find this complex to explain for the time being; will document later.
// @data = virtualNdsData
// @base = *(u32*)&virtualNdsData[vAddrOfCompression - 0x2000000]
//
void decompress(u8* data, u32 base,
    u32* decompressionStart, u32* decompressionEnd)
{
    u8* startOffset =   (&data[base & 0xFFFFFF]
                        - (*(u32*)&data[(base & 0xFFFFFF) - 8] & 0xFFFFFF));
    u8* endOffset =     (&data[base & 0xFFFFFF]
                        + (*(u32*)&data[(base & 0xFFFFFF) - 4]));
    u8* otherOffset =   (&data[base & 0xFFFFFF]
                        - (*(u32*)&data[(base & 0xFFFFFF) - 8] >> 24));

    *decompressionStart   = startOffset - data + 0x2000000;
    *decompressionEnd     = endOffset   - data + 0x2000000;

    u8 counter = 0;
    u8 block = 0;
    while (otherOffset > startOffset) {
        if (!counter) {
            block = *(--otherOffset);
            counter = 0x80;
        }

        if (counter & block) {
            s32 hibite = *(--otherOffset);
            otherOffset--;

            s32 index = (((hibite & 0xF) << 8) | *otherOffset) + 2;
            s32 innerCounter = hibite + 0x20;
            do {
                u8 temp = (endOffset--)[index];
                *endOffset = temp;
                innerCounter -= 0x10;
            } while(innerCounter >= 0);
        } else {
            *(--endOffset) = *(--otherOffset);
        }

        counter >>= 1;
    }
}
