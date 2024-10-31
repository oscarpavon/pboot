virtual_machine_path := /root/virtual_machine/disk/EFI/BOOT

CC := clang
LD := ld.lld

GCCFLAGS := -ffreestanding -fno-stack-check -fno-stack-protector -fPIC -fshort-wchar \
						-mno-red-zone -maccumulate-outgoing-args

CFLAGS := -ffreestanding -MMD -mno-red-zone -std=c11 \
	-target x86_64-unknown-windows
LDFLAGS := -flavor link -subsystem:efi_application -entry:efi_main

BOOTX64.EFI: main.o
	$(LD) $(LDFLAGS) main.o -out:BOOTX64.EFI

ldd:
	$(LD) $(LDFLAGS) main.o -out:BOOTX64.EFI

main.o: main.c config.h types.h efi.h
	$(CC) $(CFLAGS) -c main.c -o main.o

clean:
	rm *.o
	rm *.d
	rm BOOTX64.EFI

install:
	cp BOOTX64.EFI $(virtual_machine_path)/BOOTX64.EFI

release:
	cp BOOTX64.EFI /boot/EFI/PAVON/pavonx64.efi

gcc:
	gcc $(GCCFLAGS) -DEFI_FUNCTION_WRAPPER -c main.c -o main.o

ld:
	ld main.o /usr/local/lib/crt0-efi-x86_64.o -nostdlib -znocombreloc \
		-T /usr/local/lib/elf_x86_64_efi.lds \
		-shared -Bsymbolic -L/usr/local/lib -lgnuefi -lefi \
		-o main.so 	
	objcopy -j .text -j .sdata -j .data -j .rodata \
		-j .dynamic -j .dynsym  -j .rel -j .rela -j .rel.* \
		-j .rela.* -j .reloc \
		--target efi-app-x86_64 \
		--subsystem=10 main.so BOOTX64.EFI

