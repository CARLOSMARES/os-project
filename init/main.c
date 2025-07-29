#include "../include/stdio.h"
#include "../include/vga_color.h"
#include "../include/fs.h"

#ifdef __cplusplus
extern "C"
{
#endif

    __attribute__((section(".text"), used)) void _start(void);
    __attribute__((used)) void main(void);

    __attribute__((section(".text"), used)) void _start(void)
    {
        vga_initialize();

        // DEBUG: Escribir 'K' en la esquina superior izquierda (VGA)
        volatile unsigned short *vga = (unsigned short *)0xB8000;
        vga[0] = (0x0F << 8) | 'K';
        main();
        while (1)
        {
            __asm__ volatile("hlt");
        }
    }

    __attribute__((used)) void main(void)
    {
        fs_init(); // Inicializa el sistema de archivos
        //{ Mensaje centrado usando printf y padding
        const char *msg = "MicroCIOMOS";
        unsigned int len = 12; // strlen("MicroCIOMOS")
        unsigned int col = (80 - len) / 2;
        for (unsigned int i = 0; i < col; ++i)
            putchar(' ');
        printf("%s\n", msg);
        while (1)
        {
            __asm__ volatile("hlt");
        }
    }

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#endif