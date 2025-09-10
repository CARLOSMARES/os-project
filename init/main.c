#include "fs.h"
#include <stdio.h>
#include "log.h"
#include "idt.h"
#include "timer.h"
#include "keyboard.h"
#include "vga_color.h"
#include "port.h"

#ifdef __cplusplus
extern "C"
{
#endif

    // Declaración externa de kernel_main
    extern void kernel_main(void);
    // Inicializar serial temprano para poder capturar mensajes de arranque
    extern void serial_init(void);
    extern void idt_init(void);
    extern void isr_init(void);
    extern void irq_init(void);
    extern void pic_remap(void);
    extern void vga_clear_screen(void);

    // Punto de entrada del kernel
    __attribute__((section(".entry"), used)) void _start(void)
    {

        vga_clear_screen();

        printf("_start init");

        serial_init();
        printf("_start: after serial_init\n");

        outb(0xE9, 'P'); // Indicar inicio de pic_remap
        pic_remap();
        outb(0xE9, 'p'); // Indicar fin de pic_remap

        outb(0xE9, 'I'); // Indicar inicio de isr_init
        isr_init();
        outb(0xE9, 'i'); // Indicar fin de isr_init

        outb(0xE9, 'Q'); // Indicar inicio de irq_init
        irq_init();
        outb(0xE9, 'q'); // Indicar fin de irq_init

        outb(0xE9, 'D'); // Indicar inicio de idt_init
        idt_init();
        outb(0xE9, 'd'); // Indicar fin de idt_init

        outb(0xE9, 'T'); // Indicar inicio de pit_init
        pit_init(100);   // 100Hz
        outb(0xE9, 't'); // Indicar fin de pit_init

        outb(0xE9, 'K'); // Indicar inicio de keyboard_init
        keyboard_init();
        outb(0xE9, 'k'); // Indicar fin de keyboard_init

        outb(0xE9, 'F'); // Indicar inicio de fs_init
        fs_init();
        outb(0xE9, 'f'); // Indicar fin de fs_init

        outb(0xE9, 'S'); // Indicar habilitación de interrupciones
        __asm__ volatile("sti");
        outb(0xE9, 's'); // Indicar interrupciones habilitadas

        printf("_start: interrupts enabled\n");

        kernel_main(); // Llama directamente al kernel principal
    }

#ifdef __cplusplus
}
#endif
