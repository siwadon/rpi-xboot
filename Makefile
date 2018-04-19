ARMGNU ?= arm-none-eabi

COPS = -Wall -O2 -nostartfiles -ffreestanding

all : uart bootloader

clean :
	rm -f *.o
	rm -f *.bin
	rm -f *.hex
	rm -f *.elf
	rm -f *.list
	rm -f *.img
	rm -f *.bc
	rm -f *.clang.s


periph.o : periph.c
	$(ARMGNU)-gcc $(COPS) -c periph.c

xmodem.o : xmodem.c
	$(ARMGNU)-gcc $(COPS) -c xmodem.c


# UART
start_uart.o : start_uart.s
	$(ARMGNU)-as start_uart.s -o start_uart.o

uart.o : uart.c
	$(ARMGNU)-gcc $(COPS) -c uart.c

uart : uart.ld start_uart.o periph.o uart.o
	$(ARMGNU)-ld start_uart.o periph.o uart.o -T uart.ld -o uart.elf
	$(ARMGNU)-objdump uart.elf -D > uart.list
	$(ARMGNU)-objcopy uart.elf -O binary uart.img


# LZMA
lzma.o : lzma.c
	$(ARMGNU)-gcc $(COPS) -c lzma.c

7zFile.o: lib/lzma/7zFile.c
	$(ARMGNU)-gcc $(COPS) -c lib/lzma/7zFile.c

7zStream.o : lib/lzma/7zStream.c
	$(ARMGNU)-gcc $(COPS) -c lib/lzma/7zStream.c

Alloc.o : lib/lzma/Alloc.c
	$(ARMGNU)-gcc $(COPS) -c lib/lzma/Alloc.c

LzmaDec.o : lib/lzma/LzmaDec.c
	$(ARMGNU)-gcc $(COPS) -c lib/lzma/LzmaDec.c


# Boot loader
OBJS = \
	start_bootloader.o \
	periph.o \
	Alloc.o \
	LzmaDec.o \
	7zFile.o \
	7zStream.o \
	lzma.o \
	xmodem.o \
	bootloader.o \

start_bootloader.o : start_bootloader.s
	$(ARMGNU)-as start_bootloader.s -o start_bootloader.o

bootloader.o : bootloader.c
	$(ARMGNU)-gcc $(COPS) -c bootloader.c

bootloader : bootloader.ld $(OBJS)
	$(ARMGNU)-gcc $(COPS) -specs=nosys.specs -T bootloader.ld -o bootloader.elf $(OBJS)
	$(ARMGNU)-objdump bootloader.elf -D > bootloader.list
	$(ARMGNU)-objcopy bootloader.elf -O binary bootloader.img
