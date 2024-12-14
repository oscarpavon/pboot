format pe64 efi
section '.text' code executable readable

entry $
  FILE "pboot.bin" 
  call print_hello
  jmp $



print_hello:
  sub rsp,32
  mov rcx,[rdx + 64]
  mov rax,[rcx + 8]
  mov rdx,string
  call rax
  add rsp,32
  ret

string du 'Hello, World! from fasm',13,10,0
