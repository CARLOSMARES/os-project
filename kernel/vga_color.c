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

#include "stdint.h"

// Constantes de VGA
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

// Variables globales para manejo de VGA
uint16_t *vga_buffer = (uint16_t *)VGA_MEMORY;
int vga_row = 0;
int vga_col = 0;
uint8_t vga_color = VGA_COLOR_LIGHT_GREY | VGA_COLOR_BLACK << 4;

// Función para crear un color VGA
static inline uint8_t vga_entry_color(uint8_t fg, uint8_t bg)
{
    return fg | bg << 4;
}

// Función para crear una entrada VGA (carácter + color)
static inline uint16_t vga_entry(unsigned char uc, uint8_t color)
{
    return (uint16_t)uc | (uint16_t)color << 8;
}

// Función para establecer el color
void vga_set_color(uint8_t color)
{
    vga_color = color;
}

// Función para limpiar la pantalla
void vga_clear_screen(void)
{
    for (int y = 0; y < VGA_HEIGHT; y++)
    {
        for (int x = 0; x < VGA_WIDTH; x++)
        {
            const int index = y * VGA_WIDTH + x;
            vga_buffer[index] = vga_entry(' ', vga_color);
        }
    }
    vga_row = 0;
    vga_col = 0;
}

// Función para hacer scroll hacia arriba
static void vga_scroll_up(void)
{
    // Mover todas las líneas una posición hacia arriba
    for (int y = 0; y < VGA_HEIGHT - 1; y++)
    {
        for (int x = 0; x < VGA_WIDTH; x++)
        {
            vga_buffer[y * VGA_WIDTH + x] = vga_buffer[(y + 1) * VGA_WIDTH + x];
        }
    }

    // Limpiar la última línea
    for (int x = 0; x < VGA_WIDTH; x++)
    {
        vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', vga_color);
    }

    vga_row = VGA_HEIGHT - 1;
    vga_col = 0;
}

// Función para poner un carácter en una posición específica
void vga_putentryat(char c, uint8_t color, int x, int y)
{
    const int index = y * VGA_WIDTH + x;
    vga_buffer[index] = vga_entry(c, color);
}

// Función principal para escribir un carácter
void vga_putchar(char c)
{
    // Manejar caracteres especiales
    if (c == '\n')
    {
        vga_col = 0;
        vga_row++;
    }
    else if (c == '\r')
    {
        vga_col = 0;
    }
    else if (c == '\t')
    {
        // Tab = 4 espacios
        int spaces = 4 - (vga_col % 4);
        for (int i = 0; i < spaces; i++)
        {
            vga_putchar(' ');
        }
        return;
    }
    else if (c == '\b')
    {
        // Backspace
        if (vga_col > 0)
        {
            vga_col--;
            vga_putentryat(' ', vga_color, vga_col, vga_row);
        }
        else if (vga_row > 0)
        {
            vga_row--;
            vga_col = VGA_WIDTH - 1;
            vga_putentryat(' ', vga_color, vga_col, vga_row);
        }
        return;
    }
    else
    {
        // Carácter normal
        vga_putentryat(c, vga_color, vga_col, vga_row);
        vga_col++;
    }

    // Manejar fin de línea
    if (vga_col >= VGA_WIDTH)
    {
        vga_col = 0;
        vga_row++;
    }

    // Manejar fin de pantalla (scroll)
    if (vga_row >= VGA_HEIGHT)
    {
        vga_scroll_up();
    }
}

// Función para escribir una cadena
void vga_write_string(const char *data)
{
    if (!data)
        return;

    while (*data)
    {
        vga_putchar(*data);
        data++;
    }
}

// Función para inicializar VGA
void vga_initialize(void)
{
    vga_row = 0;
    vga_col = 0;
    vga_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_buffer = (uint16_t *)VGA_MEMORY;
    vga_clear_screen();
}