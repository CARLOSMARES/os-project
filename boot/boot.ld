ENTRY(start)

SECTIONS
{
  . = 0x8000;
  .boot :
  {
    *(.text*)
    *(.boot*)
  }
}
