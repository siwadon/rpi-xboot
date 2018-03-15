// The Raspberry Pi firmware at the time this was written defaults loading at address 0x8000.
// Although this bootloader could easily load at 0x0000, it loads at 0x8000
// so that the same binaries built for the SD card work with this bootloader.
// Change the ARMBASE below to use a different location.

#define ARMBASE 0x8000

extern void mmio_write8(unsigned int, unsigned int);
extern unsigned int mmio_read(unsigned int);
extern unsigned int get_pc(void);
extern void branch_to(unsigned int);
extern void delay(unsigned int);

extern void uart_init(void);
extern void uart_flush(void);
extern void uart_putc(unsigned int);
extern void uart_putx(unsigned int);
extern unsigned int uart_getc(void);
extern unsigned int uart_lcr(void);
extern int is_uart_data_ready(void);

extern void timer_init(void);
extern unsigned int timer_tick(void);

unsigned char xstring[256];

int kernel_main(void)
{
    unsigned int ra;
    unsigned int rx;
    unsigned int addr;
    unsigned int block;
    unsigned int state;
    unsigned int crc;

    uart_init();
    timer_init();

    // SOH 0x01
    // ACK 0x06
    // NAK 0x15
    // EOT 0x04

    // block numbers start with 1

    // 132 byte packet
    // starts with SOH
    // block number byte
    // 255-block number
    // 128 bytes of data
    // checksum byte (whole packet)
    // a single EOT instead of SOH when done, send an ACK on it too

    block = 1;
    addr = ARMBASE;
    state = 0;
    crc = 0;
    rx = timer_tick();

    while (1)
    {
        ra = timer_tick();

        if ((ra - rx) >= 4000000)
        {
            uart_putc(0x15);
            rx += 4000000;
        }

        if (is_uart_data_ready() == 0)
            continue;

        xstring[state] = uart_getc();
        rx = timer_tick();
        
        if (state == 0)
        {
            if (xstring[state] == 0x04)
            {
                uart_putc(0x06);

                for (ra = 0; ra < 30; ra++)
                    uart_putx(ra);

                uart_putx(0x11111111);
                uart_putx(0x22222222);
                uart_putx(0x33333333);
                uart_flush();
                branch_to(ARMBASE);
                break;
            }
        }

        switch (state)
        {
        case 0:
        {
            if (xstring[state] == 0x01)
            {
                crc = xstring[state];
                state++;
            }
            else
            {
                uart_putc(0x15);
            }
            break;
        }
        case 1:
        {
            if (xstring[state] == block)
            {
                crc += xstring[state];
                state++;
            }
            else
            {
                state = 0;
                uart_putc(0x15);
            }
            break;
        }
        case 2:
        {
            if (xstring[state] == (0xFF - xstring[state - 1]))
            {
                crc += xstring[state];
                state++;
            }
            else
            {
                uart_putc(0x15);
                state = 0;
            }
            break;
        }
        case 131:
        {
            crc &= 0xFF;
            if (xstring[state] == crc)
            {
                for (ra = 0; ra < 128; ra++)
                {
                    mmio_write8(addr++, xstring[ra + 3]);
                }
                uart_putc(0x06);
                block = (block + 1) & 0xFF;
            }
            else
            {
                uart_putc(0x15);
            }
            state = 0;
            break;
        }
        default:
        {
            crc += xstring[state];
            state++;
            break;
        }
        }
    }

    return 0;
}
