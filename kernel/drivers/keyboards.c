#include <stdint.h>
#include "log.h"

static inline uint8_t inb(uint16_t p){ uint8_t r; __asm__ volatile("inb %1,%0":"=a"(r):"Nd"(p)); return r; }
static inline int kbd_has_data(){ return inb(0x64) & 1; }

void keyboard_init(void){ /* nada especial por ahora */ }

int keyboard_read_scancode(void){
    if (!kbd_has_data()) return -1;
    return (int)inb(0x60);
}
