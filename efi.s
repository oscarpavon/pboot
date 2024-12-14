;we use fasm to generate the EFI header. None assembly here
format pe64 efi
section '.text' code executable readable

entry $
  FILE "pboot.bin" ;this is the binary compiled with gcc
  jmp $ ;we never got here



