CC := clang
LD := ld.lld

CFLAGS := -ffreestanding -MMD -mno-red-zone -std=c11 \
	-target x86_64-unknown-windows
LDFLAGS := -flavor link -subsystem:efi_application -entry:efi_main

SRCS := main.c

default: all

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

BOOTX64.EFI: main.o
	$(LD) $(LDFLAGS) $< -out:/root/virtual_machine/disk/EFI/BOOT/$@ -verbose

-include $(SRCS:.c=.d)

.PHONY: clean all default install

install:
	cp /root/virtual_machine/disk/EFI/BOOT/BOOTX64.EFI /boot/EFI/PAVON/pavonx64.efi

all: BOOTX64.EFI
