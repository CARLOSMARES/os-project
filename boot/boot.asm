
times 510-($-$$) db 0


; boot/boot.asm - Boot sector para ISO (El Torito, no emulación, INT 13h extensions)
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

    ; Mostrar 'I' (inicio)
    mov ax, 0xB800
    mov es, ax
    mov byte [es:0], 'I'
    mov byte [es:1], 0x0F

    ; Preparar buffer de DAP (Disk Address Packet) para INT 13h función 42h
    mov si, dap

    mov ah, 0x42        ; Extended Read
    mov dl, 0xE0        ; Unidad típica de CD-ROM en BIOS El Torito (puede ser 0x80-0xE0)
    int 0x13
    jc disk_error


    ; Mostrar 'L' si la lectura fue exitosa
    mov ax, 0xB800
    mov es, ax
    mov byte [es:2], 'L'
    mov byte [es:3], 0x0E

    ; Saltar a loader64 (modo real, 0x10000:0x0000)
    jmp 0x1000:0x0000

disk_error:
    mov ax, 0xB800
    mov es, ax
    mov byte [es:4], 'E'
    mov byte [es:5], 0x0C
    jmp $

; Disk Address Packet (DAP) para INT 13h función 42h
; Debe estar alineado a 16 bits y dentro de los primeros 64K
dap:
    db 0x10         ; Tamaño del DAP (16 bytes)
    db 0            ; Reservado
    dw 1            ; Número de sectores a leer
    dw 0x0000       ; Offset destino (0x0000)
    dw 0x1000       ; Segmento destino (0x10000)
    dq 17           ; LBA a leer (sector 17, justo después del boot sector)

; El sector de arranque para ISO debe ser de 2048 bytes
times 2048-($-$$) db 0