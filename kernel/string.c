#include "string.h"
#include <stdint.h>

// --- Cadenas ---

size_t strlen(const char *s)
{
    size_t len = 0;
    while (*s++)
        len++;
    return len;
}

char *strcpy(char *dest, const char *src)
{
    char *ret = dest;
    while ((*dest++ = *src++))
        ;
    return ret;
}

char *strncpy(char *dest, const char *src, size_t n)
{
    char *ret = dest;
    size_t i = 0;
    for (; i < n && src[i] != '\0'; i++)
        dest[i] = src[i];
    for (; i < n; i++)
        dest[i] = '\0';
    return ret;
}

int strcmp(const char *s1, const char *s2)
{
    while (*s1 && (*s1 == *s2))
    {
        s1++;
        s2++;
    }
    return (unsigned char)*s1 - (unsigned char)*s2;
}

int strncmp(const char *s1, const char *s2, size_t n)
{
    size_t i;
    for (i = 0; i < n; i++)
    {
        if (s1[i] != s2[i] || s1[i] == '\0' || s2[i] == '\0')
            return (unsigned char)s1[i] - (unsigned char)s2[i];
    }
    return 0;
}

char *strcat(char *dest, const char *src)
{
    char *ret = dest;
    while (*dest)
        dest++;
    while ((*dest++ = *src++))
        ;
    return ret;
}

char *strncat(char *dest, const char *src, size_t n)
{
    char *ret = dest;
    while (*dest)
        dest++;
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++)
        dest[i] = src[i];
    dest[i] = '\0';
    return ret;
}

// --- Memoria ---

void *memcpy(void *dest, const void *src, size_t n)
{
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;
    for (size_t i = 0; i < n; i++)
        d[i] = s[i];
    return dest;
}

void *memset(void *dest, int c, size_t n)
{
    unsigned char *d = (unsigned char *)dest;
    for (size_t i = 0; i < n; i++)
        d[i] = (unsigned char)c;
    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
    const unsigned char *a = (const unsigned char *)s1;
    const unsigned char *b = (const unsigned char *)s2;
    for (size_t i = 0; i < n; i++)
    {
        if (a[i] != b[i])
            return a[i] - b[i];
    }
    return 0;
}

void *memmove(void *dest, const void *src, size_t n)
{
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;
    if (d < s)
    {
        for (size_t i = 0; i < n; i++)
            d[i] = s[i];
    }
    else if (d > s)
    {
        for (size_t i = n; i != 0; i--)
            d[i - 1] = s[i - 1];
    }
    return dest;
}

void itoa(int value, char *str, int base)
{
    char *ptr = str, *ptr1 = str, tmp_char;
    int tmp_value;
    int negative = 0;

    if (value == 0)
    {
        *str++ = '0';
        *str = '\0';
        return;
    }

    if (value < 0 && base == 10)
    {
        negative = 1;
        value = -value;
    }

    while (value != 0)
    {
        tmp_value = value % base;
        *ptr++ = (tmp_value < 10) ? ('0' + tmp_value) : ('a' + tmp_value - 10);
        value /= base;
    }

    if (negative)
        *ptr++ = '-';

    *ptr-- = '\0';

    while (ptr1 < ptr)
    {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
}

void uitoa(unsigned int value, char *str, int base)
{
    char *ptr = str, *ptr1 = str, tmp_char;
    unsigned int tmp_value;

    if (value == 0)
    {
        *str++ = '0';
        *str = '\0';
        return;
    }

    while (value != 0)
    {
        tmp_value = value % base;
        *ptr++ = (tmp_value < 10) ? ('0' + tmp_value) : ('a' + tmp_value - 10);
        value /= base;
    }

    *ptr-- = '\0';

    while (ptr1 < ptr)
    {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
}
