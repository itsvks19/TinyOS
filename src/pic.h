#pragma once
#include <stdint.h>

#define PIC1_CMD 0x20
#define PIC1_DATA 0x21
#define PIC2_CMD 0xA0
#define PIC2_DATA 0xA1
#define PIC_EOI 0x20
#define IRQ_BASE 0x20 // remap IRQs to vectors 32-47

void pic_init(void);
void pic_send_eoi(uint8_t irq);
