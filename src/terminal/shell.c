#include "shell.h"
#include "terminal.h"
#include "../drivers/timer/timer.h"
#include "../io.h"
#include "../system/system.h"
#include "../memory/kmalloc.h"

#define CMD_MAX 128

static char cmd_buf[CMD_MAX];
static int cmd_len = 0;

static int str_eq(const char *a, const char *b) {
    while (*a && *b) {
        if (*a != *b)
            return 0;
        a++;
        b++;
    }

    return (*a == '\0' && *b == '\0');
}

static void show_prompt(void) {
    terminal_print("> ");
}

static void execute_command(void) {
    cmd_buf[cmd_len] = '\0';
    char *args = 0;

    for (int i = 0; cmd_buf[i]; i++) {
        if (cmd_buf[i] == ' ') {
            cmd_buf[i] = '\0';
            args = &cmd_buf[i + 1];
            break;
        }
    }

    terminal_putchar('\n');

    if (str_eq(cmd_buf, "ver")) {
        terminal_print("Nano OS v0.1\n");
    }
    else if (str_eq(cmd_buf, "help")) {
        terminal_print("Commands:\n");
        terminal_print("  ver\n");
        terminal_print("  clear\n");
        terminal_print("  help\n");
        terminal_print("  say [text]\n");
        terminal_print("  uptime\n");
        terminal_print("  meminfo\n");
        terminal_print("  alloc\n");
        terminal_print("  reboot\n");
    }
    else if (str_eq(cmd_buf, "clear")) {
        terminal_clear();
    }
    else if (str_eq(cmd_buf, "say")) {
        if (args && *args) {
            terminal_print(args);
            terminal_putchar('\n');
        } else {
            terminal_print("Usage: say [text]\n");
        }
    }
    else if (str_eq(cmd_buf, "uptime")) {
        uint64_t seconds = timer_get_ticks() / 100;

        terminal_print("Uptime: ");
        print_uint(seconds);
        terminal_print(" seconds\n");
    }
    else if (str_eq(cmd_buf, "reboot")) {
        terminal_print("Rebooting...\n");
        reboot_system();
    }
    else if (str_eq(cmd_buf, "meminfo")) {
        terminal_print("Framebuffer: ");
        print_uint(system_width);
        terminal_putchar('x');
        print_uint(system_height);
        terminal_putchar('\n');

        terminal_print("Timer: ");
        print_uint(system_timer_hz);
        terminal_print(" Hz\n");

        terminal_print("Ticks: ");
        print_uint(timer_get_ticks());
        terminal_putchar('\n');
        terminal_print("Heap: ");
        print_uint(kmalloc_used());
        terminal_print(" / ");
        print_uint(kmalloc_total());
        terminal_print(" bytes\n");
        terminal_putchar('\n');
    }
    else if (str_eq(cmd_buf, "alloc")) {
        void *p = kmalloc(64);

        if (p)
            terminal_print("Allocated 64 bytes\n");
        else
            terminal_print("Out of memory\n");
    }
    else if (cmd_len != 0) {
        terminal_print("Unknown command\n");
    }

    cmd_len = 0;

    show_prompt();
}

void shell_init(void) {
    show_prompt();
}

void shell_input(char c) {
    if (c == '\n') {
        execute_command();
        return;
    }

    if (c == '\b') {
        if (cmd_len > 0) {
            cmd_len--;
            terminal_putchar('\b');
        }

        return;
    }

    if (cmd_len >= CMD_MAX - 1)
        return;

    cmd_buf[cmd_len++] = c;
    terminal_putchar(c);
}

void print_uint(uint64_t value) {
    char buf[21];
    int i = 0;

    if (value == 0) {
        terminal_putchar('0');
        return;
    }

    while (value > 0) {
        buf[i++] = '0' + (value % 10);
        value /= 10;
    }

    while (i > 0) {
        terminal_putchar(buf[--i]);
    }
}

void reboot_system(void) {
    while (inb(0x64) & 0x02)
        ;

    outb(0x64, 0xFE);

    for (;;)
        __asm__ volatile("hlt");
}