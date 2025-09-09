#include <stdint.h>
#include "idt.h"
#include "log.h"

#define IDT_ENTRIES 256

struct idt_entry
{
    uint16_t offset_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t type_attr;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t zero;
} __attribute__((packed));

struct idt_ptr
{
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

extern void *isr_stub_table[]; // desde ASM
static struct idt_entry idt[IDT_ENTRIES];

static inline void lidt(void *base, uint16_t size)
{
    struct idt_ptr idtp = {.limit = size - 1, .base = (uint64_t)base};
    __asm__ volatile("lidt %0" : : "m"(idtp));
}

static void set_gate(int n, void *handler, uint8_t flags)
{
    uint64_t addr = (uint64_t)handler;
    idt[n].offset_low = addr & 0xFFFF;
    idt[n].selector = 0x08; // kernel code
    idt[n].ist = 0;
    idt[n].type_attr = flags; // 0x8E interrupt gate
    idt[n].offset_mid = (addr >> 16) & 0xFFFF;
    idt[n].offset_high = (addr >> 32) & 0xFFFFFFFF;
    idt[n].zero = 0;
}

static inline void outb(uint16_t p, uint8_t v) { __asm__ volatile("outb %0,%1" ::"a"(v), "Nd"(p)); }
static inline uint8_t inb(uint16_t p)
{
    uint8_t r;
    __asm__ volatile("inb %1,%0" : "=a"(r) : "Nd"(p));
    return r;
}

void pic_remap(void)
{
    // remap PIC1->0x20, PIC2->0x28
    uint8_t a1 = inb(0x21), a2 = inb(0xA1);
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, a1);
    outb(0xA1, a2);
}

void idt_init(void)
{
    for (int i = 0; i < 48; i++)
        set_gate(i, isr_stub_table[i], 0x8E);
    lidt(idt, sizeof(idt));
    KLOG_INFO("IDT loaded");
}

void isr_common_handler(uint64_t vec, uint64_t err)
{
    KLOG_ERR("CPU EXCEPTION vec=%d err=0x%x", (int)vec, (unsigned)err);
    for (;;)
    {
        __asm__ volatile("hlt");
    }
}

void irq_common_handler(uint64_t vec)
{
    // End of interrupt
    if (vec >= 40)
        outb(0xA0, 0x20);
    outb(0x20, 0x20);
}
