#include "../include/stdio.h"
#include "../include/stdint.h"

// Declaraciones externas de funciones VGA
extern void vga_initialize(void);
extern void vga_set_color(uint8_t color);

// Función principal del kernel
__attribute__((used)) void kernel_main(void)
{
    // Inicializar VGA
    vga_initialize();

    // Probar las funciones stdio
    printf("MicroCIOMOS\n");

    // Loop infinito
    while (1)
    {
        __asm__ volatile("hlt");
    }
}
