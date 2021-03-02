SECTIONS
{
  . = 0x80000;

  .text :
  {
    *(.text)
  }

  .data :
  {
    start_ctors = .;
    KEEP(*( .init_array ));
    KEEP(*(SORT_BY_INIT_PRIORITY( .init_array.* )));
    end_ctors = .;
    *(.data)
  }

  .bss  :
  {
    _bss_start = .;
    *(.bss)
    _bss_end = .;
  }
}
