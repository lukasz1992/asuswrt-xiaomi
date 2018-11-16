include ../common.mak
include $(SRCBASE)/.config

DSTDIR = $(INSTALLDIR)/lib/modules/$(LINUX_KERNEL)/kernel/drivers/net/wireless

install:
	@echo "[mt76xx]"
ifeq ($(BUILD_NAME),RT-MIR3G)
	@mkdir -p $(DSTDIR)
	@echo "[mt7603] Installing..."
	@cp -f prebuilt/mt_wifi.ko $(DSTDIR)
	@echo "[mt76x2] Installing..."
	@cp -f prebuilt/rlt_wifi.ko $(DSTDIR)
endif

.phony: install

