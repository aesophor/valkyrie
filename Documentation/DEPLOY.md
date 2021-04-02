## Set up the USB-TO-TTL (USB-TO-SERIAL) Converter

| RPI3 Pin | USB-TO-TTL Pin |
| --- | --- |
| GND | GND |
| UART0 TX | RXD |
| UART0 RX | TXD |

![](https://docs.microsoft.com/en-us/windows/iot-core/media/pinmappingsrpi/rp2_pinout.png)

<br>

## Manually Deploying the Kernel on a Real RPI3

1. Flash a [bootable image](https://github.com/GrassLab/osdi/raw/master/supplement/nctuos.img) to the SD card.
2. Mount the SD card and replace the `kernel8.img` on the SD card with the one we've just built.
3. Eject the SD card and plug it into RPI3.
4. Plug in the USB-TO-TTL converter to your computer
   - for macOS, run `screen /dev/tty.usbserial-0001 115200`
   - for linux, run `screen /dev/ttyUSB0 115200`
