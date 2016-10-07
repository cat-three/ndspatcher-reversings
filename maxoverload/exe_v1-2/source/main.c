#include "types.h"
#include "arm7.h"
#include "arm9.h"

//int writePAT(FILE* patFp, struct NDSArm9Data ndsA9, struct NDSArm7Data ndsA7);

int main(int argc, char* argv[])
{
    // Parse parameters, add more later
    if (!argv[1]) {
        printf("Missing paramter. Usage:\n");
        printf("./ndsptcher2 [name].nds\n");
        return 1;
    }
    FILE* fp = fopen(argv[1], "rb");
    if (!fp) {
        printf("Could not open file.\n");
        return 1;
    }

    // In the future, remove directory; or remove printf's
    char *filename = argv[1];
    printf("\nFile:\t%s\n", filename);

    char *patFilename = malloc(strlen(filename) + 1);
    strcpy(patFilename, filename);
    patFilename[strlen(patFilename) - 3] = 'P';
    patFilename[strlen(patFilename) - 2] = 'A';
    patFilename[strlen(patFilename) - 1] = 'T';
    FILE* patFp = fopen(patFilename, "wb");
    if (!patFp) {
        printf("Could not create patch file.\n");
        return 1;
    }
    printf("Patch:\t%s\n", patFilename);

    fseek(fp, 0, 2);
    u32 filesize = ftell(fp);
    fseek(fp, 0, 0);
    printf("Size:\t%i\n", filesize);

    // Read in file for processing later
    u8 *ndsData = malloc(filesize);
    memset(ndsData, 0x00, filesize);
    fread(ndsData, sizeof(u8), filesize, fp);

    // Process the NDS image and write to patch file
    if (!analyzeArm9(ndsData, filesize, patFp)) {
        printf("Fatal error in patching arm9, exiting...\n");
    } else {
        if (!analyzeArm7(ndsData, filesize, patFp)) {
            if (!retryAnalyzeArm7(ndsData, filesize, patFp)) {
                  printf("analysis/re-analysis of arm7 failed, exiting...\n");
            }
        }
    }


    // TODO: this needs more investigating, but this seems to work for some
    u32 zero = 0;
    fwrite(&zero, sizeof(u8), 4, patFp);
    fwrite(&zero, sizeof(u8), 4, patFp);

    free(patFilename);
    if (ndsData) free(ndsData);    // crash here?
    fclose(fp); fclose(patFp);
    // Remove this check when the issue above is resolved
    if (patFp)

    return 0;
}
