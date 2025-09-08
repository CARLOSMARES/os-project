disk_error_loader_chs:
disk_error_kernel:
; boot.asm - Boot sector 16 bits: debug y salto a 32 bits (carga loader)
[BITS 16]
[ORG 0x7C00]

%include "kernel_sectors.inc"

start:
    cli
    cld
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; Si DL < 0x80 (floppy), usar CHS con AH=02h
    cmp dl, 0x80
    jb .read_floppy

    ; --- HDD: usar DAP con AH=42h ---
    mov si, dap
    mov byte [si+0], 0x10               ; tamaño = 16 bytes
    mov byte [si+1], 0x00               ; reservado
    mov word [si+2], KERNEL_SECTORS     ; número de sectores (512B)
    mov word [si+4], 0x0000             ; offset destino
    mov word [si+6], 0x0800             ; segmento destino (0x0800:0000 => 0x8000)
    mov word [si+8], 0x0001             ; LBA bajo (lo)
    mov word [si+10], 0x0000            ; LBA bajo (hi)
    mov word [si+12], 0x0000            ; LBA alto (lo)
    mov word [si+14], 0x0000            ; LBA alto (hi)
    mov ah, 0x42
    int 0x13
    jc hang
    jmp .loaded

.read_floppy:
    ; Floppy 1.44MB: C/H/S = 80/2/18. Leer desde sector 2, cabeza 0, cilindro 0
    mov ax, 0x0800
    mov es, ax
    xor bx, bx                          ; ES:BX destino
    mov ah, 0x02                        ; leer sectores
    mov al, KERNEL_SECTORS              ; cantidad (debe ser <= 17)
    mov ch, 0x00                        ; cilindro 0
    mov dh, 0x00                        ; cabeza 0
    mov cl, 0x02                        ; sector inicial = 2
    int 0x13
    jc hang

.loaded:

    ; Habilitar A20 (puerto 0x92)
    in al, 0x92
    or al, 00000010b
    out 0x92, al

    ; Entrar a modo protegido
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
    mov fs, ax
    mov gs, ax
    mov esp, 0x0090000
    jmp dword 0x08:0x00008000

times 510-($-$$) db 0
dw 0xAA55

; ----- Datos -----
dap:
    times 16 db 0

hang:
    hlt
    jmp hang