#include "idt.h"
#include "keyboard.h"

// Minimal VGA text-mode console
#define VGA_COLS 80
#define VGA_ROWS 25
#define VGA_BASE ((volatile unsigned short *)0xB8000)

static int cursor_col = 0;
static int cursor_row = 1;   // row 0 = banner, start typing from row 1

static void vga_putchar(char c) {
    volatile unsigned short *vga = VGA_BASE;

    if (c == '\b') {
        if (cursor_col > 0) {
            cursor_col--;
            vga[cursor_row * VGA_COLS + cursor_col] = 0x0F00 | ' ';
        }
        return;
    }
    if (c == '\n' || cursor_col >= VGA_COLS) {
        cursor_col = 0;
        cursor_row++;
        if (cursor_row >= VGA_ROWS) {
            // scroll: copy every row up by one
            for (int r = 1; r < VGA_ROWS; r++)
                for (int col = 0; col < VGA_COLS; col++)
                    vga[(r-1)*VGA_COLS + col] = vga[r*VGA_COLS + col];
            // clear last row
            for (int col = 0; col < VGA_COLS; col++)
                vga[(VGA_ROWS-1)*VGA_COLS + col] = 0x0F00 | ' ';
            cursor_row = VGA_ROWS - 1;
        }
        if (c == '\n') return;
    }
    vga[cursor_row * VGA_COLS + cursor_col] = (unsigned short)(0x0F00 | (unsigned char)c);
    cursor_col++;
}

static void vga_puts(const char *s) {
    for (int i = 0; s[i]; i++) vga_putchar(s[i]);
}

void kmain(void) {
    // banner on row 0
    volatile unsigned short *vga = VGA_BASE;
    const char *banner = "TinyOS  [64-bit]  --  type something!";
    for (int i = 0; banner[i]; i++)
        vga[i] = (unsigned short)(0x2F00 | (unsigned char)banner[i]); // green on black

    idt_init();
    keyboard_init();

    // prompt
    vga_puts("> ");

    for (;;) {
        char c = keyboard_getchar();
        if (c) {
            vga_putchar(c);
            if (c == '\n') vga_puts("> ");
        }
        __asm__ volatile("hlt"); // sleep until next interrupt
    }
}
