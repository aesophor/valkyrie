SECTIONS {
  . = 0x0;

  .text ALIGN(0x10) : {
    *(.text)
  }

  .data ALIGN(0x10) : {
    *(.data)
  }

  .bss ALIGN(0x10) : {
    *(.bss)
  }
}
