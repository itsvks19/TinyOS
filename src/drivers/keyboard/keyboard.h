#pragma once
#include <stdint.h>

void keyboard_init(void);
void keyboard_handler(void); // called from isr_handler on IRQ1
char keyboard_getchar(void); // returns next char from ring buffer, 0 if empty
