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

    ; Debug: 'L' (modo real)
    mov ax, 0xB800
    mov es, ax
    mov byte [es:4], 'L'
    mov byte [es:5], 0x0F

    ; GDT 32 bits
    lgdt [gdt_desc]

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

    ; Debug: 'P' (modo protegido)
    mov byte [0xB8000+8], 'P'
    mov byte [0xB8000+9], 0x0F

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

    ; Debug: '6' (modo largo)
    mov byte [0xB8000+16], '6'
    mov byte [0xB8000+17], 0x0F

    ; Saltar al kernel (main en C, 64 bits)
    jmp 0x10000

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
