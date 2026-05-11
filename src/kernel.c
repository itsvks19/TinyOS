
void kmain(void) {
    volatile unsigned short *vga = (unsigned short *)0xB8000;
    const char *msg = "Hello from TinyOS  [64-bit long mode!]";
    for (int i = 0; msg[i] != '\0'; ++i)
        vga[i] = (unsigned short)(0x0F00 | (unsigned char)msg[i]);
    for (;;)
        __asm__ volatile("hlt");
}
