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
    wifi_fwd.c
 
    Abstract:

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
     Annie Lu  2014-06-30	  Initial version
 */

#include "wifi_fwd.h"

struct net_device *ap_2g = NULL, *apcli_2g = NULL, *ap_5g = NULL, *apcli_5g = NULL, *br0 = NULL;
struct net_device *ap1_2g = NULL, *apcli1_2g = NULL, *ap1_5g = NULL, *apcli1_5g = NULL;
struct FwdPair *WiFiFwdBase = NULL;
struct PacketSource *pkt_src = NULL;
struct TxSourceEntry *tx_src_tbl = NULL;
unsigned int tx_src_count = 0;

void * adapter_2g = NULL;
void * adapter_5g = NULL;
void * adapter_tmp_1 = NULL;
void * adapter_tmp_2 = NULL;


static unsigned long wifi_fwd_op_flag;
static unsigned char rep_net_dev;
static unsigned char fwd_counter;
static signed char main_5g_link_cnt;
static signed char main_2g_link_cnt;
static signed char guest_5g_link_cnt;
static signed char guest_2g_link_cnt;

static unsigned int eth_rep5g_wrg_uni_cnt;
static unsigned int eth_rep5g_wrg_bct_cnt;
static unsigned int eth_rep2g_wrg_uni_cnt;
static unsigned int eth_rep2g_wrg_bct_cnt;

static unsigned int band_cb_offset;
static unsigned int recv_from_cb_offset;

REPEATER_ADAPTER_DATA_TABLE global_map_2g_tbl , global_map_5g_tbl;

struct APCLI_BRIDGE_LEARNING_MAPPING_MAP global_map_tbl;
spinlock_t global_map_tbl_lock;
unsigned long global_map_tbl_irq_flags;
spinlock_t global_band_tbl_lock;
unsigned long global_band_tbl_irq_flags;


#define ARP_ETH_TYPE	0x0806
#define LLTD_ETH_TYPE	0x88D9

#ifndef TCP
#define TCP				0x06
#endif /* !TCP */

#ifndef UDP
#define UDP				0x11
#endif /* !UDP */

#ifndef ETH_P_IPV6
#define ETH_P_IPV6		0x86DD
#endif /* !ETH_P_IPV6 */

#ifndef ETH_P_IP
#define ETH_P_IP		0x0800          /* Internet Protocol packet */
#endif /* ETH_P_IP */

#ifndef LENGTH_802_3
#define LENGTH_802_3	14
#endif /* !LENGTH_802_3 */

#define IPV6_NEXT_HEADER_UDP	0x11
#define IPV6_HDR_LEN  40

struct net_device *ra_dev_get_by_name(const char *name)
{
	struct net_device *dev;
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
	dev = dev_get_by_name(&init_net, name);
#else
	dev = dev_get_by_name(name);
#endif

	if (dev)
		dev_put(dev);
	
	return dev;
}


static unsigned char is_wifi_fastlane_mode(struct net_device *dev)
{
	if (IS_MAIN_SSID_DEV(dev)) {
		if (((main_2g_link_cnt >= 2) && (main_5g_link_cnt == 0)) || 
			((main_5g_link_cnt >= 2) && (main_2g_link_cnt == 0)))
			return 1;
		else
			return 0;
	} else if (IS_GUEST_SSID_DEV(dev)) {
		if ((guest_2g_link_cnt >= 2) || (guest_5g_link_cnt >= 2))
			return 1;
		else
			return 0;
	}
}


static unsigned char is_wifi_concurrent_mode(struct net_device *dev)
{
	if (IS_MAIN_SSID_DEV(dev)) {
		if (((main_2g_link_cnt == 1) && (main_5g_link_cnt == 1)) ||
			((main_2g_link_cnt == 2) && (main_5g_link_cnt == 2)))
			return 1;
		else
			return 0;
	} else if (IS_GUEST_SSID_DEV(dev)) {
		if ((guest_2g_link_cnt == 1) && (guest_5g_link_cnt == 1))
			return 1;
		else
			return 0;
	}
}


static void cal_link_count_by_net_device(struct net_device *dev, unsigned char policy)
{
	if (dev == apcli_2g)
	{
		if (policy > 0)
			main_2g_link_cnt++;
		else
			main_2g_link_cnt--;

		if (main_2g_link_cnt < 0)
			main_2g_link_cnt = 0;
	}
	else if (dev == apcli_5g)
	{
		if (policy > 0)
			main_5g_link_cnt++;
		else
			main_5g_link_cnt--;

		if (main_5g_link_cnt < 0)
			main_5g_link_cnt = 0;
	} 
	else if (dev == apcli1_2g)
	{
		if (policy > 0)
			guest_2g_link_cnt++;
		else
			guest_2g_link_cnt--;

		if (guest_2g_link_cnt < 0)
			guest_2g_link_cnt = 0;
	}
	else if (dev == apcli1_5g)
	{
		if (policy > 0)
			guest_5g_link_cnt++;
		else
			guest_5g_link_cnt--;

		if (guest_5g_link_cnt < 0)
			guest_5g_link_cnt = 0;
	}

	if (policy > 0)
		fwd_counter++;
	else
		fwd_counter--;
}


static void dump_net_device_by_name(void)
{
	ap_2g = ra_dev_get_by_name("ra0");
	apcli_2g = ra_dev_get_by_name("apcli0");
	ap_5g = ra_dev_get_by_name("rai0");
	apcli_5g = ra_dev_get_by_name("apclii0");

	/* for Guest SSID */
	ap1_2g = ra_dev_get_by_name("ra1");
	apcli1_2g = ra_dev_get_by_name("apcli1");
	ap1_5g = ra_dev_get_by_name("rai1");
	apcli1_5g = ra_dev_get_by_name("apclii1");

	br0 = ra_dev_get_by_name("br0");

	dbg_print("[dump]ap_2g=0x%08X, apcli_2g=0x%08X, ap_5g=0x%08X, apcli_5g=0x%08X, br0=0x%08X\n",
		(int)ap_2g, (int)apcli_2g, (int)ap_5g, (int)apcli_5g, (int)br0);

	dbg_print("[dump]ap1_2g=0x%08X, apcli1_2g=0x%08X, ap1_5g=0x%08X, apcli1_5g=0x%08X\n",
		(int)ap1_2g, (int)apcli1_2g, (int)ap1_5g, (int)apcli1_5g);

}


static void hex_dump_(char *str, unsigned char *pSrcBufVA, unsigned int SrcBufLen)
{
	unsigned char *pt;
	unsigned int x;

	pt = pSrcBufVA;
	dbg_print("%s: %p, len = %d\n", str, pSrcBufVA, SrcBufLen);
	for (x=0; x<SrcBufLen; x++) {
		if (x % 16 == 0)
			dbg_print("0x%04x : ", x);
		dbg_print("%02x ", ((unsigned char)pt[x]));
		if (x % 16 == 15)
			dbg_print("\n");
	}
	dbg_print("\n");
}


static void wifi_fwd_reset_link_count(void)
{
	fwd_counter = 0;
	main_5g_link_cnt = 0;
	main_2g_link_cnt = 0;
	guest_5g_link_cnt = 0;
	guest_2g_link_cnt = 0;
}

static bool wifi_fwd_needed(void)
{
	if (!WIFI_FWD_TEST_FLAG(fOP_WIFI_FWD_ACTIVE) || !WIFI_FWD_TEST_FLAG(fOP_WIFI_FWD_ENABLED) || (fwd_counter == 0))
		return FALSE;

	return TRUE;
}


static void wifi_fwd_show_entry(void)
{
	int idx = 0;

	dbg_print("\n[%s]fwd_link=%d, [5g main]=%d, [2g main]=%d, [5g guest]=%d, [2g guest]=%d\n", 
		__FUNCTION__, fwd_counter, main_5g_link_cnt, main_2g_link_cnt, 
		guest_5g_link_cnt, guest_2g_link_cnt);

	dbg_print("[%s][5g main: 0x%08X <--> 0x%08X] : %d\n",
		__FUNCTION__, (int)apcli_5g, (int)ap_5g, main_5g_link_cnt);

	dbg_print("[%s][2g main: 0x%08X <--> 0x%08X] : %d\n",
		__FUNCTION__, (int)apcli_2g, (int)ap_2g, main_2g_link_cnt);

	dbg_print("[%s][5g guest: 0x%08X <--> 0x%08X] : %d\n",
		__FUNCTION__, (int)apcli1_5g, (int)ap1_5g, guest_5g_link_cnt);

	dbg_print("[%s][2g guest: 0x%08X <--> 0x%08X] : %d\n",
		__FUNCTION__, (int)apcli1_2g, (int)ap1_2g, guest_2g_link_cnt);

	for (idx=0; idx<WIFI_FWD_TBL_SIZE; idx++)
	{
		if (WiFiFwdBase[idx].valid)
			dbg_print("[%s]index=%d, valid=%d, src_dev=0x%08X, dest_dev=0x%08X\n", 
				__FUNCTION__, idx, WiFiFwdBase[idx].valid, (int)WiFiFwdBase[idx].src, (int)WiFiFwdBase[idx].dest);
	}

	dbg_print("[%s]eth_rep5g_wrg_uni_cnt=%d, eth_rep5g_wrg_bct_cnt=%d\n", 
		__FUNCTION__, eth_rep5g_wrg_uni_cnt, eth_rep5g_wrg_bct_cnt);

	dbg_print("[%s]eth_rep2g_wrg_uni_cnt=%d, eth_rep2g_wrg_bct_cnt=%d\n", 
		__FUNCTION__, eth_rep2g_wrg_uni_cnt, eth_rep2g_wrg_bct_cnt);
	
}


static void wifi_fwd_delete_entry_by_idx(unsigned char idx)
{
	int i = 0;

	dbg_print("\n[%s]--------------------------------------------\n", __FUNCTION__);

	if (idx < WIFI_FWD_TBL_SIZE) 
	{
		if (WiFiFwdBase[idx].valid)
		{
			memset(&WiFiFwdBase[idx], 0, sizeof(struct FwdPair));
			cal_link_count_by_net_device(WiFiFwdBase[idx].src, 0);

			dbg_print("[%s]index=%d, valid=%d, src dev=0x%08X, dest dev=0x%08X\n", 
				__FUNCTION__, idx, WiFiFwdBase[idx].valid, (int)WiFiFwdBase[idx].src, (int)WiFiFwdBase[idx].dest);
		} else
			dbg_print("[%s]index=%d is void originally\n", __FUNCTION__, idx);			
	}
	else
	{
		for (i=0; i<WIFI_FWD_TBL_SIZE; i++)
			memset(&WiFiFwdBase[i], 0, sizeof(struct FwdPair));

		wifi_fwd_reset_link_count();

		dbg_print("[%s] flush all entries\n", __FUNCTION__);
	}
}


static void packet_source_show_entry(void)
{
	int idx = 0;

	dbg_print("\n[%s]--------------------------------------------\n", __FUNCTION__);
	
	for (idx=0; idx<WIFI_FWD_TBL_SIZE; idx++)
	{
		if (pkt_src[idx].valid)
			dbg_print("[%s]index=%d, valid=%d, src=0x%08X, peer=0x%08X, " 
				"h_source=%02X:%02X:%02X:%02X:%02X:%02X, h_dest=%02X:%02X:%02X:%02X:%02X:%02X\n", 
				__FUNCTION__, idx, pkt_src[idx].valid, (int)pkt_src[idx].src, (int)pkt_src[idx].peer, 
				pkt_src[idx].h_source[0], pkt_src[idx].h_source[1], pkt_src[idx].h_source[2],
				pkt_src[idx].h_source[3], pkt_src[idx].h_source[4], pkt_src[idx].h_source[5],
				pkt_src[idx].h_dest[0], pkt_src[idx].h_dest[1], pkt_src[idx].h_dest[2],
				pkt_src[idx].h_dest[3], pkt_src[idx].h_dest[4], pkt_src[idx].h_dest[5]);
	}
}


static void packet_source_delete_entry(unsigned char idx)
{
	int i = 0;

	if (!WIFI_FWD_TEST_FLAG(fOP_WIFI_FWD_ENABLED))
		return;

	dbg_print("\n[%s]--------------------------------------------\n", __FUNCTION__);

	if (idx < WIFI_FWD_TBL_SIZE) 
	{
		if (pkt_src[idx].valid)
		{
			memset(&pkt_src[idx], 0, sizeof(struct PacketSource));
			
			dbg_print("[%s]index=%d, valid=%d, src=0x%08X, peer=0x%08X, "
				"h_source=%02X:%02X:%02X:%02X:%02X:%02X, h_dest=%02X:%02X:%02X:%02X:%02X:%02X\n", 
				__FUNCTION__, idx, pkt_src[idx].valid, (int)pkt_src[idx].src, (int)pkt_src[idx].peer, 
				pkt_src[idx].h_source[0], pkt_src[idx].h_source[1], pkt_src[idx].h_source[2],
				pkt_src[idx].h_source[3], pkt_src[idx].h_source[4], pkt_src[idx].h_source[5],
				pkt_src[idx].h_dest[0], pkt_src[idx].h_dest[1], pkt_src[idx].h_dest[2],
				pkt_src[idx].h_dest[3], pkt_src[idx].h_dest[4], pkt_src[idx].h_dest[5]);
		} else
			dbg_print("[%s]index=%d is void originally\n", __FUNCTION__, idx);			
	}
	else if (idx == WIFI_FWD_TBL_SIZE)
	{
		eth_rep5g_wrg_uni_cnt = 0;
		eth_rep5g_wrg_bct_cnt = 0;
		eth_rep2g_wrg_uni_cnt = 0;
		eth_rep2g_wrg_bct_cnt = 0;

		dbg_print("[%s] reset counters\n", __FUNCTION__);
	}
	else
	{
		for (i=0; i<WIFI_FWD_TBL_SIZE; i++)
			memset(&pkt_src[i], 0, sizeof(struct PacketSource));
		
		dbg_print("[%s] flush all entries\n", __FUNCTION__);
	}
}


/* 
	return value:
	>=0:	array index of WiFiFwdBase
	-1:		search failed
*/
static int wifi_fwd_find_empty_entry(void)
{
	int idx = 0;

	for (idx=0; idx<WIFI_FWD_TBL_SIZE; idx++)
	{
		if (WiFiFwdBase[idx].valid == 0)
			return idx;
	}

	dbg_print("[%s] table full\n", __FUNCTION__);
	return -1;
}


/* 
	return value:
	>=0:	array index of WiFiFwdBase
	-1:		search failed
*/
static int wifi_fwd_find_entry(struct net_device *src, struct net_device *dest)
{
	int idx = 0;

	for (idx=0; idx<WIFI_FWD_TBL_SIZE; idx++)
	{
		if ((WiFiFwdBase[idx].valid) &&
			(WiFiFwdBase[idx].src == src) &&
			(WiFiFwdBase[idx].dest == dest))
			return idx;
	}

	//dbg_print("[%s] no entry found\n", __FUNCTION__);
	return -1;
}


/* 
	return value:
	1:	clear OK
	0:	clear failed (wrong input index)
*/
static int wifi_fwd_clear_entry(int index)
{
	struct FwdPair *entry = NULL;
	entry = WiFiFwdBase + index;

	dbg_print("[wifi_fwd_clear_entry] original: index=%d, valid=%d, src_dev=0x%08X, dest_dev=0x%08X\n", 
		index, entry->valid, (int)entry->src, (int)entry->dest);

	if (entry->valid) {	
		memset(entry, 0, sizeof(struct FwdPair));
		return 1;
	}

	return 0;
}

static void wifi_fwd_set_cb_num(unsigned int band_cb_num, unsigned int receive_cb_num)
{
	band_cb_offset = band_cb_num;
	recv_from_cb_offset = receive_cb_num;

	dbg_print("[%s] band_cb_offset=%d, recv_from_cb_offset=%d\n", __FUNCTION__, band_cb_offset, recv_from_cb_offset);	
}

static void wifi_fwd_get_rep(unsigned char rep)
{
	if (!WIFI_FWD_TEST_FLAG(fOP_WIFI_FWD_ENABLED))
		return;

	if (rep == 0)
		rep_net_dev = eth_traffic_band_2g;
	else
		rep_net_dev = eth_traffic_band_5g;		

	dbg_print("[%s] rep=%d\n", __FUNCTION__, rep_net_dev);	
}


static void wifi_fwd_pro_active(void)
{
	WIFI_FWD_SET_FLAG(fOP_WIFI_FWD_ACTIVE);
	dbg_print("[%s]\n", __FUNCTION__);	
}


static void wifi_fwd_pro_halt(void)
{
	WIFI_FWD_CLEAR_FLAG(fOP_WIFI_FWD_ACTIVE);
	dbg_print("[%s]\n", __FUNCTION__);	
}

static void wifi_fwd_pro_enabled(void)
{
	WIFI_FWD_SET_FLAG(fOP_WIFI_FWD_ENABLED);
	dbg_print("[%s]\n", __FUNCTION__);	
}


static void wifi_fwd_pro_disabled(void)
{
	WIFI_FWD_CLEAR_FLAG(fOP_WIFI_FWD_ENABLED);
	dbg_print("[%s]\n", __FUNCTION__);	
}

static void wifi_fwd_access_schedule_active(void)
{
	WIFI_FWD_SET_FLAG(fOP_WIFI_FWD_ACCESS_SCHED_ACTIVE);
	dbg_print("[%s]\n", __FUNCTION__);	
}


static void wifi_fwd_access_schedule_halt(void)
{
	WIFI_FWD_CLEAR_FLAG(fOP_WIFI_FWD_ACCESS_SCHED_ACTIVE);
	dbg_print("[%s]\n", __FUNCTION__);	
}


static void wifi_fwd_hijack_active(void)
{
	WIFI_FWD_SET_FLAG(fOP_WIFI_FWD_HIJACK_ACTIVE);
	dbg_print("[%s]\n", __FUNCTION__);	
}


static void wifi_fwd_hijack_halt(void)
{
	WIFI_FWD_CLEAR_FLAG(fOP_WIFI_FWD_HIJACK_ACTIVE);
	dbg_print("[%s]\n", __FUNCTION__);	
}


static void packet_tx_source_flush_all()
{
	if(!tx_src_tbl) 
		return;
	
	memset(tx_src_tbl, 0, sizeof(struct TxSourceEntry)*WIFI_FWD_TBL_SIZE);
	tx_src_count = 0;
}


static void wf_fwd_delete_entry_inform(unsigned char *addr)
{
	int i = 0,count = 0;
	struct TxSourceEntry *entry = NULL;

	if (!WIFI_FWD_TEST_FLAG(fOP_WIFI_FWD_ENABLED))
		return;

	if(!addr || (tx_src_count == 0))
		return;

	for (i=0; i<WIFI_FWD_TBL_SIZE; i++)
	{
		entry = tx_src_tbl + i;
		if (entry->valid == 1) 
		{
			count++;
			if(MAC_ADDR_EQUAL(addr, entry->h_source))
			{
				entry->valid = 0;
				tx_src_count--;
				if(tx_src_count < 0)
					tx_src_count = 0;
				
				return;
			}
			
			if(count >= tx_src_count)
				return;
		}
	}
}


/* 
	return value:
	0:	insert failed
	1:	insert success
	2:	do nothing
*/
static int wifi_fwd_insert_tx_source_entry(struct sk_buff *skb,struct net_device *tx_dev)
{
	struct ethhdr *mh = eth_hdr(skb);
	unsigned int recv_from = WIFI_FWD_GET_PACKET_RECV_FROM(skb, recv_from_cb_offset);
	struct TxSourceEntry *entry = NULL;
	int i = 0, count = 0;
	unsigned char bInsert = 0;

	if(!is_wifi_concurrent_mode(tx_dev) || !mh) 
		return 2;

	if(tx_dev == apcli_2g || tx_dev == apcli_5g)
	{
		for (i=0; i<WIFI_FWD_TBL_SIZE; i++)
		{
			entry = tx_src_tbl + i;
			if ((entry->valid == 1) &&
				MAC_ADDR_EQUAL(mh->h_source, entry->h_source))
				return 2;
		}
		bInsert = 1;
	}

	if(!bInsert)
		return 2;
	
	for (i=0; i<WIFI_FWD_TBL_SIZE; i++)
	{
		entry = tx_src_tbl + i;
		if (entry->valid == 0) 
		{
			tx_src_count++;
			entry->valid = 1;
			COPY_MAC_ADDR(entry->h_source, mh->h_source);
            dbg_print("[%s %d] valid=%d, src_mac=%02X:%02X:%02X:%02X:%02X:%02X\n",
				__FUNCTION__,i, entry->valid,
				entry->h_source[0], entry->h_source[1], entry->h_source[2],
				entry->h_source[3], entry->h_source[4], entry->h_source[5]);
			return 1;
		}
	}	
	
	dbg_print("[%s] No free space for insert tx source entry.\n",__FUNCTION__);
	return 0;
}

/* 
	return value: 
	0:	no looping
	1:	looping found
	2:	forward to bridge
*/
static unsigned char wifi_fwd_check_looping(
	struct sk_buff *skb,
	struct net_device *dev)
{
	struct ethhdr *mh = eth_hdr(skb);
	unsigned int recv_from = WIFI_FWD_GET_PACKET_RECV_FROM(skb, recv_from_cb_offset);
	struct TxSourceEntry *entry = NULL;
	int i = 0,count = 0;
	unsigned char bMulticast = 0;

	if (!is_wifi_concurrent_mode(dev) ||
		!IS_PACKET_FROM_APCLI(recv_from) ||
		(tx_src_count == 0) ||
		!mh) 
		return 0;

	if (dev == apcli_2g || dev == apcli_5g || dev == apcli1_2g || dev == apcli1_5g)
	{
		if ((mh->h_dest[0] & 0x1) == 0x1)
			bMulticast = 1;
		
		for (i=0; i<WIFI_FWD_TBL_SIZE; i++)
		{
			entry = tx_src_tbl + i;
			if ((entry->valid == 1))
			{
				count++;
				if(MAC_ADDR_EQUAL(mh->h_source, entry->h_source))
				{
					// Mean the souce has changed to other side.
					if(!bMulticast)
					{
						wf_fwd_delete_entry_inform(entry->h_source);
						return 0;
					}

					return 1;
				}

				if(count >= tx_src_count)
					break;;
			}
		}
	}
	
	return 0;
}


/* 
	return value:
	1:	insert success
	0:	insert failed
*/
static unsigned char wifi_fwd_establish_entry(struct net_device *src, struct net_device *dest)
{
	struct FwdPair *entry = NULL;
	int idx = 0;

	if (!src || !dest)
		return 0;
	
	/* check if it is an existed entry */
	idx = wifi_fwd_find_entry(src, dest);
	if (idx >= 0)
		return 0;

	/* to establish the path between src and dest */
	idx = wifi_fwd_find_empty_entry();
	if (idx == -1)
		return 0;

	entry = WiFiFwdBase + idx;
	entry->valid = 1;
	entry->src = src;
	entry->dest = dest;
	cal_link_count_by_net_device(entry->src, 1);

	dbg_print("[%s %d] valid=%d, src=0x%08X, dest=0x%08X\n", 
		__FUNCTION__, idx, entry->valid, (unsigned int)entry->src, (unsigned int)entry->dest);
}


static void main_ssid_cover_guest_ssid(struct net_device *src)
{
	struct net_device *second_src = NULL, *second_dest = NULL, *first_src = NULL, *first_dest = NULL;
	int idx = 0;

	idx = wifi_fwd_find_entry(apcli_2g, ap1_2g);
	if (idx >= 0) {
		wifi_fwd_clear_entry(idx);
		cal_link_count_by_net_device(apcli_2g, 0);
	}
	
	idx = wifi_fwd_find_entry(apcli_2g, ap1_5g);
	if (idx >= 0) {
		wifi_fwd_clear_entry(idx);
		cal_link_count_by_net_device(apcli_2g, 0);
	}
	
	idx = wifi_fwd_find_entry(apcli_5g, ap1_2g);
	if (idx >= 0) {
		wifi_fwd_clear_entry(idx);
		cal_link_count_by_net_device(apcli_5g, 0);
	}
	
	idx = wifi_fwd_find_entry(apcli_5g, ap1_5g);
	if (idx >= 0) {
		wifi_fwd_clear_entry(idx);
		cal_link_count_by_net_device(apcli_5g, 0);
	}

	if (GUEST_SSID_OP(src)) {		
		dbg_print("[%s]no need to cover guest ssid\n", __FUNCTION__);	
		return;
	}

	if (is_wifi_concurrent_mode(apcli_2g))
	{
		idx = wifi_fwd_find_entry(apcli_2g, ap1_2g);
		if (idx < 0)
			wifi_fwd_establish_entry(apcli_2g, ap1_2g);	

		idx = wifi_fwd_find_entry(apcli_5g, ap1_5g);
		if (idx < 0)
			wifi_fwd_establish_entry(apcli_5g, ap1_5g);

		dbg_print("[%s]need to cover guest ssid under concurrent mode\n", __FUNCTION__);
	} 
	else if (is_wifi_fastlane_mode(apcli_2g))
	{
		idx = wifi_fwd_find_entry(apcli_2g, ap_2g);
		if (idx >= 0) {
			src = apcli_2g;
			first_dest = ap1_2g;
			second_dest = ap1_5g;
		} else {
			src = apcli_5g;
			first_dest = ap1_5g;
			second_dest = ap1_2g;
		}
	
		idx = wifi_fwd_find_entry(src, first_dest);
		if (idx < 0)
			wifi_fwd_establish_entry(src, first_dest);	

		idx = wifi_fwd_find_entry(src, second_dest);
		if (idx < 0)
			wifi_fwd_establish_entry(src, second_dest);

		dbg_print("[%s]need to cover guest ssid under fastLane mode\n", __FUNCTION__);
	}
	else 
		dbg_print("[%s]no need to cover guest ssid under AP mode\n", __FUNCTION__);
}


/* 
	return value:
	1:	delete success
	0:	delete failed
*/
static unsigned char wifi_fwd_delete_entry(struct net_device *src, struct net_device *dest, unsigned char link_down)
{
	struct net_device *second_src = NULL, *second_dest = NULL, *first_dest = NULL;
	int idx = 0;

	if (!WIFI_FWD_TEST_FLAG(fOP_WIFI_FWD_ENABLED))
		return 1;

	if (src == apcli_2g) {
		first_dest = ap_2g;
		second_src = apcli_5g;
		second_dest = ap_5g;
	} else if (src == apcli_5g) {
		first_dest = ap_5g;
		second_src = apcli_2g;
		second_dest = ap_2g;
	} else if (src == apcli1_2g) {
		first_dest = ap1_2g;
		second_src = apcli1_5g;
		second_dest = ap1_5g;
	} else if (src == apcli1_5g) {
		first_dest = ap1_5g;
		second_src = apcli1_2g;
		second_dest = ap1_2g;
	}

	if (link_down == 0)
		first_dest = dest;

	idx = wifi_fwd_find_entry(src, first_dest);
	if (idx == -1)
		return 0;

	if (wifi_fwd_clear_entry(idx) == 0)
		return 0;

	cal_link_count_by_net_device(src, 0);
	dbg_print("[%s] index=%d, src=0x%08X, dest=0x%08X\n", 
		__FUNCTION__, idx, (int)src, (int)first_dest);

	/* 
		check if there exists FastLane case
		if yes, delete the cross path of FastLane as well
	*/
	idx = wifi_fwd_find_entry(src, second_dest);
	if (idx >= 0) {
		wifi_fwd_clear_entry(idx);
		cal_link_count_by_net_device(src, 0);
	}

	/* 
		check if there exists the connection of the other band
		if yes, need to do FastLane case 
	*/
	idx = wifi_fwd_find_entry(second_src, second_dest);
	if (idx == -1) 
		goto done;

	/* try to establish the FastLane path between second_src and dest */
	if (wifi_fwd_establish_entry(second_src, first_dest) == 0)
		return 0;

done:
	main_ssid_cover_guest_ssid(src);

	/* flush packet source table */
	packet_source_delete_entry(WIFI_FWD_TBL_SIZE+1);

	/* flush tx packet source table */
	packet_tx_source_flush_all();

	WIFI_FWD_CLEAR_FLAG(fOP_GET_NET_DEVICE_STATUS_DONE);	
	return 1;
}


/* 
	return value:
	1:	insert success
	0:	insert failed
*/
static unsigned char wifi_fwd_insert_entry(struct net_device *src, struct net_device *dest, void *adapter)
{
	struct net_device *second_src = NULL, *second_dest = NULL, *first_dest = NULL;
	struct FwdPair *entry = NULL;
	int idx = 0;

	if (!WIFI_FWD_TEST_FLAG(fOP_WIFI_FWD_ENABLED))
		return 0;

	if (!src || !dest)
		return 0;

	if (!WIFI_FWD_TEST_FLAG(fOP_GET_NET_DEVICE_STATUS_DONE)) {
		dump_net_device_by_name();
		WIFI_FWD_SET_FLAG(fOP_GET_NET_DEVICE_STATUS_DONE);
	}

	if (src == apcli_2g) {
		first_dest = ap_2g;
		second_src = apcli_5g;
		second_dest = ap_5g;
		adapter_2g = adapter;

 		if ((adapter_tmp_1 == adapter_2g) && (adapter_tmp_2 != NULL))
			adapter_5g = adapter_tmp_2;
		else if ((adapter_tmp_2 == adapter_2g) && (adapter_tmp_1 != NULL))
			adapter_5g = adapter_tmp_1;
		
		dbg_print("[%s] input adapter=0x%08X, adapter_2g=0x%08X, adapter_5g=0x%08X\n", 
			__FUNCTION__, (int)adapter, (int)adapter_2g, (int)adapter_5g);
	} else if (src == apcli_5g) {
		first_dest = ap_5g;
		second_src = apcli_2g;
		second_dest = ap_2g;
		adapter_5g = adapter;

		if ((adapter_tmp_1 == adapter_5g) && (adapter_tmp_2 != NULL))
			adapter_2g = adapter_tmp_2;
		else if ((adapter_tmp_2 == adapter_5g) && (adapter_tmp_1 != NULL))
			adapter_2g = adapter_tmp_1;

		dbg_print("[%s] input adapter=0x%08X, adapter_5g=0x%08X, adapter_2g=0x%08X\n", 
			__FUNCTION__, (int)adapter, (int)adapter_5g, (int)adapter_2g);
	} else if (src == apcli1_2g) {
		first_dest = ap1_2g;
		second_src = apcli1_5g;
		second_dest = ap1_5g;

		if (adapter_2g == NULL) {
			adapter_2g = adapter;

	 		if ((adapter_tmp_1 == adapter_2g) && (adapter_tmp_2 != NULL))
				adapter_5g = adapter_tmp_2;
			else if ((adapter_tmp_2 == adapter_2g) && (adapter_tmp_1 != NULL))
				adapter_5g = adapter_tmp_1;

			dbg_print("[%s] input adapter=0x%08X, adapter_2g=0x%08X, adapter_5g=0x%08X\n", 
			__FUNCTION__, (int)adapter, (int)adapter_2g, (int)adapter_5g);
		}
	} else if (src == apcli1_5g) {
		first_dest = ap1_5g;
		second_src = apcli1_2g;
		second_dest = ap1_2g;

		if (adapter_5g == NULL) {
			adapter_5g = adapter;

			if ((adapter_tmp_1 == adapter_5g) && (adapter_tmp_2 != NULL))
				adapter_2g = adapter_tmp_2;
			else if ((adapter_tmp_2 == adapter_5g) && (adapter_tmp_1 != NULL))
				adapter_2g = adapter_tmp_1;

			dbg_print("[%s] input adapter=0x%08X, adapter_5g=0x%08X, adapter_2g=0x%08X\n", 
				__FUNCTION__, (int)adapter, (int)adapter_5g, (int)adapter_2g);
		}
	}

	/* 
		check if there exists FastLane case
		if yes, delete the cross path of FastLane
	*/
	idx = wifi_fwd_find_entry(second_src, first_dest);
	if (idx >= 0)
		wifi_fwd_delete_entry(second_src, first_dest, 0);

	/* try to establish the path between src and dest */
	if (wifi_fwd_establish_entry(src, first_dest) == 0)
		return 0;

	/* 
		check if there exists the connection of the other band
		if yes, no need to do FastLane case 
	*/
	idx = wifi_fwd_find_entry(second_src, second_dest);
	if (idx >= 0)
		goto done;

	/* try to establish the FastLane path between src and second_dest */
	if (wifi_fwd_establish_entry(src, second_dest) == 0)
		goto done;

done:
	main_ssid_cover_guest_ssid(src);

	/* flush packet source table */
	packet_source_delete_entry(WIFI_FWD_TBL_SIZE+1);

	return 1;
}


/* 
	return value:
	0:	Success, skb is handled by wifi_fwd module.
	1:	FAIL, do nothing
*/
static struct net_device * wifi_fwd_insert_packet_source(struct sk_buff *skb, struct net_device *dev, struct net_device *peer)
{
	struct ethhdr *mh = eth_hdr(skb);
	struct PacketSource *entry = NULL;
	bool need_flush = false;
	int i = 0;
	
	for (i=0; i<WIFI_FWD_TBL_SIZE; i++)
	{
		entry = pkt_src + i;
		
		if (entry->valid == 1) {
			if (peer == entry->peer) {
				if (MAC_ADDR_EQUAL(mh->h_source, entry->h_source)) {
					if (dev == entry->src) {
						return peer;
					} else {
						need_flush = true;
						break;
					}	
				}
			}
		}
	}

	if (need_flush == true)
		packet_source_delete_entry(WIFI_FWD_TBL_SIZE+1);
	else
		dbg_print("[%s] packet source cannot be found\n", __FUNCTION__); 

	for (i=0; i<WIFI_FWD_TBL_SIZE; i++)
	{
		entry = pkt_src + i;
		
		if (entry->valid == 0) {
			COPY_MAC_ADDR(entry->h_source, mh->h_source);
			COPY_MAC_ADDR(entry->h_dest, mh->h_dest);
			entry->valid = 1;
			entry->peer = peer;
			entry->src = dev;

			dbg_print("[%s %d] valid=%d, from=0x%08X, will send to=0x%08X, "
				"src_mac=%02X:%02X:%02X:%02X:%02X:%02X, dest_mac=%02X:%02X:%02X:%02X:%02X:%02X\n",
				__FUNCTION__, i, entry->valid, entry->src, entry->peer,
				entry->h_source[0], entry->h_source[1], entry->h_source[2],
				entry->h_source[3], entry->h_source[4], entry->h_source[5],
				entry->h_dest[0], entry->h_dest[1], entry->h_dest[2],
				entry->h_dest[3], entry->h_dest[4],entry->h_dest[5]);
			
			return entry->peer;
		}
	}
}

static void wifi_fwd_tx_count_by_source_addr(
	struct sk_buff *skb, 
	struct net_device *tx_dev,
	unsigned char *found_path,
	unsigned char *entry_cnt)
{
	struct PacketSource *ppkt_src = NULL;
	struct FwdPair *entry = NULL;
	int i = 0;
	
	for (i=0; i<WIFI_FWD_TBL_SIZE; i++)
	{
		ppkt_src = pkt_src + i;
		
		if (ppkt_src->valid == 1)
		{
			if (MAC_ADDR_EQUAL(&skb->data[6], ppkt_src->h_source))
			{
				*entry_cnt = *entry_cnt + 1;

				if ((ppkt_src->src == tx_dev) || (ppkt_src->peer == tx_dev))
				{
					for (i=0; i<WIFI_FWD_TBL_SIZE; i++)
					{
						entry = WiFiFwdBase + i;

						if (entry->valid == 1)
						{
							if (((entry->src == ppkt_src->src) && (entry->dest == ppkt_src->peer)) ||
								((entry->src == ppkt_src->peer) && (entry->dest == ppkt_src->src)))
								*found_path = *found_path + 1;
						}
					}
				}
			}
		}
		else
			break;
	}
}


/* 
	return value:
	1:	find entry
	0:	find nothing
*/
static unsigned char wifi_fwd_find_sa_entry_by_sa_and_nd(
	struct sk_buff *skb, 
	struct net_device *sender_dev,
	unsigned char idx)
{
	struct PacketSource *entry = NULL;
	struct ethhdr *mh = eth_hdr(skb);
	int i = 0;
	
	for (i=0; i<WIFI_FWD_TBL_SIZE; i++)
	{
		entry = pkt_src + i;
		
		if (entry->valid == 1)
		{
			if (MAC_ADDR_EQUAL(mh->h_source, entry->h_source))
			{
				if (idx == 0)
				{
					if (entry->src == sender_dev)
						return 1;
				}
				else
				{
					if (entry->peer== sender_dev)
						return 1;
				}
			}
		}
		else
			break;
	}

	return 0;
}


/* 
	return value: # of found entries
*/
static unsigned char wifi_fwd_sa_count_by_source_addr(
	struct sk_buff *skb, 
	struct net_device *receive_dev)
{
	struct PacketSource *entry = NULL;
	struct ethhdr *mh = eth_hdr(skb);
	int i = 0, sa_cnt = 0;
	
	for (i=0; i<WIFI_FWD_TBL_SIZE; i++)
	{
		entry = pkt_src + i;
		
		if (entry->valid == 1)
		{
			if ((mh->h_source[0] & 0x2) == 0x2)
			{
				if (memcmp(&mh->h_source[3], &entry->h_source[3], 3) == 0)
					sa_cnt++;
			}
			else
			{
				if (MAC_ADDR_EQUAL(mh->h_source, entry->h_source))
					sa_cnt++;
			}
		}
		else
			break;
	}

	return sa_cnt;
}


/* 
	return value:
	2:	should be forwarded to bridge
	1:	not forwarded from bridge
	0:	suppose it is forwarded from bridge
*/
static unsigned char wifi_fwd_check_from_bridge_by_dest_addr(
	struct sk_buff *skb, 
	struct net_device *sender_dev)
{
	struct PacketSource *entry = NULL;
	struct ethhdr *mh = eth_hdr(skb);
	int i = 0;
	
	for (i=0; i<WIFI_FWD_TBL_SIZE; i++)
	{
		entry = pkt_src + i;
		
		if (entry->valid == 1)
		{
			if (((sender_dev == ap_2g) && (entry->src== ap_5g)) ||
				((sender_dev == ap_5g) && (entry->src == ap_2g)) ||
				((sender_dev == ap1_5g) && (entry->src == ap1_2g)) ||
				((sender_dev == ap1_5g) && (entry->src == ap1_2g)))
			{
				if (MAC_ADDR_EQUAL(mh->h_dest, entry->h_source))
					return 2;
			}
			else
			{
				if (MAC_ADDR_EQUAL(mh->h_dest, entry->h_source))
					return 1;	
			}
		}
		else
			break;
	}

	return 0;
}

/* 
	return value: 
	0:	no need to drop this packet
	1:	drop this packet
*/
static unsigned char wifi_fwd_tx_lookup_entry(struct net_device *tx_dev, struct sk_buff *skb)
{
	unsigned char found = 0, entry_cnt = 0;
	unsigned int recv_from = 0, band_from = 0;
	struct sk_buff	 *clone_pkt = NULL;
	bool need_redirect = false;

	band_from = WIFI_FWD_GET_PACKET_BAND(skb, band_cb_offset); 
	recv_from = WIFI_FWD_GET_PACKET_RECV_FROM(skb, recv_from_cb_offset);

	if (IS_PACKET_FROM_APCLI(recv_from) && !MAIN_SSID_OP(tx_dev))
		return 1;

	/* drop the packet from guest ssid to main ssid, and vice versa */
	if (!IS_PACKET_FROM_ETHER(band_from) && 
		((guest_2g_link_cnt != 0) || (guest_5g_link_cnt != 0)))
	{
		if (IS_MAIN_SSID_DEV(tx_dev)) {
			if (IS_PACKET_FROM_GUEST_SSID(recv_from))
				return 1;
		} else if (IS_GUEST_SSID_DEV(tx_dev)) {
			if (IS_PACKET_FROM_MAIN_SSID(recv_from))
				return 1;
		}
	}

	/* drop the packet from the other band */
	if (is_wifi_concurrent_mode(tx_dev))	
	{
		if ((tx_dev == apcli_5g) || (tx_dev == apcli1_5g))
		{
			if (IS_PACKET_FROM_2G(band_from)) {
				if(IS_TAG_PACKET(band_from))
					need_redirect = true;
				else
					return 1;
			}		
			
			if ((rep_net_dev == eth_traffic_band_2g) && IS_PACKET_FROM_ETHER(band_from)) {
				if ((((unsigned char *)skb->data)[0] & 0x1) == 0x0) {
					eth_rep2g_wrg_uni_cnt++;
					need_redirect = true;
				} else
					eth_rep2g_wrg_bct_cnt++;
			}

			if (need_redirect == true) {
				clone_pkt = skb_clone(skb, GFP_ATOMIC);
				if (tx_dev == apcli_5g)
					clone_pkt->dev = apcli_2g;
				else
					clone_pkt->dev = apcli1_2g;
				dev_queue_xmit(clone_pkt);
				return 1;
			}
		}
		else if ((tx_dev == apcli_2g) || (tx_dev == apcli1_2g))
		{
			if (IS_PACKET_FROM_5G(band_from)) {
				if(IS_TAG_PACKET(band_from))
					need_redirect = true;
				else
					return 1;
			}
			
			if ((rep_net_dev == eth_traffic_band_5g) && IS_PACKET_FROM_ETHER(band_from)) {
				if ((((unsigned char *)skb->data)[0] & 0x1) == 0x0) {
					eth_rep5g_wrg_uni_cnt++;
					need_redirect = true;
				} else
					eth_rep5g_wrg_bct_cnt++;
			}

			if (need_redirect == true) {
				clone_pkt = skb_clone(skb, GFP_ATOMIC);
				if (tx_dev == apcli_2g)
					clone_pkt->dev = apcli_5g;
				else
					clone_pkt->dev = apcli1_5g;
				dev_queue_xmit(clone_pkt);
				return 1;
			}
		}
	}

	/* 
		in FastLane topology: 
		1. forward Tx packets to driver and handle without any drop 
		2. only for unicast 
	*/
	if (((((unsigned char *)skb->data)[0] & 0x1) == 0x0) && (is_wifi_fastlane_mode(tx_dev) && !GUEST_SSID_OP(tx_dev)))
		return 0;

	wifi_fwd_tx_count_by_source_addr(skb, tx_dev, &found, &entry_cnt);
	
	if ((((unsigned char *)skb->data)[0] & 0x1) == 0x1)
	{
		if(wifi_fwd_insert_tx_source_entry(skb,tx_dev) == 0)
			return 0;
		
		if (found == 0)
		{
			if (entry_cnt == 0)
			{
				if (is_wifi_concurrent_mode(tx_dev) && 
					IS_PACKET_FROM_ETHER(band_from))	
				{
					if ((tx_dev == apcli_5g) || (tx_dev == apcli1_5g))
					{
						if (rep_net_dev == eth_traffic_band_5g)
							return 0;
						else
							return 1;
					}
					else if ((tx_dev == apcli_2g) || (tx_dev == apcli1_2g))
					{
						if (rep_net_dev == eth_traffic_band_2g)
							return 0;
						else
							return 1;
					}
				}
			}
			
			return 0;
		}
		else
		{			
			if (entry_cnt >= 2)
				return 0;
			else
			{
				if (wifi_fwd_find_sa_entry_by_sa_and_nd(skb, tx_dev, 1) == 0)
					return 1;
				else
					return 0;
			}
		}
	}
	else
	{	
#if 0 /* move to earlier and handle it */
		/* in FastLane case, forward Rx packets to bridge and let flooding work done by bridge */
		if (is_wifi_fastlane_mode(tx_dev))
			return 0;
#endif
		if (found == 0)
		{
			if (entry_cnt == 0)
			{
				if (is_wifi_concurrent_mode(tx_dev) && 
					IS_PACKET_FROM_ETHER(band_from))	
				{
					if (rep_net_dev == eth_traffic_band_5g)
					{
						if (REP_IS_5G(tx_dev))
							return 0;
					}
					else if (rep_net_dev == eth_traffic_band_2g)
					{
						if (REP_IS_2G(tx_dev))
							return 0;	
					}
				}
			}
		}
		
		return 0;
	}
}


/* 
	return value of struct net_device: 
	NULL:	search fail
	other:	partner net_device
*/
static struct net_device * wifi_fwd_lookup_entry(struct net_device *dev, unsigned char *type, struct sk_buff *skb)
{
	struct FwdPair *entry = NULL;
	struct net_device *src = NULL;
	int i = 0;

	for (i=0; i<WIFI_FWD_TBL_SIZE; i++)
	{
		entry = WiFiFwdBase + i;

		if (entry->valid == 1)
		{
			if (entry->src == dev)
			{
				*type = ENTRY_TYPE_SRC;
				src = wifi_fwd_insert_packet_source(skb, dev, entry->dest);

				if (src != NULL)
					return src;
			}
			else if (entry->dest == dev)
			{
				*type = ENTRY_TYPE_DEST;
				src = wifi_fwd_insert_packet_source(skb, dev, entry->src);

				if (src != NULL)
					return src;
			}
		}
	}

	*type = ENTRY_TYPE_INVALID;
	return NULL;
}


static void wifi_fwd_entry_dump(void)
{
	struct FwdPair *entry = NULL;
	int i =0;
	
	dbg_print("\ndump wifi fwd table\n");

	for (i=0; i<WIFI_FWD_TBL_SIZE; i++)
	{
		entry = WiFiFwdBase + i;

		dbg_print("[%d] valid=%d, src=0x%08X, dest=0x%08X\n", 
			i, entry->valid, (int)entry->src, (int)entry->dest);
	}

	dbg_print("\n");
}


void wifi_fwd_probe_adapter(void *adapter)
{	
	if (!WIFI_FWD_TEST_FLAG(fOP_WIFI_FWD_ENABLED))
		return;

	if ((adapter != NULL) && (adapter_tmp_1 == NULL)) {
		adapter_tmp_1 = adapter;
	} else if ((adapter != NULL) && (adapter_tmp_2 == NULL)) {
		if (adapter != adapter_tmp_1)
			adapter_tmp_2 = adapter;
	}
}


void wifi_fwd_feedback_peer_adapter(void *adapter, void **peer)
{
	if (!WIFI_FWD_TEST_FLAG(fOP_WIFI_FWD_ENABLED))
		return;

	if ((adapter == adapter_2g) && (adapter_5g != NULL)) {
		*peer = adapter_5g;
	} else if ((adapter == adapter_5g) && (adapter_2g != NULL)) {
		*peer = adapter_2g;
	} else {
		*peer = NULL;
	}
}

void wifi_fwd_feedback_map_table(void *adapter, void **peer, void **opp_peer)
{
	if (!WIFI_FWD_TEST_FLAG(fOP_WIFI_FWD_ENABLED))
		return;


	FWD_IRQ_LOCK(&global_band_tbl_lock, global_band_tbl_irq_flags);

	if (adapter == adapter_2g) {
		
		if (global_map_2g_tbl.Enabled == TRUE)
			*peer = &global_map_2g_tbl;
		else 
			*peer = NULL;

		if (global_map_5g_tbl.Enabled == TRUE)
			*opp_peer = &global_map_5g_tbl;
		else
			*opp_peer = NULL;	
		
	
	} else if (adapter == adapter_5g) {
	
		if (global_map_5g_tbl.Enabled == TRUE)
			*peer = &global_map_5g_tbl;
		else 
			*peer = NULL;

		if (global_map_2g_tbl.Enabled == TRUE)
			*opp_peer = &global_map_2g_tbl;
		else
			*opp_peer = NULL;	

		
	} else {
			*peer = NULL;
			*opp_peer = NULL;
	}
	
	FWD_IRQ_UNLOCK(&global_band_tbl_lock, global_band_tbl_irq_flags);
	
}

void wifi_fwd_insert_repeater_mapping(void *adapter, void *lock, void *cli_mapping, void *map_mapping, void *ifAddr_mapping)
{

	if (!WIFI_FWD_TEST_FLAG(fOP_WIFI_FWD_ENABLED))
		return;

	FWD_IRQ_LOCK(&global_band_tbl_lock, global_band_tbl_irq_flags);

	if ( adapter == adapter_2g ) {
		global_map_2g_tbl.EntryLock = lock;
		global_map_2g_tbl.CliHash = cli_mapping;
		global_map_2g_tbl.MapHash = map_mapping;
		global_map_2g_tbl.Wdev_ifAddr = ifAddr_mapping;
		global_map_2g_tbl.Enabled = TRUE;
		
	} else if ( adapter == adapter_5g ) {
		global_map_5g_tbl.EntryLock = lock;
		global_map_5g_tbl.CliHash = cli_mapping;
		global_map_5g_tbl.MapHash = map_mapping;
		global_map_5g_tbl.Wdev_ifAddr = ifAddr_mapping;
		global_map_5g_tbl.Enabled = TRUE;
	}

	if (0) {
		dbg_print("[%s] global_map_2g_tbl address %p\n", __FUNCTION__, &global_map_2g_tbl);
		dbg_print("[%s] global_map_5g_tbl address %p\n", __FUNCTION__, &global_map_5g_tbl);
	}
	

	FWD_IRQ_UNLOCK(&global_band_tbl_lock, global_band_tbl_irq_flags);
	
}

void wifi_fwd_insert_bridge_mapping(struct sk_buff *skb)
{
	struct net_device *net_dev = NULL;
	struct APCLI_BRIDGE_LEARNING_MAPPING_STRUCT *map_tbl_entry = NULL;
	unsigned char pkt_from, tbl_size = 0;

	net_dev = skb->dev;

	if (!WIFI_FWD_TEST_FLAG(fOP_WIFI_FWD_ENABLED))
		return;

	if ((net_dev == ap_5g) || (net_dev == apcli_5g))
		pkt_from = WIFI_FWD_PACKET_SPECIFIC_5G;
	else
		pkt_from = WIFI_FWD_PACKET_SPECIFIC_2G;

	FWD_IRQ_LOCK(&global_map_tbl_lock, global_map_tbl_irq_flags);

	if (global_map_tbl.entry_num < 0) {
		FWD_IRQ_UNLOCK(&global_map_tbl_lock, global_map_tbl_irq_flags);
		dbg_print("[%s] no entry found in the list\n", __FUNCTION__);
		return;
	}
		
	tbl_size = sizeof(struct APCLI_BRIDGE_LEARNING_MAPPING_STRUCT);
	map_tbl_entry = (struct APCLI_BRIDGE_LEARNING_MAPPING_STRUCT *) kmalloc(tbl_size, GFP_ATOMIC);	

	if (map_tbl_entry) {
		memset(map_tbl_entry, 0, tbl_size);
		dbg_print("[%s] size of map_tbl_entry = %dbytes\n", __FUNCTION__, tbl_size);
	} else
		return;

	map_tbl_entry->rcvd_net_dev = net_dev;
	map_tbl_entry->entry_from = pkt_from;
	memcpy(map_tbl_entry->src_addr, ((unsigned char *)skb->data)[6], ETH_ALEN);

	if (global_map_tbl.entry_num == 0) {
		global_map_tbl.pHead = map_tbl_entry;
		global_map_tbl.pTail = map_tbl_entry;
		map_tbl_entry->pBefore = NULL;
		map_tbl_entry->pNext = NULL;
	}
	else if (global_map_tbl.entry_num > 0) {
		global_map_tbl.pTail->pNext = map_tbl_entry;
		map_tbl_entry->pBefore = global_map_tbl.pTail;
		global_map_tbl.pTail = map_tbl_entry;
	}
	
	global_map_tbl.entry_num++;
	FWD_IRQ_UNLOCK(&global_map_tbl_lock, global_map_tbl_irq_flags);
		
	if (1) {
		dbg_print("[%s] %s\n", __FUNCTION__, (pkt_from == WIFI_FWD_PACKET_SPECIFIC_5G ? "5G" : "2.4G"));
		dbg_print("[%s] inserting mac addr = %02X:%02X:%02X:%02X:%02X:%02X\n", 
			__FUNCTION__, PRINT_MAC(map_tbl_entry->src_addr));
		dbg_print("[%s] rcvd from %s\n", __FUNCTION__, WIFI_FWD_NETDEV_GET_DEVNAME(map_tbl_entry->rcvd_net_dev));
	}
	
	return;
}


/* 
	return value:
	1:	success
	0:	fail
*/
static int wifi_fwd_search_mapping_table(struct sk_buff *skb, struct APCLI_BRIDGE_LEARNING_MAPPING_STRUCT **tbl_entry)
{
	struct net_device *net_dev = NULL;
	struct APCLI_BRIDGE_LEARNING_MAPPING_STRUCT *map_tbl_entry = NULL;
	unsigned char pkt_from, idx = 0;

	net_dev = skb->dev;

	if (!WIFI_FWD_TEST_FLAG(fOP_WIFI_FWD_ENABLED))
		return 0;

	if ((net_dev == ap_5g) || (net_dev == apcli_5g))
		pkt_from = WIFI_FWD_PACKET_SPECIFIC_5G;
	else
		pkt_from = WIFI_FWD_PACKET_SPECIFIC_2G;

	FWD_IRQ_LOCK(&global_map_tbl_lock, global_map_tbl_irq_flags);

	if (global_map_tbl.entry_num <= 0) {
		FWD_IRQ_UNLOCK(&global_map_tbl_lock, global_map_tbl_irq_flags);
		dbg_print("[%s] no entry found in the list\n", __FUNCTION__);
		return 0;
	}
	else
	{
		if (global_map_tbl.pHead != NULL) {
			map_tbl_entry = global_map_tbl.pHead;
		}
		else 
		{
			FWD_IRQ_UNLOCK(&global_map_tbl_lock, global_map_tbl_irq_flags);
			return 0;
		}

		for (idx=0; idx<global_map_tbl.entry_num; idx++)
		{
			if (MAC_ADDR_EQUAL(map_tbl_entry->src_addr, ((unsigned char *)skb->data)[6]) && 
				(net_dev != map_tbl_entry->rcvd_net_dev))
			{
				if (map_tbl_entry->entry_from == WIFI_FWD_PACKET_SPECIFIC_5G) {
					/* indicate this entry exist in dual band. packets sending to this entry need to be monitored */
					map_tbl_entry->entry_from |= pkt_from; 
					map_tbl_entry->rcvd_net_dev = net_dev;
				}
				else 
				{
					/* make sure the net device reported in the packet is up */
					if (dev_get_flags(map_tbl_entry->rcvd_net_dev) & IFF_UP) {
						map_tbl_entry->entry_from |= pkt_from; /* indicate this entry exist in dual band. packet need to send to this */
						SET_OS_PKT_NETDEV(RTPKT_TO_OSPKT(skb), map_tbl_entry->rcvd_net_dev); /* change net_device of packet to 2G */
					}
				}
				
				*tbl_entry = map_tbl_entry;
				FWD_IRQ_UNLOCK(&global_map_tbl_lock, global_map_tbl_irq_flags);
				return 1;
			}
			else if (MAC_ADDR_EQUAL(map_tbl_entry->src_addr, ((unsigned char *)skb->data)[6]) && 
				(net_dev == map_tbl_entry->rcvd_net_dev))
			{
				*tbl_entry = map_tbl_entry;
				FWD_IRQ_UNLOCK(&global_map_tbl_lock, global_map_tbl_irq_flags);
				return 1;
			}
			
			map_tbl_entry = map_tbl_entry->pNext;
		}	

		FWD_IRQ_UNLOCK(&global_map_tbl_lock, global_map_tbl_irq_flags);
	}

	return 0;
}


/* 
	return value:
	1:	Allocate Success
	0:	Allocate FAIL
*/
static int wifi_fwd_alloc_tbl(unsigned int NumOfEntry)
{
	unsigned int TblSize = 0;

	TblSize = NumOfEntry * sizeof(struct FwdPair);
	WiFiFwdBase = (struct FwdPair *) kmalloc(TblSize, GFP_ATOMIC);
	if (WiFiFwdBase) {
		memset(WiFiFwdBase, 0, TblSize);
		dbg_print("[%s] size of WiFiFwdBase = %dbytes\n", __FUNCTION__, TblSize);
	}
	else
		return 0;

	TblSize = NumOfEntry * sizeof(struct PacketSource);
	pkt_src = (struct PacketSource *) kmalloc(TblSize, GFP_ATOMIC);
	if (pkt_src) {
		memset(pkt_src, 0, TblSize);
		dbg_print("[%s] size of pkt_src = %dbytes\n", __FUNCTION__, TblSize);
	}
	else
		return 0;

	TblSize = NumOfEntry * sizeof(struct TxSourceEntry);
	tx_src_tbl = (struct TxSourceEntry *) kmalloc(TblSize, GFP_ATOMIC);
	if (tx_src_tbl) {
		memset(tx_src_tbl, 0, TblSize);
		dbg_print("[%s] size of tx_src_tbl = %dbytes\n", __FUNCTION__, TblSize);
	}
	else
		return 0;
	
	TblSize = sizeof(struct APCLI_BRIDGE_LEARNING_MAPPING_MAP);
	memset(&global_map_tbl, 0, TblSize);

	TblSize = sizeof(struct _REPEATER_ADAPTER_DATA_TABLE);
	memset(&global_map_2g_tbl, 0, TblSize);
	memset(&global_map_5g_tbl, 0, TblSize);
	
	return 1;
}


static unsigned char wifi_fwd_check_and_forward(struct sk_buff *skb)
{
	struct ethhdr *mh = NULL;
	struct iphdr *iph = NULL;
	struct udphdr *uh = NULL;
	struct tcphdr *th = NULL;
	struct ipv6hdr *ipv6h = NULL;
	void *ptr = NULL;
	unsigned short type = 0;

	mh = eth_hdr(skb);
	if (mh)
	{		
		type = ntohs(mh->h_proto);
		switch(type)
		{
			/*
				Forward LLTD EthType: 0x88D9
			*/
			case LLTD_ETH_TYPE:
#if 0
				dbg_print("Forward - LLTD_ETH_TYPE: 0x%02x\n", type);
#endif			
				if (!WIFI_FWD_TEST_FLAG(fOP_WIFI_FWD_ACCESS_SCHED_ACTIVE))
					return TRUE;
				break;

			/*
				Forward ARP EthType: 0x0806
			*/
			case ARP_ETH_TYPE:
#if 0
				dbg_print("Forward - ARP_ETH_TYPE: 0x%02x\n", type);
#endif		
				if (WIFI_FWD_TEST_FLAG(fOP_WIFI_FWD_ACCESS_SCHED_ACTIVE))
					return TRUE;
				break;
				
			case ETH_P_IP:
				iph = (struct iphdr *)(skb->data);
#if 0
				hex_dump_("Data", skb->data, (iph->ihl<<2));
#endif
				if (iph)
				{
					ptr = (void *)(skb->data+(iph->ihl<<2));
					if (ptr)
					{
#if 0
						dbg_print("wifi_fwd_check_and_forward - iph->protocol: 0x%02x\n", iph->protocol);
#endif
						switch(iph->protocol)
						{
							case UDP:
								/*
									Forward UDP port 53 and 67
								*/
								uh = (struct udphdr*)(ptr);
								if ((ntohs(uh->source) == 53) || (ntohs(uh->dest) == 53) ||
									(ntohs(uh->source) == 67) || (ntohs(uh->dest) == 67))
								{
#if 0
									dbg_print("Forward - udp source port: %d, dest port: %d\n", ntohs(uh->source), ntohs(uh->dest));
#endif
									return TRUE;
								}
								break;
								
							case TCP:
								/*
									Forward TCP port 80 and 5000
								*/
								th = (struct tcphdr *)(ptr);
								if ((ntohs(th->source) == 80) || 
									(ntohs(th->dest) == 80) ||
									(ntohs(th->source) == 5000) || 
									(ntohs(th->dest) == 5000))
								{
#if 0
									dbg_print("Forward - tcp source port: %d, dest port: %d\n", ntohs(th->source), ntohs(th->dest));
#endif					
									if (!WIFI_FWD_TEST_FLAG(fOP_WIFI_FWD_ACCESS_SCHED_ACTIVE))
										return TRUE;
								}
								break;
								
							default:
								break;
						}
					}	
				}
				break;

			case ETH_P_IPV6:
#if 0
				dbg_print("IPv6: IPV6_HDR_LEN = %d\n", IPV6_HDR_LEN);
				hex_dump_("Data", skb->data, LENGTH_802_3+IPV6_HDR_LEN);
#endif
				ipv6h = (struct ipv6hdr *)(skb->data);
				if (ipv6h) 
				{
#if 0
					dbg_print("ipv6h->version = 0x%x\n", ipv6h->version);
					dbg_print("ipv6h->nexthdr = 0x%x\n", ipv6h->nexthdr);
#endif
					ptr = (void *)(skb->data+IPV6_HDR_LEN);
					if (ptr)
					{
						switch(ipv6h->nexthdr)
						{
							/*
								Forward IPv6 UDP port 53
							*/
							case IPV6_NEXT_HEADER_UDP:
								uh = (struct udphdr*)(ptr);
#if 0
								dbg_print("udp source port: %d, dest port: %d\n", ntohs(uh->source), ntohs(uh->dest));
#endif
								if ((ntohs(uh->source) == 53) || (ntohs(uh->dest) == 53))
								{
									return TRUE;
								}
								break;
								
							default:
								break;
						}
					}
				}
				break;
				
			default:
				break;
		}
	}
	
	return FALSE;
}

/* 
	return value:
	0:	return to driver and handle
	1:	return to driver and release
*/
static int wifi_fwd_tx_handler(struct sk_buff *skb)
{
	struct net_device *net_dev = NULL;	
	net_dev = skb->dev;
	unsigned char ret = 0;

	/* 
		return this skb to driver and handle while:
		1. path of WiFi forwarding is inactive 
		2. the skb does not exist 
		3. no forwarding connection is established
	*/
	if (!WIFI_FWD_TEST_FLAG(fOP_WIFI_FWD_ACTIVE) || !WIFI_FWD_TEST_FLAG(fOP_WIFI_FWD_ENABLED) || !skb || (fwd_counter == 0))
		return 0;

	ret = wifi_fwd_tx_lookup_entry(net_dev, skb);
	return ret;
}


/* 
	return value:
	0:	skb is handled by wifi_fwd module
	1:	return to driver and bridge
	2:   	return to driver and release
*/
static int wifi_fwd_rx_handler(struct sk_buff *skb)
{
	struct net_device *net_dev = NULL;
	struct net_device *target = NULL;
	struct ethhdr *mh = eth_hdr(skb);
	
	unsigned char type = ENTRY_TYPE_INVALID;
	net_dev = skb->dev;

	unsigned int recv_from = 0, band_from = 0, ret = 0;
	recv_from = WIFI_FWD_GET_PACKET_RECV_FROM(skb, recv_from_cb_offset);
	band_from = WIFI_FWD_GET_PACKET_BAND(skb, band_cb_offset);

	/* 
		return this skb to driver and bridge while:
		1. path of WiFi forwarding is inactive 
		2. the skb does not exist 
		3. no forwarding connection is established
		4. the destination is bridge
		5. in FastLane topology
		6. handling multicast/broadcast Rx
		7. hit hijack
	*/
	if (!WIFI_FWD_TEST_FLAG(fOP_WIFI_FWD_ACTIVE) || !WIFI_FWD_TEST_FLAG(fOP_WIFI_FWD_ENABLED) || !skb || (fwd_counter == 0))
		return 1;

	if (IS_PACKET_FROM_APCLI(recv_from) && !MAIN_SSID_OP(net_dev))
		return 2;

	/* handle access schedule */
	if (WIFI_FWD_TEST_FLAG(fOP_WIFI_FWD_ACCESS_SCHED_ACTIVE))
	{
		if (IS_PACKET_FROM_APCLI(recv_from))
		{
			if (wifi_fwd_check_and_forward(skb) == FALSE)
				return 2;
		}
	}
	
	/* handle packets from bridge no matter unicast or broadcast */
	if (MAC_ADDR_EQUAL(mh->h_dest, br0->dev_addr))
		return 1;

#if 0
	if (((mh->h_dest[0] & 0x1) == 0x1) && ((mh->h_source[0] & 0x2) == 0x2))
	{
		void * adapter = NULL;
		
		if (IS_PACKET_FROM_2G(band_from) && (adapter_2g != NULL))
			adapter = adapter_2g;
		else if (IS_PACKET_FROM_5G(band_from) && (adapter_5g != NULL))
			adapter = adapter_5g;
	
		if (IS_PACKET_FROM_APCLI(recv_from) && (adapter != NULL) &&
			RTMPQueryLookupRepeaterCliEntry(adapter, mh->h_source) == TRUE)
			return 2;
	}
#endif

	/* 
		in FastLane topology: 
		1. forward Rx packets to bridge and let flooding work done by bridge 
		2. no matter unicast or multicast/broadcast 
	*/
	if (is_wifi_fastlane_mode(net_dev) && !GUEST_SSID_OP(net_dev))
		return 1;

	/* handle looping */
	ret = wifi_fwd_check_looping(skb, net_dev);
	if(ret == 1)
		return 2;
	else if(ret == 2)
		return 1;
		
	/* handle multicast/broadcast Rx */
	if ((mh->h_dest[0] & 0x1) == 0x1)
		return 1;

	/* handle hijack */
	if (WIFI_FWD_TEST_FLAG(fOP_WIFI_FWD_HIJACK_ACTIVE))
	{
		if (wifi_fwd_check_and_forward(skb) == TRUE)
		{
			WIFI_FWD_SET_PACKET_BAND(skb,WIFI_FWD_PACKET_SPECIFIC_TAG, band_cb_offset);
			return 1;
		}
	}

	target = wifi_fwd_lookup_entry(net_dev, &type, skb);

	/* handle unicast Rx for non-FastLane cases */
	if (target != NULL)
	{
#if 0 /* move to earlier and handle it */
		/* in FastLane case, forward Rx packets to bridge and let flooding work done by bridge */
		if (is_wifi_fastlane_mode(net_dev))
			return 1;
#endif
		/* prevent from looping */
		if (wifi_fwd_find_sa_entry_by_sa_and_nd(skb, net_dev, 0) == 0)
		{
			return 2;
		}
		else
		{
			unsigned char hit = 0;

			/* forward to bridge if it is not a WiFi net device back-end packet */
			hit = wifi_fwd_check_from_bridge_by_dest_addr(skb, net_dev);
			
			if (hit == 0)
			{
				if (is_wifi_concurrent_mode(net_dev) &&
					IS_PACKET_FROM_ETHER(band_from))	
				{
					if (rep_net_dev == eth_traffic_band_5g)
					{
						if (REP_IS_5G(net_dev))
							return 1;
						else
							return 2;
					}
					else if (rep_net_dev == eth_traffic_band_2g)
					{
						if (REP_IS_2G(net_dev))
							return 1;
						else
							return 2;
					}
				}
				else
					return 1;
			}
			else if (hit == 2)
			{
				/* match inner communication case */
				return 1;
			}
			else
			{
				skb_push(skb, ETH_HLEN);	
				skb->dev = target;
				dev_queue_xmit(skb);
				return 0;
			}
		}
	} 
	else
		return 1;
}


static int wifi_fwd_init_mod(void)
{
	if (!wifi_fwd_alloc_tbl(WIFI_FWD_TBL_SIZE)) {
		return -ENOMEM; /* memory allocation failed */
	}
	
	WIFI_FWD_CLEAR_FLAG(fOP_GET_NET_DEVICE_STATUS_DONE);
	WIFI_FWD_CLEAR_FLAG(fOP_WIFI_FWD_ACCESS_SCHED_ACTIVE);
	WIFI_FWD_SET_FLAG(fOP_WIFI_FWD_ENABLED);
	WIFI_FWD_SET_FLAG(fOP_WIFI_FWD_ACTIVE);
	WIFI_FWD_SET_FLAG(fOP_WIFI_FWD_HIJACK_ACTIVE);

	wifi_fwd_reset_link_count();
	
	eth_rep5g_wrg_uni_cnt = 0;
	eth_rep5g_wrg_bct_cnt = 0;
	eth_rep2g_wrg_uni_cnt = 0;
	eth_rep2g_wrg_bct_cnt = 0;
	rep_net_dev = eth_traffic_band_5g;
	band_cb_offset = DEFAULT_BAND_CB_OFFSET;
	recv_from_cb_offset = DEFAULT_RECV_FROM_CB_OFFSET;

	wf_fwd_tx_hook = wifi_fwd_tx_handler;
	wf_fwd_rx_hook = wifi_fwd_rx_handler;
	wf_fwd_entry_insert_hook = wifi_fwd_insert_entry;
	wf_fwd_entry_delete_hook = wifi_fwd_delete_entry;
	wf_fwd_set_cb_num  = wifi_fwd_set_cb_num;
	wf_fwd_get_rep_hook = wifi_fwd_get_rep;
	wf_fwd_pro_active_hook = wifi_fwd_pro_active;
	wf_fwd_pro_halt_hook = wifi_fwd_pro_halt;
	wf_fwd_pro_enabled_hook = wifi_fwd_pro_enabled;
	wf_fwd_pro_disabled_hook = wifi_fwd_pro_disabled;
	wf_fwd_access_schedule_active_hook = wifi_fwd_access_schedule_active;
	wf_fwd_access_schedule_halt_hook = wifi_fwd_access_schedule_halt;
	wf_fwd_hijack_active_hook = wifi_fwd_hijack_active;
	wf_fwd_hijack_halt_hook = wifi_fwd_hijack_halt;
	wf_fwd_show_entry_hook = wifi_fwd_show_entry;
	wf_fwd_needed_hook = wifi_fwd_needed;
	wf_fwd_delete_entry_hook = wifi_fwd_delete_entry_by_idx;
	packet_source_show_entry_hook = packet_source_show_entry;
	packet_source_delete_entry_hook = packet_source_delete_entry;
	wf_fwd_feedback_peer_adapter = wifi_fwd_feedback_peer_adapter;
	wf_fwd_feedback_map_table = wifi_fwd_feedback_map_table;
	wf_fwd_probe_adapter = wifi_fwd_probe_adapter;
	wf_fwd_insert_bridge_mapping_hook = wifi_fwd_insert_bridge_mapping;
	wf_fwd_insert_repeater_mapping_hook = wifi_fwd_insert_repeater_mapping;
	wf_fwd_search_mapping_table_hook = wifi_fwd_search_mapping_table;
	wf_fwd_delete_entry_inform_hook = wf_fwd_delete_entry_inform;

	return 0;
}


static void wifi_fwd_cleanup_mod(void)
{
	if (WiFiFwdBase)
		kfree(WiFiFwdBase);
	WiFiFwdBase = NULL;
	
	if (pkt_src)
		kfree(pkt_src);
	pkt_src = NULL;
	
	if(tx_src_tbl)
		kfree(tx_src_tbl);
	tx_src_tbl = NULL;

	wf_fwd_tx_hook = NULL;
	wf_fwd_rx_hook = NULL;
	wf_fwd_entry_insert_hook = NULL;
	wf_fwd_entry_delete_hook = NULL;
	wf_fwd_set_cb_num  = NULL;
	wf_fwd_get_rep_hook = NULL;
	wf_fwd_pro_active_hook = NULL;
	wf_fwd_pro_halt_hook = NULL;
	wf_fwd_pro_enabled_hook = NULL;
	wf_fwd_pro_disabled_hook = NULL;
	wf_fwd_access_schedule_active_hook = NULL;
	wf_fwd_access_schedule_halt_hook = NULL;
	wf_fwd_hijack_active_hook = NULL;
	wf_fwd_hijack_halt_hook = NULL;
	wf_fwd_show_entry_hook = NULL;
	wf_fwd_needed_hook = NULL;
	wf_fwd_delete_entry_hook = NULL;
	packet_source_show_entry_hook = NULL;
	packet_source_delete_entry_hook = NULL;
	wf_fwd_feedback_peer_adapter = NULL;
	wf_fwd_feedback_map_table = NULL;
	wf_fwd_probe_adapter = NULL;
	wf_fwd_insert_bridge_mapping_hook = NULL;
	wf_fwd_insert_repeater_mapping_hook = NULL;
	wf_fwd_search_mapping_table_hook = NULL;
	wf_fwd_delete_entry_inform_hook = NULL;

	return;
}


module_init(wifi_fwd_init_mod);
module_exit(wifi_fwd_cleanup_mod);

MODULE_AUTHOR("MediaTek Inc");
MODULE_LICENSE("Proprietary");
MODULE_DESCRIPTION("MediaTek WiFi Packet Forwarding Module\n"); 

