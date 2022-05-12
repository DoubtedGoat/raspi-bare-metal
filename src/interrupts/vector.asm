no_handler:
    bx lr

.globl vector
vector:
    b =_start       @ Reset
    b =no_handler   @ Undefined Instruction
    b =no_handler   @ Hypervisor Call
    b =no_handler   @ Prefect Abort
    b =no_handler   @ Data Abort
    b =no_handler   @ Hyp Trap
    b =interrupt_handler    @ IRQ Interrupt
    b =no_handler   @ FIQ Interrupt