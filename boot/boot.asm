[BITS 16]
[ORG 0x7C00]

_start:
    cli
    cld

    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti
    
    mov si, boot_msg
    call print_string
    
    mov si, kernel_msg
    call print_string
    ; Habilitar A20
    call enable_a20
    
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp CODE_SEG:init_pm

enable_a20:
    in al, 0x92
    or al, 2
    out 0x92, al
    ret

print_string:
    mov ah, 0x0e
    mov bh, 0
.loop:
    lodsb
    test al, al
    jz .done
    int 0x10
    jmp .loop
.done:
    ret

[BITS 32]
init_pm:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x9000

    call check_64bit
    test eax, eax
    jz no_64bit

    call setup_paging
    
    mov eax, cr4
    or eax, 1 << 5      ; PAE
    mov cr4, eax
    
    mov ecx, 0xC0000080 ; EFER MSR
    rdmsr
    or eax, 1 << 8      ; LME
    wrmsr
    
    mov eax, cr0
    or eax, 1 << 31     ; PG
    mov cr0, eax
    
    ; Cargar GDT de 64 bits y saltar
    lgdt [gdt64_descriptor]
    jmp CODE64_SEG:long_mode_start

no_64bit:
    ; Mensaje de error y halt
    cli
    hlt

check_64bit:
    ; Verificar CPUID
    pushfd
    pop eax
    mov ecx, eax
    xor eax, 1 << 21
    push eax
    popfd
    pushfd
    pop eax
    xor eax, ecx
    jz .no_cpuid
    
    ; Verificar long mode
    mov eax, 0x80000000
    cpuid
    cmp eax, 0x80000001
    jb .no_64bit
    
    mov eax, 0x80000001
    cpuid
    test edx, 1 << 29
    jz .no_64bit
    
    mov eax, 1
    ret

.no_cpuid:
.no_64bit:
    xor eax, eax
    ret

setup_paging:
    ; Limpiar tablas de página
    mov edi, 0x1000
    mov cr3, edi
    xor eax, eax
    mov ecx, 4096
    rep stosd
    mov edi, cr3

    ; PML4[0] -> PDPT
    mov dword [edi], 0x2003
    add edi, 0x1000
    
    ; PDPT[0] -> PD
    mov dword [edi], 0x3003
    add edi, 0x1000
    
    ; PD entries (2MB pages)
    mov dword [edi], 0x00000083
    mov dword [edi + 8], 0x00200083
    mov dword [edi + 16], 0x00400083
    mov dword [edi + 24], 0x00600083
    ret

[BITS 64]
long_mode_start:
    ; Configurar segmentos en long mode
    mov ax, DATA64_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov rsp, 0x9000
    
    ; Limpiar pantalla
    mov rdi, 0xB8000
    mov rcx, 2000
    mov ax, 0x0720
.clear_screen:
    stosw
    loop .clear_screen
    
    ; Saltar al kernel en 0x10000
    mov rax, 0x10000
    jmp rax

; GDT para modo protegido de 32 bits
gdt_start:
gdt_null:
    dd 0x0
    dd 0x0

gdt_code:
    dw 0xffff       ; Límite 0-15
    dw 0x0          ; Base 0-15
    db 0x0          ; Base 16-23
    db 10011010b    ; Flags de acceso
    db 11001111b    ; Flags y límite 16-19
    db 0x0          ; Base 24-31

gdt_data:
    dw 0xffff       ; Límite 0-15
    dw 0x0          ; Base 0-15
    db 0x0          ; Base 16-23
    db 10010010b    ; Flags de acceso
    db 11001111b    ; Flags y límite 16-19
    db 0x0          ; Base 24-31
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

; GDT para long mode (64 bits)
gdt64_start:
gdt64_null:
    dq 0x0

gdt64_code:
    dq 0x00af9a000000ffff  ; Segmento de código 64-bit

gdt64_data:
    dq 0x00af92000000ffff  ; Segmento de datos 64-bit
gdt64_end:

gdt64_descriptor:
    dw gdt64_end - gdt64_start - 1
    dd gdt64_start

CODE64_SEG equ gdt64_code - gdt64_start
DATA64_SEG equ gdt64_data - gdt64_start

; Mensajes
boot_msg db 'MicroCIOMOS Starting from ISO...', 13, 10, 0
kernel_msg db 'Transitioning to protected mode...', 13, 10, 0

; Boot signature
times 510-($-$$) db 0
dw 0xAA55
