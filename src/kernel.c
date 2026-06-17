#include "framebuffer.h"
#include "idt.h"
#include "drivers/timer/timer.h"
#include "drivers/keyboard/keyboard.h"
#include "terminal/terminal.h"
#include "terminal/shell.h"
#include "system/system.h"
#include "memory/kmalloc.h"

void kmain(void *mb2_info) {
    if (fb_init(mb2_info) != 0) {
        for (;;)
            __asm__ volatile("hlt");
    }

    system_width = fb_width();
    system_height = fb_height();
    system_timer_hz = 100;

    terminal_init(system_width, system_height);

    idt_init();
    keyboard_init();
    timer_init(system_timer_hz);
    kmalloc_init();
    shell_init();

    for (;;) {
        __asm__ volatile("hlt");

        terminal_tick();

        char c = keyboard_getchar();

        if (c)
            shell_input(c);
    }
}