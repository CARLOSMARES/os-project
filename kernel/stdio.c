#include "stdio.h"
#include "string.h"
#include "stdint.h"
#include "sys/types.h"

// Variables globales para VGA (definidas en vga_color.c)
extern uint16_t *vga_buffer;
extern int vga_row, vga_col;
extern uint8_t vga_color;

// Streams estándar simulados
FILE stdin_file = {0, 0, 0};
FILE stdout_file = {1, 0, 0};
FILE stderr_file = {2, 0, 0};

FILE *stdin = &stdin_file;
FILE *stdout = &stdout_file;
FILE *stderr = &stderr_file;

// Función externa para escribir carácter en VGA (definida en vga_color.c)
extern void vga_putchar(char c);
extern void vga_clear_screen(void);

// ============================================================================
// FUNCIONES BÁSICAS DE ENTRADA/SALIDA
// ============================================================================

int putchar(int c)
{
    vga_putchar((char)c);
    return c;
}

int puts(const char *str)
{
    if (!str)
        return -1;

    while (*str)
        putchar(*str++);

    putchar('\n');
    return 1;
}

int getchar(void)
{
    // TODO: Implementar entrada de teclado cuando esté disponible
    return -1;
}

// ============================================================================
// FUNCIONES DE CONVERSIÓN DE NÚMEROS
// ============================================================================

int itoa(int value, char *str, int base);
int uitoa(unsigned int value, char *str, int base);
int ltoa(long value, char *str, int base);
int ultoa(unsigned long value, char *str, int base);

// ============================================================================
// IMPLEMENTACIÓN BÁSICA DE PRINTF
// ============================================================================

// Estructura para argumentos variables (va_list)
typedef char *va_list;
#define va_start(ap, last) ap = (char *)&last + sizeof(last)
#define va_arg(ap, type) (*(type *)((ap += sizeof(type)) - sizeof(type)))
#define va_end(ap) ap = NULL

static int vsnprintf_internal(char *str, size_t size, const char *format, va_list args)
{
    if (!str || !format || size == 0)
        return 0;

    char *ptr = str;
    size_t written = 0;

    while (*format && written < size - 1)
    {
        if (*format == '%')
        {
            format++;

            if (*format == '%')
            {
                *ptr++ = '%';
                written++;
                format++;
                continue;
            }

            switch (*format)
            {
            case 'd':
            case 'i':
            {
                int value = va_arg(args, int);
                char num_str[32];
                itoa(value, num_str, 10);
                size_t len = strlen(num_str);
                if (written + len < size - 1)
                {
                    strcpy(ptr, num_str);
                    ptr += len;
                    written += len;
                }
                break;
            }
            case 'u':
            {
                unsigned int value = va_arg(args, unsigned int);
                char num_str[32];
                uitoa(value, num_str, 10);
                size_t len = strlen(num_str);
                if (written + len < size - 1)
                {
                    strcpy(ptr, num_str);
                    ptr += len;
                    written += len;
                }
                break;
            }
            case 'x':
            {
                unsigned int value = va_arg(args, unsigned int);
                char num_str[32];
                uitoa(value, num_str, 16);
                size_t len = strlen(num_str);
                if (written + len < size - 1)
                {
                    strcpy(ptr, num_str);
                    ptr += len;
                    written += len;
                }
                break;
            }
            case 'X':
            {
                unsigned int value = va_arg(args, unsigned int);
                char num_str[32];
                uitoa(value, num_str, 16);
                for (int i = 0; num_str[i]; i++)
                {
                    if (num_str[i] >= 'a' && num_str[i] <= 'f')
                        num_str[i] = num_str[i] - 'a' + 'A';
                }
                size_t len = strlen(num_str);
                if (written + len < size - 1)
                {
                    strcpy(ptr, num_str);
                    ptr += len;
                    written += len;
                }
                break;
            }
            case 's':
            {
                char *s = va_arg(args, char *);
                if (s)
                {
                    size_t len = strlen(s);
                    if (written + len < size - 1)
                    {
                        strcpy(ptr, s);
                        ptr += len;
                        written += len;
                    }
                }
                else
                {
                    const char *null_str = "(null)";
                    size_t len = strlen(null_str);
                    if (written + len < size - 1)
                    {
                        strcpy(ptr, null_str);
                        ptr += len;
                        written += len;
                    }
                }
                break;
            }
            case 'c':
            {
                char c = (char)va_arg(args, int);
                *ptr++ = c;
                written++;
                break;
            }
            default:
                *ptr++ = '%';
                *ptr++ = *format;
                written += 2;
                break;
            }
            format++;
        }
        else
        {
            *ptr++ = *format++;
            written++;
        }
    }

    *ptr = '\0';
    return written;
}

int snprintf(char *str, size_t size, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int result = vsnprintf_internal(str, size, format, args);
    va_end(args);
    return result;
}

int sprintf(char *str, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int result = vsnprintf_internal(str, PRINTF_BUFFER_SIZE, format, args);
    va_end(args);
    return result;
}

int printf(const char *format, ...)
{
    char buffer[PRINTF_BUFFER_SIZE];
    va_list args;
    va_start(args, format);
    int result = vsnprintf_internal(buffer, PRINTF_BUFFER_SIZE, format, args);
    va_end(args);

    for (int i = 0; buffer[i] && i < result; i++)
        putchar(buffer[i]);

    return result;
}