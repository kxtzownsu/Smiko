# Custom Firmware
Running Custom Firmware on an H1 GSC is not as simple as it might seem at first glance. Assuming you have a way to solder to your device and actually flash the firmware, you also have to trick the chip into actually running it. This guide will cover how to do that.

### Be warned!!!!
All existing methods of Custom Firmware are _tethered_! This means you'll have to redo whatever method you choose each time the GSC resets (for BootCon, you only need to install the firmware once, but the console part you must redo each reset)!


## BootCon Method
BootCon works by flashing our custom firmware image to the RW_A section of firmware, a non-booting image to RO_A, and a dev-signed RO 0.0.9 image to the RO_B which allows us to run the ancient 0.0.6 image which we flash to RW_B, and doesnt lower the GLOBALSEC execution level, allowing our CFW more control over the chip. It uses the SPIFlash utility in the CR50's bootrom to bypass both downgrade protections and restrictions against writing to the active RO image which are present in both the typical updater and the rescue utility. It then requires you to use the outdated version's console to write arbitrary data to GLOBALSEC registers, unlocking the custom firmware's execution and then requires you to jump to that custom firmware, where we're able to take full control.

First thing's first, you'll want to build the firmware and injector. You can do this by running `make shaft smiko-cfw` in the root of this repository (make sure you clone with submodules ON). Install these utilities to your Linux system with `sudo make install`, if you ever wish to remove them, binaries are placed in /usr/local/bin, and firmware in /usr/share/smiko/firmware.

In order to flash the custom firmware we just built, you will need to solder to the CR50's (1.8v) SPI lines (and, if you dont have a suzyq, the UART lines. Instructions for both coming.. eventually). You will also need to solder to and hold the H1_BOOT_CONFIG (aka dev_mode) line high to 3.3v. Once you have all of your connections soldered and connected to your spi interface, and ensuring it's set to 1.8v, you will need to run `sudo shaft -f build/firmware/smiko-bootcon-cfw.bin --bootstrap`. If you're flashing/building on a Rasberry Pi, make sure to pass `-s /dev/spidev#.#` to the correct SPI device.
In order to flash the custom firmware we just built, you will need to solder to the CR50's (1.8v) SPI lines (and, if you dont have a suzyq, the UART lines. Instructions for both coming.. eventually). You will also need to solder to and hold the H1_BOOT_CONFIG (aka dev_mode) line high to 3.3v. Once you have all of your connections soldered and connected to your spi interface, and ensuring it's set to 1.8v, you will need to run `sudo shaft -f build/firmware/smiko-bootcon-cfw.bin --bootstrap`. If you're flashing/building on a Rasberry Pi, make sure to pass `-s /dev/spidev#.#` to the correct SPI device.

Once the firmware is installed, now open a CCD console on the target. If you soldered to get UART, you'll want to screen that console, but any old CCD will do. On the target, we first have to prepare some registers first. Cr50 0.0.6 has a much looser console we can use, so we'll use it to set up the chip. You can use `sudo shaft --socket /dev/ttyUSB0 --bootcon` where /dev/ttyUSB0 points to the CCD console to automatically modify the registers and print out the final address, or you can do it manually in the ccd console with the following:

You'll likely want to copy and paste commands from this document. Run each of the following commands, in order, on the console:

```
rw 0x40090140 0x10000
rw 0x40090144 0x20
rw 0x40090008 0x7
rw 0x4009028c 0x40000
rw 0x40090290 0x80000
rw 0x40090288 0x3
rw 0x40091004 0xe303ec7a
rw 0x40091008 0x68a03a27
rw 0x4009100c 0xdd18053e
rw 0x40091010 0x39f8dbbd
rw 0x40091014 0x9b553578
rw 0x40091018 0xb4598244
rw 0x4009101c 0xc59f62d1
rw 0x40091020 0x61b8509e
rw 0x40091024 0x0

rw 0x44404
```

Regardless of which you chose to use, this will unlock the GLOBALSEC execution on REGION2, and the last command will output an address. Copy it, and finally, run the following, with the printed address in place of "[addr]":
```
sysjump [addr]
```
This will complete the jump to Smiko CFW, which is all the code in HavenOverflow/Cr50 in chip/smiko. You can modify this code however you'd like, and the smiko builder will pack it all together for you. (tip: You can also test all of this in gscemulator without running the initial `rw` commands)



## Extortion Method (Not recommended, requires RO < 0.0.11)

The Extortion method is higher matinence than the BootCon method, and has a much higher brick risk, but it only requires access the the UART TX line and for your device to have a running RO version below 0.0.11. If your version is higher than that, you should just use the BootCon method, as you need SPIFlash either way in order to downgrade RO. 

To get started, build the required utilities and firmware with `make shaft smiko-cfw smiko` and install them with `sudo make install`. Next get an rma shim and use `sudo ./patcher.sh -i shim.bin` in the root of the repo (see [The Sh1mmer Website](https://sh1mmer.me) for info on how to flash USB drives if you don't know how).

Solder to your devices UART TX line for the CR50, and ensure it's mapped somewhere under /dev/ttyUSB# (Note: SuzyQ does not work for this due to not being active yet). Run `sudo shaft -f build/firmware/smiko-bootcon-cfw.bin --rescue --extortion`, and it will automatically flash and run the CFW image for you.

In order to prevent a brick, after the cfw image has been flashed and is running, you will need to launch your RMA shim image and run the following commands: `smiko --flash /usr/share/smiko/firmware/smiko-update-cfw.bin --section RW_B`, pop open the CCD console and run `jump RW_B`, and then `smiko --flash /opt/google/cr50/cr50-0-5-9.bin.prod --section RW_A` back on the RMA shim.
