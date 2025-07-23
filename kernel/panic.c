#include "../include/stdio.h"

void panic(const char *str)
{
    printf("Kernel panic: %s\n", str);
    // Aquí podrías agregar más acciones, como volcar el estado del sistema
    // o reiniciar el sistema.

    // Bucle infinito para detener la ejecución
    while (1)
    {
        __asm__ volatile("hlt"); // Instrucción para poner la CPU en modo de espera
    }
}