CC := clang
LD := ld.lld

CFLAGS := -ffreestanding -MMD -mno-red-zone -std=c11 \
	-target x86_64-unknown-windows
LDFLAGS := -flavor link -subsystem:efi_application -entry:efi_main

pboot: main.o
	$(LD) $(LDFLAGS) main.o -out:pboot

main.o: main.c config.h types.h efi.h
	$(CC) $(CFLAGS) -c main.c -o main.o

clean:
	rm -f *.o
	rm -f *.d
	rm -f pboot

release:
	cp pboot /boot/EFI/PAVON/pavonx64.efi

