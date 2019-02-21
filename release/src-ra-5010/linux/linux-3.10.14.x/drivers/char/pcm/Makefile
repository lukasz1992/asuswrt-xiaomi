EXTRA_CFLAGS = -Idrivers/char

CONFIG_SLIC_DRIVER := y
CONFIG_SLIC_SI3226X := y
#CONFIG_SLIC_SI32176 := y
#CONFIG_SLIC_SI3226 := y
EXTRA_CFLAGS+=-DSLIC_SILICON
#EXTRA_CFLAGS+=-DPCM_LOOPBACK_TEST
#EXTRA_CFLAGS+=-DCONFIG_PCM_SLT

PROSLIC_API := proslic_api
EXTRA_CFLAGS += -Idrivers/char/pcm/$(PROSLIC_API)/inc -Idrivers/char/pcm/$(PROSLIC_API)/custom
EXTRA_CFLAGS += -Idrivers/char/pcm/$(PROSLIC_API)/config_inc
EXTRA_CFLAGS += -Idrivers/char/pcm/$(PROSLIC_API)/example_drivers/ralink/proslic_ppt_mb
EXTRA_CFLAGS += -Idrivers/char/pcm/$(PROSLIC_API)/example_drivers/ralink/timer
EXTRA_CFLAGS += -Idrivers/char/pcm

ifeq ($(CONFIG_SLIC_SI3226X),y)
EXTRA_CFLAGS += -DSI3226X
endif
ifeq ($(CONFIG_SLIC_SI32176),y)
EXTRA_CFLAGS += -DSI3217X
endif
ifeq ($(CONFIG_SLIC_SI3226),y)
EXTRA_CFLAGS +=-DSI322X
endif

SRC_DIR = ./$(PROSLIC_API)/src
SRC_OBJ = $(SRC_DIR)/si_voice_version.o $(SRC_DIR)/vdaa.o \
			$(SRC_DIR)/proslic.o $(SRC_DIR)/si_voice.o

ifeq ($(CONFIG_SLIC_SI3226),y)
SRC_OBJ +=	./$(PROSLIC_API)/patch_files/si3226_patch_C_QCUK_2010AUG10.o \
			./$(PROSLIC_API)/patch_files/si3226_patch_D_QCUK_2010AUG10.o \
			./$(PROSLIC_API)/patch_files/si3226_patch_E_FLBK_2010AUG10.o
endif			
ifeq ($(CONFIG_SLIC_SI32176),y)
SRC_OBJ += ./$(PROSLIC_API)/patch_files/si3217x_patch_B_BB_2012DEC10.o
endif
ifeq ($(CONFIG_SLIC_SI3226X),y)
SRC_OBJ += ./$(PROSLIC_API)/patch_files/si3226x_patch_C_TSS_ISO_2012DEC06.o
endif

#$(SRC_DIR)/proslic_tstin.o
ifeq ($(CONFIG_SLIC_SI3226X),y)
SRC_OBJ += $(SRC_DIR)/si3226x_intf.o 
endif
ifeq ($(CONFIG_SLIC_SI322X),y)
SRC_OBJ += $(SRC_DIR)/si3226_intf.o
endif
ifeq ($(CONFIG_SLIC_SI32176),y)
SRC_OBJ += $(SRC_DIR)/si3217x_intf.o $(SRC_DIR)/si3217x_revb_intf.o $(SRC_DIR)/si3217x_revc_intf.o
endif
CONFIG_SRC_DIR = ./$(PROSLIC_API)/config_src
ifeq ($(CONFIG_SLIC_SI3226X),y)
CONFIG_SRC_OBJ := $(CONFIG_SRC_DIR)/si3226x_PBB_VDC_4P5_16P0_constants.o
endif
ifeq ($(CONFIG_SLIC_SI32176),y)
CONFIG_SRC_OBJ += $(CONFIG_SRC_DIR)/si3217x_BKBT_constants.o
endif
CONFIG_SRC_OBJ += $(CONFIG_SRC_DIR)/vdaa_constants.o
ifeq ($(CONFIG_SLIC_SI3226),y)
CONFIG_SRC_OBJ += $(CONFIG_SRC_DIR)/si3226_QCUK_constants.o
endif
#$(CONFIG_SRC_DIR)/proslic_tstin_limits.o

RALINK_DIR = ./$(PROSLIC_API)/example_drivers/ralink/proslic_ppt_mb/si3226
			
RALINK_OBJ = $(RALINK_DIR)/proslic_spi_api.o
TIMER_DIR = ./$(PROSLIC_API)/example_drivers/ralink/timer
RALINK_OBJ += $(TIMER_DIR)/proslic_timer_intf_linux.o ./$(PROSLIC_API)/slic_init.o

PCM_DRIVER_DIR := .
obj-m +=$(PCM_DRIVER_DIR)/ralink_pcm.o
$(PCM_DRIVER_DIR)/ralink_pcm-objs := ./codec_lib/codec_api.o \
						$(PCM_DRIVER_DIR)/pcm_ctrl.o

ifneq ($(CONFIG_RALINK_MT7621)$(CONFIG_RALINK_MT7628),)	
$(PCM_DRIVER_DIR)/ralink_pcm-objs += $(PCM_DRIVER_DIR)/slic_ctrl.o
endif

ifeq ($(CONFIG_SLIC_DRIVER),y)						
$(PCM_DRIVER_DIR)/ralink_pcm-objs += $(CONFIG_SRC_OBJ) $(SRC_OBJ) $(RALINK_OBJ)
endif

clean:
	rm -rf $(PCM_DRIVER_DIR)/*.o $(PCM_DRIVER_DIR)/*.ko $(PCM_DRIVER_DIR)/*.mod.c  $(PCM_DRIVER_DIR)/.*.cmd $(PCM_DRIVER_DIR)/*.bak
	rm -rf $(SRC_DIR)/*.o $(SRC_DIR)/*.ko $(SRC_DIR)/*.mod.c  $(SRC_DIR)/.*.cmd $(SRC_DIR)/*.bak
	rm -rf $(RALINK_DIR)/*.o $(RALINK_DIR)/*.ko $(RALINK_DIR)/*.mod.c  $(RALINK_DIR)/.*.cmd $(RALINK_DIR)/*.bak
	rm -rf $(PROSLIC_API)/*.o $(PROSLIC_API)/*.ko $(PROSLIC_API)/*.mod.c  $(PROSLIC_API)/.*.cmd $(PROSLIC_API)/*.bak
	rm -rf $(CONFIG_SRC_DIR)/*.o $(CONFIG_SRC_DIR)/*.ko $(CONFIG_SRC_DIR)/*.mod.c  $(CONFIG_SRC_DIR)/.*.cmd $(CONFIG_SRC_DIR)/*.bak
	rm -fr $(SRC_OBJ) $(RALINK_OBJ) $(CONFIG_SRC_OBJ)
	rm -fr modules.order
	rm -f Module.symvers
	@date
