#include "framebuffer.h"
#include "idt.h"
#include "drivers/timer/timer.h"
#include "drivers/keyboard/keyboard.h"
#include "drivers/mouse/mouse.h"
#include "terminal/terminal.h"
#include "terminal/shell.h"
#include "system/system.h"
#include "memory/kmalloc.h"
#include "gui/gui.h"

void kmain(void *mb2_info) {
    if (fb_init(mb2_info) != 0)
        for (;;) __asm__ volatile("hlt");

    kmalloc_init();
    fb_enable_backbuffer();

    system_width    = fb_width();
    system_height   = fb_height();
    system_timer_hz = 60;        // 60 ticks/sec = 60fps max

    terminal_init(system_width, system_height);

    idt_init();
    keyboard_init();
    mouse_init();
    timer_init(system_timer_hz);
    shell_init();

    uint64_t last_tick = 0;

    for (;;) {
        __asm__ volatile("hlt");  // sleep until any interrupt

        // only render when a new timer tick has fired
        if (timer_ticks == last_tick)
            continue;
        last_tick = timer_ticks;

        mouse_tick();
        gui_update();
        gui_draw();
    }
}

/*for  earilier debug uses */
// void kmain(void *mb2_info) {
//     if (fb_init(mb2_info) != 0) {
//         for (;;)
//             __asm__ volatile("hlt");
//     }

//     kmalloc_init();
//     fb_enable_backbuffer();

//     system_width = fb_width();
//     system_height = fb_height();

//     terminal_init(system_width, system_height);

//     idt_init();
//     keyboard_init();

//     shell_init();

//     terminal_print("NanoOS\n");
//     terminal_print("Type 'help' for available commands.\n\n/>");

//     for (;;) {
//         __asm__ volatile("hlt");
//     }
// }