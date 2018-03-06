
// Uncomment one of these
// #include "BCM2835.h" /* Raspberry A, A+, B, B+ */
#include "BCM2836.h" /* Raspberry Pi 2, 3 */

extern void mmio_write(unsigned int, unsigned int);
extern unsigned int mmio_read(unsigned int);
extern void dummy(unsigned int);

#define ARM_TIMER_CTL (PBASE + 0x0000B408)
#define ARM_TIMER_CNT (PBASE + 0x0000B420)

#define GPFSEL1 (PBASE + 0x00200004)
#define GPSET0 (PBASE + 0x0020001C)
#define GPCLR0 (PBASE + 0x00200028)
#define GPPUD (PBASE + 0x00200094)
#define GPPUDCLK0 (PBASE + 0x00200098)

#define AUX_ENABLES (PBASE + 0x00215004)
#define AUX_MU_IO_REG (PBASE + 0x00215040)
#define AUX_MU_IER_REG (PBASE + 0x00215044)
#define AUX_MU_IIR_REG (PBASE + 0x00215048)
#define AUX_MU_LCR_REG (PBASE + 0x0021504C)
#define AUX_MU_MCR_REG (PBASE + 0x00215050)
#define AUX_MU_LSR_REG (PBASE + 0x00215054)
#define AUX_MU_MSR_REG (PBASE + 0x00215058)
#define AUX_MU_SCRATCH (PBASE + 0x0021505C)
#define AUX_MU_CNTL_REG (PBASE + 0x00215060)
#define AUX_MU_STAT_REG (PBASE + 0x00215064)
#define AUX_MU_BAUD_REG (PBASE + 0x00215068)

//GPIO14  TXD0 and TXD1
//GPIO15  RXD0 and RXD1
unsigned int uart_lcr(void)
{
    return (mmio_read(AUX_MU_LSR_REG));
}

unsigned int uart_recv(void)
{
    while (1)
    {
        if (mmio_read(AUX_MU_LSR_REG) & 0x01)
            break;
    }
    return (mmio_read(AUX_MU_IO_REG) & 0xFF);
}

unsigned int uart_check(void)
{
    if (mmio_read(AUX_MU_LSR_REG) & 0x01)
        return (1);
    return (0);
}

void uart_send(unsigned int c)
{
    while (1)
    {
        if (mmio_read(AUX_MU_LSR_REG) & 0x20)
            break;
    }
    mmio_write(AUX_MU_IO_REG, c);
}

void uart_flush(void)
{
    while (1)
    {
        if ((mmio_read(AUX_MU_LSR_REG) & 0x100) == 0)
            break;
    }
}

void hexstrings(unsigned int d)
{
    //unsigned int ra;
    unsigned int rb;
    unsigned int rc;

    rb = 32;
    while (1)
    {
        rb -= 4;
        rc = (d >> rb) & 0xF;
        if (rc > 9)
            rc += 0x37;
        else
            rc += 0x30;
        uart_send(rc);
        if (rb == 0)
            break;
    }
    uart_send(0x20);
}

void hexstring(unsigned int d)
{
    hexstrings(d);
    uart_send(0x0D);
    uart_send(0x0A);
}

void uart_init(void)
{
    unsigned int ra;

    mmio_write(AUX_ENABLES, 1);
    mmio_write(AUX_MU_IER_REG, 0);
    mmio_write(AUX_MU_CNTL_REG, 0);
    mmio_write(AUX_MU_LCR_REG, 3);
    mmio_write(AUX_MU_MCR_REG, 0);
    mmio_write(AUX_MU_IER_REG, 0);
    mmio_write(AUX_MU_IIR_REG, 0xC6);
    mmio_write(AUX_MU_BAUD_REG, 270);
    ra = mmio_read(GPFSEL1);
    ra &= ~(7 << 12); //gpio14
    ra |= 2 << 12;    //alt5
    ra &= ~(7 << 15); //gpio15
    ra |= 2 << 15;    //alt5
    mmio_write(GPFSEL1, ra);
    mmio_write(GPPUD, 0);
    for (ra = 0; ra < 150; ra++)
        dummy(ra);
    mmio_write(GPPUDCLK0, (1 << 14) | (1 << 15));
    for (ra = 0; ra < 150; ra++)
        dummy(ra);
    mmio_write(GPPUDCLK0, 0);
    mmio_write(AUX_MU_CNTL_REG, 3);
}

void timer_init(void)
{
    //0xF9+1 = 250
    //250MHz/250 = 1MHz
    mmio_write(ARM_TIMER_CTL, 0x00F90000);
    mmio_write(ARM_TIMER_CTL, 0x00F90200);
}

unsigned int timer_tick(void)
{
    return (mmio_read(ARM_TIMER_CNT));
}
