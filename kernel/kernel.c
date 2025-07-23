#include "../include/stdio.h"
#include "../include/stdint.h"

// Declaraciones externas de funciones VGA
extern void vga_set_color(uint8_t color);

// Función principal del kernel (ya no es el punto de entrada)
void kernel_main(void)
{
    printf("[KERNEL] Kernel principal iniciado\n");

    // Aquí van las funciones principales del kernel
    printf("[KERNEL] Iniciando servicios del kernel...\n");

    // TODO: Inicializar scheduler, gestión de memoria, etc.
    printf("  [OK] Servicios básicos del kernel\n");

    // Mostrar que el kernel está funcionando
    printf("[KERNEL] Sistema operativo completamente cargado\n");
    printf("[KERNEL] Entrando en modo de funcionamiento normal\n");

    // Simular algunas operaciones del kernel
    printf("\n[KERNEL] Realizando operaciones de prueba...\n");

    // Cambiar color para indicar que estamos en el kernel
    vga_set_color(0x0A); // Verde claro
    printf("¡Bienvenido a MicroCIOMOS!\n");
    printf("Sistema operativo con sistema de archivos funcional\n");

    vga_set_color(0x07); // Volver a blanco
    printf("\n[KERNEL] Sistema listo para uso\n");

    // Loop principal del kernel
    printf("[KERNEL] Entrando en bucle principal...\n");
    while (1)
    {
        // Aquí iría el scheduler y otras tareas del kernel
        __asm__ volatile("hlt"); // Detener CPU hasta próxima interrupción
    }
}
