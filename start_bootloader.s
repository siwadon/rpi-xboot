.globl _start

@ Reserves the given number of bytes. The bytes are filled with zero
.space 0x200000-0x0000, 0

_start:
    b loader_start
    b loader_sleep  // undefined
    b loader_sleep  // svc
    b loader_sleep  // prefetch
    b loader_sleep  // abort
    b loader_sleep  // hypervisor
    b loader_sleep  // irq
    b loader_sleep  // fiq

loader_sleep:
    wfi
    b loader_sleep

loader_start:
    // Switch to SVC mode, all interrupts disabled
    .set PSR_MODE_SVC, 0x13
    .set PSR_MODE_IRQ_DISABLED, (1<<7)
    .set PSR_MODE_FIQ_DISABLED, (1<<6)
    msr cpsr_c, #(PSR_MODE_SVC + PSR_MODE_FIQ_DISABLED + PSR_MODE_IRQ_DISABLED)

    // Set all CPUs to wait except the primary CPU
    mrc p15, 0, r0, c0, c0, 5
    ands r0, r0, #0x03
    bne loader_sleep

    mov sp, #0x08000000
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
