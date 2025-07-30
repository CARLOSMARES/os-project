disk_error_loader_chs:
disk_error_kernel:
; boot.asm - Boot sector 16 bits: debug y salto a 32 bits (carga loader)
[BITS 16]
[ORG 0x7C00]

start:
    mov ax, 0xB800
    mov es, ax
    mov byte [es:0], 'B'      ; Debug: 'B' de Boot
    mov byte [es:1], 0x1E

    cli
    lgdt [gdt_desc]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp 0x08:protected_mode

gdt_start:
    dq 0x0000000000000000
    dq 0x00cf9a000000ffff ; code 32 bits
    dq 0x00cf92000000ffff ; data 32 bits
gdt_end:

gdt_desc:
    dw gdt_end - gdt_start - 1
    dd gdt_start

[BITS 32]
protected_mode:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax

    mov eax, 0xB8000
    mov byte [eax+2], 'L'     ; Debug: 'L' de Loader
    mov byte [eax+3], 0x2E

    ; Aquí deberías cargar el loader.bin desde disco a RAM (por INT 13h o similar)
    ; y saltar a su entrypoint.
    ; Por ahora, solo bucle infinito para debug visual.
.loop:
    hlt
    jmp .loop

times 510-($-$$) db 0
dw 0xAA55