#ifndef ISR_IRQ_H
#define ISR_IRQ_H

#include <stdint.h>

// Inicialización de ISRs y IRQs
void isr_init(void);
void irq_init(void);

// Handlers comunes llamados desde los stubs ASM (32-bit)
// void isr_common_handler(uint32_t vec, uint32_t err);
// void irq_common_handler(uint32_t vec);

#ifdef __cplusplus
extern "C"
{
#endif

    void isr_common_handler(uint32_t vec, uint32_t err);
    void irq_common_handler(uint32_t vec);

#ifdef __cplusplus
}
#endif

// Tabla de stubs definida en ASM
extern void *isr_stub_table[];

#endif
