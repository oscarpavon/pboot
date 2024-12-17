CFLAGS := -ffreestanding -fno-stack-check -fno-stack-protector -fPIC -fshort-wchar -mno-red-zone -maccumulate-outgoing-args -mabi=ms


all: pboot

start.o: start.c
	cc $(CFLAGS) -c start.c

pboot.bin: start.o main.o
	ld start.o main.o -nostdlib -znocombreloc -shared -Bsymbolic -o pboot.bin -T binary.ld 

pboot: efi.s pboot.bin
	fasm efi.s pboot


main.o: main.c config.h types.h efi.h
	cc $(CFLAGS) -c main.c

clean:
	rm -f *.o
	rm -f *.d
	rm -f pboot
	rm -f pboot.bin

install:
	cp pboot /boot/EFI/pboot/pboot.efi

