#include "xmodem.h"

extern void mmio_write8(unsigned int, unsigned int);
extern void uart_putc(unsigned int);
extern void uart_flush(void);
extern unsigned int uart_getc(void);
extern void delay(unsigned int);

void xmodem_packet_init(struct xmodem_packet *packet, unsigned int output_base)
{
    packet->addr = output_base;
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
4-130:   packet data
131-132: checksum, the sum of all of the bytes in the packet mod 256
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

int read_byte(struct xmodem_packet *packet)
{
    packet->bytes[packet->byte] = uart_getc();
    
    if (packet->byte == 0 && packet->bytes[packet->byte] == EOT)
    {
        uart_putc(ACK);
        uart_flush();

        return 1;
    }

    process_byte(packet);

    return 0;
}