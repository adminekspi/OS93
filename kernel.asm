[BITS 32]

; Multiboot header
section .multiboot
align 4
dd 0x1BADB002             ; Multiboot magic number
dd 0x0                    ; Flags (none)
dd -(0x1BADB002 + 0x0)    ; Checksum

section .text
global _start
extern kmain

_start:
    cli                   ; Clear interrupts
    mov esp, stack_space  ; Set up stack
    call kmain

.hang:
    hlt
    jmp .hang

section .bss
resb 8192                 ; 8 KB stack
stack_space:
