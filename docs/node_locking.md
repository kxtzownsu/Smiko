# Node Locking
Some GSC firmware images like to use a technique called node-locking to build unsafe developer firmware images for production GSC chips. Node locking uses 2 fuses, DEV_ID0 and DEV_ID1, which are 2 completely unique 32-bit values to each GSC in the wild. These fuses are placed in the value of `fuses` during RSA verification of the target image, and thus only a GSC with matching Dev IDs to the target image will be able to run it successfully.

These images are kept confidential, and we've only ever found 1 node-locked image inside a Google Cloud Bucket (Cr50 test production RO 0.0.11, see prebuilts/node_locked_images in our Cr50 fork, thank you Codenerd for extracting this for us!)

# EFI Images
You'll likely see this term thrown around a bit; in Cr50 and Ti50 there are custom images known as EFI images, which are short for EraseFlashInfo images. These images provide access to a console command that allows you to wipe the RW rollback and board INFO spaces on the GSC, in turn allowing you to run PrePVT images and downgrade the RW image via the Rescue utility (or, for non-googlers, our reimplementation in the form of Shaft).

EFI Images _always_ use node-locking, and are kept confidential. Unlike Cr50, On Ti50, you'll see the command `eraseflashinfo` in the console, but this command will never do anything unless the image is EFI. The phrase "Is the RO compiled to allow erasing ..." is misleading, as the RO image copy doesn't change anything here, the only thing that matters is the EFI functionality. On Cr50, this functionality has been removed in its entirety, and no longer exists. On Ti50, the value of CONFIG1 in the SignedHeader controls the EFI state. You can view this configuration value with the Smiko command line utility.

It should also be noted that on Ti50, images with node-locking will not write successfully if the Dev IDs in the header do not match the Dev IDs in the chip's fusing.