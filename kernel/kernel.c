#include "log.h"

void kernel_main(void)
{

    KLOG_INFO("MicroCIOMOS booting...");

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
