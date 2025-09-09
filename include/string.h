#ifndef _STRING_H
#define _STRING_H

#include "stdint.h"
#include "stddef.h"

// --- Manipulación de cadenas ---

// Calcula la longitud de una cadena terminada en '\0'
size_t strlen(const char *s);

// Copia la cadena src en dest (incluyendo '\0')
char *strcpy(char *dest, const char *src);

// Copia hasta n caracteres de src en dest
char *strncpy(char *dest, const char *src, size_t n);

// Compara dos cadenas (similar a strcmp)
int strcmp(const char *s1, const char *s2);

// Compara hasta n caracteres (similar a strncmp)
int strncmp(const char *s1, const char *s2, size_t n);

// Concatenar src al final de dest
char *strcat(char *dest, const char *src);

// Concatenar hasta n caracteres
char *strncat(char *dest, const char *src, size_t n);

// --- Manipulación de memoria ---

// Copia n bytes de src a dest (sobrescribe memoria)
void *memcpy(void *dest, const void *src, size_t n);

// Rellena n bytes de dest con el valor c
void *memset(void *dest, int c, size_t n);

// Compara n bytes de memoria
int memcmp(const void *s1, const void *s2, size_t n);

// Mueve n bytes de src a dest (soporta solapamiento)
void *memmove(void *dest, const void *src, size_t n);

#endif
