#ifndef _STDIO_H
#define _STDIO_H

#include "stdint.h"
#include "types.h"

// Definir size_t si no está definido
#ifndef _SIZE_T_DEFINED
#define _SIZE_T_DEFINED
typedef unsigned long size_t;
#endif

// Constantes para printf
#define PRINTF_BUFFER_SIZE 1024

// Definiciones para archivos (básico para SO desde cero)
typedef struct
{
    int fd;       // file descriptor
    int flags;    // flags de estado
    int position; // posición actual
} FILE;

// Streams estándar (simulados para SO básico)
extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

// Funciones básicas de entrada/salida de caracteres
int putchar(int c);
int puts(const char *str);
int getchar(void);

// Funciones de cadenas formateadas
int printf(const char *format, ...);
int sprintf(char *str, const char *format, ...);
int snprintf(char *str, size_t size, const char *format, ...);

// Funciones auxiliares para conversión
int itoa(int value, char *str, int base);
int uitoa(unsigned int value, char *str, int base);
int ltoa(long value, char *str, int base);
int ultoa(unsigned long value, char *str, int base);

// Funciones de cadenas básicas
size_t strlen(const char *str);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);
int strcmp(const char *str1, const char *str2);
int strncmp(const char *str1, const char *str2, size_t n);
char *strcat(char *dest, const char *src);
char *strncat(char *dest, const char *src, size_t n);
char *strchr(const char *str, int c);
char *strstr(const char *haystack, const char *needle);

// Funciones de memoria básicas
void *memset(void *ptr, int value, size_t num);
void *memcpy(void *dest, const void *src, size_t num);
void *memmove(void *dest, const void *src, size_t num);
int memcmp(const void *ptr1, const void *ptr2, size_t num);

#endif // _STDIO_H