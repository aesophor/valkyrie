SECTIONS {
  .  = 0xffff000000000000;  /* kernel space begin */
  . += 0x80000;             /* kernel load address */

  .text ALIGN(0x10) : {
    *(.text)
  }

  .data ALIGN(0x10) : {
    start_ctors = .;
    KEEP(*( .init_array ));
    KEEP(*(SORT_BY_INIT_PRIORITY( .init_array.* )));
    end_ctors = .;
    *(.data)
  }

  .bss ALIGN(0x10) : {
    _bss_start = .;
    *(.bss)
    *(.bss*)
    *(COMMON)
    _bss_end = .;
  }
}
