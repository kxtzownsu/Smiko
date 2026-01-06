# GSC Image Flashing
All GSC's have at least 3 universal means by which a firmware image can be written to its flash memory:
- Firmware Updater: Done over SPI (or I2C on some Chromebooks) and USB-C
- Rescue Mode: Done over UART on Chromebooks, SPI on Pixels
- SPI Flash: Done over SPI on all devices

This document aims to describe how to utilize all 3.



## Firmware Updater
This protocol differs from platform to platform. On the GSC's end, it's usually handled by its own application/process. On the host's end, the `smiko` command line utility offers a flash utility to combine all of these protocols into one simple utility interface.

To use smiko to flash a firmware image, simply invoke one of the following:
```bash
sudo smiko --flash /path/to/gsc-fw.bin # Flashes over SPI/I2C
sudo smiko --flash /path/to/gsc-fw.bin --suzyq # Flashes over USB-C
```
Note that USB flashing requires a special "SuzyQ" USB-C cable to interface with the GSC. USB flashing does not work on pixels.



## Rescue Mode
This protocol is pretty straight forward on most platforms. on the GSC's end it's handled purely from the RO firmware, and transfer is done over UART on Chromebooks and SPI on Pixel phones. On the host's end, the `shaft` and `fastboot` command line utilities offer a means to use rescue on virtually any device.

It should be noted that Rescue mode can only program the RW_A region of the firmware, completely wipes the RW_B region, and completely wipes the NVMEM/SFS memory regions (in other words, all TPM2, Strongbox, or Filesystem data saved to your GSC will be wiped).

Shaft Rescue requires a TTY connected to GSC UART (this is NOT the same as the GSC USB Console!!!), and can be performed as follows, assuming said TTY is mapped to /dev/ttyUSB0:
```bash
sudo shaft -f /path/to/gsc-fw.bin -s /dev/ttyUSB0
```

Pixel Rescue requires a SuzyQ connected to the target phone, and can be performed as follows:
```bash
carver -i /path/to/gsc-fw.bin -s rw -k -o gsc-fw.rec
fastboot stage gsc-fw.rec
fastboot oem citadel rescue # For Citadel chips
fastboot oem dauntless rescue # For Dauntless chips
```



## SPI Flash Mode
This protocol is similar to Rescue in terms of packet format, but is very difficult to perform on production devices. It typically requires an MPSSE USB -> UART/SPI adapter or a Rasberry Pi connected to soldered traces on the target's motherboard to access the GSC SPI Slave, as well as a seperate pin known as BOOT_CONFIG or DEV_MODE on most schematics. 

SPI Flash, otherwise known as Bootstrap Mode in some documents, is essentially the more powerful version of Rescue Mode, residing in the BootROM and granting complete control over the device's flash.

Once you have the traces set up, run the following to bootstrap a provided firmware image:
```bash
sudo shaft -f /path/to/gsc-fw.bin --bootstrap # For MPSSE adapters
sudo shaft -f /path/to/gsc-fw.bin --bootstrap -s /dev/spidev* # For Rasberr Pi devices
```