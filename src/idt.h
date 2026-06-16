#pragma once
#include <stdint.h>

// 64-bit IDT gate descriptor: 16 bytes
typedef struct __attribute__((packed)) {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t ist;       // interrupt stack table index (0 = don't use)
    uint8_t type_attr; // 0x8E = present, ring0, 64-bit interrupt gate
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t reserved;
} idt_entry_t;

typedef struct __attribute__((packed)) {
    uint16_t limit;
    uint64_t base;
} idt_ptr_t;

// Stack frame layout passed to isr_handler()
// Order matches what isr.s pushes (r15 first = lowest address)
typedef struct {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t int_num, error_code;
    // pushed automatically by CPU:
    uint64_t rip, cs, rflags, rsp, ss;
} interrupt_frame_t;

void idt_init(void);
void isr_handler(interrupt_frame_t *frame); // called from isr.s
