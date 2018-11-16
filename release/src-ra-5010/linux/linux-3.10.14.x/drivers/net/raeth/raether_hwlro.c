#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/skbuff.h>
#include <linux/if_vlan.h>
#include <linux/if_ether.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <asm/rt2880/surfboardint.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <asm/rt2880/rt_mmap.h>
#include "ra2882ethreg.h"
#include "raether.h"
#include "ra_mac.h"
#include "ra_ioctl.h"
#include "ra_rfrw.h"

extern char const *nvram_get(int index, char *name);

#if defined(CONFIG_RAETH_HW_LRO_FORCE)
int set_fe_lro_ring1_cfg(struct net_device *dev)
{
	unsigned int ip;

	netdev_printk(KERN_CRIT, dev, "set_fe_lro_ring1_cfg()\n");

	/* 1. Set RX ring mode to force port */
	SET_PDMA_RXRING_MODE(ADMA_RX_RING1, PDMA_RX_FORCE_PORT);

	/* 2. Configure lro ring */
	/* 2.1 set src/destination TCP ports */
	SET_PDMA_RXRING_TCP_SRC_PORT(ADMA_RX_RING1, 1122);
	SET_PDMA_RXRING_TCP_DEST_PORT(ADMA_RX_RING1, 3344);
	/* 2.2 set src/destination IPs */
	str_to_ip(&ip, "10.10.10.3");
	sysRegWrite(LRO_RX_RING1_SIP_DW0, ip);
	str_to_ip(&ip, "10.10.10.254");
	sysRegWrite(LRO_RX_RING1_DIP_DW0, ip);
	/* 2.3 IPv4 force port mode */
	SET_PDMA_RXRING_IPV4_FORCE_MODE(ADMA_RX_RING1, 1);
	/* 2.4 IPv6 force port mode */
	SET_PDMA_RXRING_IPV6_FORCE_MODE(ADMA_RX_RING1, 1);

	/* 3. Set Age timer: 10 msec. */
	SET_PDMA_RXRING_AGE_TIME(ADMA_RX_RING1, HW_LRO_AGE_TIME);

	/* 4. Valid LRO ring */
	SET_PDMA_RXRING_VALID(ADMA_RX_RING1, 1);

	return 0;
}

int set_fe_lro_ring2_cfg(struct net_device *dev)
{
	unsigned int ip;

	netdev_printk(KERN_CRIT, dev, "set_fe_lro_ring2_cfg()\n");

	/* 1. Set RX ring mode to force port */
	SET_PDMA_RXRING2_MODE(PDMA_RX_FORCE_PORT);

	/* 2. Configure lro ring */
	/* 2.1 set src/destination TCP ports */
	SET_PDMA_RXRING_TCP_SRC_PORT(ADMA_RX_RING2, 5566);
	SET_PDMA_RXRING_TCP_DEST_PORT(ADMA_RX_RING2, 7788);
	/* 2.2 set src/destination IPs */
	str_to_ip(&ip, "10.10.10.3");
	sysRegWrite(LRO_RX_RING2_SIP_DW0, ip);
	str_to_ip(&ip, "10.10.10.254");
	sysRegWrite(LRO_RX_RING2_DIP_DW0, ip);
	/* 2.3 IPv4 force port mode */
	SET_PDMA_RXRING_IPV4_FORCE_MODE(ADMA_RX_RING2, 1);
	/* 2.4 IPv6 force port mode */
	SET_PDMA_RXRING_IPV6_FORCE_MODE(ADMA_RX_RING2, 1);

	/* 3. Set Age timer: 10 msec. */
	SET_PDMA_RXRING_AGE_TIME(ADMA_RX_RING2, HW_LRO_AGE_TIME);

	/* 4. Valid LRO ring */
	SET_PDMA_RXRING_VALID(ADMA_RX_RING2, 1);

	return 0;
}

int set_fe_lro_ring3_cfg(struct net_device *dev)
{
	unsigned int ip;

	netdev_printk(KERN_CRIT, dev, "set_fe_lro_ring3_cfg()\n");

	/* 1. Set RX ring mode to force port */
	SET_PDMA_RXRING3_MODE(PDMA_RX_FORCE_PORT);

	/* 2. Configure lro ring */
	/* 2.1 set src/destination TCP ports */
	SET_PDMA_RXRING_TCP_SRC_PORT(ADMA_RX_RING3, 9900);
	SET_PDMA_RXRING_TCP_DEST_PORT(ADMA_RX_RING3, 99);
	/* 2.2 set src/destination IPs */
	str_to_ip(&ip, "10.10.10.3");
	sysRegWrite(LRO_RX_RING3_SIP_DW0, ip);
	str_to_ip(&ip, "10.10.10.254");
	sysRegWrite(LRO_RX_RING3_DIP_DW0, ip);
	/* 2.3 IPv4 force port mode */
	SET_PDMA_RXRING_IPV4_FORCE_MODE(ADMA_RX_RING3, 1);
	/* 2.4 IPv6 force port mode */
	SET_PDMA_RXRING_IPV6_FORCE_MODE(ADMA_RX_RING3, 1);

	/* 3. Set Age timer: 10 msec. */
	SET_PDMA_RXRING_AGE_TIME(ADMA_RX_RING3, HW_LRO_AGE_TIME);

	/* 4. Valid LRO ring */
	SET_PDMA_RXRING3_VALID(1);

	return 0;
}

int set_fe_lro_glo_cfg(struct net_device *dev)
{
	unsigned int regVal = 0;

	netdev_printk(KERN_CRIT, dev, "set_fe_lro_glo_cfg()\n");

	/* 1 Set max AGG timer: 10 msec. */
	SET_PDMA_LRO_MAX_AGG_TIME(HW_LRO_AGG_TIME);

	/* 2. Set max LRO agg count */
	SET_PDMA_LRO_MAX_AGG_CNT(HW_LRO_MAX_AGG_CNT);

	/* PDMA prefetch enable setting */
	SET_PDMA_LRO_RXD_PREFETCH_EN(0x3);

	/* 2.1 IPv4 checksum update enable */
	SET_PDMA_LRO_IPV4_CSUM_UPDATE_EN(1);

	/* 3. Polling relinguish */
	while (sysRegRead(ADMA_LRO_CTRL_DW0) & PDMA_LRO_RELINGUISH)
		;

	/* 4. Enable LRO */
	regVal = sysRegRead(ADMA_LRO_CTRL_DW0);
	regVal |= PDMA_LRO_EN;
	sysRegWrite(ADMA_LRO_CTRL_DW0, regVal);

	return 0;
}
#else

int set_fe_lro_auto_cfg(struct net_device *dev)
{
	unsigned int regVal = 0;
	char *lan_ip_tmp, *wan_ip_tmp;
	unsigned int lan_ip, wan_ip;

#ifdef CONFIG_DUAL_IMAGE
#define RT2860_NVRAM	1
#else
#define RT2860_NVRAM	0
#endif

	netdev_printk(KERN_CRIT, dev, "set_fe_lro_auto_cfg()\n");

	/* Get IP address from nvram */
	lan_ip_tmp = nvram_get(RT2860_NVRAM, "lan_ipaddr");
	str_to_ip(&lan_ip, lan_ip_tmp);
	printk("lan_ip = 0x%x\n", lan_ip);

	wan_ip_tmp = nvram_get(RT2860_NVRAM, "wan_ipaddr");
	str_to_ip(&wan_ip, wan_ip_tmp);
	printk("wan_ip = 0x%x\n", wan_ip);

	/* 1.1 Set my IP_1 */
	sysRegWrite(LRO_RX_RING0_DIP_DW0, lan_ip);
	sysRegWrite(LRO_RX_RING0_DIP_DW1, 0);
	sysRegWrite(LRO_RX_RING0_DIP_DW2, 0);
	sysRegWrite(LRO_RX_RING0_DIP_DW3, 0);
	SET_PDMA_RXRING_MYIP_VALID(ADMA_RX_RING0, 1);

#if 0
	/* 1.2 Set my IP_2 */
	sysRegWrite(LRO_RX_RING1_DIP_DW0, wan_ip);
	sysRegWrite(LRO_RX_RING1_DIP_DW1, 0);
	sysRegWrite(LRO_RX_RING1_DIP_DW2, 0);
	sysRegWrite(LRO_RX_RING1_DIP_DW3, 0);
	SET_PDMA_RXRING_MYIP_VALID(ADMA_RX_RING1, 1);

	/* 1.3 Set my IP_3 */
	sysRegWrite(LRO_RX_RING2_DIP_DW3, 0x20010238);
	sysRegWrite(LRO_RX_RING2_DIP_DW2, 0x08000000);
	sysRegWrite(LRO_RX_RING2_DIP_DW1, 0x00000000);
	sysRegWrite(LRO_RX_RING2_DIP_DW0, 0x00000254);
	SET_PDMA_RXRING_MYIP_VALID(ADMA_RX_RING2, 1);

	/* 1.4 Set my IP_4 */
	sysRegWrite(LRO_RX_RING3_DIP_DW3, 0x20010238);
	sysRegWrite(LRO_RX_RING3_DIP_DW2, 0x08010000);
	sysRegWrite(LRO_RX_RING3_DIP_DW1, 0x00000000);
	sysRegWrite(LRO_RX_RING3_DIP_DW0, 0x00000254);
	SET_PDMA_RXRING_MYIP_VALID(ADMA_RX_RING3, 1);
#endif

	/* 2.1 Set RX ring1~3 to auto-learn modes */
	SET_PDMA_RXRING_MODE(ADMA_RX_RING1, PDMA_RX_AUTO_LEARN);
	SET_PDMA_RXRING_MODE(ADMA_RX_RING2, PDMA_RX_AUTO_LEARN);
	SET_PDMA_RXRING_MODE(ADMA_RX_RING3, PDMA_RX_AUTO_LEARN);

	/* 2.2 Valid LRO ring */
	SET_PDMA_RXRING_VALID(ADMA_RX_RING0, 1);
	SET_PDMA_RXRING_VALID(ADMA_RX_RING1, 1);
	SET_PDMA_RXRING_VALID(ADMA_RX_RING2, 1);
	SET_PDMA_RXRING_VALID(ADMA_RX_RING3, 1);

	/* 2.3 Set AGE timer */
	SET_PDMA_RXRING_AGE_TIME(ADMA_RX_RING1, HW_LRO_AGE_TIME);
	SET_PDMA_RXRING_AGE_TIME(ADMA_RX_RING2, HW_LRO_AGE_TIME);
	SET_PDMA_RXRING_AGE_TIME(ADMA_RX_RING3, HW_LRO_AGE_TIME);

	/* 2.4 Set max AGG timer */
	SET_PDMA_RXRING_AGG_TIME(ADMA_RX_RING1, HW_LRO_AGG_TIME);
	SET_PDMA_RXRING_AGG_TIME(ADMA_RX_RING2, HW_LRO_AGG_TIME);
	SET_PDMA_RXRING_AGG_TIME(ADMA_RX_RING3, HW_LRO_AGG_TIME);

	/* 2.5 Set max LRO agg count */
	SET_PDMA_RXRING_MAX_AGG_CNT(ADMA_RX_RING1, HW_LRO_MAX_AGG_CNT);
	SET_PDMA_RXRING_MAX_AGG_CNT(ADMA_RX_RING2, HW_LRO_MAX_AGG_CNT);
	SET_PDMA_RXRING_MAX_AGG_CNT(ADMA_RX_RING3, HW_LRO_MAX_AGG_CNT);

	/* 3.0 IPv6 LRO enable */
	SET_PDMA_LRO_IPV6_EN(1);

	/* 3.1 IPv4 checksum update enable */
	SET_PDMA_LRO_IPV4_CSUM_UPDATE_EN(1);

	/* 3.2 TCP push option check disable */
	//SET_PDMA_LRO_IPV4_CTRL_PUSH_EN(0);

	/* PDMA prefetch enable setting */
	SET_PDMA_LRO_RXD_PREFETCH_EN(0x3);

	/* 3.2 switch priority comparison to byte count mode */
/* SET_PDMA_LRO_ALT_SCORE_MODE(PDMA_LRO_ALT_BYTE_CNT_MODE); */
	SET_PDMA_LRO_ALT_SCORE_MODE(PDMA_LRO_ALT_PKT_CNT_MODE);

	/* 3.3 bandwidth threshold setting */
	SET_PDMA_LRO_BW_THRESHOLD(HW_LRO_BW_THRE);

	/* 3.4 auto-learn score delta setting */
	sysRegWrite(LRO_ALT_SCORE_DELTA, HW_LRO_BW_THRE);

	/* 3.5 Set ALT timer to 20us: (unit: 20us) */
	SET_PDMA_LRO_ALT_REFRESH_TIMER_UNIT(HW_LRO_TIMER_UNIT);
	/* 3.6 Set ALT refresh timer to 1 sec. (unit: 20us) */
	SET_PDMA_LRO_ALT_REFRESH_TIMER(HW_LRO_REFRESH_TIME);

	/* 3.7 the least remaining room of SDL0 in RXD for lro aggregation */
	SET_PDMA_LRO_MIN_RXD_SDL(1522);

	/* 4. Polling relinguish */
	while (sysRegRead(ADMA_LRO_CTRL_DW0) & PDMA_LRO_RELINGUISH)
		;

	/* 5. Enable LRO */
	regVal = sysRegRead(ADMA_LRO_CTRL_DW0);
	regVal |= PDMA_LRO_EN;
	sysRegWrite(ADMA_LRO_CTRL_DW0, regVal);

	return 0;
}
#endif /* CONFIG_RAETH_HW_LRO_FORCE */

int fe_hw_lro_init(struct net_device *dev)
{
	int i;
	END_DEVICE *ei_local = netdev_priv(dev);

	/* Initial RX Ring 3 */
	ei_local->rx_ring3 =
	    pci_alloc_consistent(NULL, NUM_LRO_RX_DESC * sizeof(struct PDMA_rxdesc),
				 &ei_local->phy_rx_ring3);
	for (i = 0; i < NUM_LRO_RX_DESC; i++) {
		memset(&ei_local->rx_ring3[i], 0, sizeof(struct PDMA_rxdesc));
		ei_local->rx_ring3[i].rxd_info2.DDONE_bit = 0;
		ei_local->rx_ring3[i].rxd_info2.LS0 = 0;
		ei_local->rx_ring3[i].rxd_info2.PLEN0 =
		    SET_ADMA_RX_LEN0(MAX_LRO_RX_LENGTH);
		ei_local->rx_ring3[i].rxd_info2.PLEN1 =
		    SET_ADMA_RX_LEN1(MAX_LRO_RX_LENGTH >> 14);
		ei_local->rx_ring3[i].rxd_info1.PDP0 =
		    dma_map_single(NULL, ei_local->netrx3_skbuf[i]->data,
				   MAX_LRO_RX_LENGTH, PCI_DMA_FROMDEVICE);
	}
	netdev_printk(KERN_CRIT, dev,
		      "\nphy_rx_ring3 = 0x%08x, rx_ring3 = 0x%p\n",
		      ei_local->phy_rx_ring3, ei_local->rx_ring3);
	/* Initial RX Ring 2 */
	ei_local->rx_ring2 =
	    pci_alloc_consistent(NULL, NUM_LRO_RX_DESC * sizeof(struct PDMA_rxdesc),
				 &ei_local->phy_rx_ring2);
	for (i = 0; i < NUM_LRO_RX_DESC; i++) {
		memset(&ei_local->rx_ring2[i], 0, sizeof(struct PDMA_rxdesc));
		ei_local->rx_ring2[i].rxd_info2.DDONE_bit = 0;
		ei_local->rx_ring2[i].rxd_info2.LS0 = 0;
		ei_local->rx_ring2[i].rxd_info2.PLEN0 =
		    SET_ADMA_RX_LEN0(MAX_LRO_RX_LENGTH);
		ei_local->rx_ring2[i].rxd_info2.PLEN1 =
		    SET_ADMA_RX_LEN1(MAX_LRO_RX_LENGTH >> 14);
		ei_local->rx_ring2[i].rxd_info1.PDP0 =
		    dma_map_single(NULL, ei_local->netrx2_skbuf[i]->data,
				   MAX_LRO_RX_LENGTH, PCI_DMA_FROMDEVICE);
	}
	netdev_printk(KERN_CRIT, dev,
		      "\nphy_rx_ring2 = 0x%08x, rx_ring2 = 0x%p\n",
		      ei_local->phy_rx_ring2, ei_local->rx_ring2);
	/* Initial RX Ring 1 */
	ei_local->rx_ring1 =
	    pci_alloc_consistent(NULL, NUM_LRO_RX_DESC * sizeof(struct PDMA_rxdesc),
				 &ei_local->phy_rx_ring1);
	for (i = 0; i < NUM_LRO_RX_DESC; i++) {
		memset(&ei_local->rx_ring1[i], 0, sizeof(struct PDMA_rxdesc));
		ei_local->rx_ring1[i].rxd_info2.DDONE_bit = 0;
		ei_local->rx_ring1[i].rxd_info2.LS0 = 0;
		ei_local->rx_ring1[i].rxd_info2.PLEN0 =
		    SET_ADMA_RX_LEN0(MAX_LRO_RX_LENGTH);
		ei_local->rx_ring1[i].rxd_info2.PLEN1 =
		    SET_ADMA_RX_LEN1(MAX_LRO_RX_LENGTH >> 14);
		ei_local->rx_ring1[i].rxd_info1.PDP0 =
		    dma_map_single(NULL, ei_local->netrx1_skbuf[i]->data,
				   MAX_LRO_RX_LENGTH, PCI_DMA_FROMDEVICE);
	}
	netdev_printk(KERN_CRIT, dev,
		      "\nphy_rx_ring1 = 0x%08x, rx_ring1 = 0x%p\n",
		      ei_local->phy_rx_ring1, ei_local->rx_ring1);

	sysRegWrite(RX_BASE_PTR3, phys_to_bus((u32) ei_local->phy_rx_ring3));
	sysRegWrite(RX_MAX_CNT3, cpu_to_le32((u32) NUM_LRO_RX_DESC));
	sysRegWrite(RX_CALC_IDX3, cpu_to_le32((u32) (NUM_LRO_RX_DESC - 1)));
	sysRegWrite(PDMA_RST_CFG, PST_DRX_IDX3);
	sysRegWrite(RX_BASE_PTR2, phys_to_bus((u32) ei_local->phy_rx_ring2));
	sysRegWrite(RX_MAX_CNT2, cpu_to_le32((u32) NUM_LRO_RX_DESC));
	sysRegWrite(RX_CALC_IDX2, cpu_to_le32((u32) (NUM_LRO_RX_DESC - 1)));
	sysRegWrite(PDMA_RST_CFG, PST_DRX_IDX2);
	sysRegWrite(RX_BASE_PTR1, phys_to_bus((u32) ei_local->phy_rx_ring1));
	sysRegWrite(RX_MAX_CNT1, cpu_to_le32((u32) NUM_LRO_RX_DESC));
	sysRegWrite(RX_CALC_IDX1, cpu_to_le32((u32) (NUM_LRO_RX_DESC - 1)));
	sysRegWrite(PDMA_RST_CFG, PST_DRX_IDX1);

#if defined(CONFIG_RAETH_HW_LRO_FORCE)
	set_fe_lro_ring1_cfg(dev);
	set_fe_lro_ring2_cfg(dev);
	set_fe_lro_ring3_cfg(dev);
	set_fe_lro_glo_cfg(dev);
#else
	set_fe_lro_auto_cfg(dev);
#endif /* CONFIG_RAETH_HW_LRO_FORCE */

	/* HW LRO parameter settings */
	ei_local->hw_lro_alpha = HW_LRO_PKT_INT_ALPHA;
	ei_local->hw_lro_fix_setting = 1;

	return 1;
}
EXPORT_SYMBOL(fe_hw_lro_init);

