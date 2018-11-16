/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2006, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attempt
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    
    hook.c

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Name        Date            Modification logs
    Steven Liu  2006-10-06      Initial version
*/

#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/skbuff.h>

#include "../hw_nat/ra_nat.h"
#include "../hw_nat/foe_fdb.h"
#include "../hw_nat/frame_engine.h"
#include "../hw_nat/sys_rfrw.h"
#include "../hw_nat/policy.h"
#include "../hw_nat/util.h"
#include "../hw_nat/hwnat_ioctl.h"
#include "api.h"

extern struct FoeEntry		*PpeFoeBase;

int hash_ipv6(struct FoePriKey *key, struct FoeEntry *foe_entry, int del)
{
	uint32_t t_hvt_31, t_hvt_63, t_hvt_95, t_hvt_sd;
	uint32_t t_hvt_sd_23, t_hvt_sd_31_24, t_hash_32, t_hashs_16, t_ha16k, hash_index;	
	uint32_t ppeSaddr_127_96, ppeSaddr_95_64, ppeSaddr_63_32, ppeSaddr_31_0;
	uint32_t ppeDaddr_127_96, ppeDaddr_95_64, ppeDaddr_63_32, ppeDaddr_31_0;
	uint32_t ipv6_sip_127_96, ipv6_sip_95_64, ipv6_sip_63_32, ipv6_sip_31_0;
	uint32_t ipv6_dip_127_96, ipv6_dip_95_64, ipv6_dip_63_32, ipv6_dip_31_0;
	uint32_t sport, dport, ppeSportV6, ppeDportV6;
	
	ipv6_sip_127_96 = key->ipv6_routing.sip0;
	ipv6_sip_95_64 = key->ipv6_routing.sip1;
	ipv6_sip_63_32 = key->ipv6_routing.sip2;
	ipv6_sip_31_0 = key->ipv6_routing.sip3;
	ipv6_dip_127_96 = key->ipv6_routing.dip0;
	ipv6_dip_95_64 = key->ipv6_routing.dip1;
	ipv6_dip_63_32 = key->ipv6_routing.dip2;
	ipv6_dip_31_0 = key->ipv6_routing.dip3;
	sport = key->ipv6_routing.sport;
	dport = key->ipv6_routing.dport;

	t_hvt_31 = ipv6_sip_31_0 ^ ipv6_dip_31_0 ^ (sport << 16 | dport);
	t_hvt_63 = ipv6_sip_63_32 ^ ipv6_dip_63_32 ^ ipv6_dip_127_96;
	t_hvt_95 = ipv6_sip_95_64 ^ ipv6_dip_95_64 ^ ipv6_sip_127_96;
#if defined (CONFIG_RA_HW_NAT_IPV6) && defined (CONFIG_RALINK_MT7621)
	int boundary_entry_offset[7] = {12, 25, 38, 51, 76, 89, 102};/*these entries are bad every 128 entries*/
	int entry_base = 0;
	int bad_entry, i, j;
#endif
	if (DFL_FOE_HASH_MODE == 1)	// hash mode 1
		t_hvt_sd = (t_hvt_31 & t_hvt_63) | ((~t_hvt_31) & t_hvt_95);
	else                            // hash mode 2
		t_hvt_sd = t_hvt_63 ^ ( t_hvt_31 & (~t_hvt_95));
			
	t_hvt_sd_23 = t_hvt_sd & 0xffffff;
	t_hvt_sd_31_24 = t_hvt_sd & 0xff000000;
	t_hash_32 = t_hvt_31 ^ t_hvt_63 ^ t_hvt_95 ^ ( (t_hvt_sd_23 << 8) | (t_hvt_sd_31_24 >> 24));
	t_hashs_16 = ((t_hash_32 & 0xffff0000) >> 16 ) ^ (t_hash_32 & 0xfffff);

	if (FOE_4TB_SIZ == 16384)
		t_ha16k = t_hashs_16 & 0x1fff;  // FOE_16k
	else if (FOE_4TB_SIZ == 8192)
		t_ha16k = t_hashs_16 & 0xfff;  // FOE_8k
	else if (FOE_4TB_SIZ == 4096)
		t_ha16k = t_hashs_16 & 0x7ff;  // FOE_4k
	else if (FOE_4TB_SIZ == 2048)
		t_ha16k = t_hashs_16 & 0x3ff;  // FOE_2k
	else
		t_ha16k = t_hashs_16 & 0x1ff;  // FOE_1k
	hash_index = (uint32_t)t_ha16k *2;

	foe_entry = &PpeFoeBase[hash_index];
	ppeSaddr_127_96 = foe_entry->ipv6_5t_route.ipv6_sip0;
	ppeSaddr_95_64 = foe_entry->ipv6_5t_route.ipv6_sip1;
	ppeSaddr_63_32 = foe_entry->ipv6_5t_route.ipv6_sip2;
	ppeSaddr_31_0 = foe_entry->ipv6_5t_route.ipv6_sip3;
		
	ppeDaddr_127_96 = foe_entry->ipv6_5t_route.ipv6_dip0;
	ppeDaddr_95_64 = foe_entry->ipv6_5t_route.ipv6_dip1;
	ppeDaddr_63_32 = foe_entry->ipv6_5t_route.ipv6_dip2;
	ppeDaddr_31_0 = foe_entry->ipv6_5t_route.ipv6_dip3;
		
	ppeSportV6 = foe_entry->ipv6_5t_route.sport;
	ppeDportV6 = foe_entry->ipv6_5t_route.dport;
	if (del !=1) {
		if (foe_entry->ipv6_5t_route.bfib1.state == BIND)
		{
			printk("IPV6 Hash collision, hash index +1\n");
			hash_index = hash_index + 1;
			foe_entry = &PpeFoeBase[hash_index];
		}
		if (foe_entry->ipv6_5t_route.bfib1.state == BIND)
		{
			printk("IPV6 Hash collision can not bind\n");
			return -1;
		}
#if defined (CONFIG_RA_HW_NAT_IPV6) && defined (CONFIG_RALINK_MT7621)
		for(i=0; entry_base < FOE_4TB_SIZ; i++) {
			for(j=0;j<7;j++)
				bad_entry = entry_base + boundary_entry_offset[j];
			entry_base = (i+1)*128;
			if (hash_index == bad_entry)
			{
				if ((hash_index % 2) == 1){
					printk("Entry index %d can not use\n", hash_index);
					return -1;
				}
				hash_index = hash_index + 1;
				return hash_index;
			}
		}
#endif	
	}else if(del == 1) {
		if ((ipv6_sip_127_96 == ppeSaddr_127_96) && (ipv6_sip_95_64 == ppeSaddr_95_64) 
			&& (ipv6_sip_63_32 == ppeSaddr_63_32) && (ipv6_sip_31_0 == ppeSaddr_31_0) && 
			(ipv6_dip_127_96 == ppeDaddr_127_96) && (ipv6_dip_95_64 == ppeDaddr_95_64) 
			&& (ipv6_dip_63_32 == ppeDaddr_63_32) && (ipv6_dip_31_0 == ppeDaddr_31_0) &&
			(sport == ppeSportV6) && (dport == ppeDportV6)) {
		} else {
			hash_index = hash_index + 1;
			foe_entry = &PpeFoeBase[hash_index];
			ppeSaddr_127_96 = foe_entry->ipv6_5t_route.ipv6_sip0;
			ppeSaddr_95_64 = foe_entry->ipv6_5t_route.ipv6_sip1;
			ppeSaddr_63_32 = foe_entry->ipv6_5t_route.ipv6_sip2;
			ppeSaddr_31_0 = foe_entry->ipv6_5t_route.ipv6_sip3;
			
			ppeDaddr_127_96 = foe_entry->ipv6_5t_route.ipv6_dip0;
			ppeDaddr_95_64 = foe_entry->ipv6_5t_route.ipv6_dip1;
			ppeDaddr_63_32 = foe_entry->ipv6_5t_route.ipv6_dip2;
			ppeDaddr_31_0 = foe_entry->ipv6_5t_route.ipv6_dip3;
			
			ppeSportV6 = foe_entry->ipv6_5t_route.sport;
			ppeDportV6 = foe_entry->ipv6_5t_route.dport;
			if ((ipv6_sip_127_96 == ppeSaddr_127_96) && (ipv6_sip_95_64 == ppeSaddr_95_64) 
				&& (ipv6_sip_63_32 == ppeSaddr_63_32) && (ipv6_sip_31_0 == ppeSaddr_31_0) && 
				(ipv6_dip_127_96 == ppeDaddr_127_96) && (ipv6_dip_95_64 == ppeDaddr_95_64) 
				&& (ipv6_dip_63_32 == ppeDaddr_63_32) && (ipv6_dip_31_0 == ppeDaddr_31_0) &&
				(sport == ppeSportV6) && (dport == ppeDportV6)) {
			} else {
#if defined (CONFIG_HW_NAT_SEMI_AUTO_MODE)
				printk("Ipv6 Entry delete : Entry Not found\n");
#elif defined (CONFIG_HW_NAT_MANUAL_MODE)
				printk("Ipv6 hash collision hwnat can not found\n");
#endif
				return -1;
			}
		}

	}
	return hash_index;
}

int hash_ipv4(struct FoePriKey *key, struct FoeEntry *foe_entry, int del)
{
	uint32_t t_hvt_31;
	uint32_t t_hvt_63;
	uint32_t t_hvt_95;
	uint32_t t_hvt_sd;

	uint32_t t_hvt_sd_23;
	uint32_t t_hvt_sd_31_24;
	uint32_t t_hash_32;
	uint32_t t_hashs_16; 
	uint32_t t_ha16k;
	uint32_t hash_index;	
	uint32_t ppeSaddr, ppeDaddr, ppeSport, ppeDport, saddr, daddr, sport, dport;
        saddr = key->ipv4_hnapt.sip;
        daddr = key->ipv4_hnapt.dip;
        sport = key->ipv4_hnapt.sport;
        dport = key->ipv4_hnapt.dport;

	t_hvt_31 = sport << 16 | dport;
	t_hvt_63 = daddr;
	t_hvt_95 = saddr;
#if defined (CONFIG_RA_HW_NAT_IPV6) && defined (CONFIG_RALINK_MT7621)
	int boundary_entry_offset[7] = {12, 25, 38, 51, 76, 89, 102};/*these entries are bad every 128 entries*/
	int entry_base = 0;
	int bad_entry, i, j;
#endif

	//printk("saddr = %x, daddr=%x, sport=%d, dport=%d\n", saddr, daddr, sport, dport);
	if (DFL_FOE_HASH_MODE == 1)	// hash mode 1
		t_hvt_sd = (t_hvt_31 & t_hvt_63) | ((~t_hvt_31) & t_hvt_95);
	else                            // hash mode 2
		t_hvt_sd = t_hvt_63 ^ ( t_hvt_31 & (~t_hvt_95));
			
	t_hvt_sd_23 = t_hvt_sd & 0xffffff;
	t_hvt_sd_31_24 = t_hvt_sd & 0xff000000;
	t_hash_32 = t_hvt_31 ^ t_hvt_63 ^ t_hvt_95 ^ ( (t_hvt_sd_23 << 8) | (t_hvt_sd_31_24 >> 24));
	t_hashs_16 = ((t_hash_32 & 0xffff0000) >> 16 ) ^ (t_hash_32 & 0xfffff);

	if (FOE_4TB_SIZ == 16384)
		t_ha16k = t_hashs_16 & 0x1fff;  // FOE_16k
	else if (FOE_4TB_SIZ == 8192)
		t_ha16k = t_hashs_16 & 0xfff;  // FOE_8k
	else if (FOE_4TB_SIZ == 4096)
		t_ha16k = t_hashs_16 & 0x7ff;  // FOE_4k
	else if (FOE_4TB_SIZ == 2048)
		t_ha16k = t_hashs_16 & 0x3ff;  // FOE_2k
	else
		t_ha16k = t_hashs_16 & 0x1ff;  // FOE_1k
	hash_index = (uint32_t)t_ha16k *2;

	foe_entry = &PpeFoeBase[hash_index];
	ppeSaddr = foe_entry->ipv4_hnapt.sip;
	ppeDaddr = foe_entry->ipv4_hnapt.dip;
	ppeSport = foe_entry->ipv4_hnapt.sport;
	ppeDport = foe_entry->ipv4_hnapt.dport;

	if (del !=1) {
		if (foe_entry->ipv4_hnapt.bfib1.state == BIND)
		{
			printk("Hash collision, hash index +1\n");
			hash_index = hash_index + 1;
			foe_entry = &PpeFoeBase[hash_index];
		}
		if (foe_entry->ipv4_hnapt.bfib1.state == BIND)
		{
			printk("Hash collision can not bind\n");
			return -1;
		}
#if defined (CONFIG_RA_HW_NAT_IPV6) && defined (CONFIG_RALINK_MT7621)
		for(i=0; entry_base < FOE_4TB_SIZ; i++) {
			for(j=0;j<7;j++)
				bad_entry = entry_base + boundary_entry_offset[j];
			entry_base = (i+1)*128;
			if (hash_index == bad_entry)
			{
				if ((hash_index % 2) == 1) {
					printk("Entry index %d can not use\n", hash_index);
					return -1;
				}
				hash_index = hash_index + 1;
				return hash_index;
			}
		}
#endif	
	} else if(del == 1) {
		if ((saddr == ppeSaddr) && (daddr == ppeDaddr) && (sport == ppeSport)
			&& (dport == ppeDport)){
		} else {
			hash_index = hash_index + 1;
			foe_entry = &PpeFoeBase[hash_index];
			ppeSaddr = foe_entry->ipv4_hnapt.sip;
			ppeDaddr = foe_entry->ipv4_hnapt.dip;
			ppeSport = foe_entry->ipv4_hnapt.sport;
			ppeDport = foe_entry->ipv4_hnapt.dport;
			if ((saddr == ppeSaddr) && (daddr == ppeDaddr) && (sport == ppeSport)
				&& (dport == ppeDport)){
			} else {
#if defined (CONFIG_HW_NAT_SEMI_AUTO_MODE)
				printk("hash collision hwnat can not foundn");	
#elif defined (CONFIG_HW_NAT_MANUAL_MODE)
				printk("Entry delete : Entry Not found\n");							
#endif
				return -1;
			}			
		}
	}
	return hash_index;
}

int GetPpeEntryIdx(struct FoePriKey *key, struct FoeEntry *foe_entry, int del)
{
	if ((key->pkt_type) == IPV4_NAPT)
		return hash_ipv4(key, foe_entry, del);
	else if((key->pkt_type) == IPV6_ROUTING)
		return hash_ipv6(key, foe_entry, del);
	else
		return -1;
}
EXPORT_SYMBOL(GetPpeEntryIdx);


