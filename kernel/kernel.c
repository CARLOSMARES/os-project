#include "log.h"
#include "idt.h"
#include "timer.h"
#include "keyboard.h"

extern void isr_init(void);
extern void irq_init(void);

void kernel_main(void)
{
    serial_init();
    KLOG_INFO("MicroCIOMOS booting...");

    pic_remap();
    isr_init(); // si hoy no tienes, basta con declarar/llamar y luego idt_init usa stubs
    irq_init();
    idt_init();

    pit_init(100); // 100Hz

    keyboard_init();

    // habilitar interrupciones
    __asm__ volatile("sti");

    KLOG_INFO("Kernel up. Waiting for IRQs...");

    for (;;)
    {
        int sc = keyboard_read_scancode();
        if (sc >= 0)
            KLOG_INFO("kbd scancode=0x%x", (unsigned)sc);
        // opcional: cada ~100ms loggear ticks
        static uint64_t last = 0;
        extern uint64_t timer_ticks(void);
        uint64_t t = timer_ticks();
        if (t - last >= 100)
        {
            last = t;
            KLOG_INFO("ticks=%llu", (unsigned long long)t);
        }
        __asm__ volatile("hlt");
    }
}
