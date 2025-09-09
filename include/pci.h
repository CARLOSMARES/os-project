// include/pci.h
#ifndef _PCI_H
#define _PCI_H

#include "stdint.h"

typedef struct
{
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t bus;
    uint8_t slot;
    uint8_t func;
    uint8_t prog_if;
    uint8_t subclass;
    uint8_t class_code;
    uint8_t header_type;
} pci_device_t;

/* Lectura/escritura de configuración PCI vía ports 0xCF8/0xCFC */
uint32_t pci_read_config_32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
void pci_write_config_32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value);

/* Devuelve el BAR físico (64-bit si aplica) */
uint64_t pci_get_bar(uint8_t bus, uint8_t slot, uint8_t func, uint8_t bar_index);

/* Buscar primer dispositivo AHCI (class=0x01, subclass=0x06, prog-if=0x01) */
int pci_find_ahci(pci_device_t *out_dev);

#endif // _PCI_H
