
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
 * BCM2836 is identical to BCM2835
 * https://www.raspberrypi.org/documentation/hardware/raspberrypi/bcm2836/README.md
 */

// Timer registers
#define ARM_TIMER_CTL   (P_BASE + 0xB408)   // Control
#define ARM_TIMER_CNT   (P_BASE + 0xB420)   // Free running counter

// GPIO registers
#define GPFSEL1         (GPIO_BASE + 0x04)  // GPIO Function Select 1
#define GPSET0          (GPIO_BASE + 0x1C)  // GPIO Pin Output Set 1
#define GPCLR0          (GPIO_BASE + 0x28)  // GPIO Pin Output Clear 0
#define GPPUD           (GPIO_BASE + 0x94)  // GPIO Pin Pull-up/down Enable
#define GPPUDCLK0       (GPIO_BASE + 0x98)  // GPIO Pin Pull-up/down Enable Clock 0

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
    unsigned int ra;

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
    ra = mmio_read(GPFSEL1);
    ra &= ~(7 << 12); // GPIO 14
    ra |= 2 << 12;    // ALT 5
    ra &= ~(7 << 15); // GPIO 15
    ra |= 2 << 15;    // ALT 5

    mmio_write(GPFSEL1, ra);
    mmio_write(GPPUD, 0);
    delay(150);

    mmio_write(GPPUDCLK0, (1 << 14) | (1 << 15));
    delay(150);

    mmio_write(GPPUDCLK0, 0);
    mmio_write(AUX_MU_CNTL_REG, 3);
}

int is_uart_data_ready() {
    return mmio_read(AUX_MU_LSR_REG) & 0x01;
}

int is_uart_transmitter_ready() {
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
void uart_puts(const char* str)
{
    for (int i = 0; str[i] != '\0'; i++) {
        uart_putc(str[i]);
    }
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

    uart_putc(0x20); // space
    uart_putc(0x0D); // carriage return
    uart_putc(0x0A); // new line
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
