
// Uncomment one of these
// #include "BCM2835.h" /* Raspberry A, A+, B, B+ */
#include "BCM2836.h" /* Raspberry Pi 2, 3 */

extern void mmio_write(unsigned int, unsigned int);
extern unsigned int mmio_read(unsigned int);
extern void delay(unsigned int);
/**
 * BCM2835 ARM Peripherals
 * https://www.raspberrypi.org/app/uploads/2012/02/BCM2835-ARM-Peripherals.pdf
 *
 * The underlying architecture in BCM2836 is identical to BCM2835.
 * https://www.raspberrypi.org/documentation/hardware/raspberrypi/bcm2836/README.md
 */

// Timer registers
#define ARM_TIMER_CTL   (P_BASE + 0xB408)   // Control
#define ARM_TIMER_CNT   (P_BASE + 0xB420)   // Free running counter

// GPIO registers
#define GPFSEL0         (GPIO_BASE + 0x00)  // GPIO Function Select 0
#define GPFSEL1         (GPIO_BASE + 0x04)  // GPIO Function Select 1
#define GPFSEL2         (GPIO_BASE + 0x08)  // GPIO Function Select 2
#define GPSET0          (GPIO_BASE + 0x1C)  // GPIO Pin Output Set 1
#define GPCLR0          (GPIO_BASE + 0x28)  // GPIO Pin Output Clear 0
#define GPPUD           (GPIO_BASE + 0x94)  // GPIO Pin Pull-up/down Enable
#define GPPUDCLK0       (GPIO_BASE + 0x98)  // GPIO Pin Pull-up/down Enable Clock 0

#define GPFSEL_PIN_MASK (7U)                // (BIT(2) | BIT(1) | BIT(0))
#define GPFSEL_ALT_4    (3U)                // (BIT(1) | BIT(0))
#define GPFSEL_ALT_5    (2U)                // (BIT(1))
#define TIMEOUT         1000000

// Auxilary Mini UART registers
#define AUX_ENABLES     (UART_BASE + 0x04)  // Auxiliary Enables
#define AUX_MU_IO_REG   (UART_BASE + 0x40)  // Mini UART I/O Data
#define AUX_MU_IER_REG  (UART_BASE + 0x44)  // Mini UART Interrupt Enable
#define AUX_MU_IIR_REG  (UART_BASE + 0x48)  // Mini UART Interrupt Identify
#define AUX_MU_LCR_REG  (UART_BASE + 0x4C)  // Mini UART Line Control
#define AUX_MU_MCR_REG  (UART_BASE + 0x50)  // Mini UART Modem Control
#define AUX_MU_LSR_REG  (UART_BASE + 0x54)  // Mini UART Line Status
#define AUX_MU_MSR_REG  (UART_BASE + 0x58)  // Mini UART Modem Status
#define AUX_MU_SCRATCH  (UART_BASE + 0x5C)  // Mini UART Scratch
#define AUX_MU_CNTL_REG (UART_BASE + 0x60)  // Mini UART Extra Control
#define AUX_MU_STAT_REG (UART_BASE + 0x64)  // Mini UART Extra Status
#define AUX_MU_BAUD_REG (UART_BASE + 0x68)  // Mini UART Baudrate

void uart_init(void)
{
    unsigned int gpfsel1;

    mmio_write(AUX_ENABLES,     1);
    mmio_write(AUX_MU_IER_REG,  0);
    mmio_write(AUX_MU_CNTL_REG, 0);
    mmio_write(AUX_MU_LCR_REG,  3);
    mmio_write(AUX_MU_MCR_REG,  0);
    mmio_write(AUX_MU_IER_REG,  0);
    mmio_write(AUX_MU_IIR_REG,  0xC6);
    mmio_write(AUX_MU_BAUD_REG, 270);

    // GPIO 14 : TXD0 and TXD1
    // GPIO 15 : RXD0 and RXD1
    gpfsel1 = mmio_read(GPFSEL1);
    gpfsel1 &= ~(GPFSEL_PIN_MASK << 12); // GPIO 14
    gpfsel1 |=  (GPFSEL_ALT_5    << 12); // Alt5: TXD
    gpfsel1 &= ~(GPFSEL_PIN_MASK << 15); // GPIO 15
    gpfsel1 |=  (GPFSEL_ALT_5    << 15); // Alt5: RXD

    mmio_write(GPFSEL1, gpfsel1);

    // Disable pull up/down for all GPIO pins and delay for 150 cycles
    mmio_write(GPPUD, 0);
    delay(150);

    // Disable pull up/down for pin 14, 15 and delay for 150 cycles
    mmio_write(GPPUDCLK0, (1 << 14) | (1 << 15));
    delay(150);

    // Write 0 to GPPUDCLK0 to make it take effect.
    mmio_write(GPPUDCLK0, 0);

    // Enable transmit Auto flow-control using CTS
    mmio_write(AUX_MU_CNTL_REG, 3);
}

int is_uart_data_ready()
{
    return mmio_read(AUX_MU_LSR_REG) & 0x01;
}

int is_uart_transmitter_ready()
{
    return mmio_read(AUX_MU_LSR_REG) & 0x20;
}

char uart_getc(void)
{
    // Wait for UART to have received something
    while (!is_uart_data_ready()) { }

    return mmio_read(AUX_MU_IO_REG) & 0xFF;
}

// Output a character
void uart_putc(unsigned int c)
{
    // Wait for UART to become ready to transmit
    while (!is_uart_transmitter_ready()) { }

    mmio_write(AUX_MU_IO_REG, c);
}

// Output a string
void uart_puts(const char *str)
{
    for (int i = 0; str[i] != '\0'; i++)
    {
        uart_putc(str[i]);
    }
}

void uart_put_newline() {
    uart_putc(0x20); // space
    uart_putc(0x0D); // carriage return
    uart_putc(0x0A); // new line
}

// Output a string for a given hexadecimal number
void uart_putx(unsigned int d)
{
    unsigned int rb;
    unsigned int rc;

    rb = 32;
    while (rb != 0)
    {
        rb -= 4;
        rc = (d >> rb) & 0xF;

        if (rc > 9)
            rc += 0x37;
        else
            rc += 0x30;
            
        uart_putc(rc);
    }

    uart_put_newline();
}

// Flush UART
void uart_flush(void)
{
    while (1)
    {
        if ((mmio_read(AUX_MU_LSR_REG) & 0x100) == 0)
            break;
    }
}

void timer_init(void)
{
    // 0xF9 + 1 = 250
    // 250 MHz/250 = 1 MHz
    mmio_write(ARM_TIMER_CTL, 0x00F90000);
    mmio_write(ARM_TIMER_CTL, 0x00F90200);
}

unsigned int timer_tick(void)
{
    return mmio_read(ARM_TIMER_CNT);
}

void jtag_init() {
    unsigned int gpfsel0, gpfsel2;
    unsigned int ra, rb;

    gpfsel0 = mmio_read(GPFSEL0);
    gpfsel0 &= ~(GPFSEL_PIN_MASK << 12); // GPIO 4
    gpfsel0 |=  (GPFSEL_ALT_5    << 12); // Alt5: ARM_TDI
    mmio_write(GPFSEL0, gpfsel0);

    gpfsel2 = mmio_read(GPFSEL2);
    gpfsel2 &= ~(GPFSEL_PIN_MASK <<  6); // GPIO 22
    gpfsel2 |=  (GPFSEL_ALT_4    <<  6); // Alt4: ARM_TRST
    gpfsel2 &= ~(GPFSEL_PIN_MASK <<  9); // GPIO 23
    gpfsel2 |=  (GPFSEL_ALT_4    <<  9); // Alt4: ARM_RTCK
    gpfsel2 &= ~(GPFSEL_PIN_MASK << 12); // GPIO 24
    gpfsel2 |=  (GPFSEL_ALT_4    << 12); // Alt4: ARM_TDO
    gpfsel2 &= ~(GPFSEL_PIN_MASK << 15); // GPIO 25
    gpfsel2 |=  (GPFSEL_ALT_4    << 15); // Alt4: ARM_TCK
    gpfsel2 &= ~(GPFSEL_PIN_MASK << 21); // GPIO 27
    gpfsel2 |=  (GPFSEL_ALT_4    << 21); // Alt4: ARM_TMS
    mmio_write(GPFSEL2, gpfsel2);

    mmio_write(GPPUD, 0);
    delay(150);

    mmio_write(GPPUDCLK0, (1 << 4) | (1 << 22) | (1 << 23) | (1 << 24) | (1 << 25) | (1 << 27));
    delay(150);

    mmio_write(GPPUDCLK0, 0);


    rb=mmio_read(ARM_TIMER_CNT);

    while(1)
    {
        mmio_write(GPSET0, 1 << 16);

        while(1)
        {
            ra = mmio_read(ARM_TIMER_CNT);
            if ((ra-rb) >= TIMEOUT) break;
        }
        rb += TIMEOUT;
        mmio_write(GPCLR0, 1 << 16);

        while(1)
        {
            ra = mmio_read(ARM_TIMER_CNT);
            if ((ra-rb) >= TIMEOUT) break;
        }
        rb += TIMEOUT;
    }
}
