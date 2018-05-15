extern void jtag_init(void);
extern void uart_init(void);
extern void uart_puts(const char *);
extern void uart_putx(unsigned int);

int kernel_main(void)
{
    uart_init();
    uart_puts("Hello from jlink\r\n");

    jtag_init();

    return 0;
}
