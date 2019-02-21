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
    hook.c

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Name        Date            Modification logs
    Annie Lu  2014-06-30      Initial version
*/

#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/skbuff.h>
#include <linux/net.h>
#include <linux/netdevice.h>


int (*wf_fwd_rx_hook) (struct sk_buff * skb) = NULL;
int (*wf_fwd_tx_hook) (struct sk_buff * skb) = NULL;

unsigned char (*wf_fwd_entry_insert_hook) (struct net_device *src, struct net_device *dest, void *adapter) = NULL;
unsigned char (*wf_fwd_entry_delete_hook) (struct net_device *src, struct net_device *dest, unsigned char link_down) = NULL;
void (*wf_fwd_set_cb_num) (unsigned int band_cb_num, unsigned int receive_cb_num) = NULL;

void (*wf_fwd_get_rep_hook) (unsigned char idx) = NULL;
void (*wf_fwd_pro_active_hook) (void) = NULL;
void (*wf_fwd_pro_halt_hook) (void) = NULL;
void (*wf_fwd_pro_enabled_hook) (void) = NULL;
void (*wf_fwd_pro_disabled_hook) (void) = NULL;

void (*wf_fwd_access_schedule_active_hook) (void) = NULL;
void (*wf_fwd_access_schedule_halt_hook) (void) = NULL;
void (*wf_fwd_hijack_active_hook) (void) = NULL;
void (*wf_fwd_hijack_halt_hook) (void) = NULL;

void (*wf_fwd_show_entry_hook) (void) = NULL;
bool (*wf_fwd_needed_hook) (void) = NULL;
void (*packet_source_show_entry_hook) (void) = NULL;
void (*packet_source_delete_entry_hook) (unsigned char idx) = NULL;
void (*wf_fwd_delete_entry_hook) (unsigned char idx) = NULL;

void (*wf_fwd_feedback_peer_adapter)(void *adapter, void *peer) = NULL;
void (*wf_fwd_feedback_map_table) (void *adapter, void *peer, void *opp_peer) = NULL;
void (*wf_fwd_probe_adapter)(void *adapter) = NULL;
void (*wf_fwd_insert_repeater_mapping_hook)(void *adapter, void *lock, void *cli_mapping, void *map_mapping, void *ifAddr_mapping) = NULL;
void (*wf_fwd_insert_bridge_mapping_hook) (struct sk_buff *skb) = NULL;
int (*wf_fwd_search_mapping_table_hook) (struct sk_buff *skb, struct APCLI_BRIDGE_LEARNING_MAPPING_STRUCT **tbl_entry) = NULL;
void (*wf_fwd_delete_entry_inform_hook) (unsigned char *addr) = NULL;


EXPORT_SYMBOL(wf_fwd_rx_hook);
EXPORT_SYMBOL(wf_fwd_tx_hook);
EXPORT_SYMBOL(wf_fwd_entry_insert_hook);
EXPORT_SYMBOL(wf_fwd_entry_delete_hook);
EXPORT_SYMBOL(wf_fwd_set_cb_num);
EXPORT_SYMBOL(wf_fwd_get_rep_hook);
EXPORT_SYMBOL(wf_fwd_pro_active_hook);
EXPORT_SYMBOL(wf_fwd_pro_halt_hook);
EXPORT_SYMBOL(wf_fwd_pro_enabled_hook);
EXPORT_SYMBOL(wf_fwd_pro_disabled_hook);
EXPORT_SYMBOL(wf_fwd_access_schedule_active_hook);
EXPORT_SYMBOL(wf_fwd_access_schedule_halt_hook);
EXPORT_SYMBOL(wf_fwd_hijack_active_hook);
EXPORT_SYMBOL(wf_fwd_hijack_halt_hook);
EXPORT_SYMBOL(wf_fwd_show_entry_hook);
EXPORT_SYMBOL(wf_fwd_needed_hook);
EXPORT_SYMBOL(wf_fwd_delete_entry_hook);
EXPORT_SYMBOL(packet_source_show_entry_hook);
EXPORT_SYMBOL(packet_source_delete_entry_hook);
EXPORT_SYMBOL(wf_fwd_feedback_peer_adapter);
EXPORT_SYMBOL(wf_fwd_feedback_map_table);
EXPORT_SYMBOL(wf_fwd_insert_repeater_mapping_hook);
EXPORT_SYMBOL(wf_fwd_probe_adapter);
EXPORT_SYMBOL(wf_fwd_insert_bridge_mapping_hook);
EXPORT_SYMBOL(wf_fwd_search_mapping_table_hook);
EXPORT_SYMBOL(wf_fwd_delete_entry_inform_hook);

