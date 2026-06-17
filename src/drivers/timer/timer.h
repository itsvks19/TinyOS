#pragma once

#include <stdint.h>

extern volatile uint64_t timer_ticks;

void timer_init(uint32_t frequency);
void timer_handler(void);

uint64_t timer_get_ticks(void);