# Raspberry Pi 3 & xv6

This repository contains the XMODEM-based boot loader that allows us to send a binary image to execute on Raspberry Pi.

### What you need

#### Hardware

1. Raspberry Pi 3 Model B
2. SD Card
3. USB Power Cable
4. [USB TTL Serial Cable](https://www.amazon.com/JBtek-Raspberry-Micro-Cable-Switch/dp/B00JU24Z3W)

#### Software

1. [Driver for USB TTL Serial cable](http://www.prolific.com.tw/us/ShowProduct.aspx?pcid=41&showlevel=0041-0041)
2. Python 3
3. [GNU Arm Embedded Toolchain](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm)
4. [xv6](https://github.com/zhiyihuang/xv6_rpi2_port)
5. c-kermit

### Bootloader Setup

1. Install required python packages
```
$ pip3 install serial xmodem
```

2. Build the bootloader and the simple uart program
```
$ make
```

3. Download boot files or flash your sd card with Raspbian (and skip to the next step) 
```
$ curl -O https://github.com/raspberrypi/firmware/raw/master/boot/start.elf

$ curl -O https://github.com/raspberrypi/firmware/raw/master/boot/bootcode.bin

# copy to the SD card
$ cp start.elf bootcode.bin /Volumes/boot/
```

5. Copy bootloader.bin to the SD card with the name kernel7.img
```
$ cp bootloader.bin /Volumes/boot/kernel7.img
```

6. Make sure you have config.txt in the SD card with these lines
```
kernel_old=1
disable_commandline_tags=1
enable_uart=1
```

Before we can test it, please connect your serial cable to your RPi to [pin 6, 8 and 10](https://pinout.xyz/pinout/uart) like this
![https://elinux.org/RPi_Serial_Connection](serial-cable.jpg)

Now we can test it with our simple uart program

```bash
$ python3 rpi-install.py /dev/cu.usbserial uart.bin && kermit
```

You should see `Hello, UART` on your screen!