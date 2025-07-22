BITS 32

section .multiboot_header
align 8

global multiboot_header_start
global _start

%define MAGIC 0xe85250d6
%define ARCH 0  ; i386 protected mode
%define HEADER_LEN (multiboot_header_end - multiboot_header_start)
%define CHECKSUM (-(MAGIC + ARCH + HEADER_LEN) & 0xFFFFFFFF)

multiboot_header_start:
    dd MAGIC
    dd ARCH
    dd HEADER_LEN
    dd CHECKSUM
    
    ; End tag
    align 8
    dw 0    ; type
    dw 0    ; flags
    dd 8    ; size
multiboot_header_end:

section .bss
align 16
stack_space: resb 16384       ; 16 KB stack
stack_top:

; Tablas de páginas para modo largo
align 4096
p4_table:
    resb 4096
p3_table:
    resb 4096
p2_table:
    resb 4096

section .text
extern kernel_main

_start:
    ; Limpiar registros de segmento y configurar el stack
    cli
    cld
    
    ; Configurar stack pointer
    mov esp, stack_space + 16384
    
    ; Verificar soporte para modo largo
    call check_multiboot
    call check_cpuid
    call check_long_mode
    
    ; Configurar paginación
    call setup_page_tables
    call enable_paging
    
    ; Cargar GDT de 64 bits
    lgdt [gdt64.pointer]
    
    ; Salto far para cambiar a modo largo
    jmp gdt64.code_segment:long_mode_start

; Verificar que estamos en multiboot
check_multiboot:
    cmp eax, 0x36d76289
    jne .no_multiboot
    ret
.no_multiboot:
    mov al, "M"
    jmp error

; Verificar soporte CPUID
check_cpuid:
    pushfd
    pop eax
    mov ecx, eax
    xor eax, 1 << 21
    push eax
    popfd
    pushfd
    pop eax
    push ecx
    popfd
    cmp eax, ecx
    je .no_cpuid
    ret
.no_cpuid:
    mov al, "C"
    jmp error

; Verificar soporte para modo largo
check_long_mode:
    mov eax, 0x80000000
    cpuid
    cmp eax, 0x80000001
    jb .no_long_mode
    
    mov eax, 0x80000001
    cpuid
    test edx, 1 << 29
    jz .no_long_mode
    ret
.no_long_mode:
    mov al, "L"
    jmp error

; Configurar tablas de páginas
setup_page_tables:
    ; Mapear primera entrada P4 a P3
    mov eax, p3_table
    or eax, 0b11 ; presente + escribible
    mov [p4_table], eax
    
    ; Mapear primera entrada P3 a P2
    mov eax, p2_table
    or eax, 0b11 ; presente + escribible
    mov [p3_table], eax
    
    ; Mapear cada entrada P2 a una página de 2MiB
    mov ecx, 0
.map_p2_table:
    mov eax, 0x200000  ; 2MiB
    mul ecx
    or eax, 0b10000011 ; presente + escribible + huge page
    mov [p2_table + ecx * 8], eax
    inc ecx
    cmp ecx, 512
    jne .map_p2_table
    
    ret

; Habilitar paginación y modo largo
enable_paging:
    ; Cargar P4 en cr3
    mov eax, p4_table
    mov cr3, eax
    
    ; Habilitar PAE en cr4
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax
    
    ; Habilitar modo largo en EFER MSR
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr
    
    ; Habilitar paginación en cr0
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax
    
    ret

error:
    ; Mostrar código de error en VGA
    mov dword [0xb8000], 0x4f524f45
    mov dword [0xb8004], 0x4f3a4f52
    mov dword [0xb8008], 0x4f204f20
    mov byte  [0xb800a], al
    hlt

; GDT de 64 bits
section .rodata
gdt64:
    dq 0 ; entrada cero
.code_segment: equ $ - gdt64
    dq (1 << 43) | (1 << 44) | (1 << 47) | (1 << 53) ; segmento de código
.pointer:
    dw $ - gdt64 - 1
    dq gdt64

; Código de modo largo
section .text
BITS 64
long_mode_start:
    ; Limpiar registros de segmento
    mov ax, 0
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Limpiar pantalla
    call clear_screen_64
    
    ; Llamar al kernel principal
    call kernel_main

.hang:
    hlt
    jmp .hang

clear_screen_64:
    mov rdi, 0xB8000        ; Dirección de video VGA
    mov rcx, 80 * 25        ; 80 columnas x 25 filas
    mov ax, 0x0720          ; Espacio (0x20) + atributo (0x07 = blanco sobre negro)

.clear_loop:
    mov [rdi], ax
    add rdi, 2
    loop .clear_loop
    ret
