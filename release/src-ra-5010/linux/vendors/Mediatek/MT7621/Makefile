KERNELZ = $(IMAGEDIR)/zImage
RAMDISK  = $(IMAGEDIR)/ramdisk
IMAGE = $(IMAGEDIR)/$(USER)_uImage
ROMFS_DIR = $(ROOTDIR)/romfs
LIBGCC = $(shell $(CROSS_COMPILE)gcc -print-file-name=libgcc_s.so.1)

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
	/etc_ro/lighttpd \
	etc_ro/linuxigd etc_ro/Wireless/RT2860AP etc_ro/Wireless/RT61AP etc_ro/Wireless/iNIC \
	etc_ro/Wireless/WIFI3 \
	lib lib/modules lib/ipsec lib/firmware \
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
	$(ROMFSINST) /etc_ro/rcS
	$(ROMFSINST) /etc_ro/inittab
	$(ROMFSINST) /etc_ro/motd
	$(ROMFSINST) /etc/fstab
ifeq ($(CONFIG_RALINKAPP_TOUCH_PANEL), y)
	$(ROMFSINST) /etc_ro/A.bmp
	$(ROMFSINST) /etc_ro/G.bmp
	$(ROMFSINST) /etc_ro/123.bmp	
	$(ROMFSINST) /etc_ro/k.bmp
	$(ROMFSINST) /etc_ro/j.bmp
	$(ROMFSINST) /etc_ro/home.bmp	
	$(ROMFSINST) /etc_ro/mediatek_logo.bmp	
endif

ifeq    ($(CONFIG_CONFIG_SHRINK), y)
	mv RT2860_default_novlan RT2860_default_novlan.tmp
	mv RT2860_default_vlan RT2860_default_vlan.tmp
	cat RT2860_default_novlan.tmp tr069_voip_default_settings > RT2860_default_novlan
	cat RT2860_default_vlan.tmp tr069_voip_default_settings > RT2860_default_vlan
	$(ROMFSINST) /etc_ro/Wireless/RT2860AP/RT2860_default_novlan
	$(ROMFSINST) /etc_ro/Wireless/RT2860AP/RT2860_default_vlan
	mv RT2860_default_novlan.tmp RT2860_default_novlan
	mv RT2860_default_vlan.tmp RT2860_default_vlan
else
	$(ROMFSINST) /etc_ro/Wireless/RT2860AP/RT2860_default_novlan
	$(ROMFSINST) /etc_ro/Wireless/RT2860AP/RT2860_default_vlan
	$(ROMFSINST) -e CONFIG_EXTEND_NVRAM /etc_ro/tr069_voip_default_settings
endif
	$(ROMFSINST) /etc_ro/Wireless/iNIC/RT2860AP.dat
	$(ROMFSINST) /etc_ro/Wireless/WIFI3/RT2870AP.dat
	$(ROMFSINST) /etc_ro/Wireless/RT2860AP/7615_SingleSKU.dat
	$(ROMFSINST) /etc_ro/Wireless/RT2860AP/7615_SingleSKU_BF.dat
	#for sigma test
	$(ROMFSINST) -e CONFIG_RALINKAPP_HOTSPOT /etc_ro/Wireless/RT2860AP/RT2860_def.dat
	$(ROMFSINST) /etc_ro/rept_test.sh
	$(ROMFSINST) /etc_ro/rept_dbdc_test.sh

	rm -f "MT7612E_EEPROM_layout_20131121_2G_ePAeLNA_TXTC_off.bin" \
		"MT7612E_EEPROM_layout_20131121_5G_ePAeLNA_TXTC_off.bin" \
		"MT7612E_EEPROM_layout_20131121_2G_ePAeLNAcoclock_TXTC_off.bin" \
		"MT7612E_EEPROM_layout_20131121_5G_ePAeLNAcoclock_TXTC_off.bin" \
		"MT7612E_EEPROM_layout_20131121_2G_iPAeLNA_wTSSI default slope offset.bin" \
		"MT7612E_EEPROM_layout_20131121_5G_iPAeLNA_wTSSI default slope offset.bin" \
		"MT7612E_EEPROM_layout_20131121_2G_iPAeLNAcoclock_wTSSI default slope offset.bin" \
		"MT7612E_EEPROM_layout_20131121_5G_iPAeLNAcoclock_wTSSI default slope offset.bin" \
		"MT7612E_EEPROM_layout_20131121_2G_iPAiLNA_wTSSI default slope offset.bin" \
		"MT7612E_EEPROM_layout_20131121_2G_iPAiLNAcoclock_wTSSI default slope offset.bin"
ifneq	($(findstring 7612, $(CONFIG_RT_FIRST_CARD) $(CONFIG_RT_SECOND_CARD)),)
	ln -s "MT7612E_EEPROM_layout_20131121_2G5G_ePAeLNA_TXTC_off.bin" \
		"MT7612E_EEPROM_layout_20131121_2G_ePAeLNA_TXTC_off.bin"
	ln -s "MT7612E_EEPROM_layout_20131121_2G5G_ePAeLNA_TXTC_off.bin" \
		"MT7612E_EEPROM_layout_20131121_5G_ePAeLNA_TXTC_off.bin"
	ln -s "MT7612E_EEPROM_layout_20131121_2G5G_ePAeLNAcoclock_TXTC_off.bin" \
		"MT7612E_EEPROM_layout_20131121_2G_ePAeLNAcoclock_TXTC_off.bin"
	ln -s "MT7612E_EEPROM_layout_20131121_2G5G_ePAeLNAcoclock_TXTC_off.bin" \
		"MT7612E_EEPROM_layout_20131121_5G_ePAeLNAcoclock_TXTC_off.bin"
	ln -s "MT7612E_EEPROM_layout_20131121_2G5G_iPAeLNA_wTSSI default slope offset.bin" \
		"MT7612E_EEPROM_layout_20131121_2G_iPAeLNA_wTSSI default slope offset.bin"
	ln -s "MT7612E_EEPROM_layout_20131121_2G5G_iPAeLNA_wTSSI default slope offset.bin" \
		"MT7612E_EEPROM_layout_20131121_5G_iPAeLNA_wTSSI default slope offset.bin"
	ln -s "MT7612E_EEPROM_layout_20131121_2G5G_iPAeLNAcoclock_wTSSI default slope offset.bin" \
		"MT7612E_EEPROM_layout_20131121_2G_iPAeLNAcoclock_wTSSI default slope offset.bin"
	ln -s "MT7612E_EEPROM_layout_20131121_2G5G_iPAeLNAcoclock_wTSSI default slope offset.bin" \
		"MT7612E_EEPROM_layout_20131121_5G_iPAeLNAcoclock_wTSSI default slope offset.bin"
	ln -s "MT7612E_EEPROM_layout_20131121_5G_iPAiLNA_wTSSI default slope offset.bin" \
		"MT7612E_EEPROM_layout_20131121_2G_iPAiLNA_wTSSI default slope offset.bin"
	ln -s "MT7612E_EEPROM_layout_20131121_5G_iPAiLNAcoclock_wTSSI default slope offset.bin" \
		"MT7612E_EEPROM_layout_20131121_2G_iPAiLNAcoclock_wTSSI default slope offset.bin"
endif
ifneq	($(findstring 7615, $(CONFIG_RT_FIRST_CARD) $(CONFIG_RT_SECOND_CARD)),)
#//+++Add for ATED usage
	$(ROMFSINST) /etc_ro/Wireless/RT2860AP/RT2860AP-2G.dat
	$(ROMFSINST) /etc_ro/Wireless/RT2860AP/RT2860AP-5G.dat
	$(ROMFSINST) /etc_ro/Wireless/RT2860AP/RT2860AP-DBDC.dat
#//---Add  for ATED usage
	$(ROMFSINST) /etc_ro/wlan/MT7615E_EEPROM.bin
else
	rm -f *_EEPROM.bin
ifeq	($(CONFIG_CO_CLOCK_SUPPORT), y)
# Fisrt wifi interface - 2.4G
ifneq	($(CONFIG_RT_FIRST_CARD),)
	rm -f MT$(shell echo $(CONFIG_RT_FIRST_CARD) | tr a-z A-Z)_EEPROM.bin
ifeq	($(CONFIG_FIRST_CARD_EXTERNAL_PA),)
ifeq	($(CONFIG_FIRST_CARD_EXTERNAL_LNA),)
	# iPA/iLNA
	ln -s "MT$(shell echo $(CONFIG_RT_FIRST_CARD) | tr a-z A-Z)_EEPROM_layout_20131121_2G_iPAiLNAcoclock_wTSSI default slope offset.bin" \
		MT$(shell echo $(CONFIG_RT_FIRST_CARD) | tr a-z A-Z)_EEPROM.bin
else
	# iPA/eLNA
	ln -s "MT$(shell echo $(CONFIG_RT_FIRST_CARD) | tr a-z A-Z)_EEPROM_layout_20131121_2G_iPAeLNAcoclock_wTSSI default slope offset.bin" \
		MT$(shell echo $(CONFIG_RT_FIRST_CARD) | tr a-z A-Z)_EEPROM.bin
endif
else
	# ePA/eLNA, ePA/iLNA
	ln -s "MT$(shell echo $(CONFIG_RT_FIRST_CARD) | tr a-z A-Z)_EEPROM_layout_20131121_2G_ePAeLNAcoclock_TXTC_off.bin" \
		MT$(shell echo $(CONFIG_RT_FIRST_CARD) | tr a-z A-Z)_EEPROM.bin
endif
endif
# Second wifi interface - 5G
ifneq	($(CONFIG_RT_SECOND_CARD),)
	rm -f MT$(shell echo $(CONFIG_RT_SECOND_CARD) | tr a-z A-Z)_EEPROM.bin
ifeq	($(CONFIG_SECOND_CARD_EXTERNAL_PA),)
ifeq	($(CONFIG_SECOND_CARD_EXTERNAL_LNA),)
	# iPA/iLNA
	ln -s "MT$(shell echo $(CONFIG_RT_SECOND_CARD) | tr a-z A-Z)_EEPROM_layout_20131121_5G_iPAiLNAcoclock_wTSSI default slope offset.bin" \
		MT$(shell echo $(CONFIG_RT_SECOND_CARD) | tr a-z A-Z)_EEPROM.bin
else
	# iPA/eLNA
	ln -s "MT$(shell echo $(CONFIG_RT_SECOND_CARD) | tr a-z A-Z)_EEPROM_layout_20131121_5G_iPAeLNAcoclock_wTSSI default slope offset.bin" \
		MT$(shell echo $(CONFIG_RT_SECOND_CARD) | tr a-z A-Z)_EEPROM.bin
endif
else
	# ePA/eLNA, ePA/iLNA
	ln -s "MT$(shell echo $(CONFIG_RT_SECOND_CARD) | tr a-z A-Z)_EEPROM_layout_20131121_5G_ePAeLNAcoclock_TXTC_off.bin" \
		MT$(shell echo $(CONFIG_RT_SECOND_CARD) | tr a-z A-Z)_EEPROM.bin
endif
endif
else # non co-Clock
# Fisrt wifi interface - 2.4G
ifneq	($(CONFIG_RT_FIRST_CARD),)
	rm -f MT$(shell echo $(CONFIG_RT_FIRST_CARD) | tr a-z A-Z)_EEPROM.bin
ifeq	($(CONFIG_FIRST_CARD_EXTERNAL_PA),)
ifeq	($(CONFIG_FIRST_CARD_EXTERNAL_LNA),)
	# iPA/iLNA
	ln -s "MT$(shell echo $(CONFIG_RT_FIRST_CARD) | tr a-z A-Z)_EEPROM_layout_20131121_2G_iPAiLNA_wTSSI default slope offset.bin" \
		MT$(shell echo $(CONFIG_RT_FIRST_CARD) | tr a-z A-Z)_EEPROM.bin
else
	# iPA/eLNA
	ln -s "MT$(shell echo $(CONFIG_RT_FIRST_CARD) | tr a-z A-Z)_EEPROM_layout_20131121_2G_iPAeLNA_wTSSI default slope offset.bin" \
		MT$(shell echo $(CONFIG_RT_FIRST_CARD) | tr a-z A-Z)_EEPROM.bin
endif
else
	# ePA/eLNA, ePA/iLNA
	ln -s "MT$(shell echo $(CONFIG_RT_FIRST_CARD) | tr a-z A-Z)_EEPROM_layout_20131121_2G_ePAeLNA_TXTC_off.bin" \
		MT$(shell echo $(CONFIG_RT_FIRST_CARD) | tr a-z A-Z)_EEPROM.bin
endif
endif
# Second wifi interface - 5G
ifneq	($(CONFIG_RT_SECOND_CARD),)
	rm -f MT$(shell echo $(CONFIG_RT_SECOND_CARD) | tr a-z A-Z)_EEPROM.bin
ifeq	($(CONFIG_SECOND_CARD_EXTERNAL_PA),)
ifeq	($(CONFIG_SECOND_CARD_EXTERNAL_LNA),)
	# iPA/iLNA
	ln -s "MT$(shell echo $(CONFIG_RT_SECOND_CARD) | tr a-z A-Z)_EEPROM_layout_20131121_5G_iPAiLNA_wTSSI default slope offset.bin" \
		MT$(shell echo $(CONFIG_RT_SECOND_CARD) | tr a-z A-Z)_EEPROM.bin
else
	# iPA/eLNA
	ln -s "MT$(shell echo $(CONFIG_RT_SECOND_CARD) | tr a-z A-Z)_EEPROM_layout_20131121_5G_iPAeLNA_wTSSI default slope offset.bin" \
		MT$(shell echo $(CONFIG_RT_SECOND_CARD) | tr a-z A-Z)_EEPROM.bin
endif
else
	# ePA/eLNA, ePA/iLNA
	ln -s "MT$(shell echo $(CONFIG_RT_SECOND_CARD) | tr a-z A-Z)_EEPROM_layout_20131121_5G_ePAeLNA_TXTC_off.bin" \
		MT$(shell echo $(CONFIG_RT_SECOND_CARD) | tr a-z A-Z)_EEPROM.bin
endif
endif
endif
	if [ -f MT$(shell echo $(CONFIG_RT_FIRST_CARD) | tr a-z A-Z)_EEPROM.bin ]; then \
		$(ROMFSINST) /etc_ro/wlan/MT$(shell echo $(CONFIG_RT_FIRST_CARD) | tr a-z A-Z)_EEPROM.bin; \
	fi
	if [ -f MT$(shell echo $(CONFIG_RT_SECOND_CARD) | tr a-z A-Z)_EEPROM.bin ]; then \
		$(ROMFSINST) /etc_ro/wlan/MT$(shell echo $(CONFIG_RT_SECOND_CARD) | tr a-z A-Z)_EEPROM.bin; \
	fi

ifneq ($(CONFIG_BT_MTK_HCI_USB),)
	$(ROMFSINST) /lib/firmware/mt7662_patch_e3_hdr.bin
endif
endif

romfs.post:: 
	[ -d $(IMAGEDIR) ] || mkdir -p $(IMAGEDIR)
#cp $(LIBGCC) $(ROOTDIR)/romfs/lib
	
	#Squash File System
	 rm -f $(RAMDISK)  
ifeq	($(LINUXDIR),linux-2.4.x)
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
ifeq	($(LINUXDIR),linux-2.6.36.x)
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
	$(CUR_DIR)/mkimage -A mips -O linux -T kernel -C $(COMP) -a 80001000 -e $$ENTRY -k $$KRN_SIZE -n "Linux Kernel Image" -d $(KERNELZ).$(COMP) $(IMAGE)
else
	@cd $(IMAGEDIR) ; $(CUR_DIR)/mkimage -A mips -O linux -T kernel -C $(COMP) -a 80001000 -e $(shell readelf -h $(ROOTDIR)/$(LINUXDIR)/vmlinux | grep "Entry" | awk '{print $$4}') -n "Linux Kernel Image"  -d $(KERNELZ).$(COMP) $(IMAGE)
endif

