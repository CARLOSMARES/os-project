ENTRY(_start)

SECTIONS
{
  . = 1M;

  .multiboot_header :
  {
    *(.multiboot_header*)
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
