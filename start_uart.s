.globl _start
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

hang:
    b hang

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

.global mailbox_write
mailbox_write:
    ldr         r1, =0x3f00b880             @; load the hex number =0x3f00b880 into register r1
                                            @; this is the base address of the mailboxes
    wait$:
        ldr     r2, [r1, #0x38]             @; load r2 with the address of the status for mailbox 1 (ARM -> VC)
        tst     r2, #0x80000000             @; check if the FIFO queue is full
        bne     wait$                       @; branch to wait$ label if the queue is full

    str         r0, [r1, #0x20]             @; put the message into mailbox 1 write register, which is at offset 0x20 from the base address
    mov         pc, lr                      @; return

.global mailbox_read
mailbox_read:
    push        {lr}                        @; push the address the function should return to
    mov         r1, r0                      @; save the channel in r1
    ldr         r0, =0x3f00b880             @; load mailbox base address into r0

    right_mail$:
        wait_read$:
            ldr     r2, [r0, #0x18]         @; get status of mailbox 0 (VC -> ARM)
            tst     r2, #0x40000000         @; check if the mailbox is empty
            bne     wait_read$              @; if VC FIFO queue is empty branch to wait_read

        ldr     r2, [r0]                    @; load the address of response data

        and     r3, r2, #0b1111             @; extract the channel, the lowest 4 bits
        teq     r3, r1                      @; check if the response channel is the same we want
        bne     right_mail$                 @; if the channel is wrong branch to wait_read

    and         r0, r2, #0xfffffff0         @; move the response (top 28 bits of mail) into r0
    pop         {pc}

.global set_led_state
set_led_state:
    push        {lr}                @; save address the function should return to
    mov         r1, r0              @; move the led state to r1
    ldr         r0, =mail_message   @; load the message into r0
    mov         r2, #0
    str         r2, [r0, #4]        @; reset request code
    str         r2, [r0, #16]       @; reset request/response size
    mov         r2, #130
    str         r2, [r0, #20]       @; reset pin number

    str         r1, [r0, #24]       @; overwrite the led state
    add         r0, #8              @; add the channel 8 as the last 4 bits of the message
    bl          mailbox_write       @; write the message to mailbox
    mov         r0, #8              @; the channel to read the message from
    bl          mailbox_read        @; read the message to prevent the FIFO queue to get full
    pop         {pc}                @; return

.section .data
.align 4                            @; last 4 bits of the next label set to 0 (16-byte alligned)
mail_message:
    .int    size                    @; #0 message header contains the size of the message
    .int    0                       @; #4 request code 0

    .int    0x00038041              @; #8 header tag ID
    .int    8                       @; #12 size of tag data
    .int    0                       @; #16 request/response size

    .int    130                     @; #20 pin number
    .int    1                       @; #24 pin state
    .int    0                       @; #28 signal the GPU that the message is over
size:
    .int    . - mail_message        @; size of the message
