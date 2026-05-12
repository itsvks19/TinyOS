bits 64

extern isr_handler

; exceptions WITH a CPU-pushed error code - just push the int number
%macro ISR_ERR 1
isr_%1:
    push qword %1
    jmp isr_common
%endmacro

; everything else - push dummy 0 first so stack layout is always identical
%macro ISR_NOERR 1
isr_%1:
    push qword 0
    push qword %1
    jmp isr_common
%endmacro

; CPU exceptions 0-31  (only 8,10,11,12,13,14,17,21,29,30 push error codes)
ISR_NOERR 0   ; #DE division error
ISR_NOERR 1   ; #DB debug
ISR_NOERR 2   ; NMI
ISR_NOERR 3   ; #BP breakpoint
ISR_NOERR 4   ; #OF overflow
ISR_NOERR 5   ; #BR bound range
ISR_NOERR 6   ; #UD invalid opcode
ISR_NOERR 7   ; #NM device not available
ISR_ERR   8   ; #DF double fault
ISR_NOERR 9
ISR_ERR   10  ; #TS invalid TSS
ISR_ERR   11  ; #NP segment not present
ISR_ERR   12  ; #SS stack fault
ISR_ERR   13  ; #GP general protection fault
ISR_ERR   14  ; #PF page fault
ISR_NOERR 15
ISR_NOERR 16  ; #MF x87 FPU error
ISR_ERR   17  ; #AC alignment check
ISR_NOERR 18  ; #MC machine check
ISR_NOERR 19  ; #XM SIMD exception
ISR_NOERR 20  ; #VE virtualization
ISR_ERR   21  ; #CP control protection
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_ERR   29
ISR_ERR   30  ; #SX security exception
ISR_NOERR 31

; IRQ 0-15 (vectors 32-47 after PIC remap)
%assign i 32
%rep 16
    ISR_NOERR i
    %assign i i+1
%endrep

; vectors 48-255 (spurious / future use)
%assign i 48
%rep 208
    ISR_NOERR i
    %assign i i+1
%endrep

; shared trampoline
; at entry: stack = [...cpu frame... | error_code | int_num]  ← rsp
isr_common:
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov rdi, rsp        ; first arg to isr_handler = pointer to interrupt_frame_t
    call isr_handler

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    add rsp, 16         ; discard int_num + error_code
    iretq

; stub pointer table (idt.c indexes into this)
section .rodata
global isr_stubs
isr_stubs:
%assign i 0
%rep 256
    dq isr_ %+ i
    %assign i i+1
%endrep
