#pragma once

void terminal_init(int width, int height);

void terminal_putchar(char c);
void terminal_print(const char *s);

void terminal_clear(void);
void terminal_draw_cursor(void);
void terminal_hide_cursor(void);
void terminal_tick(void);