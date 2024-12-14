CC := clang
LD := ld.lld

CFLAGS := -ffreestanding -MMD -mno-red-zone -std=c11 \
	-target x86_64-unknown-windows
LDFLAGS := -flavor link -subsystem:efi_application -entry:efi_main

GCCFLAGS := -ffreestanding -fno-stack-check -fno-stack-protector -fPIC -fshort-wchar -mno-red-zone -maccumulate-outgoing-args

all: pboot

pboot.o: pboot.c
	cc $(GCCFLAGS) -c pboot.c -mabi=ms

pboot.bin: pboot.o
	ld pboot.o -o pboot.bin -T binary.ld

pboot: efi.s pboot.bin
	fasm efi.s pboot


clang: main.o
	$(LD) $(LDFLAGS) main.o -out:pboot

main.o: main.c config.h types.h efi.h
	$(CC) $(CFLAGS) -c main.c -o main.o

clean:
	rm -f *.o
	rm -f *.d
	rm -f pboot

install:
	cp pboot /boot/EFI/pboot/pboot.efi

