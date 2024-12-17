OUTPUT_FORMAT(binary)
SECTIONS {
  .data : {
    *(.data*)
    *(.bss*) 
    *(.rodata*) 
  }
}
