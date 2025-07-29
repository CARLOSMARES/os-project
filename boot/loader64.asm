; boot/loader64.asm - Segundo stage robusto, debug en cada modo, salto seguro al kernel
[BITS 16]
[ORG 0x10000]

start_loader:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x9000
    sti

    ; Debug: 'L' (modo real, columna 6)
    mov ax, 0xB800
    mov es, ax
    mov byte [es:6], 'L'
    mov byte [es:7], 0x0F

    ; --- Cargar kernel desde ISO (sector 18+, LBA) a 0x11000 usando INT 13h extensions ---
    ; Asume: loader64 está en sector 17 (LBA 17), kernel inicia en sector 18 (LBA 18)
    ; Tamaño típico kernel: 64 sectores (ajusta según tu kernel real)

    mov si, kernel_lba_packet
    mov ah, 0x42            ; INT 13h extensión: leer sectores LBA
    mov dl, 0x00            ; 0x00 = CD/ISO (prueba alternativa)
    int 0x13
    jc disk_error

    ; Debug: 'K' (cargar kernel OK, columna 8)
    mov ax, 0xB800
    mov es, ax
    mov byte [es:8], 'K'
    mov byte [es:9], 0x0A

    ; Bucle infinito para depuración
    mov ax, 0xB800
    mov es, ax
    mov byte [es:12], 'X'   ; Marca que llegó aquí
    mov byte [es:13], 0x0C
dep_loop:
    jmp dep_loop

disk_error:
    mov ax, 0xB800
    mov es, ax
    mov byte [es:8], 'E'    ; Error de disco
    mov byte [es:9], 0x4C
    hlt

cargar_gdt:

    ; GDT 32 bits
    lgdt [gdt_desc]
; --- Paquete LBA para INT 13h extensions ---
kernel_lba_packet:
    db 0x10                 ; Tamaño del paquete (16 bytes)
    db 0                    ; Reservado
    dw 8                    ; Número de sectores a leer (prueba con 8 sectores)
    dq 0x11000              ; Dirección destino (RAM física)
    dq 18                   ; LBA de inicio (sector 18 en ISO)

    ; Habilitar A20
    in al, 0x92
    or al, 2
    out 0x92, al

    ; Entrar a modo protegido
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp 0x08:pmode

[BITS 32]
pmode:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp, 0x9000

    ; Debug: 'P' (modo protegido, columna 10)
    mov byte [0xB8000+10], 'P'
    mov byte [0xB8000+11], 0x0F

    ; Habilitar PAE
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    ; Tablas de paginación 1:1 (simplificado)
    mov edi, page_tables
    mov ecx, 4096*3/4
    xor eax, eax
    rep stosd

    mov eax, page_tables + 0x1000
    or eax, 3
    mov dword [page_tables], eax

    mov eax, page_tables + 0x2000
    or eax, 3
    mov dword [page_tables+0x1000], eax

    mov eax, 0
    or eax, 3
    mov dword [page_tables+0x2000], eax

    mov eax, page_tables
    mov cr3, eax

    ; Habilitar modo largo
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    mov eax, cr0
    or eax, 0x80000001
    mov cr0, eax

    lgdt [gdt64_desc]
    jmp 0x18:lmode

[BITS 64]
lmode:
    mov ax, 0x20
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov rsp, 0x9000



    ; Debug: '6' (modo largo, columna 18)
    mov byte [0xB8000+18], '6'
    mov byte [0xB8000+19], 0x0F

    ; ; Escribir patrón visible en la primera línea de la pantalla (A, B, C...)
    ; mov rcx, 80
    ; mov rdi, 0xB8000
    ; mov al, 'A'
    ; mov ah, 0x1E
;write_pattern:
    ; mov [rdi], al
    ; mov [rdi+1], ah
    ; add rdi, 2
    ; inc al
    ; loop write_pattern

    ; DEBUG: Escribir 'L' en la esquina superior izquierda (VGA modo texto)
    mov rax, 0xB8000
    mov word [rax], 0x0F4C   ; 0x0F = blanco, 'L' = 0x4C

    ; Limpieza de registros antes de saltar al kernel
    xor rax, rax
    xor rbx, rbx
    xor rcx, rcx
    xor rdx, rdx
    xor rsi, rsi
    xor rdi, rdi
    xor rbp, rbp

    ; Saltar al kernel (0x11000, debe coincidir con kernel.ld y _start)
    jmp 0x11000
    ; Limpieza de registros antes de saltar al kernel
    ;xor rax, rax
    ;xor rbx, rbx
    ;xor rcx, rcx
    ;xor rdx, rdx
    ;xor rsi, rsi
    ;xor rdi, rdi
    ;xor rbp, rbp
    ;jmp 0x11000

align 16
GDT:
    dq 0
    dq 0x00CF9A000000FFFF
    dq 0x00CF92000000FFFF
gdt_desc:
    dw GDT_end - GDT - 1
    dd GDT
GDT_end:

GDT64:
    dq 0
    dq 0x00AF9A000000FFFF
    dq 0x00AF92000000FFFF
gdt64_desc:
    dw GDT64_end - GDT64 - 1
    dq GDT64
GDT64_end:

align 4096
page_tables:
    times 4096*3 db 0
