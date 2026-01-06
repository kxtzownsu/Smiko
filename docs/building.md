# Building
In order to build the software inside this repository, first make sure you have all the required dependencies installed.

For Debian Linux and derivatives:
```bash
sudo apt-get install git make gcc g++ protobuf-compiler libtss2-dev libusb-1.0-0-dev libftdi-dev libelf-dev libxml2-dev rapidjson-dev pkg-config gcc-arm-none-eabi gcc-aarch64-linux-gnu
```

For Arch Linux and derivatives:
```bash
sudo pacman -S git make gcc protobuf tpm2-tss tpm2-tools arm-none-eabi-gcc
```

For Alpine Linux and derivatives:
```bash
sudo apk add git make gcc g++ protobuf-dev abseil-cpp protobuf pkgconf openssl-dev libusb-dev tpm2-tss-dev tpm2-tss-sys tpm2-tss-esys gcc-arm-none-eabi
```

For Fedora linux and derivatives:
```bash
sudo dnf install git make gcc gcc-c++ protobuf-devel abseil-cpp tpm2-tss-devel tpm2-tools protobuf-compiler libusb1-devel pkgconfig arm-none-eabi-gcc-cs
```

Then run `make` in the root of the repository. You can specify the architecture with `ARCH=x86_64` or `ARCH=aarch64`. Output binaries can be found in `build/$ARCH`, and built firmware images in `build/firmware`. Note that MacOS and BSD derivatives are not (officially) supported.