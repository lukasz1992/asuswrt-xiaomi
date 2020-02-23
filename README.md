# AsusWRT R3G
This is modified version of AsusWrt that works with Xiaomi Mi R3G router - currently based on RT-AC85U GPL sources.
Only version 1 (with USB 3.0 port) is supported, currently firmware will not work on any other device.
I could port this software to other devices (but you have to provide hardware to me).

## How to install
1. Download image from Releases or compile from source
2. Flash it to the router from Breed bootloader or AsusWRT - this methods were tested.
3. In any other cases, if you flash firmware manually - TRX image needs to be placed at 0x200000 offset

## How to build
1. cd release/src-ra-5010
2. make rt-mir3g

## Important notes
- First boot after takes some minutes, don't interrupt it
- I do not take responsibility of any damages - you do everything on your own risk
- I can fix some issues, but I can not provide official support

## Important differences between Xiaomi and Asus
- Asus Dual Trx implementation differs from Xiaomi flash layout, so dual image functionality was completly rewritten
- WPS funcionality was removed from image, due to lack of hardware WPS button
- Asus will not accept subsequent MAC addresses for 2G and 5G WiFi to allow Guest WiFi functionality, the check was manually disabled - but this makes Guest WiFi functionality experimental
- I do not know how to test TxBF, but AFAIK this needs additional calibration data in flash (empty in 3G router)

## Missing features (not included in GPL)
- No repeater support
- No dual-wan support
