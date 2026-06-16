#include "shell.h"
#include "terminal.h"

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

    terminal_putchar('\n');

    if (str_eq(cmd_buf, "ver")) {
        terminal_print("Nano OS v0.1\n");
    }
    else if (str_eq(cmd_buf, "help")) {
        terminal_print("Commands:\n");
        terminal_print("  ver\n");
        terminal_print("  clear\n");
        terminal_print("  help\n");
    }
    else if (str_eq(cmd_buf, "clear")) {
        terminal_clear();
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