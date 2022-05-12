.section ".text.boot"

.globl _start
_start:
    # Setup the stack.
    mov sp, #0x8000
    ldr r0, =vector
    mcr p15, 0, r0, c12, c0, 0
    bl kernel_main

.globl halt
halt:
    wfe
    b halt

.globl do_nothing
do_nothing:
    bx lr