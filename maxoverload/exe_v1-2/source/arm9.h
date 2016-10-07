#include "types.h"

int analyzeArm9(u8* ndsData, u32 fsize, FILE* patFp);

u32 getOffsetA9(u8* data, int size, void* find, u32 sizeofFind, int specifier);
void decompress(u8* data, u32 locOfCompression,
    u32* decompressionStart, u32* decompressionEnd);
