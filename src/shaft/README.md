# Shaft
Usage of Shaft and in turn Rescue and SPI flash/Bootstrap requires SERVO access! 

In order to use Rescue, you'll need to solder directly to the GSC UART RX, GSC UART TX, and GND pins on the unpopulated SERVO or C2D2 header on your motherboard, and attach them to a USB to UART adapter. In order to use SPI Flash/Bootstrap, you'll need to solder directly to the SPI traces on your board (you can find schematics from [WTT](https://files.tree123.org/schematics)) and connect them to a Rasberry PI or a USB -> SPI adapter.

If either of the above are too complicated, expensive, or time consuming to be considered worth it for you, then do NOT use Shaft! Use another exploit! Failure on your own part to heed this warning is not our fault! You have been warned!

## Usage Instructions
In order to use Shaft, connect a TTY (UART or SPI, depending on your preferred transfer mechanism) to your machine, grab a GSC firmware image (some good ones are in the lib/stock folder of this repo), and run the following code:

```bash
sudo shaft --rescue /path/to/gsc-fw.bin /dev/ttyUSB0 # Requires USB -> UART tty, can only program the RW_A image
sudo shaft --bootstrap /path/to/gsc-fw.bin # Requires USB -> SPI adapter, programs the entire FLASH
```
###### Shaft takes a while to program the image via Rescue due to a need to resync the RX buffer, so expect it to take a few minutes.

You can also specify `--certificate` to overwrite the RO_A cert and `--nvmem` to overwrite the NVMEM partition with custom contents.
