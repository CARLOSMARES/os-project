#include "../include/stdio.h"
#include "../include/stdint.h"
#include "../include/pci.h"
#include "../include/vga_color.h"
#include "../include/ahci.h"

// Declaraciones externas de funciones VGA
extern void vga_set_color(uint8_t color);
extern void vga_clear_screen(void);

void kernel_main(void)
{
    vga_clear_screen();
    vga_set_color(0x0A); // Verde claro

    for (int i = 1; i <= 20; i++)
        printf("\n");

    printf("\t\t\t\t\t\t\t\t\t MicroCIOMOS\n");

    // --- PCI / AHCI ---
    ahci_device_t dev;
    if (ahci_init(&dev) == 0)
    {
        printf("AHCI inicializado correctamente\n");
        printf("BAR5 = 0x%llx, Puerto = %u\n", (unsigned long long)dev.bar5, dev.port);

        // Ejemplo de lectura de bloque 0
        uint8_t read_buffer[AHCI_BLOCK_SIZE];
        if (ahci_read_block(&dev, 0, read_buffer) == 0)
        {
            printf("Bloque 0 leído correctamente\n");
        }
        else
        {
            printf("Error leyendo bloque 0\n");
        }

        // Ejemplo de escritura de bloque 1
        uint8_t write_buffer[AHCI_BLOCK_SIZE];
        for (int i = 0; i < AHCI_BLOCK_SIZE; i++)
            write_buffer[i] = i % 256;

        if (ahci_write_block(&dev, 1, write_buffer) == 0)
        {
            printf("Bloque 1 escrito correctamente\n");
        }
        else
        {
            printf("Error escribiendo bloque 1\n");
        }
    }
    else
    {
        printf("No se encontró controlador AHCI en PCI\n");
    }

    while (1)
    {
        __asm__ volatile("hlt"); // Espera sin consumir CPU
    }
}
