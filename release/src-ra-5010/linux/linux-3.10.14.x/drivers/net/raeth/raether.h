#ifndef RA2882ETHEND_H
#define RA2882ETHEND_H

#ifdef DSP_VIA_NONCACHEABLE
#define ESRAM_BASE	0xa0800000	/* 0x0080-0000  ~ 0x00807FFF */
#else
#define ESRAM_BASE	0x80800000	/* 0x0080-0000  ~ 0x00807FFF */
#endif

#define RX_RING_BASE	((int)(ESRAM_BASE + 0x7000))
#define TX_RING_BASE	((int)(ESRAM_BASE + 0x7800))

#define NUM_TX_RINGS 4
#ifdef MEMORY_OPTIMIZATION
#ifdef CONFIG_RAETH_ROUTER
#define NUM_RX_DESC 32 //128
#define NUM_TX_DESC 32 //128
#elif CONFIG_RT_3052_ESW
#define NUM_RX_DESC 16 //64
#define NUM_TX_DESC 16 //64
#else
#define NUM_RX_DESC 32 //128
#define NUM_TX_DESC 32 //128
#endif
//#define NUM_RX_MAX_PROCESS 32
#define NUM_RX_MAX_PROCESS 32
#else
#if defined (CONFIG_RAETH_ROUTER)
#define NUM_RX_DESC 256
#define NUM_TX_DESC 256
#elif defined (CONFIG_RT_3052_ESW)
#if defined (CONFIG_RALINK_MT7621)
#define NUM_RX_DESC 1024
#define NUM_QRX_DESC 16
#ifdef CONFIG_RAETH_QDMA
#define NUM_PQ 16
#define NUM_PQ_RESV 4
#define FFA 1024
#define QUEUE_OFFSET 0x10
#define NUM_TX_DESC (NUM_PQ * NUM_PQ_RESV + FFA)
#else
#define NUM_TX_DESC 1024
#endif
#elif defined (CONFIG_ARCH_MT7623)
#define NUM_RX_DESC 2048
#ifdef CONFIG_RAETH_QDMA
#if defined (CONFIG_RAETH_QDMATX_QDMARX) || defined (CONFIG_RAETH_PDMATX_QDMARX)
#define NUM_QRX_DESC NUM_RX_DESC
#else
#define NUM_QRX_DESC 16
#endif
#define NUM_PQ 16
#define NUM_PQ_RESV 4
#define FFA 2048
#define QUEUE_OFFSET 0x10
#define NUM_TX_DESC (NUM_PQ * NUM_PQ_RESV + FFA)
#else
#define NUM_TX_DESC 2048
#endif
#else
#define NUM_RX_DESC 512
#define NUM_QRX_DESC NUM_RX_DESC
#define NUM_TX_DESC 512
#endif
#else
#define NUM_RX_DESC 256
#define NUM_QRX_DESC NUM_RX_DESC
#define NUM_TX_DESC 256
#endif
#if defined(CONFIG_RALINK_RT3883) || defined(CONFIG_RALINK_MT7620) 
#define NUM_RX_MAX_PROCESS 2
#else
#define NUM_RX_MAX_PROCESS 16
#endif
#endif
#define NUM_LRO_RX_DESC	16

#if defined (CONFIG_SUPPORT_OPENWRT)
#define DEV_NAME        "eth0"
#define DEV2_NAME       "eth1"
#else
#define DEV_NAME        "eth2"
#define DEV2_NAME       "eth3"
#endif

#if defined (CONFIG_RALINK_RT6855A) || defined (CONFIG_RALINK_MT7621) || defined (CONFIG_ARCH_MT7623)
#define GMAC0_OFFSET    0xE000
#define GMAC2_OFFSET    0xE006
#else
#define GMAC0_OFFSET    0x28 
#define GMAC2_OFFSET    0x22
#endif

#if   defined(CONFIG_ARCH_MT7623)
#define IRQ_ENET0	232
#define IRQ_ENET1       231
#define IRQ_ENET2       230
#else
#define IRQ_ENET0	3 	/* hardware interrupt #3, defined in RT2880 Soc Design Spec Rev 0.03, pp43 */
#endif

#if defined (CONFIG_RAETH_HW_LRO)
#define	HW_LRO_TIMER_UNIT   1
#define	HW_LRO_REFRESH_TIME 50000
#define	HW_LRO_MAX_AGG_CNT	64
#define	HW_LRO_AGG_DELTA	1
#if defined(CONFIG_RAETH_PDMA_DVT)
#define	MAX_LRO_RX_LENGTH	10240
#else
#define	MAX_LRO_RX_LENGTH	(PAGE_SIZE * 3)
#endif
#define	HW_LRO_AGG_TIME		10	/* 200us */
#define	HW_LRO_AGE_TIME		50
#define	HW_LRO_BW_THRE	        3000
#define	HW_LRO_PKT_INT_ALPHA    100
#endif  /* CONFIG_RAETH_HW_LRO */
#define FE_INT_STATUS_REG (*(volatile unsigned long *)(FE_INT_STATUS))
#define FE_INT_STATUS_CLEAN(reg) (*(volatile unsigned long *)(FE_INT_STATUS)) = reg

//#define RAETH_DEBUG
#ifdef RAETH_DEBUG
#define RAETH_PRINT(fmt, args...) printk(KERN_INFO fmt, ## args)
#else
#define RAETH_PRINT(fmt, args...) { }
#endif

struct net_device_stats *ra_get_stats(struct net_device *dev);

void ei_tx_timeout(struct net_device *dev);
int rather_probe(struct net_device *dev);
int ei_open(struct net_device *dev);
int ei_close(struct net_device *dev);

int ra2882eth_init(void);
void ra2882eth_cleanup_module(void);

void ei_xmit_housekeeping(unsigned long data);

u32 mii_mgr_read(u32 phy_addr, u32 phy_register, u32 *read_data);
u32 mii_mgr_write(u32 phy_addr, u32 phy_register, u32 write_data);
u32 mii_mgr_cl45_set_address(u32 port_num, u32 dev_addr, u32 reg_addr);
u32 mii_mgr_read_cl45(u32 port_num, u32 dev_addr, u32 reg_addr, u32 *read_data);
u32 mii_mgr_write_cl45(u32 port_num, u32 dev_addr, u32 reg_addr, u32 write_data);
void fe_sw_init(void);
#if defined(CONFIG_ARCH_MT7623)
void fe_do_reset(void);
#endif

#endif
