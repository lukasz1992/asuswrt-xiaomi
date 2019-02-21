/****************************************************************************
 * Mediatek Inc.
 * 5F., No.5, Taiyuan 1st St., Zhubei City, 
 * Hsinchu County 302, Taiwan, R.O.C.
 * (c) Copyright 2014, Mediatek, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************

    Module Name:
    wifi_fwd.h

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Annie Lu  2014-06-30	  Initial version
*/

#ifndef	__WF_FWD_H__
#define	__WF_FWD_H__

#include <linux/version.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/kernel.h>
#include <linux/net.h>
#include <linux/netdevice.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/udp.h>
#include <linux/tcp.h>


#ifndef TRUE
#define TRUE						1
#endif

#ifndef FALSE
#define FALSE						0
#endif

/* enternal symbol */
extern int (*wf_fwd_rx_hook) (struct sk_buff *skb);
extern int (*wf_fwd_tx_hook) (struct sk_buff *skb);

extern unsigned char (*wf_fwd_entry_insert_hook) (struct net_device *src, struct net_device *dest, void *adapter);
extern unsigned char (*wf_fwd_entry_delete_hook) (struct net_device *src, struct net_device *dest, unsigned char link_down);
extern void (*wf_fwd_set_cb_num) (unsigned int band_cb_num, unsigned int receive_cb_num);

extern void (*wf_fwd_get_rep_hook) (unsigned char idx);
extern void (*wf_fwd_pro_active_hook) (void);
extern void (*wf_fwd_pro_halt_hook) (void);
extern void (*wf_fwd_pro_enabled_hook) (void);
extern void (*wf_fwd_pro_disabled_hook) (void);
extern void (*wf_fwd_access_schedule_active_hook) (void);
extern void (*wf_fwd_access_schedule_halt_hook) (void);
extern void (*wf_fwd_hijack_active_hook) (void);
extern void (*wf_fwd_hijack_halt_hook) (void);

extern void (*wf_fwd_show_entry_hook) (void);
extern bool (*wf_fwd_needed_hook) (void);
extern void (*wf_fwd_delete_entry_hook) (unsigned char idx);
extern void (*packet_source_show_entry_hook) (void);
extern void (*packet_source_delete_entry_hook) (unsigned char idx);

extern void (*wf_fwd_feedback_peer_adapter) (void *adapter, void *peer);
extern void (*wf_fwd_feedback_map_table) (void *adapter, void *peer, void *opp_peer);
extern void (*wf_fwd_probe_adapter) (void *adapter);
extern void (*wf_fwd_insert_repeater_mapping_hook)(void *adapter, void *lock, void *cli_mapping, void *map_mapping, void *ifAddr_mapping);
extern void (*wf_fwd_insert_bridge_mapping_hook) (struct sk_buff *skb);
extern int (*wf_fwd_search_mapping_table_hook) (struct sk_buff *skb, struct APCLI_BRIDGE_LEARNING_MAPPING_STRUCT **tbl_entry);
extern void (*wf_fwd_delete_entry_inform_hook) (unsigned char *addr);

#ifndef ETH_ALEN
#define ETH_ALEN				6
#endif
#define MAC_ADDR_LEN			6

#define fOP_GET_NET_DEVICE_STATUS_DONE			0x00000001
#define fOP_WIFI_FWD_ENABLED						0x00000002
#define fOP_WIFI_FWD_ACTIVE						0x00000010
#define fOP_WIFI_FWD_ACCESS_SCHED_ACTIVE			0x00000100
#define fOP_WIFI_FWD_HIJACK_ACTIVE				0x00001000

#define WIFI_FWD_SET_FLAG(_F)       		(wifi_fwd_op_flag |= (_F))
#define WIFI_FWD_CLEAR_FLAG(_F)    		(wifi_fwd_op_flag &= ~(_F))
#define WIFI_FWD_CLEAR_FLAGS        		(wifi_fwd_op_flag = 0)
#define WIFI_FWD_TEST_FLAG(_F)      		((wifi_fwd_op_flag & (_F)) != 0)
#define WIFI_FWD_TEST_FLAGS(_F)     		((wifi_fwd_op_flag & (_F)) == (_F))

#define REP_IS_5G(_p)			(((_p) == apcli_5g) || ((_p) == ap_5g) || ((_p) == apcli1_5g) || ((_p) == ap1_5g))
#define REP_IS_2G(_p)			(((_p) == apcli_2g) || ((_p) == ap_2g) || ((_p) == apcli1_2g) || ((_p) == ap1_2g))
#define CMP_TO_REP(_p, _x)		(REP_IS_##_x##G(_p))

#define IS_MAIN_SSID_DEV(_p)	(((_p) == ap_2g) || ((_p) == ap_5g) || ((_p) == apcli_2g) || ((_p) == apcli_5g))
#define IS_GUEST_SSID_DEV(_p)	(((_p) == ap1_2g) || ((_p) == ap1_5g) || ((_p) == apcli1_2g) || ((_p) == apcli1_5g))

#define MAIN_SSID_OP(_p)		((main_2g_link_cnt != 0) || (main_5g_link_cnt != 0))
#define GUEST_SSID_OP(_p)		((guest_2g_link_cnt != 0) || (guest_5g_link_cnt != 0))

#define WIFI_FWD_TBL_SIZE		50

#define ENTRY_TYPE_INVALID		0
#define ENTRY_TYPE_SRC			1
#define ENTRY_TYPE_DEST		2

/* macro */
#define dbg_print			printk

/* CB related */
#define CB_OFF  	10
#define RTPKT_TO_OSPKT(_p)		((struct sk_buff *)(_p))
#define GET_OS_PKT_CB(_p)		(RTPKT_TO_OSPKT(_p)->cb)
#define PACKET_CB(_p, _offset)	((RTPKT_TO_OSPKT(_p)->cb[CB_OFF + (_offset)]))

/* [CB_OFF + 34]: tag the packet is sent by which band */
#define DEFAULT_BAND_CB_OFFSET          34
#define WIFI_FWD_PACKET_SPECIFIC_2G		0x1
#define WIFI_FWD_PACKET_SPECIFIC_5G		0x2
#define WIFI_FWD_PACKET_SPECIFIC_ETHER	0x4
#define WIFI_FWD_PACKET_SPECIFIC_TAG	0x8


#define WIFI_FWD_SET_PACKET_BAND(_p, _offset, _flg)   	\
	do{										\
		if (_flg)								\
			PACKET_CB(_p, _offset) |= (_flg);		\
		else									\
			PACKET_CB(_p, _offset) &= (~_flg);		\
	}while(0)

#define WIFI_FWD_GET_PACKET_BAND(_p, _offset)		(PACKET_CB(_p, _offset))

/* [CB_OFF + 35]: tag the packet received from which net device */
#define DEFAULT_RECV_FROM_CB_OFFSET                 35
#define WIFI_FWD_PACKET_RECV_FROM_2G_CLIENT 		0x01
#define WIFI_FWD_PACKET_RECV_FROM_5G_CLIENT 		0x02
#define WIFI_FWD_PACKET_RECV_FROM_2G_AP				0x04
#define WIFI_FWD_PACKET_RECV_FROM_5G_AP     		0x08
#define WIFI_FWD_PACKET_RECV_FROM_2G_GUEST_CLIENT 	0x10
#define WIFI_FWD_PACKET_RECV_FROM_5G_GUEST_CLIENT	0x20
#define WIFI_FWD_PACKET_RECV_FROM_2G_GUEST_AP		0x40
#define WIFI_FWD_PACKET_RECV_FROM_5G_GUEST_AP  		0x80


#define WIFI_FWD_SET_PACKET_RECV_FROM(_p, _offset, _flg)	\
	do{                 	                        				\
          	if (_flg)                               				\
                	PACKET_CB(_p, _offset) |= (_flg);    		\
                else                                   	 			\
                        PACKET_CB(_p, _offset) &= (~_flg);   	\
          }while(0)

#define WIFI_FWD_GET_PACKET_RECV_FROM(_p, _offset)	(PACKET_CB(_p, _offset))

#define IS_PACKET_FROM_APCLI(_x)	\
	((((_x) & WIFI_FWD_PACKET_RECV_FROM_2G_CLIENT) == WIFI_FWD_PACKET_RECV_FROM_2G_CLIENT) ||\
		(((_x) & WIFI_FWD_PACKET_RECV_FROM_5G_CLIENT) == WIFI_FWD_PACKET_RECV_FROM_5G_CLIENT) ||\
		(((_x) & WIFI_FWD_PACKET_RECV_FROM_2G_GUEST_CLIENT) == WIFI_FWD_PACKET_RECV_FROM_2G_GUEST_CLIENT) ||\
		(((_x) & WIFI_FWD_PACKET_RECV_FROM_5G_GUEST_CLIENT) == WIFI_FWD_PACKET_RECV_FROM_5G_GUEST_CLIENT))

#define IS_PACKET_FROM_2G(_x)	\
	(((_x) & WIFI_FWD_PACKET_SPECIFIC_2G) == WIFI_FWD_PACKET_SPECIFIC_2G)

#define IS_PACKET_FROM_5G(_x)	\
	(((_x) & WIFI_FWD_PACKET_SPECIFIC_5G) == WIFI_FWD_PACKET_SPECIFIC_5G)

#define IS_TAG_PACKET(_x)	\
	(((_x) & WIFI_FWD_PACKET_SPECIFIC_TAG) == WIFI_FWD_PACKET_SPECIFIC_TAG)

#define IS_PACKET_FROM_ETHER(_x)	\
	((((_x) & WIFI_FWD_PACKET_SPECIFIC_2G) != WIFI_FWD_PACKET_SPECIFIC_2G) &&\
		(((_x) & WIFI_FWD_PACKET_SPECIFIC_5G) != WIFI_FWD_PACKET_SPECIFIC_5G))

#define IS_PACKET_FROM_MAIN_SSID(_x)	\
	(((_x) & 0x0F) != 0x0)

#define IS_PACKET_FROM_GUEST_SSID(_x)	\
	(((_x) & 0x0F) == 0x0)



typedef enum _ETHER_BAND_BINDDING {
	eth_traffic_band_2g = 2,
	eth_traffic_band_5g = 5,
} ETHER_BAND_BINDDING, *PETHER_BAND_BINDDING;

/* data structure */
struct FwdPair {
	unsigned char valid;	/* 1: valid, 0: invalid */
	struct net_device *src;
	struct net_device *dest;
};

struct PacketSource {
	unsigned char valid; /* 1: valid, 0: invalid */
	struct net_device *peer;
	struct net_device *src;
	unsigned char h_source[ETH_ALEN];
	unsigned char h_dest[ETH_ALEN];
};
			
struct TxSourceEntry {
	unsigned char valid; /* 1: valid, 0: invalid */
	unsigned char h_source[ETH_ALEN];
};
		
struct APCLI_BRIDGE_LEARNING_MAPPING_STRUCT {
	struct net_device *rcvd_net_dev;
	unsigned char	src_addr[ETH_ALEN];
	unsigned char	entry_from;
	struct APCLI_BRIDGE_LEARNING_MAPPING_STRUCT *pBefore;
	struct APCLI_BRIDGE_LEARNING_MAPPING_STRUCT *pNext;
};

struct APCLI_BRIDGE_LEARNING_MAPPING_MAP {
	struct APCLI_BRIDGE_LEARNING_MAPPING_STRUCT *pHead;
	struct APCLI_BRIDGE_LEARNING_MAPPING_STRUCT *pTail;
	unsigned int entry_num;
};


typedef struct _REPEATER_ADAPTER_DATA_TABLE {
	bool Enabled;
	void *EntryLock;
	void **CliHash;
	void **MapHash;
	void *Wdev_ifAddr;
} REPEATER_ADAPTER_DATA_TABLE;


#define FWD_IRQ_LOCK(__lock, __irqflags)                        \
{                                                                                                       \
        __irqflags = 0;                                                                 \
        spin_lock_irqsave((spinlock_t *)(__lock), __irqflags);                  \
}

#define FWD_IRQ_UNLOCK(__lock, __irqflag)                       \
{                                                                                                       \
        spin_unlock_irqrestore((spinlock_t *)(__lock), __irqflag);                      \
}

#define PRINT_MAC(addr)	\
	addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]

#define MAC_ADDR_EQUAL(addr1,addr2)          	(!memcmp((void *)(addr1), (void *)(addr2), ETH_ALEN))
#define COPY_MAC_ADDR(addr1, addr2)       	(memcpy((addr1), (addr2), ETH_ALEN))

#define WIFI_FWD_NETDEV_GET_DEVNAME(_pNetDev)	((_pNetDev)->name)
#define WIFI_FWD_NETDEV_GET_PHYADDR(_pNetDev)	((_pNetDev)->dev_addr)

#define SET_OS_PKT_NETDEV(_pkt, _pNetDev)	\
		(RTPKT_TO_OSPKT(_pkt)->dev) = (_pNetDev)
#endif
