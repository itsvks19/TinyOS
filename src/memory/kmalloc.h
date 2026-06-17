#pragma once

#include <stddef.h>

void kmalloc_init(void);
void *kmalloc(size_t size);

size_t kmalloc_used(void);
size_t kmalloc_total(void);