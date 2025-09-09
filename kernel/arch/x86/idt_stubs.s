.intel_syntax noprefix
.global isr_stub_table
.extern isr_common_handler
.extern irq_common_handler

.macro ISR_NOERR n
.global isr\n
isr\n:
    push 0               # fake err code
    push \n              # vector
    jmp isr_common
.endm

.macro ISR_ERR n
.global isr\n
isr\n:
    push \n
    jmp isr_common
.endm

.macro IRQ n
.global irq\n
irq\n:
    push 0
    push (32+\n)
    jmp irq_common
.endm

isr_common:
    push rax; push rcx; push rdx; push rbx; push rsp; push rbp; push rsi; push rdi
    call isr_common_handler
    pop rdi; pop rsi; pop rbp; pop rsp; pop rbx; pop rdx; pop rcx; pop rax
    add rsp, 16
    iretq

irq_common:
    push rax; push rcx; push rdx; push rbx; push rsp; push rbp; push rsi; push rdi
    call irq_common_handler
    pop rdi; pop rsi; pop rbp; pop rsp; pop rbx; pop rdx; pop rcx; pop rax
    add rsp, 16
    iretq

.section .rodata
.align 8
isr_stub_table:
    // 0–31: CPU exceptions
    .quad isr0, isr1, isr2, isr3, isr4, isr5, isr6, isr7
    .quad isr8, isr9, isr10, isr11, isr12, isr13, isr14, isr15
    .quad isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23
    .quad isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31
    // 32–47: IRQs
    .quad irq0, irq1, irq2, irq3, irq4, irq5, irq6, irq7
    .quad irq8, irq9, irq10, irq11, irq12, irq13, irq14, irq15

.text
// exceptions (some con error code)
ISR_NOERR 0;  ISR_NOERR 1;  ISR_NOERR 2;  ISR_NOERR 3
ISR_NOERR 4;  ISR_NOERR 5;  ISR_NOERR 6;  ISR_NOERR 7
ISR_ERR   8;  ISR_NOERR 9;  ISR_ERR  10; ISR_ERR  11
ISR_ERR  12;  ISR_ERR  13;  ISR_ERR  14; ISR_NOERR 15
ISR_NOERR 16; ISR_ERR  17; ISR_NOERR 18; ISR_NOERR 19
ISR_NOERR 20; ISR_NOERR 21; ISR_NOERR 22; ISR_NOERR 23
ISR_NOERR 24; ISR_NOERR 25; ISR_NOERR 26; ISR_NOERR 27
ISR_NOERR 28; ISR_NOERR 29; ISR_ERR  30; ISR_NOERR 31

// IRQs
IRQ 0; IRQ 1; IRQ 2; IRQ 3; IRQ 4; IRQ 5; IRQ 6; IRQ 7
IRQ 8; IRQ 9; IRQ 10; IRQ 11; IRQ 12; IRQ 13; IRQ 14; IRQ 15
