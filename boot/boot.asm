; boot/boot.asm - MBR mínimo (512 bytes)
[BITS 16]
[ORG 0x7C00]

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    ; Mostrar '1' en pantalla (debug)
    mov ax, 0xB800
    mov es, ax
    mov byte [es:0], '1'
    mov byte [es:1], 0x0F

    ; Cargar loader64 (segundo stage) a 0x10000 (lineal, sin offset)
    mov ax, 0x1000
    mov es, ax
    xor bx, bx
    mov ah, 0x02        ; función leer sectores
    mov al, 8           ; asume loader64.bin ocupa 8 sectores (4 KB) (ajusta si es necesario)
    mov ch, 0           ; cilindro 0
    mov cl, 2           ; sector inicial 2
    mov dh, 0           ; cabeza 0
    mov dl, 0x00        ; drive 0
    int 0x13
    jc disk_error

    ; Mostrar '2' en pantalla (debug)
    mov ax, 0xB800
    mov es, ax
    mov byte [es:2], '2'
    mov byte [es:3], 0x0F

    ; Saltar al loader (modo real, 0x10000)
    jmp 0x1000:0x0000

disk_error:
    mov ax, 0xB800
    mov es, ax
    mov byte [es:0], 'E'
    mov byte [es:1], 0x0C
    cli
    hlt

times 510-($-$$) db 0
dw 0xAA55