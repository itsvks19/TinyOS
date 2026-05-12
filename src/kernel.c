#include "framebuffer.h"
#include "idt.h"
#include "keyboard.h"

static void draw_swatch(int x, int y, int w, int h, color_t c, const char *name) {
    fb_fill_rect(x, y, w, h, c);
    fb_draw_string(x, y + h + 4, name, COLOR_WHITE, COLOR_DARK);
}

void kmain(void *mb2_info) {
    if (fb_init(mb2_info) != 0) {
        // framebuffer unavailable - fall back to VGA text
        volatile unsigned short *vga = (volatile unsigned short *)0xB8000;
        const char *msg = "ERROR: no framebuffer tag from GRUB";
        for (int i = 0; msg[i]; i++)
            vga[i] = (unsigned short)(0x0C00 | (unsigned char)msg[i]);
        for (;;)
            __asm__ volatile("hlt");
    }

    int W = fb_width(), H = fb_height();

    // background
    fb_clear(COLOR_DARK);

    // title bar
    fb_fill_rect(0, 0, W, 28, RGB(30, 30, 50));
    fb_fill_rect(0, 26, W, 2, RGB(80, 80, 160)); // accent line
    fb_draw_string(10, 10, "TinyOS  v0.1  [64-bit long mode + framebuffer]", COLOR_WHITE,
                   RGB(30, 30, 50));

    // color swatches
    int sx = 40, sy = 55, sw = 100, sh = 60, gap = 120;
    draw_swatch(sx + 0 * gap, sy, sw, sh, COLOR_RED, "RED   ");
    draw_swatch(sx + 1 * gap, sy, sw, sh, COLOR_GREEN, "GREEN ");
    draw_swatch(sx + 2 * gap, sy, sw, sh, COLOR_BLUE, "BLUE  ");
    draw_swatch(sx + 3 * gap, sy, sw, sh, COLOR_YELLOW, "YELLOW");
    draw_swatch(sx + 4 * gap, sy, sw, sh, COLOR_CYAN, "CYAN  ");
    draw_swatch(sx + 5 * gap, sy, sw, sh, COLOR_PURPLE, "PURPLE");
    draw_swatch(sx + 6 * gap, sy, sw, sh, COLOR_ORANGE, "ORANGE");

    // gradient bar (red -> blue across 512 pixels)
    int bar_y = 155;
    fb_draw_string(40, bar_y - 14, "gradient:", COLOR_GRAY, COLOR_DARK);
    for (int x = 0; x < 512; x++) {
        color_t c = RGB(255 - x / 2, 40, x / 2);
        fb_fill_rect(40 + x, bar_y, 1, 18, c);
    }

    // info text
    int ty = 200;
    fb_draw_string(40, ty, "Resolution : ", COLOR_GRAY, COLOR_DARK);
    fb_draw_int(144, ty, W, COLOR_CYAN, COLOR_DARK);
    fb_draw_string(192, ty, "x", COLOR_GRAY, COLOR_DARK);
    fb_draw_int(200, ty, H, COLOR_CYAN, COLOR_DARK);

    fb_draw_string(40, ty + 18, "CPU mode   : 64-bit long mode", COLOR_GRAY, COLOR_DARK);
    fb_draw_string(40, ty + 36, "Paging     : 4 GB identity map (2 MB huge pages)", COLOR_GRAY,
                   COLOR_DARK);
    fb_draw_string(40, ty + 54, "Font       : font8x8_basic  (8x8 bitmap)", COLOR_GRAY, COLOR_DARK);

    // status bar
    fb_fill_rect(0, H - 20, W, 20, RGB(30, 30, 50));
    fb_draw_string(10, H - 14, "IDT + PIC + keyboard active", RGB(100, 200, 100), RGB(30, 30, 50));

    idt_init();
    keyboard_init();

    for (;;)
        __asm__ volatile("hlt");
}
