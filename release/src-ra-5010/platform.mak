export LINUXDIR := $(SRCBASE)/linux/linux-3.10.14.x

ifeq ($(EXTRACFLAGS),)
export EXTRACFLAGS := -DBCMWPA2 -fno-delete-null-pointer-checks -mips32 -mtune=mips32
endif

export KERNEL_BINARY=$(LINUXDIR)/vmlinux
export PLATFORM := mipsel-uclibc
export PLATFORM_ARCH := mipsel-uclibc
export CROSS_COMPILE := mipsel-uclibc-
export CROSS_COMPILER := $(CROSS_COMPILE)
export READELF := mipsel-linux-readelf
export CONFIGURE := ./configure --host=mipsel-linux --build=$(BUILD)
export HOSTCONFIG := linux-mipsel
export ARCH := mips
export HOST := mipsel-linux
export KERNELCC := /opt/buildroot-gcc463/usr/bin/mipsel-linux-gcc
export KERNELLD := /opt/buildroot-gcc463/usr/bin/mipsel-linux-ld
export TOOLS := $(SRCBASE)/../../tools/brcm/hndtools-mipsel-linux
export RTVER := 0.9.30.1

EXTRA_CFLAGS := -DLINUX26 -DCONFIG_RALINK -pipe -DDEBUG_NOISY -DDEBUG_RCTEST

export CONFIG_LINUX26=y
export CONFIG_RALINK=y

EXTRA_CFLAGS += -DLINUX30
export CONFIG_LINUX30=y

define platformRouterOptions
	@( \
	if [ "$(RALINK)" = "y" ]; then \
		sed -i "/RTCONFIG_RALINK\>/d" $(1); \
		echo "RTCONFIG_RALINK=y" >>$(1); \
		sed -i "/CONFIG_RA_HW_NAT_IPV6/d" $(1); \
		echo "# CONFIG_RA_HW_NAT_IPV6 is not set" >>$(1); \
		if [ "$(RT3883)" = "y" ]; then \
			sed -i "/RTCONFIG_RALINK_RT3883/d" $(1); \
			echo "RTCONFIG_RALINK_RT3883=y" >>$(1); \
		else \
			sed -i "/RTCONFIG_RALINK_RT3883/d" $(1); \
			echo "# RTCONFIG_RALINK_RT3883 is not set" >>$(1); \
		fi; \
		if [ "$(RT3052)" = "y" ]; then \
			sed -i "/RTCONFIG_RALINK_RT3052/d" $(1); \
			echo "RTCONFIG_RALINK_RT3052=y" >>$(1); \
		else \
			sed -i "/RTCONFIG_RALINK_RT3052/d" $(1); \
			echo "# RTCONFIG_RALINK_RT3052 is not set" >>$(1); \
		fi; \
		if [ "$(MT7620)" = "y" ]; then \
			sed -i "/RTCONFIG_RALINK_MT7620/d" $(1); \
			echo "RTCONFIG_RALINK_MT7620=y" >>$(1); \
		else \
			sed -i "/RTCONFIG_RALINK_MT7620/d" $(1); \
			echo "# RTCONFIG_RALINK_MT7620 is not set" >>$(1); \
		fi; \
		if [ "$(MT7621)" = "y" ]; then \
			sed -i "/RTCONFIG_RALINK_MT7621/d" $(1); \
			echo "RTCONFIG_RALINK_MT7621=y" >>$(1); \
		else \
			sed -i "/RTCONFIG_RALINK_MT7621/d" $(1); \
			echo "# RTCONFIG_RALINK_MT7621 is not set" >>$(1); \
		fi; \
	fi; \
	)
endef

define platformBusyboxOptions
endef

BOOT_FLASH_TYPE_POOL =	\
	"NOR"		\
	"SPI"		\
	"NAND"

DRAM_POOL = 		\
	"8M"		\
	"16M"		\
	"32M"		\
	"64M"		\
	"128M"		\
	"256M"		\
	"512M"		

FIRST_IF_POOL =		\
	"NONE"	\
	"RT2860"	\
	"RT3092"	\
	"RT5392"	\
	"RT5592"	\
	"RT3593"	\
	"MT7610E"	\
	"MT7612E"	\
	"MT7603E"	\
	"MT7602E"	\
	"MT7615E"

SECOND_IF_POOL = 	\
	"NONE"		\
	"RT3092"	\
	"RT5392"	\
	"RT5592"	\
	"RT3593"	\
	"RT3572"	\
	"RT5572"	\
	"MT7620"	\
	"MT7621"	\
	"MT7610U"	\
	"MT7610E"	\
	"RT8592"	\
	"MT7612U"	\
	"MT7612E"	\
	"MT7602E"	\
	"MT7603E"	\
	"MT7615E"


define platformKernelConfig
	@( \
	if [ "$(RALINK)" = "y" ]; then \
		if [ "$(RT3052)" = "y" ]; then \
			sed -i "/CONFIG_RALINK_RT3052_MP2/d" $(1); \
			echo "CONFIG_RALINK_RT3052_MP2=y" >>$(1); \
			sed -i "/CONFIG_RALINK_RT3052/d" $(1); \
			echo "CONFIG_RALINK_RT3052=y" >>$(1); \
			sed -i "/CONFIG_MTD_PHYSMAP_START/d" $(1); \
			echo "CONFIG_MTD_PHYSMAP_START=0xBF000000" >>$(1); \
			sed -i "/CONFIG_RT3052_ASIC/d" $(1); \
			echo "CONFIG_RT3052_ASIC=y" >>$(1); \
			sed -i "/CONFIG_RT2880_DRAM_128M/d" $(1); \
			sed -i "/CONFIG_RT2880_DRAM_32M/d" $(1); \
			echo "CONFIG_RT2880_DRAM_32M=y" >>$(1); \
			sed -i "/CONFIG_RALINK_RAM_SIZE/d" $(1); \
			echo "CONFIG_RALINK_RAM_SIZE=32" >>$(1); \
			sed -i "/CONFIG_RALINK_RT3883_MP/d" $(1); \
			echo "# CONFIG_RALINK_RT3883_MP is not set" >>$(1); \
			sed -i "/CONFIG_RALINK_RT3883/d" $(1); \
			echo "# CONFIG_RALINK_RT3883 is not set" >>$(1); \
			sed -i "/CONFIG_RT3883_ASIC/d" $(1); \
			echo "# CONFIG_RT3883_ASIC is not set" >>$(1); \
			sed -i "/CONFIG_RALINK_RT3883_MP/d" $(1); \
			echo "# CONFIG_RALINK_RT3883_MP is not set" >>$(1); \
			sed -i "/CONFIG_RALINK_RT3883_MP/d" $(1); \
			echo "# CONFIG_RALINK_RT3883_MP is not set" >>$(1); \
			sed -i "/CONFIG_RALINK_RT3662_2T2R/d" $(1); \
			echo "# CONFIG_RALINK_RT3662_2T2R is not set" >>$(1); \
			sed -i "/CONFIG_RALINK_RT3052_2T2R/d" $(1); \
			echo "CONFIG_RALINK_RT3052_2T2R=y" >>$(1); \
			sed -i "/CONFIG_RALINK_RT3352/d" $(1); \
			echo "# CONFIG_RALINK_RT3352 is not set" >>$(1); \
			sed -i "/CONFIG_LAN_WAN_SUPPORT/d" $(1); \
			echo "CONFIG_LAN_WAN_SUPPORT=y" >>$(1); \
			sed -i "/CONFIG_RT_3052_ESW/d" $(1); \
			echo "CONFIG_RT_3052_ESW=y" >>$(1); \
		fi; \
		if [ "$(SPI_FAST_CLOCK)" = "y" ] ; then \
			sed -i "/CONFIG_MTD_SPI_FAST_CLOCK/d" $(1); \
			echo "CONFIG_MTD_SPI_FAST_CLOCK=y" >>$(1); \
		fi; \
		if [ "$(RTAC1200HP)" = "y" ] ; then \
			sed -i "/CONFIG_RAETH_HAS_PORT5/d" $(1); \
			echo "CONFIG_RAETH_HAS_PORT5=y" >>$(1); \
			sed -i "/CONFIG_P5_MAC_TO_PHY_MODE/d" $(1); \
			echo "CONFIG_P5_MAC_TO_PHY_MODE=y" >>$(1); \
			sed -i "/CONFIG_MAC_TO_GIGAPHY_MODE_ADDR/d" $(1); \
			echo "CONFIG_MAC_TO_GIGAPHY_MODE_ADDR=0x5" >>$(1); \
		fi; \
		if [ "$(EDCCA)" = "y" ] ; then \
			sed -i "/CONFIG_RT2860V2_AP_EDCCA_MONITOR/d" $(1); \
			echo "CONFIG_RT2860V2_AP_EDCCA_MONITOR=y" >>$(1); \
		fi; \
		if [ "$(MT7621)" = "y" ] ; then \
			sed -i "/CONFIG_RALINK_MT7620/d" $(1); \
			echo "# CONFIG_RALINK_MT7620 is not set" >>$(1); \
			sed -i "/CONFIG_MT7620_ASIC/d" $(1); \
			echo "# CONFIG_MT7620_ASIC is not set" >>$(1); \
			sed -i "/CONFIG_MT7620_BAUDRATE/d" $(1); \
			echo "# CONFIG_MT7620_BAUDRATE is not set" >>$(1); \
			sed -i "/CONFIG_RALINK_MT7621/d" $(1); \
			echo "CONFIG_RALINK_MT7621=y" >>$(1); \
			sed -i "/CONFIG_MT7621_ASIC/d" $(1); \
			echo "CONFIG_MT7621_ASIC=y" >>$(1); \
			sed -i "/CONFIG_MTK_MTD_NAND/d" $(1); \
			echo "# CONFIG_MTK_MTD_NAND is not set" >>$(1); \
			sed -i "/CONFIG_MIPS_MT_SMP/d" $(1); \
			echo "CONFIG_MIPS_MT_SMP=y" >>$(1); \
			sed -i "/CONFIG_MIPS_MT_SMTC/d" $(1); \
			echo "# CONFIG_MIPS_MT_SMTC is not set" >>$(1); \
			sed -i "/CONFIG_MIPS_VPE_LOADER/d" $(1); \
			echo "# CONFIG_MIPS_VPE_LOADER is not set" >>$(1); \
			sed -i "/CONFIG_SCHED_SMT/d" $(1); \
			echo "CONFIG_SCHED_SMT=y" >>$(1); \
			sed -i "/CONFIG_MIPS_MT_FPAFF/d" $(1); \
			echo "CONFIG_MIPS_MT_FPAFF=y" >>$(1); \
			sed -i "/CONFIG_MIPS_CMP/d" $(1); \
			echo "CONFIG_MIPS_CMP=y" >>$(1); \
			sed -i "/CONFIG_HIGHMEM/d" $(1); \
			echo "# CONFIG_HIGHMEM is not set" >>$(1); \
			sed -i "/CONFIG_NR_CPUS/d" $(1); \
			echo "CONFIG_NR_CPUS=4" >>$(1); \
			sed -i "/CONFIG_RCU_TRACE/d" $(1); \
			echo "# CONFIG_RCU_TRACE is not set" >>$(1); \
			sed -i "/CONFIG_RCU_FANOUT/d" $(1); \
			echo "CONFIG_RCU_FANOUT=32" >>$(1); \
			sed -i "/CONFIG_RCU_FANOUT_EXACT/d" $(1); \
			echo "# CONFIG_RCU_FANOUT_EXACT is not set" >>$(1); \
			sed -i "/CONFIG_PCIE_PORT0/d" $(1); \
			echo "CONFIG_PCIE_PORT0=y" >>$(1); \
			sed -i "/CONFIG_PCIE_PORT1/d" $(1); \
			echo "CONFIG_PCIE_PORT1=y" >>$(1); \
			sed -i "/CONFIG_RALINK_SPDIF/d" $(1); \
			echo "CONFIG_RALINK_SPDIF=m" >>$(1); \
			sed -i "/CONFIG_RCU_CPU_STALL_DETECTOR/d" $(1); \
			echo "CONFIG_RCU_CPU_STALL_DETECTOR=y" >>$(1); \
			sed -i "/CONFIG_CRYPTO_PCRYPT/d" $(1); \
			echo "# CONFIG_CRYPTO_PCRYPT is not set" >>$(1); \
			sed -i "/CONFIG_RAETH_HW_VLAN_RX/d" $(1); \
			echo "# CONFIG_RAETH_HW_VLAN_RX is not set" >>$(1); \
			sed -i "/CONFIG_RAETH_QDMA/d" $(1); \
			echo "CONFIG_RAETH_QDMA=y" >>$(1); \
			sed -i "/CONFIG_GE1_MII_FORCE_100/d" $(1); \
			echo "# CONFIG_GE1_MII_FORCE_100 is not set" >>$(1); \
			sed -i "/CONFIG_GE1_RGMII_FORCE_1000/d" $(1); \
			echo "CONFIG_GE1_RGMII_FORCE_1000=y" >>$(1); \
			sed -i "/CONFIG_RAETH_GMAC2/d" $(1); \
			echo "CONFIG_RAETH_GMAC2=y" >>$(1); \
			sed -i "/CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2/d" $(1); \
			echo "CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2=4" >>$(1); \
			sed -i "/CONFIG_HW_IPSEC/d" $(1); \
			echo "# CONFIG_HW_IPSEC is not set" >>$(1); \
			sed -i "/CONFIG_GE1_MII_AN/d" $(1); \
			echo "# CONFIG_GE1_MII_AN is not set" >>$(1); \
			sed -i "/CONFIG_GE2_MII_AN/d" $(1); \
			echo "# CONFIG_GE2_MII_AN is not set" >>$(1); \
			sed -i "/CONFIG_GE1_RVMII_FORCE_100/d" $(1); \
			echo "# CONFIG_GE1_RVMII_FORCE_100 is not set" >>$(1); \
			sed -i "/CONFIG_GE2_RVMII_FORCE_100/d" $(1); \
			echo "# CONFIG_GE2_RVMII_FORCE_100 is not set" >>$(1); \
			sed -i "/CONFIG_GE1_RGMII_AN/d" $(1); \
			echo "# CONFIG_GE1_RGMII_AN is not set" >>$(1); \
			sed -i "/CONFIG_GE2_RGMII_AN/d" $(1); \
			echo "# CONFIG_GE2_RGMII_AN is not set" >>$(1); \
			sed -i "/CONFIG_GE1_RGMII_NONE/d" $(1); \
			echo "# CONFIG_GE1_RGMII_NONE is not set" >>$(1); \
			sed -i "/CONFIG_GE2_RGMII_FORCE_1000/d" $(1); \
			echo "# CONFIG_GE2_RGMII_FORCE_1000 is not set" >>$(1); \
			sed -i "/CONFIG_GE2_MII_FORCE_100/d" $(1); \
			echo "# CONFIG_GE2_MII_FORCE_100 is not set" >>$(1); \
			sed -i "/CONFIG_GE2_INTERNAL_GPHY/d" $(1); \
			echo "CONFIG_GE2_INTERNAL_GPHY=y" >>$(1); \
			sed -i "/CONFIG_RALINK_HWCRYPTO/d" $(1); \
			echo "# CONFIG_RALINK_HWCRYPTO is not set" >>$(1); \
			sed -i "/CONFIG_RA_HW_NAT_PREBIND/d" $(1); \
			echo "# CONFIG_RA_HW_NAT_PREBIND is not set" >>$(1); \
			sed -i "/CONFIG_PPE_MCAST/d" $(1); \
			echo "# CONFIG_PPE_MCAST is not set" >>$(1); \
			sed -i "/CONFIG_FIRST_IF_RT2860/d" $(1); \
			echo "# CONFIG_FIRST_IF_RT2860 is not set" >>$(1); \
			sed -i "/CONFIG_RT2860V2_AP/d" $(1); \
			echo "# CONFIG_RT2860V2_AP is not set" >>$(1); \
			sed -i "/CONFIG_RT2860V2_AP_MEMORY_OPTIMIZATION/d" $(1); \
			echo "# CONFIG_RT2860V2_AP_MEMORY_OPTIMIZATION is not set" >>$(1); \
			sed -i "/CONFIG_RT3XXX_EHCI/d" $(1); \
			echo "# CONFIG_RT3XXX_EHCI is not set" >>$(1); \
			sed -i "/CONFIG_USB_EHCI_ROOT_HUB_TT/d" $(1); \
			echo "# CONFIG_USB_EHCI_ROOT_HUB_TT is not set" >>$(1); \
			sed -i "/CONFIG_USB_EHCI_TT_NEWSCHED/d" $(1); \
			echo "# CONFIG_USB_EHCI_TT_NEWSCHED is not set" >>$(1); \
			sed -i "/CONFIG_RT3XXX_OHCI/d" $(1); \
			echo "# CONFIG_RT3XXX_OHCI is not set" >>$(1); \
			sed -i "/CONFIG_USB_OHCI_LITTLE_ENDIAN/d" $(1); \
			echo "# CONFIG_USB_OHCI_LITTLE_ENDIAN is not set" >>$(1); \
			sed -i "/CONFIG_USB_XHCI_HCD/d" $(1); \
			echo "CONFIG_USB_XHCI_HCD=y" >>$(1); \
			sed -i "/CONFIG_USB_MT7621_XHCI_HCD/d" $(1); \
			echo "CONFIG_USB_MT7621_XHCI_HCD=y" >>$(1); \
			sed -i "/CONFIG_PERIODIC_ENP/d" $(1); \
			echo "CONFIG_PERIODIC_ENP=y" >>$(1); \
			sed -i "/CONFIG_XHCI_DEV_NOTE/d" $(1); \
			echo "CONFIG_XHCI_DEV_NOTE=y" >>$(1); \
			sed -i "/CONFIG_USB_XHCI_HCD_DEBUGGING/d" $(1); \
			echo "CONFIG_USB_XHCI_HCD_DEBUGGING=y" >>$(1); \
			sed -i "/CONFIG_USB_EHCI_HCD/d" $(1); \
			echo "# CONFIG_USB_EHCI_HCD is not set" >>$(1); \
			sed -i "/CONFIG_USB_OHCI_HCD/d" $(1); \
			echo "# CONFIG_USB_OHCI_HCD is not set" >>$(1); \
			sed -i "/CONFIG_USB_OHCI_LITTLE_ENDIAN/d" $(1); \
			echo "# CONFIG_USB_OHCI_LITTLE_ENDIAN is not set" >>$(1); \
			for first_if in $(FIRST_IF_POOL) ; do \
				if [ "$(FIRST_IF)" = "$${first_if}" ] ; then \
					sed -i "/CONFIG_FIRST_IF_$${first_if}/d" $(1); \
					echo "CONFIG_FIRST_IF_$${first_if}=y" >> $(1); \
				else \
					sed -i "/CONFIG_FIRST_IF_$${first_if}/d" $(1); \
					echo "# CONFIG_FIRST_IF_$${first_if} is not set" >> $(1); \
				fi; \
			done; \
		fi; \
	fi; \
	for bftype in $(BOOT_FLASH_TYPE_POOL) ; do \
		sed -i "/CONFIG_MTD_$${bftype}_RALINK\>/d" $(1); \
		if [ "$(BOOT_FLASH_TYPE)" = "$${bftype}" ] ; then \
			echo "CONFIG_MTD_$${bftype}_RALINK=y" >> $(1); \
		else \
			echo "# CONFIG_MTD_$${bftype}_RALINK is not set" >> $(1); \
		fi; \
		if [ "$${bftype}" = "NAND" ] ; then \
			sed -i "/CONFIG_MTK_MTD_NAND\>/d" $(1); \
			if [ "$(BOOT_FLASH_TYPE)" = "$${bftype}" ] ; then \
				echo "CONFIG_MTK_MTD_NAND=y" >> $(1); \
			else \
				echo "# CONFIG_MTK_MTD_NAND is not set" >> $(1); \
			fi; \
		fi; \
	done; \
	sed -i "/CONFIG_MTD_ANY_RALINK/d" $(1); \
	echo "# CONFIG_MTD_ANY_RALINK is not set" >>$(1); \
	for dram in $(DRAM_POOL) ; do \
		sed -i "/CONFIG_RT2880_DRAM_$${dram}\>/d" $(1); \
		if [ "$(DRAM)" = "$${dram}" ] ; then \
			echo "CONFIG_RT2880_DRAM_$${dram}=y" >> $(1); \
		else \
			echo "# CONFIG_RT2880_DRAM_$${dram} is not set" >> $(1); \
		fi; \
	done; \
	for sec_if in $(SECOND_IF_POOL) ; do \
		sed -i "/CONFIG_MTD_$${sec_if}_RALINK\>/d" $(1); \
		if [ "$(SECOND_IF)" = "$${sec_if}" ] ; then \
			echo "CONFIG_SECOND_IF_$${sec_if}=y" >> $(1); \
		else \
			echo "# CONFIG_SECOND_IF_$${sec_if} is not set" >> $(1); \
		fi; \
	done; \
	if [ "$(SECOND_IF)" = "MT7610E" ] ; then \
		sed -i "/CONFIG_MT7610_AP\>/d" $(1); \
		echo "CONFIG_MT7610_AP=m" >>$(1); \
		sed -i "/CONFIG_MT7610_AP_V24_DATA_STRUCTURE/d" $(1); \
		echo "CONFIG_MT7610_AP_V24_DATA_STRUCTURE=y" >>$(1); \
		sed -i "/CONFIG_MT7610_AP_LED/d" $(1); \
		echo "CONFIG_MT7610_AP_LED=y" >>$(1); \
		sed -i "/CONFIG_MT7610_AP_WSC/d" $(1); \
		echo "CONFIG_MT7610_AP_WSC=y" >>$(1); \
		sed -i "/CONFIG_MT7610_AP_WSC_V2/d" $(1); \
		echo "CONFIG_MT7610_AP_WSC_V2=y" >>$(1); \
		sed -i "/CONFIG_MT7610_AP_LLTD/d" $(1); \
		echo "CONFIG_MT7610_AP_LLTD=y" >>$(1); \
		sed -i "/CONFIG_MT7610_AP_WDS/d" $(1); \
		echo "CONFIG_MT7610_AP_WDS=y" >>$(1); \
		sed -i "/CONFIG_MT7610_AP_MBSS/d" $(1); \
		echo "CONFIG_MT7610_AP_MBSS=y" >>$(1); \
		sed -i "/CONFIG_MT7610_AP_APCLI/d" $(1); \
		echo "# CONFIG_MT7610_AP_APCLI is not set" >>$(1); \
		sed -i "/CONFIG_MT7610_AP_IGMP_SNOOP/d" $(1); \
		echo "CONFIG_MT7610_AP_IGMP_SNOOP=y" >>$(1); \
		sed -i "/CONFIG_MT7610_AP_DFS/d" $(1); \
		echo "# CONFIG_MT7610_AP_DFS is not set" >>$(1); \
		sed -i "/CONFIG_MT7610_AP_CARRIER/d" $(1); \
		echo "# CONFIG_MT7610_AP_CARRIER is not set" >>$(1); \
		sed -i "/CONFIG_MT7610_AP_80211N_DRAFT3/d" $(1); \
		echo "CONFIG_MT7610_AP_80211N_DRAFT3=y" >>$(1); \
		sed -i "/CONFIG_BAND_STEERING/d" $(1); \
		echo "# CONFIG_BAND_STEERING is not set" >>$(1); \
		sed -i "/CONFIG_MT7610_AP_ATE/d" $(1); \
		echo "CONFIG_MT7610_AP_ATE=y" >>$(1); \
		sed -i "/CONFIG_MT7610_AP_QA/d" $(1); \
		echo "CONFIG_MT7610_AP_QA=y" >>$(1); \
		sed -i "/CONFIG_MT7610_AP_FLASH/d" $(1); \
		echo "CONFIG_MT7610_AP_FLASH=y" >>$(1); \
		sed -i "/CONFIG_MT7610_AP_BIG_ENDIAN/d" $(1); \
		echo "# CONFIG_MT7610_AP_BIG_ENDIAN is not set" >>$(1); \
		sed -i "/CONFIG_MT7610_AP_TSSI_COMPENSATION/d" $(1); \
		echo "# CONFIG_MT7610_AP_TSSI_COMPENSATION is not set" >>$(1); \
		sed -i "/CONFIG_RTMP_TEMPERATURE_COMPENSATION/d" $(1); \
		echo "CONFIG_RTMP_TEMPERATURE_COMPENSATION=y" >>$(1); \
		sed -i "/CONFIG_MT7610_AP_SINGLE_SKU/d" $(1); \
		echo "# CONFIG_MT7610_AP_SINGLE_SKU is not set" >>$(1); \
		sed -i "/CONFIG_MT7610_AP_CFG80211/d" $(1); \
		echo "# CONFIG_MT7610_AP_CFG80211 is not set" >>$(1); \
		sed -i "/CONFIG_MT7610_MCAST_RATE_SPECIFIC/d" $(1); \
		echo "CONFIG_MT7610_MCAST_RATE_SPECIFIC=y" >>$(1); \
		sed -i "/CONFIG_MT7610_ED_MONITOR/d" $(1); \
		echo "CONFIG_MT7610_ED_MONITOR=y" >>$(1); \
	fi; \
	if [ "$(FIRST_IF)" = "MT7603E" ] ; then \
		sed -i "/CONFIG_WIFI_MT7603E/d" $(1); \
		echo "CONFIG_WIFI_MT7603E=m" >>$(1); \
		sed -i "/CONFIG_UAPSD/d" $(1); \
		echo "CONFIG_UAPSD=y" >>$(1); \
		sed -i "/CONFIG_MT_MAC/d" $(1); \
		echo "CONFIG_MT_MAC=y" >>$(1); \
		sed -i "/CONFIG_RLT_MAC/d" $(1); \
		echo "#CONFIG_RLT_MAC is not set" >>$(1); \
		sed -i "/CONFIG_RLT_AP_SUPPORT/d" $(1); \
		echo "#CONFIG_RLT_AP_SUPPORT is not set" >>$(1); \
		sed -i "/CONFIG_SNIFFER_SUPPORT/d" $(1); \
		echo "CONFIG_SNIFFER_SUPPORT=y" >>$(1); \
		sed -i "/CONFIG_ACL_V2_SUPPORT/d" $(1); \
		echo "CONFIG_ACL_V2_SUPPORT=y" >>$(1); \
		sed -i "/CONFIG_SNIFFER_MIB_CMD/d" $(1); \
		echo "# CONFIG_SNIFFER_MIB_CMD is not set" >>$(1); \
		sed -i "/CONFIG_ALL_NET_EVENT/d" $(1); \
		echo "# CONFIG_ALL_NET_EVENT is not set" >>$(1); \
		sed -i "/CONFIG_MCAST_RATE_SPECIFIC/d" $(1); \
		echo "# CONFIG_MCAST_RATE_SPECIFIC is not set" >>$(1); \
		sed -i "/CONFIG_BTCOEX_CONCURRENT/d" $(1); \
		echo "# CONFIG_BTCOEX_CONCURRENT is not set" >>$(1); \
		if [ "$(RTN56UB1)" = "y" ] || [ "$(RTN56UB2)" = "y" ] ; then \
			if [ "$(MT7603_INTERNAL_PA_EXTERNAL_LNA)" = "y" ] ; then \
				sed -i "/CONFIG_MT7603E_INTERNAL_PA_EXTERNAL_LNA/d" $(1); \
				echo "CONFIG_MT7603E_INTERNAL_PA_EXTERNAL_LNA=y" >>$(1); \
			else \
				sed -i "/CONFIG_MT7603E_INTERNAL_PA_EXTERNAL_LNA/d" $(1); \
				echo "# CONFIG_MT7603E_INTERNAL_PA_EXTERNAL_LNA is not set" >>$(1); \
			fi; \
			if [ "$(MT7603_EXTERNAL_PA_EXTERNAL_LNA)" = "y" ] ; then \
				sed -i "/CONFIG_MT7603E_EXTERNAL_PA_EXTERNAL_LNA/d" $(1); \
				echo "CONFIG_MT7603E_EXTERNAL_PA_EXTERNAL_LNA=y" >>$(1); \
			else \
				sed -i "/CONFIG_MT7603E_EXTERNAL_PA_EXTERNAL_LNA/d" $(1); \
				echo "# CONFIG_MT7603E_EXTERNAL_PA_EXTERNAL_LNA is not set" >>$(1); \
			fi; \
		fi; \
	fi; \
	if [ "$(SECOND_IF)" = "MT7612E" ] ; then \
		sed -i "/CONFIG_RLT_WIFI/d" $(1); \
		echo "CONFIG_RLT_WIFI=m" >>$(1); \
		sed -i "/CONFIG_WIFI_MT7612E/d" $(1); \
		echo "CONFIG_WIFI_MT7612E=m" >>$(1); \
		sed -i "/CONFIG_FIRST_IF_EEPROM_PROM/d" $(1); \
		echo "# CONFIG_FIRST_IF_EEPROM_PROM is not set" >>$(1); \
		sed -i "/CONFIG_FIRST_IF_EEPROM_EFUSE/d" $(1); \
		echo "# CONFIG_FIRST_IF_EEPROM_EFUSE is not set" >>$(1); \
		sed -i "/CONFIG_FIRST_IF_EEPROM_FLASH/d" $(1); \
		echo "CONFIG_FIRST_IF_EEPROM_FLASH=y" >>$(1); \
		sed -i "/CONFIG_RT_FIRST_CARD_EEPROM/d" $(1); \
		echo "CONFIG_RT_FIRST_CARD_EEPROM=\"flash\"" >>$(1); \
		sed -i "/CONFIG_SECOND_IF_EEPROM_PROM/d" $(1); \
		echo "# CONFIG_SECOND_IF_EEPROM_PROM is not set" >>$(1); \
		sed -i "/CONFIG_SECOND_IF_EEPROM_EFUSE/d" $(1); \
		echo "# CONFIG_SECOND_IF_EEPROM_EFUSE is not set" >>$(1); \
		sed -i "/CONFIG_SECOND_IF_EEPROM_FLASH/d" $(1); \
		echo "CONFIG_SECOND_IF_EEPROM_FLASH=y" >>$(1); \
		sed -i "/CONFIG_RT_SECOND_CARD_EEPROM/d" $(1); \
		echo "CONFIG_RT_SECOND_CARD_EEPROM=\"flash\"" >>$(1); \
		sed -i "/CONFIG_MULTI_INF_SUPPORT/d" $(1); \
		echo "CONFIG_MULTI_INF_SUPPORT=y" >>$(1); \
		sed -i "/CONFIG_WIFI_BASIC_FUNC/d" $(1); \
		echo "CONFIG_WIFI_BASIC_FUNC=y" >>$(1); \
		sed -i "/CONFIG_WSC_INCLUDED/d" $(1); \
		echo "CONFIG_WSC_INCLUDED=y" >>$(1); \
		sed -i "/CONFIG_WSC_V2_SUPPORT/d" $(1); \
		echo "CONFIG_WSC_V2_SUPPORT=y" >>$(1); \
		sed -i "/CONFIG_WSC_NFC_SUPPORT/d" $(1); \
		echo "# CONFIG_WSC_NFC_SUPPORT is not set" >>$(1); \
		sed -i "/CONFIG_DOT11N_DRAFT3/d" $(1); \
		echo "CONFIG_DOT11N_DRAFT3=y" >>$(1); \
		sed -i "/CONFIG_DOT11_VHT_AC/d" $(1); \
		echo "CONFIG_DOT11_VHT_AC=y" >>$(1); \
		sed -i "/CONFIG_DOT11W_PMF_SUPPORT/d" $(1); \
		echo "# CONFIG_DOT11W_PMF_SUPPORT is not set" >>$(1); \
		sed -i "/CONFIG_TXBF_SUPPORT/d" $(1); \
		echo "# CONFIG_TXBF_SUPPORT is not set" >>$(1); \
		sed -i "/CONFIG_LLTD_SUPPORT/d" $(1); \
		echo "CONFIG_LLTD_SUPPORT=y" >>$(1); \
		sed -i "/CONFIG_QOS_DLS_SUPPORT/d" $(1); \
		echo "CONFIG_QOS_DLS_SUPPORT=y" >>$(1); \
		sed -i "/CONFIG_WAPI_SUPPORT/d" $(1); \
		echo "# CONFIG_WAPI_SUPPORT is not set" >>$(1); \
		if [ "$(RTAC1200HP)" = "y" ] ; then \
			sed -i "/CONFIG_CARRIER_DETECTION_SUPPORT/d" $(1); \
			echo "CONFIG_CARRIER_DETECTION_SUPPORT=y" >>$(1); \
		else \
			sed -i "/CONFIG_CARRIER_DETECTION_SUPPORT/d" $(1); \
			echo "# CONFIG_CARRIER_DETECTION_SUPPORT is not set" >>$(1); \
		fi; \
		sed -i "/CONFIG_ED_MONITOR_SUPPORT/d" $(1); \
		echo "# CONFIG_ED_MONITOR_SUPPORT is not set" >>$(1); \
		sed -i "/CONFIG_IGMP_SNOOP_SUPPORT/d" $(1); \
		echo "CONFIG_IGMP_SNOOP_SUPPORT=y" >>$(1); \
		sed -i "/CONFIG_BLOCK_NET_IF/d" $(1); \
		echo "CONFIG_BLOCK_NET_IF=y" >>$(1); \
		sed -i "/CONFIG_RATE_ADAPTION/d" $(1); \
		echo "CONFIG_RATE_ADAPTION=y" >>$(1); \
		sed -i "/CONFIG_NEW_RATE_ADAPT_SUPPORT/d" $(1); \
		echo "CONFIG_NEW_RATE_ADAPT_SUPPORT=y" >>$(1); \
		sed -i "/CONFIG_AGS_SUPPORT/d" $(1); \
		echo "# CONFIG_AGS_SUPPORT is not set" >>$(1); \
		sed -i "/CONFIG_IDS_SUPPORT/d" $(1); \
		echo "# CONFIG_IDS_SUPPORT is not set" >>$(1); \
		sed -i "/CONFIG_WIFI_WORK_QUEUE/d" $(1); \
		echo "# CONFIG_WIFI_WORK_QUEUE is not set" >>$(1); \
		sed -i "/CONFIG_WIFI_SKB_RECYCLE/d" $(1); \
		echo "# CONFIG_WIFI_SKB_RECYCLE is not set " >>$(1); \
		sed -i "/CONFIG_RTMP_FLASH_SUPPORT/d" $(1); \
		echo "CONFIG_RTMP_FLASH_SUPPORT=y" >>$(1); \
		sed -i "/CONFIG_LED_CONTROL_SUPPORT/d" $(1); \
		echo "CONFIG_LED_CONTROL_SUPPORT=y" >>$(1); \
		sed -i "/CONFIG_SINGLE_SKU_V2/d" $(1); \
		echo "CONFIG_SINGLE_SKU_V2=y" >>$(1); \
		sed -i "/CONFIG_ATE_SUPPORT/d" $(1); \
		echo "CONFIG_ATE_SUPPORT=y" >>$(1); \
		sed -i "/CONFIG_RT2860V2_AP_32B_DESC/d" $(1); \
		echo "# CONFIG_RT2860V2_AP_32B_DESC is not set " >>$(1); \
		sed -i "/CONFIG_HOTSPOT/d" $(1); \
		echo "# CONFIG_HOTSPOT is not set" >>$(1); \
		sed -i "/CONFIG_CO_CLOCK_SUPPORT/d" $(1); \
		echo "# CONFIG_CO_CLOCK_SUPPORT is not set" >>$(1); \
		if [ "$(RTN56UB1)" = "y" ] || [ "$(RTN56UB2)" = "y" ] ; then \
			if [ "$(MT7603_EXTERNAL_PA_EXTERNAL_LNA)" = "y" ] ; then \
				sed -i "/CONFIG_FIRST_CARD_EXTERNAL_PA/d" $(1); \
				echo "CONFIG_FIRST_CARD_EXTERNAL_PA=y" >>$(1); \
				sed -i "/CONFIG_FIRST_CARD_EXTERNAL_LNA/d" $(1); \
				echo "CONFIG_FIRST_CARD_EXTERNAL_LNA=y" >>$(1); \
			else \
				sed -i "/CONFIG_FIRST_CARD_EXTERNAL_PA/d" $(1); \
				echo "# CONFIG_FIRST_CARD_EXTERNAL_PA is not set" >>$(1); \
				sed -i "/CONFIG_FIRST_CARD_EXTERNAL_LNA/d" $(1); \
				echo "# CONFIG_FIRST_CARD_EXTERNAL_LNA is not set " >>$(1); \
			fi; \
		else \
				sed -i "/CONFIG_FIRST_CARD_EXTERNAL_PA/d" $(1); \
				echo "# CONFIG_FIRST_CARD_EXTERNAL_PA is not set" >>$(1); \
				sed -i "/CONFIG_FIRST_CARD_EXTERNAL_LNA/d" $(1); \
				echo "# CONFIG_FIRST_CARD_EXTERNAL_LNA is not set " >>$(1); \
		fi; \
		if [ "$(RTAC1200HP)" = "y" ] ; then \
			sed -i "/CONFIG_SECOND_CARD_EXTERNAL_PA/d" $(1); \
			echo "CONFIG_SECOND_CARD_EXTERNAL_PA=y" >>$(1); \
			sed -i "/CONFIG_SECOND_CARD_EXTERNAL_LNA/d" $(1); \
			echo "CONFIG_SECOND_CARD_EXTERNAL_LNA=y" >>$(1); \
			sed -i "/CONFIG_RLT_MAC/d" $(1); \
			echo "CONFIG_RLT_MAC=y" >>$(1); \
		else \
			sed -i "/CONFIG_SECOND_CARD_EXTERNAL_PA/d" $(1); \
			echo "# CONFIG_SECOND_CARD_EXTERNAL_PA is not set " >>$(1); \
			sed -i "/CONFIG_SECOND_CARD_EXTERNAL_LNA/d" $(1); \
			echo "# CONFIG_SECOND_CARD_EXTERNAL_LNA is not set" >>$(1); \
			sed -i "/CONFIG_RLT_MAC/d" $(1); \
			echo "#CONFIG_RLT_MAC is not set" >>$(1); \
		fi; \
		if [ "$(RTN56UB1)" = "y" ] ||  [ "$(RTN56UB2)" = "y" ] ; then \
			sed -i "/CONFIG_RA_HW_NAT_IPV6/d" $(1); \
			echo "CONFIG_RA_HW_NAT_IPV6=y" >>$(1); \
			sed -i "/CONFIG_RAETH_8023AZ_EEE/d" $(1); \
			echo "CONFIG_RAETH_8023AZ_EEE=y" >>$(1); \
			sed -i "/CONFIG_DFS_SUPPORT/d" $(1); \
			echo "CONFIG_DFS_SUPPORT=y" >>$(1); \
		else \
			sed -i "/CONFIG_DFS_SUPPORT/d" $(1); \
			echo "# CONFIG_DFS_SUPPORT is not set" >>$(1); \
			sed -i "/CONFIG_RAETH_8023AZ_EEE/d" $(1); \
			echo "# CONFIG_RAETH_8023AZ_EEE is not set" >>$(1); \
		fi; \
		sed -i "/CONFIG_MEMORY_OPTIMIZATION/d" $(1); \
		echo "# CONFIG_MEMORY_OPTIMIZATION is not set" >>$(1); \
		sed -i "/CONFIG_RTMP_MAC/d" $(1); \
		echo "CONFIG_RTMP_MAC=y" >>$(1); \
		sed -i "/CONFIG_WIFI_MODE_AP/d" $(1); \
		echo "# CONFIG_WIFI_MODE_AP is not set" >>$(1); \
		sed -i "/CONFIG_RTMP_PCI_SUPPORT/d" $(1); \
		echo "CONFIG_RTMP_PCI_SUPPORT=y " >>$(1); \
		sed -i "/CONFIG_WIFI_MODE_AP/d" $(1); \
		echo "CONFIG_WIFI_MODE_AP=y " >>$(1); \
		sed -i "/CONFIG_WIFI_MODE_STA/d" $(1); \
		echo "# CONFIG_WIFI_MODE_STA is not set" >>$(1); \
		sed -i "/CONFIG_WIFI_MODE_BOTH/d" $(1); \
		echo "# CONFIG_WIFI_MODE_BOTH is not set" >>$(1); \
		sed -i "/CONFIG_RLT_AP_SUPPORT/d" $(1); \
		echo "CONFIG_RLT_AP_SUPPORT=m" >>$(1); \
		sed -i "/CONFIG_WDS_SUPPORT/d" $(1); \
		echo "CONFIG_WDS_SUPPORT=y" >>$(1); \
		sed -i "/CONFIG_MBSS_SUPPORT/d" $(1); \
		echo "CONFIG_MBSS_SUPPORT=y" >>$(1); \
		sed -i "/CONFIG_APCLI_SUPPORT/d" $(1); \
		echo "# CONFIG_APCLI_SUPPORT is not set" >>$(1); \
		sed -i "/CONFIG_NINTENDO_AP/d" $(1); \
		echo "# CONFIG_NINTENDO_AP is not set" >>$(1); \
		sed -i "/CONFIG_COC_SUPPORT/d" $(1); \
		echo "CONFIG_COC_SUPPORT=y " >>$(1); \
		sed -i "/CONFIG_DELAYED_TCP_ACK_SUPPORT/d" $(1); \
		echo "# CONFIG_DELAYED_TCP_ACK_SUPPORT is not set" >>$(1); \
		sed -i "/CONFIG_RALINK_RT28XX/d" $(1); \
		echo "# CONFIG_RALINK_RT28XX is not set" >>$(1); \
		sed -i "/CONFIG_RALINK_RT3092/d" $(1); \
		echo "# CONFIG_RALINK_RT3092 is not set" >>$(1); \
		sed -i "/CONFIG_RALINK_RT3572/d" $(1); \
		echo "# CONFIG_RALINK_RT3572 is not set" >>$(1); \
		sed -i "/CONFIG_RALINK_RT5392/d" $(1); \
		echo "# CONFIG_RALINK_RT5392 is not set " >>$(1); \
		sed -i "/CONFIG_RALINK_RT5572/d" $(1); \
		echo "# CONFIG_RALINK_RT5572 is not set" >>$(1); \
		sed -i "/CONFIG_RALINK_RT5592/d" $(1); \
		echo "# CONFIG_RALINK_RT5592 is not set" >>$(1); \
		sed -i "/CONFIG_RALINK_RT6352/d" $(1); \
		echo "# CONFIG_RALINK_RT6352 is not set" >>$(1); \
		sed -i "/CONFIG_RALINK_MT7610E/d" $(1); \
		echo "# CONFIG_RALINK_MT7610E is not set" >>$(1); \
		sed -i "/CONFIG_RALINK_MT7610U/d" $(1); \
		echo "# CONFIG_RALINK_MT7610U is not set" >>$(1); \
		sed -i "/CONFIG_RALINK_RT8592/d" $(1); \
		echo "# CONFIG_RALINK_RT8592 is not set" >>$(1); \
		sed -i "/CONFIG_RALINK_MT7612E/d" $(1); \
		echo "CONFIG_RALINK_MT7612E=y" >>$(1); \
		sed -i "/CONFIG_RALINK_MT7612U/d" $(1); \
		echo "# CONFIG_RALINK_MT7612U is not set" >>$(1); \
		sed -i "/CONFIG_RTDEV/d" $(1); \
		echo "CONFIG_RTDEV=y" >>$(1); \
		sed -i "/CONFIG_RA_NAT_NONE/d" $(1); \
		echo "# CONFIG_RA_NAT_NONE is not set" >>$(1); \
		sed -i "/CONFIG_RA_NAT_HW/d" $(1); \
		echo "CONFIG_RA_NAT_HW=y" >>$(1); \
	fi; \
	if [ "$(REPEATER)" = "y" ] ; then \
			sed -i "/CONFIG_RT2860V2_AP_APCLI/d" $(1); \
			echo "CONFIG_RT2860V2_AP_APCLI=y" >>$(1); \
			sed -i "/CONFIG_RT2860V2_AP_MAC_REPEATER/d" $(1); \
			echo "CONFIG_RT2860V2_AP_MAC_REPEATER=y" >>$(1); \
		if [ "$(SECOND_IF)" = "MT7610E" ] ; then \
			sed -i "/CONFIG_MT7610_AP_APCLI/d" $(1); \
			echo "CONFIG_MT7610_AP_APCLI=y" >>$(1); \
			sed -i "/CONFIG_MT7610_AP_MAC_REPEATER/d" $(1); \
			echo "CONFIG_MT7610_AP_MAC_REPEATER=y" >>$(1); \
			sed -i "/CONFIG_M7610_CON_WPS_SUPPORT/d" $(1); \
			echo "# CONFIG_M7610_CON_WPS_SUPPORT is not set" >>$(1); \
		fi; \
		if [ "$(SECOND_IF)" = "MT7612E" ] ; then \
			sed -i "/CONFIG_APCLI_SUPPORT/d" $(1); \
			echo "CONFIG_APCLI_SUPPORT=y" >>$(1); \
			sed -i "/CONFIG_MAC_REPEATER_SUPPORT/d" $(1); \
			echo "# CONFIG_MAC_REPEATER_SUPPORT is not set" >>$(1); \
			sed -i "/CONFIG_APCLI_CERT_SUPPORT/d" $(1); \
			echo "# CONFIG_APCLI_CERT_SUPPORT is not set" >>$(1); \
		fi; \
		if [ "$(FIRST_IF)" = "MT7615E" ] || [ "$(SECOND_IF)" = "MT7615E" ] ; then \
			sed -i "/CONFIG_APCLI_SUPPORT/d" $(1); \
			echo "CONFIG_APCLI_SUPPORT=y" >>$(1); \
			sed -i "/CONFIG_MAC_REPEATER_SUPPORT/d" $(1); \
			echo "CONFIG_MAC_REPEATER_SUPPORT=y" >>$(1); \
			sed -i "/CONFIG_APCLI_CERT_SUPPORT/d" $(1); \
			echo "CONFIG_APCLI_CERT_SUPPORT=y" >>$(1); \
		fi; \
		if [ "$(RPAC87)" = "y" ] ; then \
			sed -i "/CONFIG_RA_NAT_HW/d" $(1); \
			echo "# CONFIG_RA_NAT_HW is not set" >>$(1); \
			sed -i "/CONFIG_RAETH_GMAC2/d" $(1); \
			echo "# CONFIG_RAETH_GMAC2 is not set" >>$(1); \
			sed -i "/CONFIG_SCSI_PROC_FS/d" $(1); \
			echo "# CONFIG_SCSI_PROC_FS is not set" >>$(1); \
			sed -i "/CONFIG_BLK_DEV_SD/d" $(1); \
			echo "# CONFIG_BLK_DEV_SD is not set" >>$(1); \
			sed -i "/CONFIG_BLK_DEV_SR/d" $(1); \
			echo "# CONFIG_BLK_DEV_SR is not set" >>$(1); \
			sed -i "/CONFIG_CHR_DEV_SG/d" $(1); \
			echo "# CONFIG_CHR_DEV_SG is not set" >>$(1); \
			sed -i "/CONFIG_SCSI_MULTI_LUN/d" $(1); \
			echo "# CONFIG_SCSI_MULTI_LUN is not set" >>$(1); \
			sed -i "/CONFIG_SCSI_CONSTANTS/d" $(1); \
			echo "# CONFIG_SCSI_CONSTANTS is not set" >>$(1); \
			sed -i "/CONFIG_SCSI_LOWLEVEL/d" $(1); \
			echo "# CONFIG_SCSI_LOWLEVEL is not set" >>$(1); \
			sed -i "/CONFIG_PPP/d" $(1); \
			echo "# CONFIG_PPP is not set" >>$(1); \
			sed -i "/CONFIG_USB_USBNET/d" $(1); \
			echo "# CONFIG_USB_USBNET is not set" >>$(1); \
			sed -i "/CONFIG_USB_HSO/d" $(1); \
			echo "# CONFIG_USB_HSO is not set" >>$(1); \
			sed -i "/CONFIG_RALINK_SPDIF/d" $(1); \
			echo "# CONFIG_RALINK_SPDIF is not set" >>$(1); \
			sed -i "/CONFIG_USB_DEVICEFS/d" $(1); \
			echo "# CONFIG_USB_DEVICEFS is not set" >>$(1); \
			sed -i "/CONFIG_USB_BUS_STATS/d" $(1); \
			echo "# CONFIG_USB_BUS_STATS is not set" >>$(1); \
			sed -i "/CONFIG_USB_XHCI_HCD_DEBUGGING/d" $(1); \
			echo "# CONFIG_USB_XHCI_HCD_DEBUGGING is not set" >>$(1); \
			sed -i "/CONFIG_USB_ACM/d" $(1); \
			echo "# CONFIG_USB_ACM is not set" >>$(1); \
			sed -i "/CONFIG_USB_PRINTER/d" $(1); \
			echo "# CONFIG_USB_PRINTER is not set" >>$(1); \
			sed -i "/CONFIG_USB_WDM/d" $(1); \
			echo "# CONFIG_USB_WDM is not set" >>$(1); \
			sed -i "/CONFIG_USB_SERIAL/d" $(1); \
			echo "# CONFIG_USB_SERIAL is not set" >>$(1); \
			sed -i "/CONFIG_DMADEVICES/d" $(1); \
			echo "# CONFIG_DMADEVICES is not set" >>$(1); \
		fi; \
	fi; \
	if [ "$(RA_SKU)" = "y" ] || [ "$(RA_SKU_IN_DRV)" = "y" ] ; then \
			sed -i "/CONFIG_RT2860V2_SINGLE_SKU/d" $(1); \
			echo "CONFIG_RT2860V2_SINGLE_SKU=y" >>$(1); \
		if [ "$(SECOND_IF)" = "MT7610E" ] ; then \
			sed -i "/CONFIG_MT7610_AP_SINGLE_SKU/d" $(1); \
			echo "CONFIG_MT7610_AP_SINGLE_SKU=y" >>$(1); \
		fi; \
		if [ "$(MT7628)" = "y" ] ; then \
			sed -i "/CONFIG_MT_SINGLE_SKU_V2/d" $(1); \
			echo "CONFIG_MT_SINGLE_SKU_V2=y" >>$(1); \
		fi; \
		if [ "$(RA_SKU_IN_DRV)" = "y" ] ; then \
			sed -i "/CONFIG_SINGLE_SKU_IN_DRIVER/d" $(1); \
			echo "CONFIG_SINGLE_SKU_IN_DRIVER=y" >>$(1); \
		fi; \
	fi; \
	if [ "$(RTAC1200HP)" = "y" ] ; then \
			sed -i "/CONFIG_RT2860V2_AP_CARRIER/d" $(1); \
			echo "CONFIG_RT2860V2_AP_CARRIER=y" >>$(1); \
	fi; \
	if [ "$(RTAC85P)" = "y" ] ; then \
			sed -i "/CONFIG_NF_CT_NETLINK/d" $(1); \
			echo "CONFIG_NF_CT_NETLINK=m" >>$(1); \
			sed -i "/CONFIG_NF_CT_NETLINK_TIMEOUT/d" $(1); \
			echo "CONFIG_NF_CT_NETLINK_TIMEOUT=m" >>$(1); \
			sed -i "/CONFIG_NETFILTER_TPROXY/d" $(1); \
			echo "CONFIG_NETFILTER_TPROXY=m" >>$(1); \
	fi; \
	if [ "$(UBI)" = "y" ]; then \
		sed -i "/CONFIG_MTD_UBI\>/d" $(1); \
		echo "CONFIG_MTD_UBI=y" >>$(1); \
		sed -i "/CONFIG_MTD_UBI_WL_THRESHOLD/d" $(1); \
		echo "CONFIG_MTD_UBI_WL_THRESHOLD=4096" >>$(1); \
		sed -i "/CONFIG_MTD_UBI_BEB_RESERVE/d" $(1); \
		echo "CONFIG_MTD_UBI_BEB_RESERVE=1" >>$(1); \
		sed -i "/CONFIG_MTD_UBI_GLUEBI/d" $(1); \
		echo "CONFIG_MTD_UBI_GLUEBI=y" >>$(1); \
		sed -i "/CONFIG_FACTORY_CHECKSUM/d" $(1); \
		echo "CONFIG_FACTORY_CHECKSUM=y" >>$(1); \
		if [ "$(UBI_DEBUG)" = "y" ]; then \
			sed -i "/CONFIG_MTD_UBI_DEBUG/d" $(1); \
			echo "CONFIG_MTD_UBI_DEBUG=y" >>$(1); \
			sed -i "/CONFIG_GCOV_KERNEL/d" $(1); \
			echo "# CONFIG_GCOV_KERNEL is not set" >>$(1); \
			sed -i "/CONFIG_L2TP_DEBUGFS/d" $(1); \
			echo "# CONFIG_L2TP_DEBUGFS is not set" >>$(1); \
			sed -i "/CONFIG_MTD_UBI_DEBUG_MSG/d" $(1); \
			echo "CONFIG_MTD_UBI_DEBUG_MSG=y" >>$(1); \
			sed -i "/CONFIG_MTD_UBI_DEBUG_PARANOID/d" $(1); \
			echo "# CONFIG_MTD_UBI_DEBUG_PARANOID is not set" >>$(1); \
			sed -i "/CONFIG_MTD_UBI_DEBUG_DISABLE_BGT/d" $(1); \
			echo "# CONFIG_MTD_UBI_DEBUG_DISABLE_BGT is not set" >>$(1); \
			sed -i "/CONFIG_MTD_UBI_DEBUG_EMULATE_BITFLIPS/d" $(1); \
			echo "CONFIG_MTD_UBI_DEBUG_EMULATE_BITFLIPS=y" >>$(1); \
			sed -i "/CONFIG_MTD_UBI_DEBUG_EMULATE_WRITE_FAILURES/d" $(1); \
			echo "CONFIG_MTD_UBI_DEBUG_EMULATE_WRITE_FAILURES=y" >>$(1); \
			sed -i "/CONFIG_MTD_UBI_DEBUG_EMULATE_ERASE_FAILURES/d" $(1); \
			echo "CONFIG_MTD_UBI_DEBUG_EMULATE_ERASE_FAILURES=y" >>$(1); \
			sed -i "/CONFIG_MTD_UBI_DEBUG_MSG_BLD/d" $(1); \
			echo "CONFIG_MTD_UBI_DEBUG_MSG_BLD=y" >>$(1); \
			sed -i "/CONFIG_MTD_UBI_DEBUG_MSG_EBA/d" $(1); \
			echo "CONFIG_MTD_UBI_DEBUG_MSG_EBA=y" >>$(1); \
			sed -i "/CONFIG_MTD_UBI_DEBUG_MSG_WL/d" $(1); \
			echo "CONFIG_MTD_UBI_DEBUG_MSG_WL=y" >>$(1); \
			sed -i "/CONFIG_MTD_UBI_DEBUG_MSG_IO/d" $(1); \
			echo "CONFIG_MTD_UBI_DEBUG_MSG_IO=y" >>$(1); \
			sed -i "/CONFIG_JBD_DEBUG/d" $(1); \
			echo "# CONFIG_JBD_DEBUG is not set" >>$(1); \
			sed -i "/CONFIG_LKDTM/d" $(1); \
			echo "# CONFIG_LKDTM is not set" >>$(1); \
			sed -i "/CONFIG_DYNAMIC_DEBUG/d" $(1); \
			echo "CONFIG_DYNAMIC_DEBUG=y" >>$(1); \
			sed -i "/CONFIG_SPINLOCK_TEST/d" $(1); \
			echo "# CONFIG_SPINLOCK_TEST is not set" >>$(1); \
		else \
			sed -i "/CONFIG_MTD_UBI_DEBUG/d" $(1); \
			echo "# CONFIG_MTD_UBI_DEBUG is not set" >>$(1); \
		fi; \
		if [ "$(UBIFS)" = "y" ]; then \
			sed -i "/CONFIG_UBIFS_FS/d" $(1); \
			echo "CONFIG_UBIFS_FS=y" >>$(1); \
			sed -i "/CONFIG_UBIFS_FS_XATTR/d" $(1); \
			echo "# CONFIG_UBIFS_FS_XATTR is not set" >>$(1); \
			sed -i "/CONFIG_UBIFS_FS_ADVANCED_COMPR/d" $(1); \
			echo "CONFIG_UBIFS_FS_ADVANCED_COMPR=y" >>$(1); \
			sed -i "/CONFIG_UBIFS_FS_LZO/d" $(1); \
			echo "CONFIG_UBIFS_FS_LZO=y" >>$(1); \
			sed -i "/CONFIG_UBIFS_FS_ZLIB/d" $(1); \
			echo "CONFIG_UBIFS_FS_ZLIB=y" >>$(1); \
			sed -i "/CONFIG_UBIFS_FS_DEBUG/d" $(1); \
			echo "# CONFIG_UBIFS_FS_DEBUG is not set" >>$(1); \
		else \
			sed -i "/CONFIG_UBIFS_FS/d" $(1); \
			echo "# CONFIG_UBIFS_FS is not set" >>$(1); \
		fi; \
	fi; \
	if [ "$(DSL)" = "y" ]; then \
		sed -i "/CONFIG_RTL8367M/d" $(1); \
		echo "# CONFIG_RTL8367M is not set" >>$(1); \
		sed -i "/CONFIG_RTL8367R/d" $(1); \
		echo "CONFIG_RTL8367R=y" >>$(1); \
		sed -i "/CONFIG_RAETH_GMAC2/d" $(1); \
		echo "# CONFIG_RAETH_GMAC2 is not set" >>$(1); \
		sed -i "/CONFIG_GE2_RGMII_FORCE_1000/d" $(1); \
		echo "# CONFIG_GE2_RGMII_FORCE_1000 is not set" >>$(1); \
		sed -i "/CONFIG_RAETH_DSL/d" $(1); \
		echo "CONFIG_RAETH_DSL=y" >>$(1); \
		sed -i "/CONFIG_RA_HW_NAT/d" $(1); \
		echo "# CONFIG_RA_HW_NAT is not set" >>$(1); \
		sed -i "/CONFIG_RA_HW_NAT_AUTO_BIND/d" $(1); \
		echo "# CONFIG_RA_HW_NAT_AUTO_BIND is not set" >>$(1); \
		sed -i "/CONFIG_RA_HW_NAT_MANUAL_BIND/d" $(1); \
		echo "# CONFIG_RA_HW_NAT_MANUAL_BIND is not set" >>$(1); \
		sed -i "/CONFIG_RA_HW_NAT_SPEEDUP_UPSTREAM/d" $(1); \
		echo "# CONFIG_RA_HW_NAT_SPEEDUP_UPSTREAM is not set" >>$(1); \
		sed -i "/CONFIG_RA_HW_NAT_SPEEDUP_DOWNSTREAM/d" $(1); \
		echo "# CONFIG_RA_HW_NAT_SPEEDUP_DOWNSTREAM is not set" >>$(1); \
		sed -i "/CONFIG_RA_HW_NAT_SPEEDUP_BIDIRECTION/d" $(1); \
		echo "# CONFIG_RA_HW_NAT_SPEEDUP_BIDIRECTION is not set" >>$(1); \
		sed -i "/CONFIG_RA_HW_NAT_LAN_VLANID/d" $(1); \
		echo "# CONFIG_RA_HW_NAT_LAN_VLANID is not set" >>$(1); \
		sed -i "/CONFIG_RA_HW_NAT_WAN_VLANID/d" $(1); \
		echo "# CONFIG_RA_HW_NAT_WAN_VLANID is not set" >>$(1); \
		sed -i "/CONFIG_RA_HW_NAT_BINDING_THRESHOLD/d" $(1); \
		echo "# CONFIG_RA_HW_NAT_BINDING_THRESHOLD is not set" >>$(1); \
		sed -i "/CONFIG_RA_HW_NAT_QURT_LMT/d" $(1); \
		echo "# CONFIG_RA_HW_NAT_QURT_LMT is not set" >>$(1); \
		sed -i "/CONFIG_RA_HW_NAT_HALF_LMT/d" $(1); \
		echo "# CONFIG_RA_HW_NAT_HALF_LMT is not set" >>$(1); \
		sed -i "/CONFIG_RA_HW_NAT_FULL_LMT/d" $(1); \
		echo "# CONFIG_RA_HW_NAT_FULL_LMT is not set" >>$(1); \
		sed -i "/CONFIG_RA_HW_NAT_TBL_1K/d" $(1); \
		echo "# CONFIG_RA_HW_NAT_TBL_1K is not set" >>$(1); \
		sed -i "/CONFIG_RA_HW_NAT_TBL_2K/d" $(1); \
		echo "# CONFIG_RA_HW_NAT_TBL_2K is not set" >>$(1); \
		sed -i "/CONFIG_RA_HW_NAT_TBL_4K/d" $(1); \
		echo "# CONFIG_RA_HW_NAT_TBL_4K is not set" >>$(1); \
		sed -i "/CONFIG_RA_HW_NAT_TBL_8K/d" $(1); \
		echo "# CONFIG_RA_HW_NAT_TBL_8K is not set" >>$(1); \
		sed -i "/CONFIG_RA_HW_NAT_TBL_16K/d" $(1); \
		echo "# CONFIG_RA_HW_NAT_TBL_16K is not set" >>$(1); \
		sed -i "/CONFIG_RA_HW_NAT_HASH0/d" $(1); \
		echo "# CONFIG_RA_HW_NAT_HASH0 is not set" >>$(1); \
		sed -i "/CONFIG_RA_HW_NAT_HASH1/d" $(1); \
		echo "# CONFIG_RA_HW_NAT_HASH1 is not set" >>$(1); \
		sed -i "/CONFIG_RA_HW_NAT_PRE_ACL_SIZE/d" $(1); \
		echo "# CONFIG_RA_HW_NAT_PRE_ACL_SIZE is not set" >>$(1); \
		sed -i "/CONFIG_RA_HW_NAT_PRE_MTR_SIZE/d" $(1); \
		echo "# CONFIG_RA_HW_NAT_PRE_MTR_SIZE is not set" >>$(1); \
		sed -i "/CONFIG_RA_HW_NAT_PRE_AC_SIZE/d" $(1); \
		echo "# CONFIG_RA_HW_NAT_PRE_AC_SIZE is not set" >>$(1); \
		sed -i "/CONFIG_RA_HW_NAT_POST_MTR_SIZE/d" $(1); \
		echo "# CONFIG_RA_HW_NAT_POST_MTR_SIZE is not set" >>$(1); \
		sed -i "/CONFIG_RA_HW_NAT_POST_AC_SIZE/d" $(1); \
		echo "# CONFIG_RA_HW_NAT_POST_AC_SIZE is not set" >>$(1); \
		sed -i "/CONFIG_RA_HW_NAT_TCP_KA/d" $(1); \
		echo "# CONFIG_RA_HW_NAT_TCP_KA is not set" >>$(1); \
		sed -i "/CONFIG_RA_HW_NAT_UDP_KA/d" $(1); \
		echo "# CONFIG_RA_HW_NAT_UDP_KA is not set" >>$(1); \
		sed -i "/CONFIG_RA_HW_NAT_ACL_DLTA/d" $(1); \
		echo "# CONFIG_RA_HW_NAT_ACL_DLTA is not set" >>$(1); \
		sed -i "/CONFIG_RA_HW_NAT_UNB_DLTA/d" $(1); \
		echo "# CONFIG_RA_HW_NAT_UNB_DLTA is not set" >>$(1); \
		sed -i "/CONFIG_RA_HW_NAT_UNB_MNP/d" $(1); \
		echo "# CONFIG_RA_HW_NAT_UNB_MNP is not set" >>$(1); \
		sed -i "/CONFIG_RA_HW_NAT_UDP_DLTA/d" $(1); \
		echo "# CONFIG_RA_HW_NAT_UDP_DLTA is not set" >>$(1); \
		sed -i "/CONFIG_RA_HW_NAT_TCP_DLTA/d" $(1); \
		echo "# CONFIG_RA_HW_NAT_TCP_DLTA is not set" >>$(1); \
		sed -i "/CONFIG_RA_HW_NAT_FIN_DLTA/d" $(1); \
		echo "# CONFIG_RA_HW_NAT_FIN_DLTA is not set" >>$(1); \
		sed -i "/CONFIG_RA_HW_NAT_IPV6/d" $(1); \
		echo "# CONFIG_RA_HW_NAT_IPV6 is not set" >>$(1); \
		sed -i "/CONFIG_RA_HW_NAT_ACL2UP_HELPER/d" $(1); \
		echo "# CONFIG_RA_HW_NAT_ACL2UP_HELPER is not set" >>$(1); \
		sed -i "/CONFIG_RA_HW_NAT_DSCP2UP_HELPER/d" $(1); \
		echo "# CONFIG_RA_HW_NAT_DSCP2UP_HELPER is not set" >>$(1); \
		sed -i "/CONFIG_RA_HW_NAT_NONE2UP/d" $(1); \
		echo "# CONFIG_RA_HW_NAT_NONE2UP is not set" >>$(1); \
	fi; \
	if [ "$(RTN14U)" = "y" ] || [ "$(RTAC52U)" = "y" ] || [ "$(RTAC51U)" = "y" ] || [ "$(RTN11P)" = "y" ] || [ "$(RTAC54U)" = "y" ] || [ "$(RTN54U)" = "y" ] || [ "$(RTAC1200HP)" = "y" ]; then \
		sed -i "/CONFIG_RAETH_HW_VLAN_TX/d" $(1); \
		echo "# CONFIG_RAETH_HW_VLAN_TX is not set" >>$(1); \
		echo "# CONFIG_RA_HW_NAT_PPTP_L2TP is not set" >>$(1); \
	fi; \
	)
endef
