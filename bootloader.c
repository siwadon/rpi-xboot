#include "xmodem.h"

// The Raspberry Pi firmware at the time this was written defaults loading at address 0x8000.
// Although this bootloader could easily load at 0x0000, it loads at 0x8000
// so that the same binaries built for the SD card work with this bootloader.
// Change the ARMBASE below to use a different location.

#define ARMBASE 0x0000

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
extern int is_uart_data_ready(void);

extern void timer_init(void);
extern unsigned int timer_tick(void);

void xmodem_packet_init(struct xmodem_packet *packet)
{
    packet->addr = ARMBASE;
    packet->number = 1;
    packet->byte = 0;
    packet->crc = 0;
}

void add_checksum_byte(struct xmodem_packet *packet)
{
    packet->crc += packet->bytes[packet->byte];
    packet->byte++;
}

/* 
XMODEM packet format
132 bytes packet containing 128 bytes of data

1:       SOH
2:       packet number (0 - 255)
3:       1s' complement of the packet number (255 - packet number)
4-131:   packet data
132-133: checksum, the sum of all of the bytes in the packet mod 256
*/
void process_byte(struct xmodem_packet *packet)
{
    int i;
    int expect_byte = packet->bytes[packet->byte];

    switch (packet->byte)
    {
    case 0:
    {
        if (expect_byte == SOH)
        {
            add_checksum_byte(packet);
        }
        else
        {
            uart_putc(NAK);
        }

        break;
    }
    case 1:
    {
        if (expect_byte == packet->number)
        {
            add_checksum_byte(packet);
        }
        else
        {
            uart_putc(NAK);
            packet->byte = 0;
        }

        break;
    }
    case 2:
    {
        if (expect_byte == (255 - packet->bytes[1]))
        {
            add_checksum_byte(packet);
        }
        else
        {
            uart_putc(NAK);
            packet->byte = 0;
        }

        break;
    }
    case 131:
    {
        if (expect_byte == (packet->crc % 256))
        {
            for (i = 0; i < 128; i++)
            {
                mmio_write8(packet->addr++, packet->bytes[i + 3]);
            }
            uart_putc(ACK);
            packet->crc = 0;
            packet->number = (packet->number + 1) % 256;
        }
        else
        {
            uart_putc(NAK);
        }

        packet->byte = 0;
        break;
    }
    default:
    {
        add_checksum_byte(packet);
        break;
    }
    }
}

void read_byte(struct xmodem_packet *packet)
{
    packet->bytes[packet->byte] = uart_getc();
    
    if (packet->byte == 0 && packet->bytes[packet->byte] == EOT)
    {
        uart_putc(ACK);
        uart_flush();
        delay(10000000);
        branch_to(ARMBASE);
    }

    process_byte(packet);
}

int kernel_main(void)
{
    struct xmodem_packet packet;
    unsigned int tx;
    unsigned int ty;

    uart_init();
    timer_init();
    xmodem_packet_init(&packet);
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

        read_byte(&packet);
        tx = timer_tick();
    }

    return 0;
}
