# Raspberry Pi 3 & xv6

This repository contains the XMODEM-based boot loader that allows us to send a binary image to execute on Raspberry Pi. The boot loader supports LZMA compressed kernel image to reduce transfer time (in our case, from 60s to 10s).


![](images/xv6.gif)


## What you need

### Hardware

1. Raspberry Pi 3 Model B
2. SD Card
3. USB Power Cable
4. [USB TTL Serial Cable](https://www.amazon.com/JBtek-Raspberry-Micro-Cable-Switch/dp/B00JU24Z3W)

### Software

1. [Driver for USB TTL Serial cable](http://www.prolific.com.tw/us/ShowProduct.aspx?pcid=41&showlevel=0041-0041)
2. Python 3
3. [GNU Arm Embedded Toolchain](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm)
4. [xv6](https://github.com/idewz/xv6_rpi2_port)
5. c-kermit


## Boot Loader Setup

1. Clone this repository with its submodules
```bash
git clone --recurse-submodules git@github.com:idewz/rpi3.git

# if you have already cloned
git submodule update --init
```

2. Install required python packages
```bash
pip3 install pyserial xmodem
```

3. Copy necessary boot files `bootcode.bin`, `start.elf`, `fixup.dat` and `config.txt` to your SD card ([info](https://elinux.org/RPi_Software))
```bash
cp firmware/*.* /Volumes/boot

cp bootcode.bin start.elf fixup.dat /Volumes/boot/
```

4. Build the boot loader and the simple uart program
```bash
make
```

5. Copy `bootloader.img` to the SD card with the name `kernel7.img`
```bash
cp bootloader.img /Volumes/boot/kernel7.img
```

6. Connect your serial cable to [pin 6, 8 and 10](https://pinout.xyz/pinout/uart) of your RPi like [this](https://elinux.org/File:Adafruit-connection.jpg)
![](images/serial-cable.jpg)

7. Now we can test it with our simple uart program.
```bash
python3 rpi-install.py /dev/cu.usbserial uart.img && kermit
```

Then you should see `Hello, UART` on your screen!


## xv6

To run [xv6](https://github.com/idewz/xv6_rpi2_port), you just need to update the location of the TOOLCHAIN in the [Makefile](https://github.com/idewz/xv6_rpi2_port/blob/master/Makefile#L6) and run

```bash
cd xv6
make
python3 ../rpi-install.py /dev/cu.usbserial kernel7.img && kermit
```
