disk_error_loader_chs:
disk_error_kernel:
; boot.asm - Boot sector 16 bits: debug y salto a 32 bits (carga loader)
[BITS 16]
[ORG 0x7C00]

%include "kernel_sectors.inc"

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; Cargar el kernel desde el disco a la dirección 0x8000
    mov ax, 0x0800
    mov es, ax
    xor bx, bx
    mov ah, 0x02
    mov al, KERNEL_SECTORS
    mov ch, 0x00
    mov dh, 0x00
    mov cl, 0x02
    int 0x13
    jc disk_error

    ; Habilitar la línea A20
    in al, 0x92
    or al, 00000010b
    out 0x92, al

    ; Saltar al kernel en modo protegido
    jmp 0x0000:kernel_entry

disk_error:
    hlt
    jmp disk_error

[BITS 32]
kernel_entry:
    mov esp, 0x9FC00
    jmp 0x8000  ; Saltar directamente a la dirección física del kernel

    hlt
    jmp $

; Firma del sector de arranque
TIMES 510-($-$$) db 0
DW 0xAA55