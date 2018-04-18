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
extern void uart_puts(const char*);
extern void uart_putx(unsigned int);
extern void uart_put_newline(void);
extern unsigned int uart_getc(void);
extern int is_uart_data_ready(void);

extern void timer_init(void);
extern unsigned int timer_tick(void);

int kernel_main(void)
{
    struct xmodem_packet packet;
    unsigned int tx;
    unsigned int ty;
    int res;

    uart_init();
    timer_init();
    xmodem_packet_init(&packet, BUFFER_BASE, ARM_BASE);
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
        {
            continue;
        }

        res = read_byte(&packet);

        if (res != 0) {
            break;
        }

        tx = timer_tick();
    }

    delay(5000000);

    // TODO: uart_puts() stops working
    uart_putc('D');
    uart_putc('e');
    uart_putc('c');
    uart_putc('o');
    uart_putc('m');
    uart_putc('p');
    uart_putc('r');
    uart_putc('e');
    uart_putc('s');
    uart_putc('s');
    uart_putc('?');
    uart_putc(' ');
    uart_putc('[');
    uart_putc('y');
    uart_putc('/');
    uart_putc('N');
    uart_putc(']');
    uart_putc(' ');

    char key = uart_getc();

    if (key == 'Y' || key == 'y') {
        uart_put_newline();
        // not yet working, it stops in the middle
        res = decode(BUFFER_BASE, ARM_BASE, IN_BUF_SIZE);
        uart_putc(res == 0 ? '0' : '?');

    } else {
        uart_put_newline();
    }

    branch_to(ARM_BASE);

    return 0;
}
