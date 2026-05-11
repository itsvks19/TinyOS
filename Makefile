ASM     = nasm
CC      = gcc
LD      = ld
CFLAGS  = -ffreestanding -O2 -Wall -Wextra -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -g
LDFLAGS = -m elf_x86_64 -T linker.ld -nostdlib
QEMU    = qemu-system-x86_64
BUILD   = build
SRC     = src

all: $(BUILD)/os.iso

$(BUILD)/boot.o: $(SRC)/boot.s
	mkdir -p $(BUILD)
	$(ASM) -f elf64 $(SRC)/boot.s -o $(BUILD)/boot.o

$(BUILD)/kernel.o: $(SRC)/kernel.c
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -c $(SRC)/kernel.c -o $(BUILD)/kernel.o

$(BUILD)/kernel.elf: $(BUILD)/boot.o $(BUILD)/kernel.o linker.ld
	$(LD) $(LDFLAGS) $(BUILD)/boot.o $(BUILD)/kernel.o -o $(BUILD)/kernel.elf

$(BUILD)/os.iso: $(BUILD)/kernel.elf grub.cfg
	mkdir -p $(BUILD)/iso/boot/grub
	cp $(BUILD)/kernel.elf $(BUILD)/iso/boot/
	cp grub.cfg $(BUILD)/iso/boot/grub/
	grub2-mkrescue -o $(BUILD)/os.iso $(BUILD)/iso 2>/dev/null || \
		xorriso -as mkisofs -R -J -o $(BUILD)/os.iso $(BUILD)/iso

run: all
	$(QEMU) -cdrom $(BUILD)/os.iso -m 512M

rebuild: clean all

clean:
	rm -rf $(BUILD)

.PHONY: all clean run rebuild
