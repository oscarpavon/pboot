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
	$(LD) $(LDFLAGS) $< -out:../disk/EFI/BOOT/$@

-include $(SRCS:.c=.d)

.PHONY: clean all default

all: BOOTX64.EFI
