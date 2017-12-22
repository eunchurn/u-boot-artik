
# u-boot-artik for ARTIK05x

[![license](https://img.shields.io/github/license/SamsungARTIK/u-boot-artik.svg)](./Licenses/gpl-2.0.txt)

This project is used for ARTIK05x (ARTIK053, ARTIK053S, ARTIK055S).

## How to Build

This project uses the `artik-05x` branch.

#### Clone git sources
```bash
git clone --depth=50 --branch=artik-05x https://github.com/SamsungARTIK/u-boot-artik.git
```

#### Pre-Install
```bash
sudo apt-get install device-tree-compiler
```

### For Standard Module (ARTIK053)

#### Compile
```bash
make artik05x_defconfig
make CROSS_COMPILE=arm-none-eabi-
```

#### Attach Header
```bash
./tools/attachns2.py ./u-boot.bin ./u-boot.head.bin
```

#### Copy to TizenRT
```bash
cp u-boot.head.bin ~/TizenRT/build/configs/artik053/bin/bl2.bin
```

### For Secure Module (ARTIK053S / ARTIK055S)

Verify OS(TizenRT) image with Customer's Pub.key and then jump to OS entry point.
```
                bl2.bin                                tinyara_head.bin
       +------------------------+                 +------------------------+
       |     U-boot binary      |                 |                        |
       +------------------------+                 |                        |
       |     Secure Context     |                 |                        |
       | +--------------------+ |                 |      OS (TizenRT)      |
       | | Customer's Pub.key | |                 |                        |
       | +--------------------+ |                 |                        |
       | +--------------------+ |                 |                        |
       | |     Signature      | |                 +------------------------+
       | +--------------------+ |                 |       Signature        |
       +------------------------+                 +------------------------+
```

#### Compile
```bash
make artik05x_defconfig
make CROSS_COMPILE=arm-none-eabi-
```

#### Attach Header & Add Key
```bash
./tools/attachns2-s.py ./u-boot.bin ./u-boot.head.bin ./rsa_public.key
```

#### Signing
> Please download those files from [**artik.io**](https://developer.artik.io/downloads/artik-053s/download) with SLA agreement to continue to sign.
> Once you download those files, please locate them to the root path.

```
./artik05x_codesigner -sign u-boot.head.bin
```

#### Copy to TizenRT
```bash
cp u-boot.head.bin-signed ~/TizenRT/build/configs/artik053s/bin/bl2.bin
cp u-boot.head.bin-signed ~/TizenRT/build/configs/artik055s/bin/bl2.bin
```

## How to Flashing

After moving to the [TizenRT](https://github.com/SamsungARTIK/TizenRT) project, you can download the u-boot binary to ARTIK05x with the following command:
```bash
$ cd os
$ make download bl2
```
