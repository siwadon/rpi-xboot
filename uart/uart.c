extern void mmio_write(unsigned int, unsigned int);
extern unsigned int mmio_read(unsigned int);
extern unsigned int GETPC(void);

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
    uart_putx(GETPC());

    return 0;
}
