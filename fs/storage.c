#include "fs.h"

// Almacén global para simulación de disco (utilizado por ahci.c)
uint8_t fs_storage[MAX_BLOCKS * BLOCK_SIZE];
