ASM     = nasm
CC      = gcc
LD      = ld
CFLAGS  = -ffreestanding -O2 -Wall -Wextra -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -g
LDFLAGS = -m elf_x86_64 -T linker.ld -nostdlib
QEMU    = qemu-system-x86_64
BUILD   = build
SRC     = src

OBJS = $(BUILD)/boot.o $(BUILD)/isr.o $(BUILD)/pic.o \
       $(BUILD)/idt.o $(BUILD)/keyboard.o \
       $(BUILD)/framebuffer.o $(BUILD)/terminal.o \
       $(BUILD)/shell.o $(BUILD)/timer.o \
	   $(BUILD)/system.o $(BUILD)/kmalloc.o \
       $(BUILD)/ata.o $(BUILD)/fs.o $(BUILD)/mouse.o $(BUILD)/gui.o \
	   $(BUILD)/kernel.o

all: $(BUILD)/os.iso

$(BUILD)/boot.o: $(SRC)/boot.s
	mkdir -p $(BUILD)
	$(ASM) -f elf64 $(SRC)/boot.s -o $(BUILD)/boot.o

$(BUILD)/isr.o: $(SRC)/isr.s
	$(ASM) -f elf64 $(SRC)/isr.s -o $(BUILD)/isr.o

$(BUILD)/pic.o: $(SRC)/pic.c
	$(CC) $(CFLAGS) -c $(SRC)/pic.c -o $(BUILD)/pic.o

$(BUILD)/idt.o: $(SRC)/idt.c
	$(CC) $(CFLAGS) -c $(SRC)/idt.c -o $(BUILD)/idt.o

$(BUILD)/terminal.o: $(SRC)/terminal/terminal.c
	$(CC) $(CFLAGS) -c $(SRC)/terminal/terminal.c -o $(BUILD)/terminal.o

$(BUILD)/shell.o: $(SRC)/terminal/shell.c
	$(CC) $(CFLAGS) -c $(SRC)/terminal/shell.c -o $(BUILD)/shell.o

$(BUILD)/system.o: $(SRC)/system/system.c
	$(CC) $(CFLAGS) -c $(SRC)/system/system.c -o $(BUILD)/system.o

$(BUILD)/kmalloc.o: $(SRC)/memory/kmalloc.c
	$(CC) $(CFLAGS) -c $(SRC)/memory/kmalloc.c -o $(BUILD)/kmalloc.o

$(BUILD)/ata.o: $(SRC)/drivers/ata/ata.c
	$(CC) $(CFLAGS) -c $(SRC)/drivers/ata/ata.c -o $(BUILD)/ata.o

$(BUILD)/fs.o: $(SRC)/FileSystem/fs.c
	$(CC) $(CFLAGS) -c $(SRC)/FileSystem/fs.c -o $(BUILD)/fs.o

$(BUILD)/timer.o: $(SRC)/drivers/timer/timer.c
	$(CC) $(CFLAGS) -c $(SRC)/drivers/timer/timer.c -o $(BUILD)/timer.o

$(BUILD)/keyboard.o: $(SRC)/drivers/keyboard/keyboard.c
	$(CC) $(CFLAGS) -c $(SRC)/drivers/keyboard/keyboard.c -o $(BUILD)/keyboard.o

$(BUILD)/mouse.o: $(SRC)/drivers/Mouse/mouse.c
	$(CC) $(CFLAGS) -c $(SRC)/drivers/Mouse/mouse.c -o $(BUILD)/mouse.o

$(BUILD)/framebuffer.o: $(SRC)/framebuffer.c
	$(CC) $(CFLAGS) -c $(SRC)/framebuffer.c -o $(BUILD)/framebuffer.o

$(BUILD)/gui.o: $(SRC)/gui/gui.c
	$(CC) $(CFLAGS) -c $(SRC)/gui/gui.c -o $(BUILD)/gui.o

$(BUILD)/kernel.o: $(SRC)/kernel.c
	$(CC) $(CFLAGS) -c $(SRC)/kernel.c -o $(BUILD)/kernel.o

$(BUILD)/kernel.elf: $(OBJS) linker.ld
	$(LD) $(LDFLAGS) $(OBJS) -o $(BUILD)/kernel.elf

$(BUILD)/os.iso: $(BUILD)/kernel.elf grub.cfg
	mkdir -p $(BUILD)/iso/boot/grub
	cp $(BUILD)/kernel.elf $(BUILD)/iso/boot/
	cp grub.cfg $(BUILD)/iso/boot/grub/
	grub-mkrescue -o $(BUILD)/os.iso $(BUILD)/iso || \
    	xorriso -as mkisofs -R -J -o $(BUILD)/os.iso $(BUILD)/iso

run: all
	$(QEMU) -cdrom $(BUILD)/os.iso \
	        -drive file=disk.img,format=raw \
	        -m 512M

rebuild: clean all
clean:
	rm -rf $(BUILD)

.PHONY: all clean run rebuild
