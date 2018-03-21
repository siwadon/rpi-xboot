ARMGNU ?= arm-none-eabi

COPS = -Wall -O2 -nostdlib -nostartfiles -ffreestanding 

gcc : uart.bin bootloader.bin

all : gcc clang

clean :
	rm -f *.o
	rm -f *.bin
	rm -f *.hex
	rm -f *.elf
	rm -f *.list
	rm -f *.img
	rm -f *.bc
	rm -f *.clang.s

start_uart.o : start_uart.s
	$(ARMGNU)-as start_uart.s -o start_uart.o

uart.o : uart.c
	$(ARMGNU)-gcc $(COPS) -c uart.c -o uart.o

periph.o : periph.c
	$(ARMGNU)-gcc $(COPS) -c periph.c -o periph.o

uart.bin : uart.ld start_uart.o periph.o uart.o 
	$(ARMGNU)-ld start_uart.o periph.o uart.o -T uart.ld -o uart.elf
	$(ARMGNU)-objcopy uart.elf -O binary uart.bin

start_bootloader.o : start_bootloader.s
	$(ARMGNU)-as start_bootloader.s -o start_bootloader.o

bootloader.o : bootloader.c
	$(ARMGNU)-gcc $(COPS) -c bootloader.c -o bootloader.o

bootloader.bin : bootloader.ld start_bootloader.o periph.o bootloader.o
	$(ARMGNU)-ld start_bootloader.o periph.o bootloader.o -T bootloader.ld -o bootloader.elf
	$(ARMGNU)-objcopy bootloader.elf -O binary bootloader.bin


LOPS = -Wall -m32 -emit-llvm
LLCOPS0 = -march=arm 
LLCOPS1 = -march=arm -mcpu=arm1176jzf-s
LLCOPS = $(LLCOPS1)
COPS = -Wall  -O2 -nostdlib -nostartfiles -ffreestanding
OOPS = -std-compile-opts

clang : uart.clang.bin

uart.bc : uart.c
	clang $(LOPS) -c uart.c -o uart.bc

periph.bc : periph.c
	clang $(LOPS) -c periph.c -o periph.bc

uart.clang.elf : uart.ld start_uart.o uart.bc periph.bc
	llvm-link periph.bc uart.bc -o uart.nopt.bc
	opt $(OOPS) uart.nopt.bc -o uart.opt.bc
	llc $(LLCOPS) uart.opt.bc -o uart.clang.s
	$(ARMGNU)-as uart.clang.s -o uart.clang.o
	$(ARMGNU)-ld -o uart.clang.elf -T uart.ld start_uart.o uart.clang.o
	$(ARMGNU)-objdump -D uart.clang.elf > uart.clang.list

uart.clang.bin : uart.clang.elf
	$(ARMGNU)-objcopy uart.clang.elf uart.clang.hex -O ihex
	$(ARMGNU)-objcopy uart.clang.elf uart.clang.bin -O binary


