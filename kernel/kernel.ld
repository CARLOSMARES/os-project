/* kernel/kernel.ld - Linker script para kernel x86 32-bit */

ENTRY(_start)

SECTIONS
{
    /* Dirección de carga del kernel (coincide con tu bootloader: 0x8000) */
    . = 0x8000;

    /* Entrada principal del kernel */
    .entry :
    {
        KEEP(*(.entry))
    }

    /* Código ejecutable del kernel */
    .text ALIGN(4K) :
    {
        *(.text*)
    }

    /* Datos de solo lectura */
    .rodata ALIGN(4K) :
    {
        *(.rodata*)
    }

    /* Datos inicializados */
    .data ALIGN(4K) :
    {
        *(.data*)
    }

    /* BSS: datos no inicializados */
    .bss ALIGN(4K) :
    {
        *(COMMON)
        *(.bss*)
    }

    /* Secciones opcionales para compatibilidad con C++ */
    .eh_frame ALIGN(4K) :
    {
        *(.eh_frame*)
    }

    .gcc_except_table ALIGN(4K) :
    {
        *(.gcc_except_table*)
    }

    /* Agregar una sección de encabezado para compatibilidad con QEMU */
    .note ALIGN(4K) :
    {
        BYTE(0x00)
    }

    /* Nota PVH requerida por QEMU */
    .pvh_note ALIGN(4K) :
    {
        BYTE(0x00)
    }
}
