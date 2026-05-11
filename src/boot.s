section .multiboot
align 4
    dd 0x1BADB002            ; magic number (Multiboot)
    dd 0x00                  ; flags (no memory info)
    dd -(0x1BADB002 + 0x00)  ; checksum

section .bss
align 4096
pml4_table: resb 4096   ; Page-Map Level 4
pdpt_table: resb 4096   ; Page-Directory Pointer Table
pd_table:   resb 4096   ; Page Directory  (2 MB huge pages)
stack_bottom:
    resb 16384
stack_top:

section .rodata
gdt64:
    dq 0                                          ; null descriptor
.code: equ $ - gdt64
    dq (1<<43)|(1<<44)|(1<<47)|(1<<53)            ; 64-bit code segment
                                                  ;  bit43=executable  bit44=non-system
                                                  ;  bit47=present     bit53=L (long mode)
gdt64_ptr:
    dw $ - gdt64 - 1   ; limit
    dq gdt64           ; base (lgdt in 32-bit mode only reads low 32 bits — fine here)

section .text
bits 32
global _start
extern kmain

_start:
    mov esp, stack_top

    ; Identity-map the first 2 MB so code keeps running after paging on
    ; PML4[0] -> PDPT
    mov eax, pdpt_table
    or  eax, 0x3               ; present + writable
    mov [pml4_table], eax
    ;  PDPT[0] -> PD
    mov eax, pd_table
    or  eax, 0x3
    mov [pdpt_table], eax
    ;    PD[0]  -> 2 MB huge page at physical 0x0
    ;    bit0=present  bit1=writable  bit7=PS (Page Size = huge)
    mov dword [pd_table], 0x83

    ; Load CR3 with PML4 address
    mov eax, pml4_table
    mov cr3, eax

    ; Enable PAE (Physical Address Extension). PAE is required for long mode
    mov eax, cr4
    or  eax, (1 << 5)
    mov cr4, eax

    ; Set Long Mode Enable bit in the EFER MSR
    mov ecx, 0xC0000080
    rdmsr
    or  eax, (1 << 8)
    wrmsr

    ; Enable paging (CR0.PG). PE is already set by GRUB.
    mov eax, cr0
    or  eax, (1 << 31)
    mov cr0, eax
    ; CPU is now in "compatibility mode" (IA-32e, 32-bit sub-mode)

    ; Load 64-bit GDT and far-jump -> this flips the CPU into true 64-bit mode
    lgdt [gdt64_ptr]
    jmp  gdt64.code:long_mode_start

; 64-bit code from here on
bits 64
long_mode_start:
    xor ax, ax     ; null out all data segment registers
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call kmain

.hang:
    cli
    hlt
    jmp .hang
