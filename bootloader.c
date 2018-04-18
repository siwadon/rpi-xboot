#include "xmodem.h"

#include "lib/lzma/Precomp.h"
#include "lib/lzma/CpuArch.h"
#include "lib/lzma/Alloc.h"
#include "lib/lzma/7zFile.h"
#include "lib/lzma/7zVersion.h"
#include "lib/lzma/LzmaDec.h"

// The Raspberry Pi firmware at the time this was written defaults loading at address 0x8000.
// Although this bootloader could easily load at 0x0000, it loads at 0x8000
// so that the same binaries built for the SD card work with this bootloader.
// Change the ARMBASE below to use a different location.

#define ARM_BASE 0x0000
#define BUFFER_BASE 0x00400000

#define IN_BUF_SIZE (1 << 20)
#define OUT_BUF_SIZE (1 << 20)

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
extern unsigned int uart_getc(void);
extern int is_uart_data_ready(void);

extern void timer_init(void);
extern unsigned int timer_tick(void);


// LZMA
// TODO: decompress to output buffer start at ARM_BASE

static SRes decode(unsigned int file_size) {
    UInt64 unpackSize = 0;
    int i;
    SRes res = 0;
    CLzmaDec state;
    int header_size = LZMA_PROPS_SIZE + 8;

    // header: 5 bytes of LZMA properties and 8 bytes of uncompressed size
    Byte (*header)[header_size] = (Byte (*)[header_size]) BUFFER_BASE;

    // Debug input buffer
    uart_putx((unsigned int) header);
    uart_putx((*header)[0]);
    uart_putx((*header)[1]);
    uart_putx((*header)[2]);
    uart_putx((*header)[3]);

    // Read and parse header
    for (i = 0; i < 8; i++)
        unpackSize += (UInt64) (*header)[LZMA_PROPS_SIZE + i] << (i * 8);

    LzmaDec_Construct(&state);
    RINOK(LzmaDec_Allocate(&state, (*header), LZMA_PROPS_SIZE, &g_Alloc));

    uart_putc('$');

    int sizeUnknown = (unpackSize == (UInt64)(Int64)-1);

    Byte *dest = (Byte *) ARM_BASE;
    Byte *src = (*header + 13);

    size_t destLen = sizeUnknown ? OUT_BUF_SIZE : unpackSize;
    size_t srcLen = file_size;

    ELzmaFinishMode finishMode = LZMA_FINISH_ANY;
    ELzmaStatus status;

    res = LzmaDecode(dest, &destLen, src, &srcLen,
        (*header), LZMA_PROPS_SIZE, finishMode,
        &status, &g_Alloc);

    LzmaDec_Free(&state, &g_Alloc);

    return res;
}

void xmodem_packet_init(struct xmodem_packet *packet)
{
    packet->addr = BUFFER_BASE;
    packet->number = 1;
    packet->byte = 0;
    packet->size = 0;
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
            // TODO: Correct file size, might need to implement YMODEM or ZMODEM
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
    SRes res;
    packet->bytes[packet->byte] = uart_getc();
    
    if (packet->byte == 0 && packet->bytes[packet->byte] == EOT)
    {
        uart_putc(ACK);
        uart_flush();
        delay(10000000);

        res = decode(packet->size);
        uart_putc(res == 0 ? 'Y' : 'N');

        branch_to(ARM_BASE);
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
