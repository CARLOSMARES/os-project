// include/io.h
#ifndef _IO_H
#define _IO_H

#include "stdint.h"

/*
 * Acceso a puertos I/O. Estas funciones usan instrucciones x86 in/out.
 * Si tu SO corre en otra arquitectura o en un entorno sin I/O ports,
 * reemplázalas por las llamadas apropiadas.
 */

static inline uint8_t inb(uint16_t port) {
    uint8_t val;
    __asm__ volatile ("inb %w1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %w1" :: "a"(val), "Nd"(port));
}

static inline uint16_t inw(uint16_t port) {
    uint16_t val;
    __asm__ volatile ("inw %w1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

static inline void outw(uint16_t port, uint16_t val) {
    __asm__ volatile ("outw %0, %w1" :: "a"(val), "Nd"(port));
}

static inline uint32_t inl(uint16_t port) {
    uint32_t val;
    __asm__ volatile ("inl %w1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

static inline void outl(uint16_t port, uint32_t val) {
    __asm__ volatile ("outl %0, %w1" :: "a"(val), "Nd"(port));
}

#endif // _IO_H
