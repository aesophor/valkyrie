SECTIONS {
  . = 0x60000;

  .text : { *(.text) }
  .rodata ALIGN(8) : { *(.rodata) }

  .data ALIGN(8) : {
    start_ctors = .;
    KEEP(*( .init_array ));
    KEEP(*(SORT_BY_INIT_PRIORITY( .init_array.* )));
    end_ctors = .;
    *(.data)
  }

  .bss : {
    _bss_start = .;
    *(.bss)
    _bss_end = .;
  }
}
