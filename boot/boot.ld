ENTRY(_start)

SECTIONS
{
  /* Boot sector cargado en 0x7c00 */
  . = 0x7c00;
  
  .boot :
  {
    *(.boot*)
  }
}
