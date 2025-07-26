#include "../include/stdio.h"
#include "../include/stdint.h"
#include "../include/sys/types.h"

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
    {
        putchar(*str);
        str++;
    }
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

int itoa(int value, char *str, int base)
{
    if (!str)
        return 0;

    char *ptr = str;
    char *ptr1 = str;
    char tmp_char;
    int tmp_value;

    // Manejar valor 0
    if (value == 0)
    {
        *ptr++ = '0';
        *ptr = '\0';
        return 1;
    }

    // Manejar números negativos para base 10
    int negative = 0;
    if (value < 0 && base == 10)
    {
        negative = 1;
        value = -value;
    }

    // Convertir dígitos
    while (value)
    {
        tmp_value = value % base;
        *ptr++ = (tmp_value < 10) ? (tmp_value + '0') : (tmp_value - 10 + 'a');
        value /= base;
    }

    // Agregar signo negativo
    if (negative)
    {
        *ptr++ = '-';
    }

    *ptr-- = '\0';

    // Invertir cadena
    while (ptr1 < ptr)
    {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }

    return 1;
}

int uitoa(unsigned int value, char *str, int base)
{
    if (!str)
        return 0;

    char *ptr = str;
    char *ptr1 = str;
    char tmp_char;
    unsigned int tmp_value;

    if (value == 0)
    {
        *ptr++ = '0';
        *ptr = '\0';
        return 1;
    }

    while (value)
    {
        tmp_value = value % base;
        *ptr++ = (tmp_value < 10) ? (tmp_value + '0') : (tmp_value - 10 + 'a');
        value /= base;
    }

    *ptr-- = '\0';

    while (ptr1 < ptr)
    {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }

    return 1;
}

int ltoa(long value, char *str, int base)
{
    if (!str)
        return 0;

    char *ptr = str;
    char *ptr1 = str;
    char tmp_char;
    long tmp_value;

    if (value == 0)
    {
        *ptr++ = '0';
        *ptr = '\0';
        return 1;
    }

    int negative = 0;
    if (value < 0 && base == 10)
    {
        negative = 1;
        value = -value;
    }

    while (value)
    {
        tmp_value = value % base;
        *ptr++ = (tmp_value < 10) ? (tmp_value + '0') : (tmp_value - 10 + 'a');
        value /= base;
    }

    if (negative)
    {
        *ptr++ = '-';
    }

    *ptr-- = '\0';

    while (ptr1 < ptr)
    {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }

    return 1;
}

int ultoa(unsigned long value, char *str, int base)
{
    if (!str)
        return 0;

    char *ptr = str;
    char *ptr1 = str;
    char tmp_char;
    unsigned long tmp_value;

    if (value == 0)
    {
        *ptr++ = '0';
        *ptr = '\0';
        return 1;
    }

    while (value)
    {
        tmp_value = value % base;
        *ptr++ = (tmp_value < 10) ? (tmp_value + '0') : (tmp_value - 10 + 'a');
        value /= base;
    }

    *ptr-- = '\0';

    while (ptr1 < ptr)
    {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }

    return 1;
}

// ============================================================================
// FUNCIONES DE CADENAS
// ============================================================================

size_t strlen(const char *str)
{
    if (!str)
        return 0;

    size_t len = 0;
    while (str[len])
    {
        len++;
    }
    return len;
}

char *strcpy(char *dest, const char *src)
{
    if (!dest || !src)
        return dest;

    char *d = dest;
    while ((*d++ = *src++))
        ;
    return dest;
}

char *strncpy(char *dest, const char *src, size_t n)
{
    if (!dest || !src)
        return dest;

    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++)
    {
        dest[i] = src[i];
    }
    for (; i < n; i++)
    {
        dest[i] = '\0';
    }
    return dest;
}

int strcmp(const char *str1, const char *str2)
{
    if (!str1 || !str2)
        return 0;

    while (*str1 && (*str1 == *str2))
    {
        str1++;
        str2++;
    }
    return *(unsigned char *)str1 - *(unsigned char *)str2;
}

int strncmp(const char *str1, const char *str2, size_t n)
{
    if (!str1 || !str2)
        return 0;

    while (n && *str1 && (*str1 == *str2))
    {
        str1++;
        str2++;
        n--;
    }

    if (n == 0)
    {
        return 0;
    }
    else
    {
        return *(unsigned char *)str1 - *(unsigned char *)str2;
    }
}

char *strcat(char *dest, const char *src)
{
    if (!dest || !src)
        return dest;

    char *d = dest;
    while (*d)
        d++; // Ir al final de dest
    while ((*d++ = *src++))
        ; // Copiar src
    return dest;
}

char *strncat(char *dest, const char *src, size_t n)
{
    if (!dest || !src)
        return dest;

    char *d = dest;
    while (*d)
        d++; // Ir al final de dest

    while (n-- && *src)
    {
        *d++ = *src++;
    }
    *d = '\0';

    return dest;
}

char *strchr(const char *str, int c)
{
    if (!str)
        return NULL;

    while (*str)
    {
        if (*str == c)
        {
            return (char *)str;
        }
        str++;
    }

    if (c == '\0')
    {
        return (char *)str;
    }

    return NULL;
}

char *strstr(const char *haystack, const char *needle)
{
    if (!haystack || !needle)
        return NULL;

    if (*needle == '\0')
    {
        return (char *)haystack;
    }

    while (*haystack)
    {
        const char *h = haystack;
        const char *n = needle;

        while (*h && *n && (*h == *n))
        {
            h++;
            n++;
        }

        if (*n == '\0')
        {
            return (char *)haystack;
        }

        haystack++;
    }

    return NULL;
}

// ============================================================================
// FUNCIONES DE MEMORIA
// ============================================================================

void *memset(void *ptr, int value, size_t num)
{
    if (!ptr)
        return ptr;

    unsigned char *p = (unsigned char *)ptr;
    while (num--)
    {
        *p++ = (unsigned char)value;
    }
    return ptr;
}

void *memcpy(void *dest, const void *src, size_t num)
{
    if (!dest || !src)
        return dest;

    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;

    while (num--)
    {
        *d++ = *s++;
    }

    return dest;
}

void *memmove(void *dest, const void *src, size_t num)
{
    if (!dest || !src)
        return dest;

    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;

    if (d < s)
    {
        // Copiar hacia adelante
        while (num--)
        {
            *d++ = *s++;
        }
    }
    else
    {
        // Copiar hacia atrás
        d += num;
        s += num;
        while (num--)
        {
            *(--d) = *(--s);
        }
    }

    return dest;
}

int memcmp(const void *ptr1, const void *ptr2, size_t num)
{
    if (!ptr1 || !ptr2)
        return 0;

    const unsigned char *p1 = (const unsigned char *)ptr1;
    const unsigned char *p2 = (const unsigned char *)ptr2;

    while (num--)
    {
        if (*p1 != *p2)
        {
            return *p1 - *p2;
        }
        p1++;
        p2++;
    }

    return 0;
}

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

            // Manejar %%
            if (*format == '%')
            {
                *ptr++ = '%';
                written++;
                format++;
                continue;
            }

            // Parsear especificadores simples
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
                // Convertir a mayúsculas
                for (int i = 0; num_str[i]; i++)
                {
                    if (num_str[i] >= 'a' && num_str[i] <= 'f')
                    {
                        num_str[i] = num_str[i] - 'a' + 'A';
                    }
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
                // Carácter desconocido, imprimir literal
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

    // Enviar a la salida estándar (VGA)
    for (int i = 0; buffer[i] && i < result; i++)
    {
        putchar(buffer[i]);
    }

    return result;
}