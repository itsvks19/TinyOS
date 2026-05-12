#include "pic.h"
#include "io.h"

void pic_init(void) {
    // ICW1 - start init, ICW4 will follow
    outb(PIC1_CMD, 0x11);
    io_wait();
    outb(PIC2_CMD, 0x11);
    io_wait();
    // ICW2 - vector offsets
    outb(PIC1_DATA, IRQ_BASE);
    io_wait(); // master: IRQ0-7  -> vectors 32-39
    outb(PIC2_DATA, IRQ_BASE + 8);
    io_wait(); // slave:  IRQ8-15 -> vectors 40-47
    // ICW3 — master/slave wiring
    outb(PIC1_DATA, 0x04);
    io_wait(); // master: slave is on IRQ2
    outb(PIC2_DATA, 0x02);
    io_wait(); // slave: its cascade identity = 2
    // ICW4 — 8086 mode
    outb(PIC1_DATA, 0x01);
    io_wait();
    outb(PIC2_DATA, 0x01);
    io_wait();
    // mask all IRQs for now - unmask individually when you add drivers
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);
}

void pic_send_eoi(uint8_t irq) {
    if (irq >= 8)
        outb(PIC2_CMD, PIC_EOI); // notify slave too
    outb(PIC1_CMD, PIC_EOI);
}
