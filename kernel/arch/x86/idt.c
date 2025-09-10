#include <stdint.h>
#include "idt.h"
#include "log.h"
#include "isr_irq.h"
#include "port.h"

#define IDT_ENTRIES 256
#define IRQ_BASE 32
#define IRQ_COUNT 16

struct idt_entry
{
    uint16_t offset_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t type_attr;
    uint16_t offset_high;
} __attribute__((packed));

struct idt_ptr
{
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

static struct idt_entry idt[IDT_ENTRIES];

extern void *isr_stub_table[];

static inline void lidt(void *base, uint16_t size)
{
    struct idt_ptr idtp = {.limit = size - 1, .base = (uint32_t)base};
    __asm__ volatile("lidt %0" : : "m"(idtp));
}

static void set_gate(int n, void *handler, uint8_t flags)
{
    uint32_t addr = (uint32_t)handler;
    idt[n].offset_low = addr & 0xFFFF;
    idt[n].selector = 0x08; // code segment
    idt[n].zero = 0;
    idt[n].type_attr = flags;
    idt[n].offset_high = (addr >> 16) & 0xFFFF;
}

void pic_remap(void)
{
    uint8_t a1 = inb(0x21);
    uint8_t a2 = inb(0xA1);

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

void isr_init(void)
{
    for (int i = 0; i < 32; i++)
        set_gate(i, isr_stub_table[i], 0x8E);

    KLOG_INFO("ISRs initialized");
}

void irq_init(void)
{
    pic_remap();
    for (int i = 0; i < IRQ_COUNT; i++)
        set_gate(IRQ_BASE + i, isr_stub_table[32 + i], 0x8E);

    KLOG_INFO("IRQs initialized");
}

void idt_init(void)
{
    lidt(idt, sizeof(idt));
    KLOG_INFO("IDT loaded");
}

void isr_common_handler(uint32_t vec, uint32_t err)
{
    KLOG_ERR("CPU EXCEPTION vec=%d err=0x%x", vec, err);
    outb(0xE9, 'E');               // Indicar excepción
    outb(0xE9, vec & 0xFF);        // Enviar vector bajo
    outb(0xE9, (vec >> 8) & 0xFF); // Enviar vector alto
    for (;;)
        __asm__ volatile("hlt");
}

void irq_common_handler(uint32_t vec)
{
    KLOG_INFO("IRQ HANDLER vec=%d", vec);
    outb(0xE9, 'I');               // Indicar IRQ
    outb(0xE9, vec & 0xFF);        // Enviar vector bajo
    outb(0xE9, (vec >> 8) & 0xFF); // Enviar vector alto

    if (vec >= 40)
        outb(0xA0, 0x20);
    outb(0x20, 0x20);
}
