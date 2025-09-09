#ifndef IDT_H
#define IDT_H
#include <stdint.h>

void idt_init(void);
void pic_remap(void);
void isr_init(void); // stubs de 0–31
void irq_init(void); // stubs de 32–47

#endif
