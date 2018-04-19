#include "lzma.h"
#include "xmodem.h"

// The Raspberry Pi firmware at the time this was written defaults loading at address 0x8000.
// Although this bootloader could easily load at 0x0000, it loads at 0x8000
// so that the same binaries built for the SD card work with this bootloader.
// Change the ARMBASE below to use a different location.

#define ARM_BASE 0x0000
#define BUFFER_BASE 0x00400000

extern void mmio_write8(unsigned int, unsigned int);
extern unsigned int mmio_read(unsigned int);
extern unsigned int get_pc(void);
extern void branch_to(unsigned int);
extern void delay(unsigned int);

extern void uart_init(void);
extern void uart_flush(void);
extern void uart_putc(unsigned int);
extern void uart_puts(const char *);
extern void uart_putx(unsigned int);
extern void uart_put_newline(void);
extern unsigned int uart_getc(void);
extern int is_uart_data_ready(void);

extern void timer_init(void);
extern unsigned int timer_tick(void);

int print_error_message(int status)
{
    uart_puts("Error: ");

    if (status == SZ_ERROR_MEM)
        uart_puts(kCantAllocateMessage);

    else if (status == SZ_ERROR_DATA)
        uart_puts(kDataErrorMessage);

    else if (status == SZ_ERROR_WRITE)
        uart_puts(kCantWriteMessage);

    else if (status == SZ_ERROR_READ)
        uart_puts(kCantReadMessage);

    return status;
}

int kernel_main(void)
{
    struct xmodem_packet packet;
    unsigned int tx;
    unsigned int ty;
    int res;

    uart_init();
    timer_init();

    uart_puts("Hello from booloader\r\n");
    xmodem_packet_init(&packet, BUFFER_BASE);

    tx = timer_tick();

    while (1)
    {
        ty = timer_tick();

        if ((ty - tx) >= 4000000)
        {
            uart_putc(NAK);
            tx += 4000000;
        }

        if (is_uart_data_ready() == 0)
            continue;

        res = read_byte(&packet);

        if (res != 0)
            break;

        tx = timer_tick();
    }

    delay(5000000);

    uart_puts("Decompressing..\r\n");
    res = decode(BUFFER_BASE, ARM_BASE, IN_BUF_SIZE);

    if (res != SZ_OK)
        return print_error_message(res);
    else
        uart_puts("Done\r\n\r\n");

    delay(1000000);
    branch_to(ARM_BASE);

    return 0;
}
