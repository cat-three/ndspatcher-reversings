#ifndef STRUCTS_H
#define STRUCTS_H

#include <stdio.h>
#include <stdint.h>

// The following represent data allocated by this program,
// that contain data read from the input file.
// sizeof(struct NDSData) reads as 0xA78
struct NDSData {
    uint8_t   ntrHeaderData[0x200];			// first 0x200 bytes
    uint8_t*  arm9code;						// at 0x200
    uint8_t*  arm7code;						// at 0x204
    uint8_t*  fileNameTable;				// at 0x208
    uint8_t*  fileAllocationTableRef;		// at 0x20C
    uint8_t*  arm9overlay;					// at 0x210
    uint8_t*  arm7overlay;					// at 0x214
    uint8_t   ntrIconTitleData[0x840];		// 0x218+0x840=at 0xA58
    uint8_t** fileAllocationTableData;		// at 0xA58
};

#endif