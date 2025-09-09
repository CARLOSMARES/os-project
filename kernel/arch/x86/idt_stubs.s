; idt_stubs.s - NASM 32-bit
[bits 32]
[global isr_stub_table]
[extern isr_common_handler]
[extern irq_common_handler]

; -----------------------------------------
; Funciones dummy para evitar linker errors
; -----------------------------------------
[global isr_init]
[global irq_init]
[global idt_init]

isr_init:
    ret
irq_init:
    ret
idt_init:
    ret

; -----------------------------------------
; Macros para ISR y IRQ
; -----------------------------------------
%macro ISR_NOERR 1
[global isr%1]
isr%1:
    push 0
    push %1
    jmp isr_common
%endmacro

%macro ISR_ERR 1
[global isr%1]
isr%1:
    push %1
    jmp isr_common
%endmacro

%macro IRQ 1
[global irq%1]
irq%1:
    push 0
    push 32 + %1
    jmp irq_common
%endmacro

; -----------------------------------------
; Handlers comunes
; -----------------------------------------
isr_common:
    pusha                     ; empuja eax, ecx, edx, ebx, esp, ebp, esi, edi
    mov eax, [esp + 32 + 4]   ; err code
    mov ebx, [esp + 32 + 8]   ; vector
    push ebx
    push eax
    call isr_common_handler
    add esp, 8
    popa
    iret

irq_common:
    pusha
    mov eax, [esp + 32 + 4]   ; vector
    push eax
    call irq_common_handler
    add esp, 4
    popa
    iret

; -----------------------------------------
; Stubs (ISR 0-31)
; -----------------------------------------
ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
ISR_ERR   8
ISR_NOERR 9
ISR_ERR  10
ISR_ERR  11
ISR_ERR  12
ISR_ERR  13
ISR_ERR  14
ISR_NOERR 15
ISR_NOERR 16
ISR_ERR  17
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_NOERR 21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_NOERR 29
ISR_ERR  30
ISR_NOERR 31

; IRQ 0-15
IRQ 0
IRQ 1
IRQ 2
IRQ 3
IRQ 4
IRQ 5
IRQ 6
IRQ 7
IRQ 8
IRQ 9
IRQ 10
IRQ 11
IRQ 12
IRQ 13
IRQ 14
IRQ 15

; -----------------------------------------
; Tabla de punteros
; -----------------------------------------
section .rodata
align 4
isr_stub_table:
    dd isr0, isr1, isr2, isr3, isr4, isr5, isr6, isr7
    dd isr8, isr9, isr10, isr11, isr12, isr13, isr14, isr15
    dd isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23
    dd isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31
    dd irq0, irq1, irq2, irq3, irq4, irq5, irq6, irq7
    dd irq8, irq9, irq10, irq11, irq12, irq13, irq14, irq15
