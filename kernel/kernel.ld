ENTRY(_start)

SECTIONS
{
  /* Kernel cargado en 0x8000 (coincide con el salto del bootloader) */
  . = 0x8000;

  /* Colocar el entry justo al inicio del binario */
  .entry :
  {
    KEEP(*(.entry))
  }

  .text ALIGN(4K) :
  {
    *(.text*)
  }

  .rodata ALIGN(4K) :
  {
    *(.rodata*)
  }

  .data ALIGN(4K) :
  {
    *(.data*)
  }

  .bss ALIGN(4K) :
  {
    *(COMMON)
    *(.bss*)
  }

  /* Secciones adicionales para 64 bits */
  .eh_frame ALIGN(4K) :
  {
    *(.eh_frame*)
  }

  .gcc_except_table ALIGN(4K) :
  {
    *(.gcc_except_table*)
  }
}
