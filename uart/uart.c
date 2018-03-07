extern void mmio_write(unsigned int, unsigned int);
extern unsigned int mmio_read(unsigned int);
extern unsigned int GETPC(void);

extern void uart_init(void);
extern void uart_putc(unsigned int);
extern void uart_puts(const char*);
extern unsigned int uart_getc(void);
extern void hexstring(unsigned int);
extern void hexstrings(unsigned int);

int kernel_main(void)
{
    uart_init();
    hexstring(0x12345678);
    hexstring(GETPC());
    uart_puts("Hello, UART\r\n");

    return 0;
}
