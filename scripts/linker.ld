ENTRY(_start);

SECTIONS
{
    . = 0x80000;

    .text :
    {
      _stack_top = .;
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
