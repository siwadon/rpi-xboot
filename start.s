.globl _start
_start:
    mov sp, #0x8000
    bl kernel_main
hang: b hang

@ Memory-Mapped I/O output
.globl mmio_write
mmio_write:
    str r1, [r0]
    bx lr

@ Memory-Mapped I/O input
.globl mmio_read
mmio_read:
    ldr r0, [r0]
    bx lr

@ Loop delay times
.globl delay
delay:
    subs r0, r0, #1
    bne delay
    bx lr

.globl GETPC
GETPC:
    mov r0,lr
    bx lr


.globl BRANCHTO
BRANCHTO:
    bx r0
