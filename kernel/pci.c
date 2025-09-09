// kernel/pci.c
#include "pci.h"
#include "io.h"
#include "stdint.h"
#include "stdio.h" // si tienes printf en kernel; si no, elimina las prints

/* I/O ports for legacy PCI config */
#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

/* Construye la dirección para accessar config space */
static inline uint32_t pci_config_address(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
    /* Alineamos offset a dword (bits 0..1 zero) */
    uint32_t addr = (1u << 31) | ((uint32_t)bus << 16) | ((uint32_t)slot << 11) | ((uint32_t)func << 8) | (offset & 0xFC);
    return addr;
}

uint32_t pci_read_config_32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
    uint32_t address = pci_config_address(bus, slot, func, offset);
    outl(PCI_CONFIG_ADDRESS, address);
    uint32_t val = inl(PCI_CONFIG_DATA);
    return val;
}

void pci_write_config_32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value)
{
    uint32_t address = pci_config_address(bus, slot, func, offset);
    outl(PCI_CONFIG_ADDRESS, address);
    outl(PCI_CONFIG_DATA, value);
}

/* Lee vendor/device word desde offset 0x00 (dword 0) */
static inline uint16_t pci_read_vendor(uint8_t bus, uint8_t slot, uint8_t func)
{
    uint32_t d = pci_read_config_32(bus, slot, func, 0x00);
    return (uint16_t)(d & 0xFFFF);
}

static inline uint8_t pci_read_header_type(uint8_t bus, uint8_t slot, uint8_t func)
{
    uint32_t d = pci_read_config_32(bus, slot, func, 0x0C);
    return (uint8_t)((d >> 16) & 0xFF);
}

/* Obtiene BAR (0..5). Para memoria puede ser 64-bit (consumes next BAR). */
uint64_t pci_get_bar(uint8_t bus, uint8_t slot, uint8_t func, uint8_t bar_index)
{
    if (bar_index >= 6)
        return 0;
    uint8_t offset = 0x10 + (bar_index * 4);
    uint32_t bar_lo = pci_read_config_32(bus, slot, func, offset);

    /* Si es IO space (bit0 = 1) */
    if (bar_lo & 0x1)
    {
        uint64_t io_addr = (uint64_t)(bar_lo & 0xFFFFFFFC);
        return io_addr; // IO ports address
    }
    else
    {
        /* Memory BAR: mask lower bits */
        uint32_t type = (bar_lo >> 1) & 0x3;
        if (type == 0x2)
        {
            // 64-bit BAR: read next dword as high
            uint32_t bar_hi = pci_read_config_32(bus, slot, func, offset + 4);
            uint64_t addr = ((uint64_t)bar_hi << 32) | (bar_lo & 0xFFFFFFF0);
            return addr;
        }
        else
        {
            uint64_t addr = (uint64_t)(bar_lo & 0xFFFFFFF0);
            return addr;
        }
    }
}

/* Escanea funciones de un dispositivo (0..7). Si vendor==0xFFFF el dispositivo no existe */
static int probe_function(uint8_t bus, uint8_t slot, uint8_t func, pci_device_t *out)
{
    uint16_t vendor = pci_read_vendor(bus, slot, func);
    if (vendor == 0xFFFF)
        return -1;

    uint32_t d8 = pci_read_config_32(bus, slot, func, 0x08);
    uint8_t prog_if = (d8 >> 8) & 0xFF;
    uint8_t subclass = (d8 >> 16) & 0xFF;
    uint8_t classcode = (d8 >> 24) & 0xFF;
    uint32_t d0 = pci_read_config_32(bus, slot, func, 0x00);
    uint16_t device = (uint16_t)((d0 >> 16) & 0xFFFF);
    uint8_t header = pci_read_header_type(bus, slot, func);

    if (out)
    {
        out->vendor_id = vendor;
        out->device_id = device;
        out->bus = bus;
        out->slot = slot;
        out->func = func;
        out->prog_if = prog_if;
        out->subclass = subclass;
        out->class_code = classcode;
        out->header_type = header;
    }
    return 0;
}

/*
 * pci_find_ahci: busca un dispositivo con class=0x01 (Mass Storage),
 * subclass=0x06 (SATA), prog-if=0x01 (AHCI)
 */
int pci_find_ahci(pci_device_t *out_dev)
{
    for (uint8_t bus = 0; bus < 256; bus++)
    {
        for (uint8_t slot = 0; slot < 32; slot++)
        {
            // Probar función 0
            if (probe_function(bus, slot, 0, NULL) != 0)
                continue;

            uint8_t header = pci_read_header_type(bus, slot, 0);
            int multifunc = header & 0x80;

            uint8_t max_func = multifunc ? 8 : 1;
            for (uint8_t func = 0; func < max_func; func++)
            {
                pci_device_t dev;
                if (probe_function(bus, slot, func, &dev) != 0)
                    continue;

                if (dev.class_code == 0x01 && dev.subclass == 0x06 && dev.prog_if == 0x01)
                {
                    if (out_dev)
                        *out_dev = dev;
                    return 0;
                }
            }
        }
    }
    return -1;
}

/* OPTIONAL: función de debug para listar dispositivos PCI (útil en QEMU) */
#ifdef PCI_DEBUG
void pci_dump_all(void)
{
    pci_device_t dev;
    for (uint8_t bus = 0; bus < 256; bus++)
    {
        for (uint8_t slot = 0; slot < 32; slot++)
        {
            if (probe_function(bus, slot, 0, NULL) != 0)
                continue;
            uint8_t header = pci_read_header_type(bus, slot, 0);
            int multifunc = header & 0x80;
            uint8_t max_func = multifunc ? 8 : 1;
            for (uint8_t func = 0; func < max_func; func++)
            {
                if (probe_function(bus, slot, func, &dev) != 0)
                    continue;
                printf("PCI dev: bus=%u slot=%u func=%u vendor=0x%04x device=0x%04x class=0x%02x subclass=0x%02x prog-if=0x%02x\n",
                       dev.bus, dev.slot, dev.func, dev.vendor_id, dev.device_id,
                       dev.class_code, dev.subclass, dev.prog_if);
            }
        }
    }
}
#endif
