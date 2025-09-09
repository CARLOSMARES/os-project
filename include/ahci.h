#ifndef _AHCI_H
#define _AHCI_H

#include "stdint.h"

// Tamaño de bloque típico de disco
#define AHCI_BLOCK_SIZE 512

// Dispositivo AHCI
typedef struct
{
    uint64_t bar5; // Dirección base de la memoria mapeada
    uint8_t port;  // Puerto AHCI
} ahci_device_t;

// Inicializa el controlador AHCI y detecta dispositivos SATA
int ahci_init(ahci_device_t *dev);

// Leer un bloque (512 bytes) desde el disco AHCI
int ahci_read_block(ahci_device_t *dev, uint32_t lba, void *buffer);

// Escribir un bloque (512 bytes) al disco AHCI
int ahci_write_block(ahci_device_t *dev, uint32_t lba, const void *buffer);

#endif
