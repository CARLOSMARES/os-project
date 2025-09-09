#include <stdint.h>
#include "idt.h"
#include "log.h"

static volatile uint64_t ticks = 0;
static inline void outb(uint16_t p, uint8_t v){ __asm__ volatile("outb %0,%1"::"a"(v),"Nd"(p)); }

void pit_irq_handler(void) { ticks++; }

void pit_init(uint32_t hz){
    uint32_t divisor = 1193180 / hz;
    outb(0x43, 0x36);
    outb(0x40, (uint8_t)(divisor & 0xFF));
    outb(0x40, (uint8_t)((divisor>>8)&0xFF));
    KLOG_INFO("PIT %u Hz", hz);
}

uint64_t timer_ticks(void){ return ticks; }
