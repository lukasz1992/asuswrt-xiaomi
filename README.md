# AsusWRT Xiaomi
This is version of AsusWRT that works with Xiaomi Mi routers, based on MT7621 CPU.
Currently R3G v1 is supported, tester is needed for R3Gv2 (or R4A Gigabit version) models.

## How to install
1. Download image from Releases page or build it from source
2. Flash it to a router from stock firmware or bootloader

## Installation from stock firmware
Installation process is similar to OpenWRT
- NAND flash - image needs to be split into two parts: first 4MB and the rest - first part needs to be written to kernel1 partition, the rest to rootfs0. nvram variable flag_try_sys1_failed needs to be to 1, kernel0 partition should be erased
- NOR flash - image needs to be written to OS1 partition

## Installation from bootloader
- SPI flash - image needs to be written at 0x180000 offset
- NAND flash - image needs to be written at 0x600000 offset

## How to build image from source
1. cd release/src-ra-5010
2. make model (currently available models are: rt-mir3g, rt-mir4a)

## Missing features
- No dual-wan support

## Important note
- I do not take responsibility for any damages - you do everything on your own risk
