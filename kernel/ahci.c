#include "ahci.h"
#include "pci.h"
#include "stdio.h"
#include "stdint.h"

// Para simplificar, solo un dispositivo
static ahci_device_t ahci_dev_instance;

// Export pointer for other modules that expect 'ahci_dev'. Start as NULL
// so code knows AHCI is not initialized yet.
ahci_device_t *ahci_dev = NULL;

// Inicializa AHCI detectando puerto SATA
int ahci_init(ahci_device_t *dev)
{
    pci_device_t pci_dev;
    if (pci_find_ahci(&pci_dev) != 0)
    {
        printf("AHCI no encontrado\n");
        return -1;
    }

    dev->bar5 = pci_get_bar(pci_dev.bus, pci_dev.slot, pci_dev.func, 5);
    dev->port = 0; // asumimos primer puerto AHCI
    ahci_dev_instance = *dev;
    /* mark global pointer as initialized */
    ahci_dev = &ahci_dev_instance;

    printf("AHCI inicializado: BAR5=0x%llx, puerto=%u\n", (unsigned long long)dev->bar5, dev->port);
    return 0;
}

// --- Mini función dummy para leer un bloque ---
int ahci_read_block(ahci_device_t *dev, uint32_t lba, void *buffer)
{
    // Por ahora copiamos del fs_storage como si fuera disco
    extern uint8_t fs_storage[];
    if (!buffer)
        return -1;

    uint8_t *src = fs_storage + (lba * AHCI_BLOCK_SIZE);
    for (int i = 0; i < AHCI_BLOCK_SIZE; i++)
        ((uint8_t *)buffer)[i] = src[i];

    return 0;
}

// --- Mini función dummy para escribir un bloque ---
int ahci_write_block(ahci_device_t *dev, uint32_t lba, const void *buffer)
{
    extern uint8_t fs_storage[];
    if (!buffer)
        return -1;

    uint8_t *dst = fs_storage + (lba * AHCI_BLOCK_SIZE);
    for (int i = 0; i < AHCI_BLOCK_SIZE; i++)
        dst[i] = ((uint8_t *)buffer)[i];

    return 0;
}
