#include "types.h"

int analyzeArm7(u8* ndsData, u32 fsize, FILE* patFp);
int retryAnalyzeArm7(u8* ndsData, u32 fsize, FILE* patFp);

u32 generateA7Instr(int arg1, int arg2);
// TODO: This is in both arm7.h and arm9.h; but types.h isn't suitable
// for simply containing all shared functions like this. Find a better place?
u32 getOffsetA7(u8* data, int size, void* find, int sizeofFind, int specifier);
