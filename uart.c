extern void mmio_write(unsigned int, unsigned int);
extern unsigned int mmio_read(unsigned int);
extern unsigned int get_pc(void);
extern void delay(unsigned int);
extern void set_led_state(unsigned int);

extern void uart_init(void);
extern void uart_putc(unsigned int);
extern void uart_puts(const char*);
extern void uart_putx(unsigned int);
extern char uart_getc(void);

int kernel_main(void)
{
    uart_init();

    uart_puts("Hello, UART\r\n");
    uart_putx(0x12345678);
    uart_putx(0xABCDEF);
    uart_putx(get_pc());

    while(1) {
        uart_puts(".");
        set_led_state(1);
        delay(5000000);
        set_led_state(0);
        delay(5000000);
    }

    return 0;
}
