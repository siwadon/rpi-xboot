.globl _start

_start:
    b skip

.space 0x200000-0x8004, 0

skip:
    mov sp,#0x08000000
    bl kernel_main

hang: b hang

@ Memory-Mapped I/O output 32-bit
.globl mmio_write
mmio_write:
    str r1, [r0]
    bx lr

@ Memory-Mapped I/O output 16-bit (halfword)
.globl mmio_write16
mmio_write16:
    strh r1, [r0]
    bx lr

@ Memory-Mapped I/O output 8-bit (byte)
.globl mmio_write8
mmio_write8:
    strb r1, [r0]
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

.globl get_pc
get_pc:
    mov r0, lr
    bx lr

.globl branch_to
branch_to:
    bx r0
