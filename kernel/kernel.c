#include "../include/stdio.h"
#include "../include/stdint.h"

// Declaraciones externas de funciones VGA
extern void vga_set_color(uint8_t color);

extern void vga_clear_screen(void);

// Función principal del kernel (ya no es el punto de entrada)
void kernel_main(void)
{

    vga_clear_screen();

    vga_set_color(0x0A); // Verde claro

    for (int i = 1; i <= 20; i++)
    {
        printf("\n");
    }

    printf("\t\t\t\t\t\t\t\t\t MicroCIOMOS");

    while (1)
    {
        // Aquí iría el scheduler y otras tareas del kernel
        __asm__ volatile("hlt"); // Detener CPU hasta próxima interrupción
    }
    
}
