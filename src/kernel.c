#include "framebuffer.h"
#include "idt.h"
#include "drivers/timer/timer.h"
#include "drivers/keyboard/keyboard.h"
#include "drivers/Mouse/mouse.h"
#include "terminal/terminal.h"
#include "terminal/shell.h"
#include "system/system.h"
#include "memory/kmalloc.h"
#include "gui/gui.h"

/*NOT DEVELOPED FULLY BUGS LIKE FLICKERING AND LAG MOUSE INPUT*/

// void kmain(void *mb2_info) {
//     if (fb_init(mb2_info) != 0) {
//         for (;;)
//             __asm__ volatile("hlt");
//     }

//     kmalloc_init();
//     fb_enable_backbuffer();

//     system_width = fb_width();
//     system_height = fb_height();
//     system_timer_hz = 100;

//     terminal_init(system_width, system_height);

//     idt_init();
//     keyboard_init();
//     mouse_init();
//     timer_init(system_timer_hz);
//     shell_init();

//     for (;;) {
//         __asm__ volatile("hlt");

//         mouse_tick();

//         gui_update();
//         gui_draw();

//         cursor_draw();
//     }
// }

void kmain(void *mb2_info) {
    if (fb_init(mb2_info) != 0) {
        for (;;)
            __asm__ volatile("hlt");
    }

    kmalloc_init();
    fb_enable_backbuffer();

    system_width = fb_width();
    system_height = fb_height();

    terminal_init(system_width, system_height);

    idt_init();
    keyboard_init();

    shell_init();

    terminal_print("NanoOS\n");
    terminal_print("Type 'help' for available commands.\n\n/>");

    for (;;) {
        __asm__ volatile("hlt");
    }
}