obj-$(CONFIG_RAETH) += raeth.o
raeth-objs := ra_mac.o mii_mgr.o ra_rfrw.o

#EXTRA_CFLAGS   += -DCONFIG_RAETH_MULTIPLE_RX_RING

ifeq ($(CONFIG_RAETH_QOS),y)
raeth-objs += ra_qos.o
endif

ifeq ($(CONFIG_RAETH_QDMA),y)
raeth-objs += raether_qdma.o
endif

ifneq ($(CONFIG_RAETH_QDMA),y)
raeth-objs += raether_pdma.o
endif

raeth-objs += raether.o

ifeq ($(CONFIG_ETHTOOL),y)
raeth-objs += ra_ethtool.o
endif

ifeq ($(CONFIG_RALINK_RT3052_MP2),y)
raeth-objs += mcast.o
endif

ifeq ($(CONFIG_RAETH_NETLINK),y)
raeth-objs += ra_netlink.o
endif

ifeq ($(CONFIG_RAETH_HW_LRO),y)
raeth-objs += raether_hwlro.o
endif

ifeq ($(CONFIG_RAETH_GMAC2),y)
EXTRA_CFLAGS += -DCONFIG_PSEUDO_SUPPORT
endif

ifeq ($(CONFIG_ETH_MEMORY_OPTIMIZATION),y)
EXTRA_CFLAGS   += -DMEMORY_OPTIMIZATION
endif

ifeq ($(CONFIG_RT2860V2_AP_MEMORY_OPTIMIZATION),y)
EXTRA_CFLAGS   += -DMEMORY_OPTIMIZATION
endif

ifeq ($(CONFIG_RA_NETWORK_WORKQUEUE_BH),y)
EXTRA_CFLAGS   += -DWORKQUEUE_BH
endif

ifeq ($(CONFIG_TASKLET_WORKQUEUE_SW),y)
EXTRA_CFLAGS   += -DTASKLET_WORKQUEUE_SW
endif
