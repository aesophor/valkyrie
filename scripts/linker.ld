ENTRY(__kernel_loader)

SECTIONS
{
  . = 0x80000;

  .text :
  {
    *(.text)
  }

  .data :
  {
    *(.data)
  }

  .bss  :
  {
    _bss_start = .;
    *(.bss)
    _bss_end = .;
  }
}
