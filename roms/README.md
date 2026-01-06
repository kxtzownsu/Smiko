# Google Chip Boot ROMs
The following folder contains BootROM (Boot Read-Only Memory) dumps from various Google chips.

## Basics
The BootROM contains the features necessary to prepare the flash for a firmware image, and verify the image against a hardcoded RSA public key before jumping to its code.

Bootstrapping is done via SPI over a closed-source utility called spiflash. This erases the entire flash memory bank and overwrites it with the provided image. No verification of the written contents is performed before flashing, as it is expected for the bootloader to be able to properly validate written contents. 

The bootloader launches between the newest of 2 write-protected firmware sectors, verifying them with a specified keyset (production or developer), and if the image signature decrypts and the hashes match, the image is launched.

## Mapping
- [Haven/Cr50 BootROM](haven.rom): Mapped to 0x0, 8kB in length
- Citadel/Nuggets BootROM: Mapped to 0x0, 8kB in length (Unobtained at this time)
- Dauntless/Ti50 BootROM: Mapped to 0x0, 32kB in length (Unobtained at this time)
- [OpenTitan/Ti50 BootROM](opentitan.rom): Mapped to 0x8000, 32kB in length