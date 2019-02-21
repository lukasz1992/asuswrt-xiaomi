CUR_DIR = $(shell pwd)
KERNELZ = $(ROOTDIR)/$(LINUXDIR)/arch/arm/boot/zImage
KERNEL_IMG = $(IMAGEDIR)/kernel_$(TARGET_PRODUCT).bin
MKBOOTFS = $(CUR_DIR)/mkbootfs
MKIMAGE = $(CUR_DIR)/mkimage
MKIMAGE_FLASH = $(CUR_DIR)/flashtool/mkimage
MKIMAGE_DIR = $(ROOTDIR)/user/mkimage
MKBOOTIMG = $(CUR_DIR)/mkbootimg
MINIGZIP = $(CUR_DIR)/minigzip
IMAGE = $(IMAGEDIR)/$(USER)_uImage
IMAGE_FLASH = $(IMAGEDIR)/$(USER)_uImage.bin
COMP = lzma
#LIBGCC = $(shell $(CROSS_COMPILE)gcc -print-file-name=libgcc_s.so.1)

ROMFS_DIRS = \
	tmp \
	bin \
	dev dev/pts \
	home \
	etc \
	etc_ro etc_ro/web etc_ro/ppp etc_ro/ppp/peers etc_ro/ppp/plugins etc_ro/ppp/3g etc_ro/wmt\
	etc_ro/wlan etc_ro/Wireless etc_ro/xml etc_ro/usb \
	/etc_ro/lighttpd \
	etc_ro/linuxigd etc_ro/Wireless/RT2860AP etc_ro/Wireless/RT61AP etc_ro/Wireless/iNIC \
	etc_ro/Wireless/WIFI3 \
	lib lib/modules lib/ipsec \
	mnt \
	proc \
	sbin \
	sys \
	media \
	usr usr/bin usr/sbin usr/codepages \
	var 

clean:
	

romfs: 
	[ -d $(ROMFSDIR)/$$i ] || mkdir -p $(ROMFSDIR)
	for i in $(ROMFS_DIRS); do \
		[ -d $(ROMFSDIR)/$$i ] || mkdir -p $(ROMFSDIR)/$$i; \
	done


	$(ROMFSINST) /etc_ro/rcS
	$(ROMFSINST) -e CONFIG_MT7623_FPGA inittab.fpga /etc_ro/inittab
	$(ROMFSINST) -e CONFIG_MT7623_EVB_BOARD inittab.asic /etc_ro/inittab
	$(ROMFSINST) /etc_ro/motd
	$(ROMFSINST) /etc/fstab

	$(ROMFSINST) /etc_ro/Wireless/RT2860AP/RT2860_default_novlan
	$(ROMFSINST) /etc_ro/Wireless/RT2860AP/RT2860_default_vlan
	$(ROMFSINST) /etc_ro/Wireless/iNIC/RT2860AP.dat
	$(ROMFSINST) /etc_ro/Wireless/WIFI3/RT2870AP.dat
	$(ROMFSINST) /etc_ro/Wireless/RT2860AP/7615_SingleSKU.dat
	$(ROMFSINST) /etc_ro/Wireless/RT2860AP/7615_SingleSKU_BF.dat
	
	$(ROMFSINST) /etc_ro/wlan/MT7615E_EEPROM.bin

romfs.post:: 
	[ -d $(IMAGEDIR) ] || mkdir -p $(IMAGEDIR)
#cp $(LIBGCC) $(ROOTDIR)/romfs/lib
	
mkimage: $(MKIMAGE_DIR)/crc32.c $(MKIMAGE_DIR)/mkimage.c
	 $(HOSTCC) $(HOST_CFLAGS) -DUSE_HOSTCC -o $@ -I$(MKIMAGE_DIR)/include $^

image: mkimage
	#$(MKBOOTFS) $(ROOTDIR)/romfs | $(MINIGZIP) > $(IMAGEDIR)/ramdisk.gzip
	#$(MKIMAGE) $(IMAGEDIR)/ramdisk.gzip ROOTFS > $(IMAGEDIR)/ramdisk.img

	$(OBJCOPY) -S -O binary -R .note -R .comment $(ROOTDIR)/$(LINUXDIR)/vmlinux $(TFTPDIR)/$(USER)_vmlinux.bin 
	cp $(ROOTDIR)/$(LINUXDIR)/vmlinux $(TFTPDIR)/$(USER)_vmlinux

	#uImage for flashtool
	touch $(IMAGEDIR)/fake_ramdisk
	$(MKIMAGE_FLASH) $(IMAGEDIR)/fake_ramdisk ROOTFS > $(IMAGEDIR)/fake_ramdisk.img
	$(MKIMAGE_FLASH) $(KERNELZ) KERNEL > $(KERNEL_IMG)
	$(MKBOOTIMG) --kernel $(KERNEL_IMG) --ramdisk $(IMAGEDIR)/fake_ramdisk.img --board 1384501662 --output $(IMAGE_FLASH)
	cp $(IMAGE_FLASH) $(TFTPDIR)
	
	#uImage for uBoot
	#$(CROSS_COMPILE)objcopy -O binary -R .note -R .comment -S $(ROOTDIR)/$(LINUXDIR)/vmlinux  $(KERNELZ)
	#cd $(IMAGEDIR) ; rm -f $(KERNELZ).*; $(COMP) -9 -f -S .$(COMP) $(KERNELZ)
	#$(CUR_DIR)/mkimage -A arm -O linux -T kernel -C $(COMP) -a 80008000 -e $(shell readelf -h $(ROOTDIR)/$(LINUXDIR)/vmlinux | grep "Entry" | awk '{print $$4}') -n "Linux Kernel Image"  -d $(KERNELZ).$(COMP) $(IMAGE)
	#$(CUR_DIR)/mkimage -A arm -O linux -T kernel -C $(COMP) -a 80008000 -e 80008000 -n "Linux Kernel Image"  -d $(KERNELZ).$(COMP) $(IMAGE)
	$(CUR_DIR)/mkimage -A arm -O linux -T kernel -C none -a 80008000 -e 80008000 -n "Linux Kernel Image"  -d $(KERNELZ) $(IMAGE)
	cp $(IMAGE) $(TFTPDIR)
