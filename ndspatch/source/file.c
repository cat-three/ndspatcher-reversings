#include "file.h"

struct NDSData initNDSData(char* filename)
{
    FILE *fp;
    struct NDSData nds_data = { { 0 } };
    uint32_t  arm9size, arm7size, fntSize, fatSize,
        a9olSize, a7olSize, iconOffset;

    fp = fopen(filename, "rb");
    if ( fp )
    {
        // Load header
        fseek(fp, 0, 0);
        fread(nds_data.ntrHeaderData, 1u, 0x200, fp);

        // ARM9 Offset
        fseek(fp, *(uint32_t*)&nds_data.ntrHeaderData[0x20], 0);
        arm9size = *(uint32_t*)&nds_data.ntrHeaderData[0x2C];
        nds_data.arm9code = malloc(arm9size);
        fread(nds_data.arm9code, 1u, arm9size, fp);

        // ARM7 Offset
        fseek(fp, *(uint32_t*)&nds_data.ntrHeaderData[0x30], 0);
        arm7size = *(uint32_t*)&nds_data.ntrHeaderData[0x3C];
        nds_data.arm7code = malloc(arm7size);
        fread(nds_data.arm7code, 1u, arm7size, fp);

        // File Name Table (FNT) Offset
        fseek(fp, *(uint32_t*)&nds_data.ntrHeaderData[0x40], 0);
        fntSize = *(uint32_t*)&nds_data.ntrHeaderData[0x44];
        nds_data.fileNameTable = malloc(fntSize);
        fread(nds_data.fileNameTable, 1u, fntSize, fp);

        // File Allocation Table (FAT) Offset
        fseek(fp, *(uint32_t*)&nds_data.ntrHeaderData[0x48], 0);
        fatSize = *(uint32_t*)&nds_data.ntrHeaderData[0x4C];
        nds_data.fileAllocationTableRef = malloc(fatSize);
        fread(nds_data.fileAllocationTableRef, 1u, fatSize, fp);


        // ARM9 Overlay Offset
        if (*(uint32_t*)&nds_data.ntrHeaderData[0x54])
        {
            fseek(fp, *(uint32_t*)&nds_data.ntrHeaderData[0x50], 0);
            a9olSize = *(uint32_t*)&nds_data.ntrHeaderData[0x54];
            nds_data.arm9overlay = malloc(a9olSize);
            fread(nds_data.arm9overlay, 1u, a9olSize, fp);
        }
        else
        {
            *(uint32_t*)&nds_data.arm9overlay = 0;
        }

        // ARM7 Overlay Offset
        if (*(uint32_t*)&nds_data.ntrHeaderData[0x5C])
        {
            fseek(fp, *(uint32_t*)&nds_data.ntrHeaderData[0x58], 0);
            a7olSize = *(uint32_t*)&nds_data.ntrHeaderData[0x5C];
            nds_data.arm7overlay = malloc(a7olSize);
            fread(nds_data.arm7overlay, 1u, a7olSize, fp);
        }
        else
        {
            *(uint32_t*)&nds_data.arm7overlay = 0;
        }

        // Icon Banner Offset
        iconOffset = *(uint32_t*)&nds_data.ntrHeaderData[0x68];
        if (iconOffset)
        {
            fseek(fp, iconOffset, 0);
            fread(nds_data.ntrIconTitleData, 1u, 0x840u, fp);
        }
        

        // Initialize storage of File Allocation Table data
        uint32_t fileAllocationTableSize = *(uint32_t*)&nds_data.ntrHeaderData[0x4C];
        nds_data.fileAllocationTableData = malloc(sizeof(void*) * (fileAllocationTableSize / 8));
        if (fileAllocationTableSize > 0xFF) // originally & 0xFFFFFFF8
        {
            uint32_t counter = 0;
            do
            {
                uint32_t fatStart   = *(uint32_t*)&nds_data.fileAllocationTableRef[8*counter];
                uint32_t fatEnd     = *(uint32_t*)&nds_data.fileAllocationTableRef[(8*counter)+4];
                uint32_t fatSize    = fatEnd - fatStart;
                fseek(fp, fatStart, 0);

                nds_data.fileAllocationTableData[counter] = malloc(fatSize);
                fread(nds_data.fileAllocationTableData[counter], 1u, fatSize, fp);

                counter++;
            }
            while (counter < (fileAllocationTableSize / 8));
        }
        fclose(fp);
    }

    // TODO: else case here, and error handling
    return nds_data;
}

void freeNDSData(struct NDSData nds_data)
{
    uint32_t fileAllocationTableSize = *(uint32_t*)&nds_data.ntrHeaderData[0x4C];
    if (fileAllocationTableSize > 0xFF)
    {
        uint32_t counter = 0;
        do
        {   
            free((void*)nds_data.fileAllocationTableData[counter]);
            nds_data.fileAllocationTableData[counter] = NULL;
            counter++;
        }
        while (counter < (fileAllocationTableSize / 8));
    }   
    free(nds_data.fileAllocationTableData);
    nds_data.fileAllocationTableData = NULL;

    free(nds_data.arm9code);
    nds_data.arm9code = NULL;
    free(nds_data.arm7code);
    nds_data.arm7code = NULL;
    free(nds_data.fileNameTable);
    nds_data.fileNameTable = NULL;
    free(nds_data.fileAllocationTableRef);
    nds_data.fileAllocationTableRef = NULL;
    free(nds_data.arm9overlay);
    nds_data.arm9overlay = NULL;
    free(nds_data.arm7overlay);
    nds_data.arm7overlay = NULL;
}
