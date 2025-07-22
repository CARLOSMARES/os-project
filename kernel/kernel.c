#include <stdint.h>

// Atributos de color VGA
#define VGA_COLOR_BLACK 0
#define VGA_COLOR_BLUE 1
#define VGA_COLOR_GREEN 2
#define VGA_COLOR_CYAN 3
#define VGA_COLOR_RED 4
#define VGA_COLOR_MAGENTA 5
#define VGA_COLOR_BROWN 6
#define VGA_COLOR_LIGHT_GREY 7
#define VGA_COLOR_DARK_GREY 8
#define VGA_COLOR_LIGHT_BLUE 9
#define VGA_COLOR_LIGHT_GREEN 10
#define VGA_COLOR_LIGHT_CYAN 11
#define VGA_COLOR_LIGHT_RED 12
#define VGA_COLOR_LIGHT_MAGENTA 13
#define VGA_COLOR_LIGHT_BROWN 14
#define VGA_COLOR_WHITE 15

static inline uint8_t vga_entry_color(uint8_t fg, uint8_t bg)
{
    return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color)
{
    return (uint16_t)uc | (uint16_t)color << 8;
}

void terminal_writestring(const char *data)
{
    uint16_t *const VGA_MEMORY = (uint16_t *)0xB8000;
    const uint8_t color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

    // Limpiar pantalla primero
    for (int i = 0; i < 80 * 25; i++)
    {
        VGA_MEMORY[i] = vga_entry(' ', color);
    }

    // Escribir el string
    for (int i = 0; data[i] != '\0'; i++)
    {
        VGA_MEMORY[i] = vga_entry(data[i], color);
    }
}

__attribute__((used)) void kernel_main(void)
{
    terminal_writestring("MicroCIOMOS x64 - Sistema iniciado correctamente!");

    // Loop infinito
    while (1)
    {
        __asm__ volatile("hlt");
    }
}
