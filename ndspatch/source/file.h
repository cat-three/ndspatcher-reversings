#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "structs.h"

// Corresponds to sub_401450
struct NDSData    initNDSData(char* filename);           // sub_401450

// Corresponds to sub_4018C0
void              freeNDSData(struct NDSData nds_data);            // sub_4018C0
