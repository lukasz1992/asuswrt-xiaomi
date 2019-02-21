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
#if defined (CONFIG_RAETH_TSO)
#include <linux/tcp.h>
#include <net/ipv6.h>
#include <linux/ip.h>
#include <net/ip.h>
#include <net/tcp.h>
#include <linux/in.h>
#include <linux/ppp_defs.h>
#include <linux/if_pppox.h>
#endif
#if defined (CONFIG_RAETH_LRO)
#include <linux/inet_lro.h>
#endif
#include <linux/delay.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
#include <linux/sched.h>
#endif

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
#include <asm/rt2880/rt_mmap.h>
#else
#include <linux/libata-compat.h>
#endif
 
#include "ra2882ethreg.h"
#include "raether.h"
#include "ra_mac.h"
#include "ra_ioctl.h"
#include "ra_rfrw.h"
#ifdef CONFIG_RAETH_NETLINK
#include "ra_netlink.h"
#endif
#if defined (CONFIG_RAETH_QOS)
#include "ra_qos.h"
#endif

#if defined (CONFIG_RA_HW_NAT) || defined (CONFIG_RA_HW_NAT_MODULE)
#include "../../../net/nat/hw_nat/ra_nat.h"
#endif
#if defined(CONFIG_RAETH_PDMA_DVT)
#include "dvt/raether_pdma_dvt.h"
#endif  /* CONFIG_RAETH_PDMA_DVT */

#if !defined(CONFIG_RA_NAT_NONE)
/* bruce+
 */
extern int (*ra_sw_nat_hook_rx)(struct sk_buff *skb);
extern int (*ra_sw_nat_hook_tx)(struct sk_buff *skb, int gmac_no);
#endif

#if defined(CONFIG_RA_CLASSIFIER)||defined(CONFIG_RA_CLASSIFIER_MODULE)
/* Qwert+
 */
#include <asm/mipsregs.h>
extern int (*ra_classifier_hook_tx)(struct sk_buff *skb, unsigned long cur_cycle);
extern int (*ra_classifier_hook_rx)(struct sk_buff *skb, unsigned long cur_cycle);
#endif /* CONFIG_RA_CLASSIFIER */

#if defined (CONFIG_RALINK_RT3052_MP2)
int32_t mcast_rx(struct sk_buff * skb);
int32_t mcast_tx(struct sk_buff * skb);
#endif

#if 0
#ifdef RA_MTD_RW_BY_NUM
int ra_mtd_read(int num, loff_t from, size_t len, u_char *buf);
#else
int ra_mtd_read_nm(char *name, loff_t from, size_t len, u_char *buf);
#endif
#endif
/* gmac driver feature set config */
#if defined (CONFIG_RAETH_NAPI) || defined (CONFIG_RAETH_QOS)
#undef DELAY_INT
#else
#if defined     (CONFIG_ARCH_MT7623)
#define DELAY_INT       1
#else
#define DELAY_INT       1
#endif
#endif

//#define CONFIG_UNH_TEST
/* end of config */

#if defined (CONFIG_RAETH_JUMBOFRAME)
#define	MAX_RX_LENGTH	4096
#else
#define	MAX_RX_LENGTH	1536
#endif

extern struct net_device		*dev_raether;


#if defined (CONFIG_RAETH_MULTIPLE_RX_RING)
#ifdef CONFIG_RAETH_RW_PDMAPTR_FROM_VAR
extern int rx_calc_idx1;
#endif
#endif
#ifdef CONFIG_RAETH_RW_PDMAPTR_FROM_VAR
extern int rx_calc_idx0;
static unsigned long tx_cpu_owner_idx0=0;
#endif
extern unsigned long tx_ring_full;

#if defined (CONFIG_ETHTOOL) /*&& defined (CONFIG_RAETH_ROUTER)*/
#include "ra_ethtool.h"
extern struct ethtool_ops	ra_ethtool_ops;
#ifdef CONFIG_PSEUDO_SUPPORT
extern struct ethtool_ops	ra_virt_ethtool_ops;
#endif // CONFIG_PSEUDO_SUPPORT //
#endif // (CONFIG_ETHTOOL //

#ifdef CONFIG_RALINK_VISTA_BASIC
int is_switch_175c = 1;
#endif

#ifdef CONFIG_RAETH_PDMATX_QDMARX	/* QDMA RX */
struct QDMA_txdesc *free_head = NULL;
#endif

//#if defined (CONFIG_RAETH_LRO)
#if 0
unsigned int lan_ip;
struct lro_para_struct lro_para; 
int lro_flush_needed;
extern char const *nvram_get(int index, char *name);
#endif

#define KSEG1                   0xa0000000
#define PHYS_TO_VIRT(x)         ((void *)((x) | KSEG1))
#define VIRT_TO_PHYS(x)         ((unsigned long)(x) & ~KSEG1)

extern void set_fe_dma_glo_cfg(void);

/*
 *  @brief cal txd number for a page
 *
 *  @parm size
 *
 *  @return frag_txd_num
 */

unsigned int cal_frag_txd_num(unsigned int size)
{
	unsigned int frag_txd_num = 0;
	if(size == 0)
		return 0;
	while(size > 0){
		if(size > MAX_TXD_LEN){
			frag_txd_num++;
			size -= MAX_TXD_LEN;
		}else{
			frag_txd_num++;
			size = 0;
		}
	}
	return frag_txd_num;

}

#ifdef CONFIG_RAETH_PDMATX_QDMARX	/* QDMA RX */
bool fq_qdma_init(struct net_device *dev)
{
	END_DEVICE* ei_local = netdev_priv(dev);
	unsigned int phy_free_head;
	unsigned int phy_free_tail;
	unsigned int *free_page_head = NULL;
	unsigned int phy_free_page_head;
	int i;
    
	free_head = pci_alloc_consistent(NULL, NUM_QDMA_PAGE * sizeof(struct QDMA_txdesc), &phy_free_head);
	if (unlikely(free_head == NULL)){
		printk(KERN_ERR "QDMA FQ decriptor not available...\n");
		return 0;
	}
	memset(free_head, 0x0, sizeof(struct QDMA_txdesc) * NUM_QDMA_PAGE);

	free_page_head = pci_alloc_consistent(NULL, NUM_QDMA_PAGE * QDMA_PAGE_SIZE, &phy_free_page_head);
	if (unlikely(free_page_head == NULL)){
		printk(KERN_ERR "QDMA FQ page not available...\n");
		return 0;
	}	
	for (i=0; i < NUM_QDMA_PAGE; i++) {
		free_head[i].txd_info1.SDP = (phy_free_page_head + (i * QDMA_PAGE_SIZE));
		if(i < (NUM_QDMA_PAGE-1)){
			free_head[i].txd_info2.NDP = (phy_free_head + ((i+1) * sizeof(struct QDMA_txdesc)));


#if 0
			printk("free_head_phy[%d] is 0x%x!!!\n",i, VIRT_TO_PHYS(&free_head[i]) );
			printk("free_head[%d] is 0x%x!!!\n",i, &free_head[i] );
			printk("free_head[%d].txd_info1.SDP is 0x%x!!!\n",i, free_head[i].txd_info1.SDP );
			printk("free_head[%d].txd_info2.NDP is 0x%x!!!\n",i, free_head[i].txd_info2.NDP );
#endif
		}
		free_head[i].txd_info3.SDL = QDMA_PAGE_SIZE;

	}
	phy_free_tail = (phy_free_head + (u32)((NUM_QDMA_PAGE-1) * sizeof(struct QDMA_txdesc)));

	printk("phy_free_head is 0x%x!!!\n", phy_free_head);
	printk("phy_free_tail_phy is 0x%x!!!\n", phy_free_tail);
	sysRegWrite(QDMA_FQ_HEAD, (u32)phy_free_head);
	sysRegWrite(QDMA_FQ_TAIL, (u32)phy_free_tail);
	sysRegWrite(QDMA_FQ_CNT, ((NUM_TX_DESC << 16) | NUM_QDMA_PAGE));
	sysRegWrite(QDMA_FQ_BLEN, QDMA_PAGE_SIZE << 16);

	ei_local->free_head = free_head;
	ei_local->phy_free_head = phy_free_head;
	ei_local->free_page_head = free_page_head;
	ei_local->phy_free_page_head = phy_free_page_head;
    return 1;
}
#endif

int fe_dma_init(struct net_device *dev)
{

	int		i;
	unsigned int	regVal;
	END_DEVICE* ei_local = netdev_priv(dev);
#if defined (CONFIG_RAETH_QOS)
	int		j;
#endif

	while(1)
	{
		regVal = sysRegRead(PDMA_GLO_CFG);
		if((regVal & RX_DMA_BUSY))
		{
			printk("\n  RX_DMA_BUSY !!! ");
			continue;
		}
		if((regVal & TX_DMA_BUSY))
		{
			printk("\n  TX_DMA_BUSY !!! ");
			continue;
		}
		break;
	}

#if defined(CONFIG_RAETH_PDMA_DVT)
	pdma_dvt_set_dma_mode();
#endif  /* CONFIG_RAETH_PDMA_DVT */

#if defined (CONFIG_RAETH_QOS)
	for (i=0;i<NUM_TX_RINGS;i++){
		for (j=0;j<NUM_TX_DESC;j++){
			ei_local->skb_free[i][j]=0;
		}
                ei_local->free_idx[i]=0;
	}
	/*
	 * RT2880: 2 x TX_Ring, 1 x Rx_Ring
	 * RT2883: 4 x TX_Ring, 1 x Rx_Ring
	 * RT3883: 4 x TX_Ring, 1 x Rx_Ring
	 * RT3052: 4 x TX_Ring, 1 x Rx_Ring
	 */
	fe_tx_desc_init(dev, 0, 3, 1);
	if (ei_local->tx_ring0 == NULL) {
		printk("RAETH: tx ring0 allocation failed\n");
		return 0;
	}

	fe_tx_desc_init(dev, 1, 3, 1);
	if (ei_local->tx_ring1 == NULL) {
		printk("RAETH: tx ring1 allocation failed\n");
		return 0;
	}

	printk("\nphy_tx_ring0 = %08x, tx_ring0 = %p, size: %d bytes\n", ei_local->phy_tx_ring0, ei_local->tx_ring0, sizeof(struct PDMA_txdesc));

	printk("\nphy_tx_ring1 = %08x, tx_ring1 = %p, size: %d bytes\n", ei_local->phy_tx_ring1, ei_local->tx_ring1, sizeof(struct PDMA_txdesc));

	fe_tx_desc_init(dev, 2, 3, 1);
	if (ei_local->tx_ring2 == NULL) {
		printk("RAETH: tx ring2 allocation failed\n");
		return 0;
	}

	fe_tx_desc_init(dev, 3, 3, 1);
	if (ei_local->tx_ring3 == NULL) {
		printk("RAETH: tx ring3 allocation failed\n");
		return 0;
	}

	printk("\nphy_tx_ring2 = %08x, tx_ring2 = %p, size: %d bytes\n", ei_local->phy_tx_ring2, ei_local->tx_ring2, sizeof(struct PDMA_txdesc));

	printk("\nphy_tx_ring3 = %08x, tx_ring3 = %p, size: %d bytes\n", ei_local->phy_tx_ring3, ei_local->tx_ring3, sizeof(struct PDMA_txdesc));

#else
	for (i=0;i<NUM_TX_DESC;i++){
		ei_local->skb_free[i]=0;
	}
	ei_local->free_idx =0;
#if defined (CONFIG_MIPS)
	ei_local->tx_ring0 = pci_alloc_consistent(NULL, NUM_TX_DESC * sizeof(struct PDMA_txdesc), &ei_local->phy_tx_ring0);
#else
	ei_local->tx_ring0 = dma_alloc_coherent(NULL, NUM_TX_DESC * sizeof(struct PDMA_txdesc), &ei_local->phy_tx_ring0, GFP_KERNEL);
#endif
	printk("\nphy_tx_ring = 0x%08x, tx_ring = 0x%p\n", ei_local->phy_tx_ring0, ei_local->tx_ring0);

	for (i=0; i < NUM_TX_DESC; i++) {
		memset(&ei_local->tx_ring0[i],0,sizeof(struct PDMA_txdesc));
		ei_local->tx_ring0[i].txd_info2.LS0_bit = 1;
		ei_local->tx_ring0[i].txd_info2.DDONE_bit = 1;

	}
#endif // CONFIG_RAETH_QOS

#ifdef CONFIG_RAETH_PDMATX_QDMARX	/* QDMA RX */

	fq_qdma_init(dev);

	while(1)
	{
		regVal = sysRegRead(QDMA_GLO_CFG);
		if((regVal & RX_DMA_BUSY))
		{
			printk("\n  RX_DMA_BUSY !!! ");
			continue;
		}
		if((regVal & TX_DMA_BUSY))
		{
			printk("\n  TX_DMA_BUSY !!! ");
			continue;
		}
		break;
	}

	/* Initial RX Ring 0*/
	
#ifdef CONFIG_32B_DESC
	ei_local->qrx_ring = kmalloc(NUM_QRX_DESC * sizeof(struct PDMA_rxdesc), GFP_KERNEL);
	ei_local->phy_qrx_ring = virt_to_phys(ei_local->qrx_ring);
#else
	ei_local->qrx_ring = pci_alloc_consistent(NULL, NUM_QRX_DESC * sizeof(struct PDMA_rxdesc), &ei_local->phy_qrx_ring);
#endif
	for (i = 0; i < NUM_QRX_DESC; i++) {
		memset(&ei_local->qrx_ring[i],0,sizeof(struct PDMA_rxdesc));
		ei_local->qrx_ring[i].rxd_info2.DDONE_bit = 0;
#if defined (CONFIG_RAETH_SCATTER_GATHER_RX_DMA)
		ei_local->qrx_ring[i].rxd_info2.LS0 = 0;
		ei_local->qrx_ring[i].rxd_info2.PLEN0 = MAX_RX_LENGTH;
#else
		ei_local->qrx_ring[i].rxd_info2.LS0 = 1;
#endif
		ei_local->qrx_ring[i].rxd_info1.PDP0 = dma_map_single(NULL, ei_local->netrx0_skbuf[i]->data, MAX_RX_LENGTH, PCI_DMA_FROMDEVICE);
	}
	printk("\nphy_qrx_ring = 0x%08x, qrx_ring = 0x%p\n",ei_local->phy_qrx_ring,ei_local->qrx_ring);

	regVal = sysRegRead(QDMA_GLO_CFG);
	regVal &= 0x000000FF;

	sysRegWrite(QDMA_GLO_CFG, regVal);
	regVal=sysRegRead(QDMA_GLO_CFG);

	/* Tell the adapter where the TX/RX rings are located. */

	sysRegWrite(QRX_BASE_PTR_0, phys_to_bus((u32) ei_local->phy_qrx_ring));
	sysRegWrite(QRX_MAX_CNT_0,  cpu_to_le32((u32) NUM_QRX_DESC));
	sysRegWrite(QRX_CRX_IDX_0, cpu_to_le32((u32) (NUM_QRX_DESC - 1)));
#ifdef CONFIG_RAETH_RW_PDMAPTR_FROM_VAR
	rx_calc_idx0 = rx_dma_owner_idx0 =  sysRegRead(QRX_CRX_IDX_0);
#endif
	sysRegWrite(QDMA_RST_CFG, PST_DRX_IDX0);

        ei_local->rx_ring0 = ei_local->qrx_ring;

#else	/* PDMA RX */

	/* Initial RX Ring 0*/
#ifdef CONFIG_32B_DESC
    	ei_local->rx_ring0 = kmalloc(NUM_RX_DESC * sizeof(struct PDMA_rxdesc), GFP_KERNEL);
	ei_local->phy_rx_ring0 = virt_to_phys(ei_local->rx_ring0);
#else
#if defined (CONFIG_MIPS)
	ei_local->rx_ring0 = pci_alloc_consistent(NULL, NUM_RX_DESC * sizeof(struct PDMA_rxdesc), &ei_local->phy_rx_ring0);
#else	
	ei_local->rx_ring0 = dma_alloc_coherent(NULL, NUM_RX_DESC * sizeof(struct PDMA_rxdesc), &ei_local->phy_rx_ring0, GFP_KERNEL);
#endif
#endif
	for (i = 0; i < NUM_RX_DESC; i++) {
		memset(&ei_local->rx_ring0[i],0,sizeof(struct PDMA_rxdesc));
	    	ei_local->rx_ring0[i].rxd_info2.DDONE_bit = 0;
#if defined (CONFIG_RAETH_SCATTER_GATHER_RX_DMA)
		ei_local->rx_ring0[i].rxd_info2.LS0 = 0;
		ei_local->rx_ring0[i].rxd_info2.PLEN0 = MAX_RX_LENGTH;
#else
		ei_local->rx_ring0[i].rxd_info2.LS0 = 1;
#endif
		ei_local->rx_ring0[i].rxd_info1.PDP0 = dma_map_single(NULL, ei_local->netrx0_skbuf[i]->data, MAX_RX_LENGTH + NET_IP_ALIGN, PCI_DMA_FROMDEVICE);
	}
	printk("\nphy_rx_ring0 = 0x%08x, rx_ring0 = 0x%p\n",ei_local->phy_rx_ring0,ei_local->rx_ring0);

#if defined (CONFIG_RAETH_MULTIPLE_RX_RING)
	/* Initial RX Ring 1*/
#ifdef CONFIG_32B_DESC
    	ei_local->rx_ring1 = kmalloc(NUM_RX_DESC * sizeof(struct PDMA_rxdesc), GFP_KERNEL);
	ei_local->phy_rx_ring1 = virt_to_phys(ei_local->rx_ring1);
#else
#if defined (CONFIG_MIPS)
	ei_local->rx_ring1 = pci_alloc_consistent(NULL, NUM_RX_DESC * sizeof(struct PDMA_rxdesc), &ei_local->phy_rx_ring1);
#else
	ei_local->rx_ring1 = dma_alloc_coherent(NULL, NUM_RX_DESC * sizeof(struct PDMA_rxdesc), &ei_local->phy_rx_ring1, GFP_KERNEL);

#endif
#endif
	for (i = 0; i < NUM_RX_DESC; i++) {
		memset(&ei_local->rx_ring1[i],0,sizeof(struct PDMA_rxdesc));
	    	ei_local->rx_ring1[i].rxd_info2.DDONE_bit = 0;
#if defined (CONFIG_RAETH_SCATTER_GATHER_RX_DMA)
		ei_local->rx_ring1[i].rxd_info2.LS0 = 0;
		ei_local->rx_ring1[i].rxd_info2.PLEN0 = MAX_RX_LENGTH;
#else
		ei_local->rx_ring1[i].rxd_info2.LS0 = 1;
#endif
		ei_local->rx_ring1[i].rxd_info1.PDP0 = dma_map_single(NULL, ei_local->netrx1_skbuf[i]->data, MAX_RX_LENGTH + NET_IP_ALIGN, PCI_DMA_FROMDEVICE);
	}
	printk("\nphy_rx_ring1 = 0x%08x, rx_ring1 = 0x%p\n",ei_local->phy_rx_ring1,ei_local->rx_ring1);
#if defined(CONFIG_ARCH_MT7623)
    /* Initial RX Ring 2*/
    ei_local->rx_ring2 = pci_alloc_consistent(NULL, NUM_RX_DESC * sizeof(struct PDMA_rxdesc), &ei_local->phy_rx_ring2);
    for (i = 0; i < NUM_RX_DESC; i++) {
        memset(&ei_local->rx_ring2[i],0,sizeof(struct PDMA_rxdesc));
        ei_local->rx_ring2[i].rxd_info2.DDONE_bit = 0;
        ei_local->rx_ring2[i].rxd_info2.LS0 = 0;
        ei_local->rx_ring2[i].rxd_info2.PLEN0 = SET_ADMA_RX_LEN0(MAX_RX_LENGTH);
        ei_local->rx_ring2[i].rxd_info2.PLEN1 = SET_ADMA_RX_LEN1(MAX_RX_LENGTH >> 14);
        ei_local->rx_ring2[i].rxd_info1.PDP0 = dma_map_single(NULL, ei_local->netrx2_skbuf[i]->data, MAX_RX_LENGTH + NET_IP_ALIGN, PCI_DMA_FROMDEVICE);
    }
    printk("\nphy_rx_ring2 = 0x%08x, rx_ring2 = 0x%p\n",ei_local->phy_rx_ring2,ei_local->rx_ring2);
    /* Initial RX Ring 3*/
	ei_local->rx_ring3 = pci_alloc_consistent(NULL, NUM_RX_DESC * sizeof(struct PDMA_rxdesc), &ei_local->phy_rx_ring3);
	for (i = 0; i < NUM_RX_DESC; i++) {
		memset(&ei_local->rx_ring3[i],0,sizeof(struct PDMA_rxdesc));
    	ei_local->rx_ring3[i].rxd_info2.DDONE_bit = 0;
        ei_local->rx_ring3[i].rxd_info2.LS0 = 0;
        ei_local->rx_ring3[i].rxd_info2.PLEN0 = SET_ADMA_RX_LEN0(MAX_RX_LENGTH);
        ei_local->rx_ring3[i].rxd_info2.PLEN1 = SET_ADMA_RX_LEN1(MAX_RX_LENGTH >> 14);
    	ei_local->rx_ring3[i].rxd_info1.PDP0 = dma_map_single(NULL, ei_local->netrx3_skbuf[i]->data, MAX_RX_LENGTH + NET_IP_ALIGN, PCI_DMA_FROMDEVICE);
	}
	printk("\nphy_rx_ring3 = 0x%08x, rx_ring3 = 0x%p\n",ei_local->phy_rx_ring3,ei_local->rx_ring3);	
#endif  /* CONFIG_ARCH_MT7623 */
#endif

#endif

	regVal = sysRegRead(PDMA_GLO_CFG);
	regVal &= 0x000000FF;
   	sysRegWrite(PDMA_GLO_CFG, regVal);
	regVal=sysRegRead(PDMA_GLO_CFG);

	/* Tell the adapter where the TX/RX rings are located. */
#if !defined (CONFIG_RAETH_QOS)
        sysRegWrite(TX_BASE_PTR0, phys_to_bus((u32) ei_local->phy_tx_ring0));
	sysRegWrite(TX_MAX_CNT0, cpu_to_le32((u32) NUM_TX_DESC));
	sysRegWrite(TX_CTX_IDX0, 0);
#ifdef CONFIG_RAETH_RW_PDMAPTR_FROM_VAR
	tx_cpu_owner_idx0 = 0;
#endif
	sysRegWrite(PDMA_RST_CFG, PST_DTX_IDX0);
#endif

#ifdef CONFIG_RAETH_PDMATX_QDMARX	/* QDMA RX */
	sysRegWrite(QRX_BASE_PTR_0, phys_to_bus((u32) ei_local->phy_qrx_ring));
	sysRegWrite(QRX_MAX_CNT_0,  cpu_to_le32((u32) NUM_QRX_DESC));
	sysRegWrite(QRX_CRX_IDX_0, cpu_to_le32((u32) (NUM_QRX_DESC - 1)));
#else	/* PDMA RX */
	sysRegWrite(RX_BASE_PTR0, phys_to_bus((u32) ei_local->phy_rx_ring0));
	sysRegWrite(RX_MAX_CNT0,  cpu_to_le32((u32) NUM_RX_DESC));
	sysRegWrite(RX_CALC_IDX0, cpu_to_le32((u32) (NUM_RX_DESC - 1)));
#endif

#ifdef CONFIG_RAETH_RW_PDMAPTR_FROM_VAR
	rx_calc_idx0 =  sysRegRead(RX_CALC_IDX0);
#endif
	sysRegWrite(PDMA_RST_CFG, PST_DRX_IDX0);
#if defined (CONFIG_RAETH_MULTIPLE_RX_RING)
	sysRegWrite(RX_BASE_PTR1, phys_to_bus((u32) ei_local->phy_rx_ring1));
	sysRegWrite(RX_MAX_CNT1,  cpu_to_le32((u32) NUM_RX_DESC));
	sysRegWrite(RX_CALC_IDX1, cpu_to_le32((u32) (NUM_RX_DESC - 1)));
#ifdef CONFIG_RAETH_RW_PDMAPTR_FROM_VAR
	rx_calc_idx1 =  sysRegRead(RX_CALC_IDX1);
#endif
	sysRegWrite(PDMA_RST_CFG, PST_DRX_IDX1);
#if defined(CONFIG_ARCH_MT7623)
	sysRegWrite(RX_BASE_PTR2, phys_to_bus((u32) ei_local->phy_rx_ring2));
	sysRegWrite(RX_MAX_CNT2,  cpu_to_le32((u32) NUM_RX_DESC));
	sysRegWrite(RX_CALC_IDX2, cpu_to_le32((u32) (NUM_RX_DESC - 1)));
	sysRegWrite(PDMA_RST_CFG, PST_DRX_IDX2);
    sysRegWrite(RX_BASE_PTR3, phys_to_bus((u32) ei_local->phy_rx_ring3));
	sysRegWrite(RX_MAX_CNT3,  cpu_to_le32((u32) NUM_RX_DESC));
	sysRegWrite(RX_CALC_IDX3, cpu_to_le32((u32) (NUM_RX_DESC - 1)));
	sysRegWrite(PDMA_RST_CFG, PST_DRX_IDX3);
#endif  /* CONFIG_ARCH_MT7623 */
#endif

#if defined (CONFIG_RAETH_QOS)
	set_scheduler_weight();
	set_schedule_pause_condition();
	set_output_shaper();
#endif

	set_fe_dma_glo_cfg();

	return 1;
}

inline int rt2880_eth_send(struct net_device* dev, struct sk_buff *skb, int gmac_no)
{
	unsigned int	length=skb->len;
	END_DEVICE*	ei_local = netdev_priv(dev);
#ifndef CONFIG_RAETH_RW_PDMAPTR_FROM_VAR
	unsigned long	tx_cpu_owner_idx0 = sysRegRead(TX_CTX_IDX0);
#endif
#if defined (CONFIG_RAETH_TSO)
#ifdef CONFIG_RAETH_RW_PDMAPTR_FROM_VAR
	unsigned long   ctx_idx_start_addr = tx_cpu_owner_idx0;
#endif
        struct iphdr *iph = NULL;
        struct tcphdr *th = NULL;
	struct skb_frag_struct *frag;
	unsigned int nr_frags = skb_shinfo(skb)->nr_frags;
	int i=0;
	unsigned int len, size, offset, frag_txd_num, skb_txd_num ;
#endif // CONFIG_RAETH_TSO //

#if defined (CONFIG_RAETH_TSOV6)
	struct ipv6hdr *ip6h = NULL;
#endif

#ifdef CONFIG_PSEUDO_SUPPORT
	PSEUDO_ADAPTER *pAd;
#endif

	while(ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info2.DDONE_bit == 0)
	{
#ifdef CONFIG_PSEUDO_SUPPORT
		if (gmac_no == 2) {
			if (ei_local->PseudoDev != NULL) {
				pAd = netdev_priv(ei_local->PseudoDev);
				pAd->stat.tx_errors++;
			}
		} else
#endif
			ei_local->stat.tx_errors++;
	}

#if !defined (CONFIG_RAETH_TSO)
	ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info1.SDP0 = virt_to_phys(skb->data);
	ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info2.SDL0 = length;
#if defined (CONFIG_RALINK_MT7620)
	ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.FP_BMAP = 0;
#elif defined (CONFIG_RALINK_MT7621) || defined (CONFIG_ARCH_MT7623)
	if (gmac_no == 1) {
		ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.FPORT = 1;
	}else {
		ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.FPORT = 2;
	}
#else
	ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.PN = gmac_no;
	ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.QN = 3;
#endif

#if defined (CONFIG_RAETH_CHECKSUM_OFFLOAD) && ! defined(CONFIG_RALINK_RT5350) && !defined(CONFIG_RALINK_MT7628)
	if (skb->ip_summed == CHECKSUM_PARTIAL){
	    ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.TUI_CO = 7;
	}else {
	    ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.TUI_CO = 0;
	}
#endif

#ifdef CONFIG_RAETH_HW_VLAN_TX
	if(vlan_tx_tag_present(skb)) {
#if defined (CONFIG_RALINK_MT7621) || defined (CONFIG_ARCH_MT7623)
	    ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.VLAN_TAG = 0x10000 | vlan_tx_tag_get(skb);
#else
	    ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.VPRI_VIDX = 0x80 | (vlan_tx_tag_get(skb) >> 13) << 4 | (vlan_tx_tag_get(skb) & 0xF);
#endif
	}else {
#if defined (CONFIG_RALINK_MT7621) || defined (CONFIG_ARCH_MT7623)
	    ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.VLAN_TAG = 0;
#else
	    ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.VPRI_VIDX = 0;
#endif
	}
#endif

#if defined(CONFIG_RAETH_PDMA_DVT)
    raeth_pdma_tx_vlan_dvt( ei_local, tx_cpu_owner_idx0 );
#endif  /* CONFIG_RAETH_PDMA_DVT */

#if defined (CONFIG_RA_HW_NAT) || defined (CONFIG_RA_HW_NAT_MODULE)
	if(FOE_MAGIC_TAG(skb) == FOE_MAGIC_PPE) {
	    if(ra_sw_nat_hook_rx!= NULL){
#if defined (CONFIG_RALINK_MT7620)
		ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.FP_BMAP = (1 << 7); /* PPE */
#elif defined (CONFIG_RALINK_MT7621) || defined (CONFIG_ARCH_MT7623)
		ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.FPORT = 4; /* PPE */
#else
		ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.PN = 6; /* PPE */
#endif
		FOE_MAGIC_TAG(skb) = 0;
	    }
	}
#endif
	
#if defined(CONFIG_RAETH_PDMA_DVT)
    raeth_pdma_tx_desc_dvt( ei_local, tx_cpu_owner_idx0 );
#endif  /* CONFIG_RAETH_PDMA_DVT */

	ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info2.DDONE_bit = 0;

#if 0	
	printk("---------------\n");
	printk("tx_info1=%x\n",ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info1);
	printk("tx_info2=%x\n",ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info2);
	printk("tx_info3=%x\n",ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info3);
	printk("tx_info4=%x\n",ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4);
#endif

#else
	ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info1.SDP0 = virt_to_phys(skb->data);
	ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info2.SDL0 = (length - skb->data_len);
	ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info2.LS0_bit = nr_frags ? 0:1;
#if defined (CONFIG_RALINK_MT7620)
	ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.FP_BMAP = 0;
#elif defined (CONFIG_RALINK_MT7621) || defined (CONFIG_ARCH_MT7623)
	if (gmac_no == 1) {
		ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.FPORT = 1;
	}else {
		ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.FPORT = 2;
	}
#else
	ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.PN = gmac_no;
	ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.QN = 3;
#endif
	ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.TSO = 0;

#if defined (CONFIG_RAETH_CHECKSUM_OFFLOAD) && ! defined(CONFIG_RALINK_RT5350) && !defined(CONFIG_RALINK_MT7628)
	if (skb->ip_summed == CHECKSUM_PARTIAL){
	    ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.TUI_CO = 7;
	}else {
	    ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.TUI_CO = 0;
	}
#endif

#ifdef CONFIG_RAETH_HW_VLAN_TX
	if(vlan_tx_tag_present(skb)) {
#if defined (CONFIG_RALINK_MT7621) || defined (CONFIG_ARCH_MT7623)
	    ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.VLAN_TAG = 0x10000 | vlan_tx_tag_get(skb);
#else
	    ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.VPRI_VIDX = 0x80 | (vlan_tx_tag_get(skb) >> 13) << 4 | (vlan_tx_tag_get(skb) & 0xF);
#endif
	}else {
#if defined (CONFIG_RALINK_MT7621) || defined (CONFIG_ARCH_MT7623)
	    ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.VLAN_TAG = 0;
#else
	    ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.VPRI_VIDX = 0;
#endif
	}
#endif
   
#if defined(CONFIG_RAETH_PDMA_DVT)
    raeth_pdma_tx_vlan_dvt( ei_local, tx_cpu_owner_idx0 );
#endif  /* CONFIG_RAETH_PDMA_DVT */

#if defined (CONFIG_RA_HW_NAT) || defined (CONFIG_RA_HW_NAT_MODULE)
	if(FOE_MAGIC_TAG(skb) == FOE_MAGIC_PPE) {
	    if(ra_sw_nat_hook_rx!= NULL){
#if defined (CONFIG_RALINK_MT7620)
		ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.FP_BMAP = (1 << 7); /* PPE */
#elif defined (CONFIG_RALINK_MT7621) || defined (CONFIG_ARCH_MT7623)
		ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.FPORT = 4; /* PPE */
#else
		ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.PN = 6; /* PPE */
#endif
		FOE_MAGIC_TAG(skb) = 0;
	    }
	}
#endif

	skb_txd_num = 1;

	if(nr_frags > 0) {

		for(i=0;i<nr_frags;i++) {
			frag = &skb_shinfo(skb)->frags[i];
			offset = frag->page_offset;
			len = frag->size;
			frag_txd_num = cal_frag_txd_num(len);

			while(frag_txd_num > 0){
				if(len < MAX_TXD_LEN)
					size = len;
				else
					size = MAX_TXD_LEN;
				if(skb_txd_num%2 == 0) { 
					tx_cpu_owner_idx0 = (tx_cpu_owner_idx0+1) % NUM_TX_DESC; 

					while(ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info2.DDONE_bit == 0)
					{
#ifdef config_pseudo_support
						if (gmac_no == 2) {
							if (ei_local->pseudodev != null) {
								pad = netdev_priv(ei_local->pseudodev);
								pad->stat.tx_errors++;
							}
						} else
#endif
							ei_local->stat.tx_errors++;
					}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,2,0)					
					ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info1.SDP0 = pci_map_page(NULL, frag->page, offset, size, PCI_DMA_TODEVICE);
#else
					ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info1.SDP0 = pci_map_page(NULL, frag->page.p, offset, size, PCI_DMA_TODEVICE);
#endif
					ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info2.SDL0 = size;

					if( (i==(nr_frags-1)) && (frag_txd_num == 1))
						ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info2.LS0_bit = 1;
					else
						ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info2.LS0_bit = 0;
					ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info2.DDONE_bit = 0;
				}else { 
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,2,0)					
					ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info3.SDP1 = pci_map_page(NULL, frag->page, offset, size, PCI_DMA_TODEVICE);
#else
					ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info3.SDP1 = pci_map_page(NULL, frag->page.p, offset, size, PCI_DMA_TODEVICE);

#endif
					ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info2.SDL1 = size;
					if( (i==(nr_frags-1)) && (frag_txd_num == 1))
						ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info2.LS1_bit = 1;
					else
						ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info2.LS1_bit = 0;
				}
				offset += size;
				len -= size;
				frag_txd_num--;
				skb_txd_num++;
			}
		}
	}

#if defined(CONFIG_RAETH_PDMA_DVT)
    if( (pdma_dvt_get_debug_test_config() & PDMA_TEST_TSO_DEBUG) ){
        printk("skb_shinfo(skb)->gso_segs = %d\n", skb_shinfo(skb)->gso_segs);
    }
#endif  /* CONFIG_RAETH_PDMA_DVT */
	/* fill in MSS info in tcp checksum field */
	if(skb_shinfo(skb)->gso_segs > 1) {

#if defined (CONFIG_RAETH_TSO_DBG)
		TsoLenUpdate(skb->len);
#endif

		/* TCP over IPv4 */
		iph = (struct iphdr *)skb_network_header(skb);
#if defined (CONFIG_RAETH_TSOV6)
		/* TCP over IPv6 */
		ip6h = (struct ipv6hdr *)skb_network_header(skb);
#endif				
		if((iph->version == 4) && (iph->protocol == IPPROTO_TCP)) {
			th = (struct tcphdr *)skb_transport_header(skb);
#ifdef CONFIG_RAETH_RW_PDMAPTR_FROM_VAR
			ei_local->tx_ring0[ctx_idx_start_addr].txd_info4.TSO = 1;
#else
			ei_local->tx_ring0[sysRegRead(TX_CTX_IDX0)].txd_info4.TSO = 1;
#endif
			th->check = htons(skb_shinfo(skb)->gso_size);
#if defined (CONFIG_MIPS)
			dma_cache_sync(NULL, th, sizeof(struct tcphdr), DMA_TO_DEVICE);
#else
			dma_sync_single_for_device(NULL, virt_to_phys(th), sizeof(struct tcphdr), DMA_TO_DEVICE);
#endif
		} 
	    
#if defined (CONFIG_RAETH_TSOV6)
		/* TCP over IPv6 */
		else if ((ip6h->version == 6) && (ip6h->nexthdr == NEXTHDR_TCP)) {
			th = (struct tcphdr *)skb_transport_header(skb);
#ifdef CONFIG_RAETH_RW_PDMAPTR_FROM_VAR
			ei_local->tx_ring0[ctx_idx_start_addr].txd_info4.TSO = 1;
#else
			ei_local->tx_ring0[sysRegRead(TX_CTX_IDX0)].txd_info4.TSO = 1;
#endif
			th->check = htons(skb_shinfo(skb)->gso_size);
#if defined (CONFIG_MIPS)
			dma_cache_sync(NULL, th, sizeof(struct tcphdr), DMA_TO_DEVICE);
#else
			dma_sync_single_for_device(NULL, virt_to_phys(th), sizeof(struct tcphdr), DMA_TO_DEVICE);
#endif
		}
#endif // CONFIG_RAETH_TSOV6 //
	}

#if defined(CONFIG_RAETH_PDMA_DVT)
    raeth_pdma_tx_desc_dvt( ei_local, tx_cpu_owner_idx0 );
#endif  /* CONFIG_RAETH_PDMA_DVT */

#ifdef CONFIG_RAETH_RW_PDMAPTR_FROM_VAR
	ei_local->tx_ring0[ctx_idx_start_addr].txd_info2.DDONE_bit = 0;
#else
	ei_local->tx_ring0[sysRegRead(TX_CTX_IDX0)].txd_info2.DDONE_bit = 0;
#endif
#endif // CONFIG_RAETH_TSO //

	wmb();

    	tx_cpu_owner_idx0 = (tx_cpu_owner_idx0+1) % NUM_TX_DESC;
	while(ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info2.DDONE_bit == 0)
	{
//		printk(KERN_ERR "%s: TXD=%lu TX DMA is Busy !!\n", dev->name, tx_cpu_owner_idx0);
#ifdef CONFIG_PSEUDO_SUPPORT
		if (gmac_no == 2) {
			if (ei_local->PseudoDev != NULL) {
				pAd = netdev_priv(ei_local->PseudoDev);
				pAd->stat.tx_errors++;
			}
		} else
#endif
			ei_local->stat.tx_errors++;
	}
	sysRegWrite(TX_CTX_IDX0, cpu_to_le32((u32)tx_cpu_owner_idx0));

#ifdef CONFIG_PSEUDO_SUPPORT
	if (gmac_no == 2) {
		if (ei_local->PseudoDev != NULL) {
			pAd = netdev_priv(ei_local->PseudoDev);
			pAd->stat.tx_packets++;
			pAd->stat.tx_bytes += length;
		}
	} else
#endif
	{
		ei_local->stat.tx_packets++;
		ei_local->stat.tx_bytes += length;
	}
#ifdef CONFIG_RAETH_NAPI
	if ( ei_local->tx_full == 1) {
		ei_local->tx_full = 0;
		netif_wake_queue(dev);
	}
#endif

	return length;
}

int ei_start_xmit(struct sk_buff* skb, struct net_device *dev, int gmac_no)
{
	END_DEVICE *ei_local = netdev_priv(dev);
	unsigned long flags;
	unsigned long tx_cpu_owner_idx;
	unsigned int tx_cpu_owner_idx_next;
	unsigned int num_of_txd = 0;
#if defined (CONFIG_RAETH_TSO)
	unsigned int nr_frags = skb_shinfo(skb)->nr_frags, i;
	struct skb_frag_struct *frag;
#endif
#if	!defined(CONFIG_RAETH_QOS)
	unsigned int tx_cpu_owner_idx_next2;
#else
	int ring_no, queue_no, port_no;
#endif
#ifdef CONFIG_RALINK_VISTA_BASIC
	struct vlan_ethhdr *veth;
#endif
#ifdef CONFIG_PSEUDO_SUPPORT
	PSEUDO_ADAPTER *pAd;
#endif

#if !defined(CONFIG_RA_NAT_NONE)
	if(ra_sw_nat_hook_tx!= NULL)
	{
#if defined (CONFIG_RA_HW_NAT) || defined (CONFIG_RA_HW_NAT_MODULE)
	    if(FOE_MAGIC_TAG(skb) != FOE_MAGIC_PPE)
#endif
	    {
		//spin_lock_irqsave(&ei_local->page_lock, flags);
		if(ra_sw_nat_hook_tx(skb, gmac_no)==1){
		    //spin_unlock_irqrestore(&ei_local->page_lock, flags);
		}else{
		    kfree_skb(skb);
		    //spin_unlock_irqrestore(&ei_local->page_lock, flags);
		    return 0;
		}
	    }
	}
#endif
#if defined(CONFIG_RA_CLASSIFIER)||defined(CONFIG_RA_CLASSIFIER_MODULE)
		/* Qwert+
		 */
		if(ra_classifier_hook_tx!= NULL)
		{
#if defined(CONFIG_RALINK_EXTERNAL_TIMER)
			ra_classifier_hook_tx(skb, (*((volatile u32 *)(0xB0000D08))&0x0FFFF));
#else			
			ra_classifier_hook_tx(skb, read_c0_count());
#endif			
		}
#endif /* CONFIG_RA_CLASSIFIER */

#if defined (CONFIG_RALINK_RT3052_MP2)
	mcast_tx(skb);
#endif

#if !defined (CONFIG_RALINK_RT6855) && !defined (CONFIG_RALINK_RT6855A) && \
    !defined(CONFIG_RALINK_MT7621) && !defined (CONFIG_ARCH_MT7623)

#define MIN_PKT_LEN  60
	 if (skb->len < MIN_PKT_LEN) {
	     if (skb_padto(skb, MIN_PKT_LEN)) {
		 printk("raeth: skb_padto failed\n");
		 return 0;
	     }
	     skb_put(skb, MIN_PKT_LEN - skb->len);
	 }
#endif

	dev->trans_start = jiffies;	/* save the timestamp */
	spin_lock_irqsave(&ei_local->page_lock, flags);
#if defined (CONFIG_MIPS)
	dma_cache_sync(NULL, skb->data, skb_headlen(skb), DMA_TO_DEVICE);
#else
	dma_sync_single_for_device(NULL, virt_to_phys(skb->data), skb_headlen(skb), DMA_TO_DEVICE);

#endif

#ifdef CONFIG_RALINK_VISTA_BASIC
	veth = (struct vlan_ethhdr *)(skb->data);
	if (is_switch_175c && veth->h_vlan_proto == __constant_htons(ETH_P_8021Q)) {
		if ((veth->h_vlan_TCI & __constant_htons(VLAN_VID_MASK)) == 0) {
			veth->h_vlan_TCI |= htons(VLAN_DEV_INFO(dev)->vlan_id);
		}
	}
#endif

#if defined (CONFIG_RAETH_QOS)
	if(pkt_classifier(skb, gmac_no, &ring_no, &queue_no, &port_no)) {
		get_tx_ctx_idx(ring_no, &tx_cpu_owner_idx);
		tx_cpu_owner_idx_next = (tx_cpu_owner_idx + 1) % NUM_TX_DESC;
	  if(((ei_local->skb_free[ring_no][tx_cpu_owner_idx]) ==0) && (ei_local->skb_free[ring_no][tx_cpu_owner_idx_next]==0)){
	    fe_qos_packet_send(dev, skb, ring_no, queue_no, port_no);
	  }else{
	    ei_local->stat.tx_dropped++;
	    //QWERT kfree_skb(skb);
		dev_kfree_skb_any(skb);
	    spin_unlock_irqrestore(&ei_local->page_lock, flags);
	    return 0;
	  }
	}
#else
#ifdef CONFIG_RAETH_RW_PDMAPTR_FROM_VAR
	tx_cpu_owner_idx = tx_cpu_owner_idx0;
#else
	tx_cpu_owner_idx = sysRegRead(TX_CTX_IDX0);
#endif
#if defined (CONFIG_RAETH_TSO)
//	num_of_txd = (nr_frags==0) ? 1 : ((nr_frags>>1) + 1);
//	NumOfTxdUpdate(num_of_txd);
	if(nr_frags != 0){
		for(i=0;i<nr_frags;i++) {
			frag = &skb_shinfo(skb)->frags[i];
			num_of_txd  += cal_frag_txd_num(frag->size);
		}
		num_of_txd = (num_of_txd >> 1) + 1;
	}else
		num_of_txd = 1;

#else
	num_of_txd = 1;
#endif
	tx_cpu_owner_idx_next = (tx_cpu_owner_idx + num_of_txd) % NUM_TX_DESC;

	if(((ei_local->skb_free[tx_cpu_owner_idx]) ==0) && (ei_local->skb_free[tx_cpu_owner_idx_next]==0)){
		rt2880_eth_send(dev, skb, gmac_no);

		tx_cpu_owner_idx_next2 = (tx_cpu_owner_idx_next + 1) % NUM_TX_DESC;

		if(ei_local->skb_free[tx_cpu_owner_idx_next2]!=0){
#if defined (CONFIG_RAETH_SW_FC) 		    
				netif_stop_queue(dev);
#ifdef CONFIG_PSEUDO_SUPPORT
				netif_stop_queue(ei_local->PseudoDev);
#endif
				tx_ring_full=1;
#endif
		}
	}else {
#ifdef CONFIG_PSEUDO_SUPPORT
		if (gmac_no == 2) {
			if (ei_local->PseudoDev != NULL) {
				pAd = netdev_priv(ei_local->PseudoDev);
				pAd->stat.tx_dropped++;
			}
		} else
#endif
			ei_local->stat.tx_dropped++;
#if defined (CONFIG_RAETH_SW_FC) 		    
		printk("tx_ring_full, drop packet\n");
#endif
		//QWERT kfree_skb(skb);
		dev_kfree_skb_any(skb);
		spin_unlock_irqrestore(&ei_local->page_lock, flags);
		return 0;
	}

#if defined (CONFIG_RAETH_TSO)
	/* SG: use multiple TXD to send the packet (only have one skb) */
	ei_local->skb_free[(tx_cpu_owner_idx + num_of_txd - 1) % NUM_TX_DESC] = skb;
	while(--num_of_txd) {
		ei_local->skb_free[(tx_cpu_owner_idx + num_of_txd -1) % NUM_TX_DESC] = (struct  sk_buff *)0xFFFFFFFF; //MAGIC ID
	}
#else
	ei_local->skb_free[tx_cpu_owner_idx] = skb;
#endif
#endif
	spin_unlock_irqrestore(&ei_local->page_lock, flags);
	return 0;
}

void ei_xmit_housekeeping(unsigned long unused)
{
    struct net_device *dev = dev_raether;
    END_DEVICE *ei_local = netdev_priv(dev);
    struct PDMA_txdesc *tx_desc;
    unsigned long skb_free_idx;
    unsigned long tx_dtx_idx __maybe_unused;
#ifndef CONFIG_RAETH_NAPI
    unsigned long reg_int_mask=0;
#endif

#ifdef CONFIG_RAETH_QOS
    int i;
    for (i=0;i<NUM_TX_RINGS;i++){
        skb_free_idx = ei_local->free_idx[i];
    	if((ei_local->skb_free[i][skb_free_idx])==0){
		continue;
	}

	get_tx_desc_and_dtx_idx(ei_local, i, &tx_dtx_idx, &tx_desc);

	while(tx_desc[skb_free_idx].txd_info2.DDONE_bit==1 && (ei_local->skb_free[i][skb_free_idx])!=0 ){
		dev_kfree_skb_any((ei_local->skb_free[i][skb_free_idx]));

	    ei_local->skb_free[i][skb_free_idx]=0;
	    skb_free_idx = (skb_free_idx +1) % NUM_TX_DESC;
	}
	ei_local->free_idx[i] = skb_free_idx;
    }
#else
	tx_dtx_idx = sysRegRead(TX_DTX_IDX0);
	tx_desc = ei_local->tx_ring0;
	skb_free_idx = ei_local->free_idx;
	if ((ei_local->skb_free[skb_free_idx]) != 0 && tx_desc[skb_free_idx].txd_info2.DDONE_bit==1) {
		while(tx_desc[skb_free_idx].txd_info2.DDONE_bit==1 && (ei_local->skb_free[skb_free_idx])!=0 ){
#if defined (CONFIG_RAETH_TSO)
	    if(ei_local->skb_free[skb_free_idx]!=(struct  sk_buff *)0xFFFFFFFF) {
		    dev_kfree_skb_any(ei_local->skb_free[skb_free_idx]);
	    }
#else
	    dev_kfree_skb_any(ei_local->skb_free[skb_free_idx]);
#endif
	    ei_local->skb_free[skb_free_idx]=0;
	    skb_free_idx = (skb_free_idx +1) % NUM_TX_DESC;
	}

	netif_wake_queue(dev);
#ifdef CONFIG_PSEUDO_SUPPORT
		netif_wake_queue(ei_local->PseudoDev);
#endif
		tx_ring_full=0;
		ei_local->free_idx = skb_free_idx;
	}  /* if skb_free != 0 */
#endif

#ifndef CONFIG_RAETH_NAPI
    reg_int_mask=sysRegRead(FE_INT_ENABLE);
#if defined (DELAY_INT)
    sysRegWrite(FE_INT_ENABLE, reg_int_mask| TX_DLY_INT);
#else

    sysRegWrite(FE_INT_ENABLE, reg_int_mask | TX_DONE_INT0 \
		    			    | TX_DONE_INT1 \
					    | TX_DONE_INT2 \
					    | TX_DONE_INT3);
#endif
#endif //CONFIG_RAETH_NAPI//
}



EXPORT_SYMBOL(ei_start_xmit);
EXPORT_SYMBOL(ei_xmit_housekeeping);
EXPORT_SYMBOL(fe_dma_init);
EXPORT_SYMBOL(rt2880_eth_send);
