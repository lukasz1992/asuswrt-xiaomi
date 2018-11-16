#!/bin/sh
# Copyright (c) 2008-2012 Broadcom Corporation

if [ $# -lt 1 ]; then
	echo "$0: No kernel source directory provided." 1>&2
	exit 255
fi

srcdir=$1
shift

while [ $# != 0 ]; do
	case $1 in
		TG3_NO_EEE)
			echo "#define TG3_DISABLE_EEE_SUPPORT"
			;;
		*)
			;;
	esac
	shift
done

UAPI=
if [ -d $srcdir/include/uapi ]
then
	UAPI=uapi
fi

if grep -q "netdump_mode" $srcdir/include/linux/kernel.h ; then
	echo "#define BCM_HAS_NETDUMP_MODE"
fi

if grep -q "bool" $srcdir/include/linux/types.h ; then 
	echo "#define BCM_HAS_BOOL"
fi

if grep -q "__le32" $srcdir/include/$UAPI/linux/types.h; then
	echo "#define BCM_HAS_LE32"
fi

if grep -q "resource_size_t" $srcdir/include/linux/types.h ; then
	echo "#define BCM_HAS_RESOURCE_SIZE_T"
fi

if grep -q "kzalloc" $srcdir/include/linux/slab.h ; then
	echo "#define BCM_HAS_KZALLOC"
fi

for symbol in jiffies_to_usecs usecs_to_jiffies msecs_to_jiffies; do
	if [ -f $srcdir/include/linux/jiffies.h ]; then
		if grep -q "$symbol" $srcdir/include/linux/jiffies.h ; then
			echo "#define BCM_HAS_`echo $symbol | tr '[a-z]' '[A-Z]'`"
			continue
		fi
	fi
	if [ -f $srcdir/include/linux/time.h ]; then
		if grep -q "$symbol" $srcdir/include/linux/time.h ; then
			echo "#define BCM_HAS_`echo $symbol | tr '[a-z]' '[A-Z]'`"
			continue
		fi
	fi
	if [ -f $srcdir/include/linux/delay.h ]; then
		if grep -q "$symbol" $srcdir/include/linux/delay.h ; then
			echo "#define BCM_HAS_`echo $symbol | tr '[a-z]' '[A-Z]'`"
			continue
		fi
	fi
done

if grep -q "msleep" $srcdir/include/linux/delay.h ; then
	echo "#define BCM_HAS_MSLEEP"
fi

if grep -q "msleep_interruptible" $srcdir/include/linux/delay.h ; then
	echo "#define BCM_HAS_MSLEEP_INTERRUPTIBLE"
fi

if grep -q "skb_copy_from_linear_data" $srcdir/include/linux/skbuff.h ; then
	echo "#define BCM_HAS_SKB_COPY_FROM_LINEAR_DATA"
fi

if grep -q "skb_is_gso_v6" $srcdir/include/linux/skbuff.h ; then
	echo "#define BCM_HAS_SKB_IS_GSO_V6"
fi

if grep -q "skb_checksum_none_assert" $srcdir/include/linux/skbuff.h ; then
	echo "#define BCM_HAS_SKB_CHECKSUM_NONE_ASSERT"
fi

if grep -q "skb_shared_hwtstamps" $srcdir/include/linux/skbuff.h ; then
	echo "#define BCM_KERNEL_SUPPORTS_TIMESTAMPING"
fi

if grep -q "union skb_shared_tx" $srcdir/include/linux/skbuff.h ; then
	echo "#define BCM_HAS_SKB_SHARED_TX_UNION"
fi

if grep -q "skb_tx_timestamp" $srcdir/include/linux/skbuff.h ; then
	echo "#define BCM_HAS_SKB_TX_TIMESTAMP"
fi

if grep -q "skb_frag_size" $srcdir/include/linux/skbuff.h ; then
	echo "#define BCM_HAS_SKB_FRAG_SIZE"
fi

if grep -q "skb_frag_dma_map" $srcdir/include/linux/skbuff.h ; then
	echo "#define BCM_HAS_SKB_FRAG_DMA_MAP"
fi

if grep -q "pci_pcie_cap" $srcdir/include/linux/pci.h ; then
	echo "#define BCM_HAS_PCI_PCIE_CAP"
fi

if grep -q "pci_is_pcie" $srcdir/include/linux/pci.h ; then
	echo "#define BCM_HAS_PCI_IS_PCIE"
fi

if grep -q "pci_ioremap_bar" $srcdir/include/linux/pci.h ; then
	echo "#define BCM_HAS_PCI_IOREMAP_BAR"
fi

if grep -q "pci_read_vpd" $srcdir/include/linux/pci.h ; then
	echo "#define BCM_HAS_PCI_READ_VPD"
fi

if grep -q "PCI_DEV_FLAGS_MSI_INTX_DISABLE_BUG" $srcdir/include/linux/pci.h ; then
	echo "#define BCM_HAS_INTX_MSI_WORKAROUND"
fi

if grep -q "pci_target_state" $srcdir/include/linux/pci.h ; then
	echo "#define BCM_HAS_PCI_TARGET_STATE"
fi

if grep -q "pci_choose_state" $srcdir/include/linux/pci.h ; then
	echo "#define BCM_HAS_PCI_CHOOSE_STATE"
fi

if grep -q "pci_pme_capable" $srcdir/include/linux/pci.h ; then
	echo "#define BCM_HAS_PCI_PME_CAPABLE"
fi

if grep -q "pci_enable_wake" $srcdir/include/linux/pci.h ; then
	echo "#define BCM_HAS_PCI_ENABLE_WAKE"
fi

if grep -q "pci_wake_from_d3" $srcdir/include/linux/pci.h ; then
	echo "#define BCM_HAS_PCI_WAKE_FROM_D3"
fi

if grep -q "pci_set_power_state" $srcdir/include/linux/pci.h ; then
	echo "#define BCM_HAS_PCI_SET_POWER_STATE"
fi

if grep -q "err_handler" $srcdir/include/linux/pci.h ; then
	echo "#define BCM_HAS_PCI_EEH_SUPPORT"
fi

if grep -q "busn_res" $srcdir/include/linux/pci.h ; then
	echo "#define BCM_HAS_PCI_BUSN_RES"
fi

if [ -e "$srcdir/include/linux/pm_wakeup.h" ]; then
	TGT_H="$srcdir/include/linux/pm_wakeup.h"
elif [ -e "$srcdir/include/linux/pm.h" ]; then
	TGT_H="$srcdir/include/linux/pm.h"
fi

if [ -n "$TGT_H" ]; then
	if grep -q "device_can_wakeup"        $TGT_H && \
	   grep -q "device_may_wakeup"        $TGT_H && \
	   grep -q "device_set_wakeup_enable" $TGT_H ; then
		echo "#define BCM_HAS_DEVICE_WAKEUP_API"
	fi
	if grep -q "device_set_wakeup_capable" $TGT_H ; then
		echo "#define BCM_HAS_DEVICE_SET_WAKEUP_CAPABLE"
	fi
fi

if [ -f $srcdir/include/asm-generic/pci-dma-compat.h ]; then
	TGT_H=$srcdir/include/asm-generic/pci-dma-compat.h
	num_args=`awk '/pci_dma_mapping_error/,/[;{]/ {printf $0; next}' $TGT_H | awk -F ',' '{print NF}'`
	if [ $num_args -eq 2 ]; then
		echo "#define BCM_HAS_NEW_PCI_DMA_MAPPING_ERROR"
	elif grep -q "pci_dma_mapping_error" $TGT_H ; then
		echo "#define BCM_HAS_PCI_DMA_MAPPING_ERROR"
	fi
fi

if grep -q "pcie_get_readrq" $srcdir/include/linux/pci.h ; then
	echo "#define BCM_HAS_PCIE_GET_READRQ"
fi

if grep -q "pcie_set_readrq" $srcdir/include/linux/pci.h ; then
	echo "#define BCM_HAS_PCIE_SET_READRQ"
fi

if grep -q "print_mac" $srcdir/include/linux/if_ether.h ; then
	echo "#define BCM_HAS_PRINT_MAC"
fi

# ethtool_op_set_tx_ipv6_csum() first appears in linux-2.6.23
if grep -q "ethtool_op_set_tx_ipv6_csum" $srcdir/include/linux/ethtool.h ; then
	echo "#define BCM_HAS_ETHTOOL_OP_SET_TX_IPV6_CSUM"
fi

# ethtool_op_set_tx_hw_csum() first appears in linux-2.6.12
if grep -q "ethtool_op_set_tx_hw_csum" $srcdir/include/linux/ethtool.h ; then
	echo "#define BCM_HAS_ETHTOOL_OP_SET_TX_HW_CSUM"
fi

if grep -q "ethtool_op_set_sg" $srcdir/include/linux/ethtool.h ; then
	echo "#define BCM_HAS_ETHTOOL_OP_SET_SG"
fi

if grep -q "ethtool_op_set_tso" $srcdir/include/linux/ethtool.h ; then
	echo "#define BCM_HAS_ETHTOOL_OP_SET_TSO"
fi

if grep -q "eth_tp_mdix" $srcdir/include/linux/ethtool.h ; then
	echo "#define BCM_HAS_MDIX_STATUS"
fi

if grep -q "(*set_phys_id)" $srcdir/include/linux/ethtool.h ; then
	echo "#define BCM_HAS_SET_PHYS_ID"
fi

# set_tx_csum first appears in linux-2.4.23
if grep -q "(*set_tx_csum)" $srcdir/include/linux/ethtool.h ; then
	echo "#define BCM_HAS_SET_TX_CSUM"
fi

if grep -q "ethtool_cmd_speed_set" $srcdir/include/$UAPI/linux/ethtool.h ; then
	echo "#define BCM_HAS_ETHTOOL_CMD_SPEED_SET"
fi

if grep -q "ethtool_cmd_speed(" $srcdir/include/$UAPI/linux/ethtool.h ; then
	echo "#define BCM_HAS_ETHTOOL_CMD_SPEED"
fi

if grep -q "ETH_TEST_FL_EXTERNAL_LB_DONE" $srcdir/include/linux/ethtool.h ; then
	echo "#define BCM_HAS_EXTERNAL_LB_DONE"
fi

RXNFC=`sed -ne "/get_rxnfc).*$/{ N; s/\n/d/; P }" $srcdir/include/linux/ethtool.h`
if [ ! -z "$RXNFC" ]; then
	if `echo $RXNFC | grep -q "void"` ; then
		echo "#define BCM_HAS_OLD_GET_RXNFC_SIG"
	fi 
	echo "#define BCM_HAS_GET_RXNFC"
fi

if grep -q "get_rxfh_indir" $srcdir/include/linux/ethtool.h ; then
	if grep -q "ethtool_rxfh_indir_default" $srcdir/include/linux/ethtool.h ; then
		echo "#define BCM_HAS_ETHTOOL_RXFH_INDIR_DEFAULT"
	fi
	if grep -q "get_rxfh_indir_size" $srcdir/include/linux/ethtool.h ; then
		echo "#define BCM_HAS_GET_RXFH_INDIR_SIZE"
	fi
	echo "#define BCM_HAS_GET_RXFH_INDIR"
fi

if grep -q "lp_advertising" $srcdir/include/linux/ethtool.h ; then
	echo "#define BCM_HAS_LP_ADVERTISING"
fi

if grep -q "skb_transport_offset" $srcdir/include/linux/skbuff.h ; then
	echo "#define BCM_HAS_SKB_TRANSPORT_OFFSET"
fi

if grep -q "skb_get_queue_mapping" $srcdir/include/linux/skbuff.h ; then
	echo "#define BCM_HAS_SKB_GET_QUEUE_MAPPING"
fi

if grep -q "ip_hdr" $srcdir/include/linux/ip.h ; then
	echo "#define BCM_HAS_IP_HDR"
fi

if grep -q "ip_hdrlen" $srcdir/include/net/ip.h ; then
	echo "#define BCM_HAS_IP_HDRLEN"
fi

if grep -q "tcp_hdr" $srcdir/include/linux/tcp.h ; then
	echo "#define BCM_HAS_TCP_HDR"
fi

if grep -q "tcp_hdrlen" $srcdir/include/linux/tcp.h ; then
	echo "#define BCM_HAS_TCP_HDRLEN"
fi

if grep -q "tcp_optlen" $srcdir/include/linux/tcp.h ; then
	echo "#define BCM_HAS_TCP_OPTLEN"
fi

TGT_H=$srcdir/include/linux/netdevice.h
if grep -q "struct netdev_queue" $TGT_H ; then
	echo "#define BCM_HAS_STRUCT_NETDEV_QUEUE"
else
	num_args=`awk '/ netif_rx_complete\(struct/,/\)/ {printf $0; next}' $TGT_H | awk -F ',' '{print NF}'`
	if [ -n "$num_args" -a $num_args -eq 2 ]; then
		# Define covers netif_rx_complete, netif_rx_schedule,
		# __netif_rx_schedule, and netif_rx_schedule_prep
		echo "#define BCM_HAS_NEW_NETIF_INTERFACE"
	fi
fi

if grep -q "netif_set_real_num_tx_queues" $TGT_H ; then
	echo "#define BCM_HAS_NETIF_SET_REAL_NUM_TX_QUEUES"
fi

if grep -q "netif_set_real_num_rx_queues" $TGT_H ; then
	echo "#define BCM_HAS_NETIF_SET_REAL_NUM_RX_QUEUES"
fi

if grep -q "netdev_priv" $TGT_H ; then
	echo "#define BCM_HAS_NETDEV_PRIV"
fi

if grep -q "netdev_tx_t" $TGT_H ; then
	echo "#define BCM_HAS_NETDEV_TX_T"
fi

if grep -q "netdev_hw_addr" $TGT_H ; then
	echo "#define BCM_HAS_NETDEV_HW_ADDR"
fi

if grep -q "netdev_name" $TGT_H ; then
	echo "#define BCM_HAS_NETDEV_NAME"
fi

if grep -q "netdev_sent_queue" $TGT_H ; then
	echo "#define BCM_HAS_NETDEV_SENT_QUEUE"
fi

if grep -q "netdev_tx_sent_queue" $TGT_H ; then
	echo "#define BCM_HAS_NETDEV_TX_SENT_QUEUE"
fi

if grep -q "netdev_completed_queue" $TGT_H ; then
	echo "#define BCM_HAS_NETDEV_COMPLETED_QUEUE"
fi

if grep -q "netdev_tx_completed_queue" $TGT_H ; then
	echo "#define BCM_HAS_NETDEV_TX_COMPLETED_QUEUE"
fi

if grep -q "netdev_reset_queue" $TGT_H ; then
	echo "#define BCM_HAS_NETDEV_RESET_QUEUE"
fi

if grep -q "netdev_tx_reset_queue" $TGT_H ; then
	echo "#define BCM_HAS_NETDEV_TX_RESET_QUEUE"
fi

if grep -q "struct net_device_ops" $TGT_H ; then
	echo "#define BCM_HAS_NET_DEVICE_OPS"
fi

if grep -q "(*ndo_get_stats64)" $TGT_H ; then
	echo "#define BCM_HAS_GET_STATS64"
fi

if grep -q "(*ndo_set_multicast_list)" $TGT_H ; then
	echo "#define BCM_HAS_SET_MULTICAST_LIST"
fi

if grep -q "(*ndo_fix_features)" $TGT_H ; then
	echo "#define BCM_HAS_FIX_FEATURES"
fi

if grep -q "hw_features" $TGT_H ; then
	echo "#define BCM_HAS_HW_FEATURES"
fi

if grep -q "vlan_features" $TGT_H ; then
	echo "#define BCM_HAS_VLAN_FEATURES"
fi

if grep -q "netdev_update_features" $TGT_H ; then
	echo "#define BCM_HAS_NETDEV_UPDATE_FEATURES"
fi

if grep -q "alloc_etherdev_mq" $srcdir/include/linux/etherdevice.h ; then
	echo "#define BCM_HAS_ALLOC_ETHERDEV_MQ"
fi

if grep -q "napi_gro_receive" $TGT_H ; then
	echo "#define BCM_HAS_NAPI_GRO_RECEIVE"
fi

if grep -q "netif_tx_lock" $TGT_H ; then
	echo "#define BCM_HAS_NETIF_TX_LOCK"
fi

if grep -q "txq_trans_update" $TGT_H ; then
	echo "#define BCM_HAS_TXQ_TRANS_UPDATE"
fi

if grep -q "netdev_features_t" $TGT_H ; then
	echo "#define BCM_HAS_NETDEV_FEATURES_T"
fi

if grep -q "vlan_gro_receive" $srcdir/include/linux/if_vlan.h ; then
	echo "#define BCM_HAS_VLAN_GRO_RECEIVE"
elif grep -q "__vlan_hwaccel_put_tag" $srcdir/include/linux/if_vlan.h &&
     ! grep -q "vlan_hwaccel_receive_skb" $srcdir/include/linux/if_vlan.h ; then
	echo "#define BCM_HAS_NEW_VLAN_INTERFACE"
fi

if grep -q "vlan_group_set_device" $srcdir/include/linux/if_vlan.h ; then
	echo "#define BCM_HAS_VLAN_GROUP_SET_DEVICE"
fi

if [ -f $srcdir/include/linux/device.h ]; then
	if grep -q "dev_driver_string" $srcdir/include/linux/device.h ; then
		echo "#define BCM_HAS_DEV_DRIVER_STRING"
	fi

	if grep -q "dev_name" $srcdir/include/linux/device.h ; then
		echo "#define BCM_HAS_DEV_NAME"
	fi
fi

if [ -f $srcdir/include/linux/mdio.h ]; then
	echo "#define BCM_HAS_MDIO_H"
fi

if [ -f $srcdir/include/linux/mii.h ]; then
	if grep -q "mii_resolve_flowctrl_fdx" $srcdir/include/linux/mii.h ; then
		echo "#define BCM_HAS_MII_RESOLVE_FLOWCTRL_FDX"
	fi
	if grep -q "mii_advertise_flowctrl" $srcdir/include/linux/mii.h ; then
		echo "#define BCM_HAS_MII_ADVERTISE_FLOWCTRL"
	fi

	if grep -q "ethtool_adv_to_mii_adv_t" $srcdir/include/linux/mii.h ; then
		echo "#define BCM_HAS_ETHTOOL_ADV_TO_MII_ADV_T"
	fi
fi

if [ -f $srcdir/include/linux/phy.h ]; then
	if grep -q "mdiobus_alloc" $srcdir/include/linux/phy.h ; then
		echo "#define BCM_HAS_MDIOBUS_ALLOC"
	fi

	if grep -q "struct device *parent" $srcdir/include/linux/phy.h ; then
		echo "#define BCM_MDIOBUS_HAS_PARENT"
	fi
fi

if [ -f $srcdir/include/linux/dma-mapping.h ]; then
	if grep -q "dma_data_direction" $srcdir/include/linux/dma-mapping.h ; then
		echo "#define BCM_HAS_DMA_DATA_DIRECTION"
	fi

	if grep -q "dma_unmap_addr" $srcdir/include/linux/dma-mapping.h ; then
		echo "#define BCM_HAS_DMA_UNMAP_ADDR"
		echo "#define BCM_HAS_DMA_UNMAP_ADDR_SET"
	fi
fi

if [ -f $srcdir/include/$UAPI/linux/net_tstamp.h ]; then
	if grep -q "HWTSTAMP_FILTER_PTP_V2_EVENT" $srcdir/include/$UAPI/linux/net_tstamp.h ; then
		echo "#define BCM_HAS_IEEE1588_SUPPORT"
	fi
fi

if [ -f $srcdir/linux/ssb/ssb_driver_gige.h ]; then
	echo "#define BCM_HAS_SSB"
fi

