#
# (C) Copyright 2000-2004
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

include $(TOPDIR)/config.mk

# CFLAGS += -DET_DEBUG -DDEBUG

LIB	= libdrivers.a

OBJS	= rt2880_eth.o #i2c_drv.o

ifeq ($(RALINK_MDIO_ACCESS_FUN),ON)
OBJS	+= mii_mgr.o
endif

ifeq ($(CFG_ENV_IS), IN_FLASH)
OBJS	+= spi_drv.o 
endif

ifeq ($(CFG_ENV_IS), IN_SPI)
 ifeq ($(RT6855A_MP), y)
 OBJS	+= spi_rt6855A_flash.o 
 else
 ifeq ($(MT7621_MP), y)
 OBJS	+= spi_rt6855A_flash.o 
 else
 ifeq ($(MT7628_MP), y)
 OBJS	+= spi_rt6855A_flash.o 
 else
 OBJS	+= spi_flash.o 
 endif
 endif
 endif
 ifeq ($(RT3883_MP), y)
 OBJS	+= spi_drv.o 
 endif
endif

ifeq ($(CFG_ENV_IS), IN_NAND)
 ifeq ($(MT7621_MP), y)
	OBJS    += nand/mtk_nand.o nand/bmt.o nand/nand_base.o nand/nand_ids.o
 else
OBJS	+= nand_flash.o 
 endif
 ifeq ($(RT3883_MP), y)
 OBJS	+= spi_drv.o 
 endif
endif

ifeq ($(NAND_SUPPORT),y)
OBJS	+= nand_flash.o
endif

ifeq ($(NAND_FLASH_BMT_MTK),y)
OBJS += bmt.o
endif

ifeq ($(SWITCH_CHIP_RTL8367RB),y)
SWITCH_DIR := RTL8367RB_API_Driver_V1.2.12-20140609

CFLAGS += -D_LITTLE_ENDIAN -I$(TOPDIR)/include/$(SWITCH_DIR)/
#CFLAGS += -DDSL_ENABLE_ALL_RTK_FUNC

OBJS += $(SWITCH_DIR)/smi.o $(SWITCH_DIR)/myrtk.o

OBJS += $(SWITCH_DIR)/rtk_api.o
OBJS += $(SWITCH_DIR)/rtl8367b_asicdrv.o
OBJS += $(SWITCH_DIR)/rtl8367b_asicdrv_acl.o
OBJS += $(SWITCH_DIR)/rtl8367b_asicdrv_cputag.o
OBJS += $(SWITCH_DIR)/rtl8367b_asicdrv_dot1x.o
OBJS += $(SWITCH_DIR)/rtl8367b_asicdrv_eee.o
#OBJS += $(SWITCH_DIR)/rtl8370_asicdrv_eeelldp.o
OBJS += $(SWITCH_DIR)/rtl8367b_asicdrv_fc.o
OBJS += $(SWITCH_DIR)/rtl8367b_asicdrv_green.o
OBJS += $(SWITCH_DIR)/rtl8367b_asicdrv_igmp.o
OBJS += $(SWITCH_DIR)/rtl8367b_asicdrv_inbwctrl.o
OBJS += $(SWITCH_DIR)/rtl8367b_asicdrv_interrupt.o
OBJS += $(SWITCH_DIR)/rtl8367b_asicdrv_hsb.o
OBJS += $(SWITCH_DIR)/rtl8367b_asicdrv_led.o
OBJS += $(SWITCH_DIR)/rtl8367b_asicdrv_lut.o
OBJS += $(SWITCH_DIR)/rtl8367b_asicdrv_meter.o
OBJS += $(SWITCH_DIR)/rtl8367b_asicdrv_mib.o
OBJS += $(SWITCH_DIR)/rtl8367b_asicdrv_mirror.o
OBJS += $(SWITCH_DIR)/rtl8367b_asicdrv_misc.o
#OBJS += $(SWITCH_DIR)/rtl8370_asicdrv_oam.o
OBJS += $(SWITCH_DIR)/rtl8367b_asicdrv_phy.o
OBJS += $(SWITCH_DIR)/rtl8367b_asicdrv_port.o
OBJS += $(SWITCH_DIR)/rtl8367b_asicdrv_portIsolation.o
OBJS += $(SWITCH_DIR)/rtl8367b_asicdrv_qos.o
#OBJS += $(SWITCH_DIR)/rtl8370_asicdrv_rldp.o
OBJS += $(SWITCH_DIR)/rtl8367b_asicdrv_rma.o
#OBJS += $(SWITCH_DIR)/rtl8370_asicdrv_rrcp.o
OBJS += $(SWITCH_DIR)/rtl8367b_asicdrv_scheduling.o
#OBJS += $(SWITCH_DIR)/rtl8370_asicdrv_specialCongest.o
OBJS += $(SWITCH_DIR)/rtl8367b_asicdrv_storm.o
OBJS += $(SWITCH_DIR)/rtl8367b_asicdrv_svlan.o
OBJS += $(SWITCH_DIR)/rtl8367b_asicdrv_trunking.o
OBJS += $(SWITCH_DIR)/rtl8367b_asicdrv_unknownMulticast.o
OBJS += $(SWITCH_DIR)/rtl8367b_asicdrv_vlan.o
OBJS += $(SWITCH_DIR)/rtl8367b_asicdrv_eav.o
endif

ifeq ($(MTK_XHCI), ON)
OBJS	+= usb/host/xhci.o usb/host/xhci-mem.o usb/host/xhci-ring.o usb/host/xhci-mtk.o usb/host/xhci-mtk-power.o usb/host/xhci-mtk-scheduler.o usb/host/mtk-phy.o usb/host/mtk-phy-ahb.o usb/host/mtk-phy-7621.o
endif

ifeq ($(RALINK_OHCI), ON)
OBJS	+= ohci-hcd.o
endif

ifeq ($(RALINK_EHCI), ON)
OBJS	+= ehci-hcd.o ehci-ra.o
endif
 
ifeq ($(RALINK_EPHY_TESTER), ON)
OBJS   += phy_val.o 
endif
ifeq ($(MTK_MSDC), ON)
OBJS	+= msdc/mmc_core.o msdc/mmc_test.o msdc/msdc.o msdc/msdc_cust.o msdc/ts_msdc.o
endif

OBJS += flash_wrapper.o

all:	$(LIB)

$(LIB): $(OBJS)
	$(AR) crv $@ $(OBJS)

#########################################################################

.depend:	Makefile $(OBJS:.o=.c)
		$(CC) -M $(CFLAGS) $(OBJS:.o=.c) > $@

sinclude .depend

#########################################################################
