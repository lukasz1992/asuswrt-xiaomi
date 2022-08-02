# AsusWRT Xiaomi
This was AsusWRT fork to Xiaomi Mi routers, based on MT7621 CPU.
Due to lack time and hardware I no longer develop this project on this SDK.
I plan to continue support for WiFi6 routers when Asus releases newer source code (386 branch).

## Supported devices
None

## Previously supported devices
- Xiaomi MI R3G
- Xiaomi MI R3P
- Xiaomi Redmi AC2100
- Xiaomi AC2100

## How to install
1. Download image from Releases page or build it from source
2. Flash it to a router from stock firmware or bootloader

## Installation from stock firmware
Installation process is similar to OpenWRT
- NAND flash - image needs to be split into two parts: first 4MB and the rest - first part needs to be written to kernel1 partition, the rest to rootfs0. nvram variable flag_try_sys1_failed needs to be to 1, kernel0 partition should be erased

## Installation from bootloader
- NAND flash - image needs to be written at 0x600000 offset

## How to build image from source
1. cd release/src-ra-5010
2. make model (currently available models are: rt-mir3g, rt-mir3p, rt-rm2100, rt-r2100)
