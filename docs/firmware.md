

## Obtaining GSC Firmware
Obtaining firmware for the Google Security Chip is not as straight-forward as it may seem. This document aims to make it as simple as possible.

### HOv Prebuilts
You can download copies of all the firmware utilitized by HavenOverflow in their respective repositories. 

Cr50 Prebuilts (for Production, Pre-Production, Developer, and CFW images) can be found [here](https://github.com/HavenOverflow/Cr50/tree/main/prebuilts).

Ti50 Prebuilts (for Production, Pre-Production, and Developer images for both Dauntless and OpenTitan) can be found [here](https://github.com/HavenOverflow/Ti50/tree/main/prebuilts).

Nugget-OS Prebuilts (for Citadel and Dauntless, images of all types) can be found [here](https://github.com/HavenOverflow/nugget-os/prebuilts). (please note that Haven is not officially supported by Nugget-OS)

### Google Prebuilts
Google stores some prebuilt images on their servers, which can be obtained using the cloud bucket API. If you don't have a ChromiumOS build chroot already, [follow the official guide](https://chromium.googlesource.com/chromiumos/docs/+/master/developer_guide.md) until you've entered your chroot. It will take up a large amount of space, so be prepared for that.

From within the chroot, you can view the cloud bucket storing GSC images with the `gsutil` command, e.g. `gsutil ls gs://chromeos-localmirror/distfiles/ | grep cr50` (replace cr50 with ti50 if you're looking for ti50 images, of course). Using gsutil, you can copy firmware files out of the bucket and decompress them, and use Smiko's utilities to interact with them. Note that Nugget-OS images cannot be obtained with this method.



## Manipulating GSC Firmware
You're more than likely going to want to dissect or analyze firmware images you obtain, regardless of where it comes from. Smiko offers a suite of utilities to do that.

To install Smiko's utilities, open the root of the repository, and run `make utils && sudo make install`. This will place all built binaries in `/usr/local/bin`, in case you wish to remove them for any reason. If you want to patch ChromeOS images to have Smiko's toolchain, add the environment variable `STATIC=1` to avoid libc linking issues.

To view the basic header information of a target GSC image, run `smiko -H image.bin`. This will give you information like the SoC the image was built for, the version of the firmware image, the date it was signed, the ID (montgomery n0inv) of the key used to sign the image, and more.

To view system information for the GSC running on the target device, run `sudo smiko -i`. This will output a neofetch-like display of the connected Google Security Chip's diagnostics, e.g. version information, Board ID, running image Key IDs, the running GSC SoC type, and more. If you wish to communicate to the GSC over SuzyQ (does not work for Pixels), you can also add `--suzyq` to your command line arguments. If you're running on a ChromeOS image, you'll also need to pass `--trunks` to your command line arguments for Smiko to communicate with the chip properly.

To make a .rec file for a target GSC image (which is used by fastboot in the GSC rescue protocol on Google Pixel devices), run `shaft -f image.bin -o image.rec`. This will generate image.rec in the current directory, and you can send it to a target Pixel device by putting it in fastboot and running 
```bash
fastboot oem citadel stage image.rec
fastboot oem citadel rescue
```
If the Pixel device is a 6 or newer, replace `fastboot oem citadel` with `fastboot oem gsc`. Do note that this will wipe all user data on the device.