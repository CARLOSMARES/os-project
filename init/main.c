#include "../include/stdio.h"
#include "../include/vga_color.h"

__attribute__((used)) void main(void)
{
    vga_initialize();
    // Mensaje centrado usando printf y padding
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