#include "framebuffer.h"
#include "idt.h"
#include "drivers/keyboard/keyboard.h"
#include "terminal/terminal.h"
#include "terminal/shell.h"

void kmain(void *mb2_info) {
    if (fb_init(mb2_info) != 0) {
        for (;;)
            __asm__ volatile("hlt");
    }

    terminal_init(fb_width(), fb_height());

    idt_init();
    keyboard_init();

    shell_init();

    for (;;) {
        __asm__ volatile("hlt");

        terminal_tick();

        char c = keyboard_getchar();

        if (c)
            shell_input(c);
    }
}