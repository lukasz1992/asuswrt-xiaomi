KERNELZ = $(IMAGEDIR)/zImage
RAMDISK  = $(IMAGEDIR)/ramdisk
IMAGE = $(IMAGEDIR)/$(USER)_uImage
ROMFS_DIR = $(ROOTDIR)/romfs

RAMDISK_SIZE  := $(shell echo $(CONFIG_BLK_DEV_RAM_SIZE))
MKIMAGE_DIR = $(ROOTDIR)/user/mkimage
CUR_DIR = $(shell pwd)
COMP = lzma

ROMFS_DIRS = \
	tmp \
	bin \
	dev dev/pts \
	home \
	etc \
	etc_ro etc_ro/web etc_ro/ppp etc_ro/ppp/peers etc_ro/ppp/plugins etc_ro/ppp/3g \
	etc_ro/wlan etc_ro/Wireless etc_ro/xml etc_ro/usb \
	etc_ro/linuxigd etc_ro/Wireless/RT2860AP etc_ro/Wireless/RT61AP etc_ro/Wireless/iNIC \
	/etc_ro/lighttpd \
	lib lib/modules lib/ipsec \
	mnt \
	proc \
	sbin \
	sys \
	media \
	usr usr/bin usr/sbin usr/codepages \
	var 

clean:
	rm -f mkimage

romfs: 
	[ -d $(ROMFSDIR)/$$i ] || mkdir -p $(ROMFSDIR)
	for i in $(ROMFS_DIRS); do \
		[ -d $(ROMFSDIR)/$$i ] || mkdir -p $(ROMFSDIR)/$$i; \
	done
	#generate device node
#ifneq	($(LINUXDIR),linux-2.4.x)
#	@if [ \! -d $(ROMFSDIR)/dev ];then mkdir -p $(ROMFSDIR)/dev;fi;
#	./makedevlinks $(ROMFSDIR)
#endif
ifeq	($(CONFIG_DEFSETTING_2M8M),y)
	ln -sf rcS.kn rcS
else
	ln -sf rcS.an rcS
endif
	$(ROMFSINST) /etc_ro/rcS
	$(ROMFSINST) /etc_ro/inittab
	$(ROMFSINST) /etc_ro/motd
	$(ROMFSINST) /etc/fstab

	# Handle several power design for MT7628
	rm -rf MT7628_AP_2T2R-4L_V15.BIN
ifeq	($(CONFIG_INTERNAL_PA_INTERNAL_LNA),y)
	# user choose iPA + iLNA
	ln -s "MT7628_AP_2T2R-4L_internal LNA_internal PA_V15.BIN" MT7628_AP_2T2R-4L_V15.BIN
else ifeq ($(CONFIG_INTERNAL_PA_EXTERNAL_LNA),y)
	# user choose iPA + eLNA
	ln -s "MT7628_AP_2T2R-4L_external LNA_internal PA_V15.BIN" MT7628_AP_2T2R-4L_V15.BIN
else ifeq ($(CONFIG_EXTERNAL_PA_EXTERNAL_LNA),y)
	# user choose ePA + eLNA
	ln -s "MT7628_AP_2T2R-4L_external LNA_external PA_V15.BIN" MT7628_AP_2T2R-4L_V15.BIN
else
	# no any config?! use iLAN+iPA as default.
	ln -s "MT7628_AP_2T2R-4L_internal LNA_internal PA_V15.BIN" MT7628_AP_2T2R-4L_V15.BIN
endif
ifeq	($(CONFIG_DEFSETTING_2M8M),y)
	ln -sf MT7628K_v2_0.bin MT7628_EEPROM_20140317.bin
else
	ln -sf MT7628A_v2_0.bin MT7628_EEPROM_20140317.bin
endif
	$(ROMFSINST) "/etc_ro/wlan/MT7628_AP_2T2R-4L_V15.BIN"
	$(ROMFSINST) "/etc_ro/wlan/MT7628_EEPROM_20140317.bin"

	ln -sf MT7612_V13_iPAcoclk_power_cal.bin MT7612E_EEPROM.bin
	$(ROMFSINST) "/etc_ro/wlan/MT7612E_EEPROM.bin"

	$(ROMFSINST) /etc_ro/Wireless/RT2860AP/RT2860_default_novlan
	$(ROMFSINST) /etc_ro/Wireless/RT2860AP/RT2860_default_vlan
	$(ROMFSINST) /etc_ro/Wireless/iNIC/RT2860AP.dat
	$(ROMFSINST) -e CONFIG_EXTEND_NVRAM /etc_ro/tr069_voip_default_settings

	$(ROMFSINST) -e CONFIG_RT5592_AP /etc_ro/Wireless/iNIC/RT5592_PCIe_2T2R_V1_7.bin

romfs.post:: 
	[ -d $(IMAGEDIR) ] || mkdir -p $(IMAGEDIR)

	#Squash File System
	rm -f $(RAMDISK)  
ifeq ($(LINUXDIR),linux-2.4.x)
	$(CONFIG_CROSS_COMPILER_PATH)/mksquashfs_lzma-3.0 $(ROMFSDIR) $(RAMDISK)
else
	#linux2.6.x support squashfs with lzma
ifeq	($(CONFIG_BLK_DEV_INITRD),)

ifeq	($(LINUXDIR),linux-2.6.21.x)
	$(CONFIG_CROSS_COMPILER_PATH)/mksquashfs_lzma-3.2 $(ROMFSDIR) $(RAMDISK)
endif
ifeq	($(LINUXDIR),linux-2.6.36MT.x)
	$(CONFIG_CROSS_COMPILER_PATH)/mksquashfs_lzma-4.2 $(ROMFSDIR) $(RAMDISK)
endif
	echo $(LINUXDIR)
ifeq ($(LINUXDIR),linux-2.6.36.x)
	$(CONFIG_CROSS_COMPILER_PATH)/mksquashfs_lzma-4.2 $(ROMFSDIR) $(RAMDISK)
endif
endif
endif

ifeq	($(CONFIG_BLK_DEV_INITRD),y)
ifeq	($(LINUXDIR),linux-2.4.x)
	gzip -9 -f $(RAMDISK)
	cp $(RAMDISK).gz $(ROOTDIR)/$(LINUXDIR)/arch/mips/ramdisk/ramdisk.gz 2>/dev/null
endif
endif


mkimage: $(MKIMAGE_DIR)/crc32.c $(MKIMAGE_DIR)/mkimage.c 
	$(HOSTCC) $(HOST_CFLAGS) -DUSE_HOSTCC -o $@ -I$(MKIMAGE_DIR)/include $^ 

image: mkimage
	$(CROSS_COMPILE)objcopy -O binary -R .note -R .comment -S $(ROOTDIR)/$(LINUXDIR)/vmlinux  $(KERNELZ)
	cd $(IMAGEDIR) ; rm -f $(KERNELZ).*; $(COMP) -9 -f -S .$(COMP) $(KERNELZ)

ifeq	($(CONFIG_BLK_DEV_INITRD),)
ifeq	($(CONFIG_ROOTFS_IN_FLASH_NO_PADDING),)
	@#============================================================
	@# uboot hdr(64B) + mtdblock0(192KB) + mtdblock1(64KB) + 
	@# mtdblock2(64KB) + mtdblock3 + mtdblock4 + mtdblock5
	@# 
	@# Padding size of mtdblock3 is equal to "MTD_KRN_PART_SIZE 
	@# - 64B - the size of compressed kernel image"
	@#
	@# In Other Words:
	@# MTD Kernel Partition Size is MUST bigger than 
	@# "Original Kernel Image Size + 64B"
	@#============================================================
	#========<< This is README not Warning Message>>==============
	# Please make sure that original kernel image size can't 
	# bigger or equal to Kernel Partition Size. 
	#
	# For Example:
	# 
	# Original kernel image size is 563421 Bytes(0x898DD Bytes).
	# Flash sector size is 0x10000, so MTD kernel 
	# partition size MUST bigger than 0x90000.
	#
	# #make menuconfig
	#
	# Kernel/Library/Defaults Selection  --->
	#   [*] Customize Kernel Settings (NEW)
	#      Machine selection  --->
	#        (90000) MTD Kernel Partition Size (Unit:Bytes)
	#=============================================================
	# Original Kernel Image Size
	  @wc -c $(KERNELZ).$(COMP)
	# Padded Kernel Image Size 
ifeq    ($(findstring 0x, $(CONFIG_MTD_KERNEL_PART_SIZ)),0x)
	  @SIZE=`wc -c $(KERNELZ).$(COMP) | awk '{ print $$1 }'` ; \
	   MTD_KRN_PART_SIZE=`printf "%d" $(CONFIG_MTD_KERNEL_PART_SIZ)` ; \
	   PAD=`expr $$MTD_KRN_PART_SIZE - 64 - $$SIZE` ; \
	   dd if=/dev/zero count=1 bs=$$PAD 2> /dev/null | tr \\000 \\377 >> $(KERNELZ).$(COMP)
else
	  @SIZE=`wc -c $(KERNELZ).$(COMP) | awk '{ print $$1 }'` ; \
	   MTD_KRN_PART_SIZE=`printf "%d" 0x$(CONFIG_MTD_KERNEL_PART_SIZ)` ; \
	   PAD=`expr $$MTD_KRN_PART_SIZE - 64 - $$SIZE` ; \
	   dd if=/dev/zero count=1 bs=$$PAD 2> /dev/null | tr \\000 \\377 >> $(KERNELZ).$(COMP)
endif
endif
	@wc -c $(KERNELZ).$(COMP)
	# Original RootFs Size 
	  @du -sb $(ROMFSDIR)
	# Compressed RootFs Size 
	  @wc -c $(RAMDISK)
	# Padded Kernel Image + Compressed Rootfs Size
	  @cat $(RAMDISK) >> $(KERNELZ).$(COMP) 
	  @wc -c $(KERNELZ).$(COMP)
	#===========================================
endif

ifeq	($(CONFIG_ROOTFS_IN_FLASH_NO_PADDING),y)
	@cd $(IMAGEDIR) ; \
	ISIZE=`wc -c $(KERNELZ).$(COMP) | awk '{print $$1}'` ; \
	RSIZE=`wc -c $(RAMDISK) | awk '{print $$1}'` ; \
	KRN_SIZE=`expr $$ISIZE - $$RSIZE + 64` ; \
	ENTRY=`readelf -h $(ROOTDIR)/$(LINUXDIR)/vmlinux | grep "Entry" | awk '{print $$4}'` ; \
	$(CUR_DIR)/mkimage -A mips -O linux -T kernel -C $(COMP) -a 80000000 -e $$ENTRY -k $$KRN_SIZE -n "Linux Kernel Image" -d $(KERNELZ).$(COMP) $(IMAGE)
else
	@cd $(IMAGEDIR) ; $(CUR_DIR)/mkimage -A mips -O linux -T kernel -C $(COMP) -a 80000000 -e $(shell readelf -h $(ROOTDIR)/$(LINUXDIR)/vmlinux | grep "Entry" | awk '{print $$4}') -n "Linux Kernel Image"  -d $(KERNELZ).$(COMP) $(IMAGE)
endif

