#ifndef RA_MAC_H
#define RA_MAC_H

void ra2880stop(END_DEVICE *ei_local);
void ra2880MacAddressSet(unsigned char p[6]);
void ra2880Mac2AddressSet(unsigned char p[6]);
void ethtool_init(struct net_device *dev);

void ra2880EnableInterrupt(void);

void dump_qos(void);
void dump_reg(struct seq_file *s);
void dump_cp0(void);

int debug_proc_init(void);
void debug_proc_exit(void);

#if defined (CONFIG_RALINK_RT6855) || defined(CONFIG_RALINK_RT6855A) || \
           defined (CONFIG_RALINK_MT7620) || defined(CONFIG_RALINK_MT7621)
void enable_auto_negotiate(int unused);
#else
void enable_auto_negotiate(int ge);
#endif

void rt2880_gmac_hard_reset(void);

int TsoLenUpdate(int tso_len);
int NumOfTxdUpdate(int num_of_txd);

#ifdef CONFIG_RAETH_LRO
int LroStatsUpdate(struct net_lro_mgr *lro_mgr, bool all_flushed);
#endif
#ifdef CONFIG_RAETH_HW_LRO
int HwLroStatsUpdate(unsigned int ring_num, unsigned int agg_cnt, unsigned int agg_size);
#if defined(CONFIG_RAETH_HW_LRO_REASON_DBG)
#define HW_LRO_AGG_FLUSH        (1)
#define HW_LRO_AGE_FLUSH        (2)
#define HW_LRO_NOT_IN_SEQ_FLUSH (3)
#define HW_LRO_TIMESTAMP_FLUSH  (4)
#define HW_LRO_NON_RULE_FLUSH   (5)
int HwLroFlushStatsUpdate(unsigned int ring_num, unsigned int flush_reason);
#endif  /* CONFIG_RAETH_HW_LRO_REASON_DBG */
typedef int (*HWLRO_DBG_FUNC)(int par1, int par2);
int hwlro_agg_cnt_ctrl(int par1, int par2);
int hwlro_agg_time_ctrl(int par1, int par2);
int hwlro_age_time_ctrl(int par1, int par2);
int hwlro_pkt_int_alpha_ctrl(int par1, int par2);
int hwlro_threshold_ctrl(int par1, int par2);
int hwlro_fix_setting_switch_ctrl(int par1, int par2);
int hwlro_sdl_size_ctrl(int par1, int par2);
int hwlro_ring_enable_ctrl(int par1, int par2);
#endif  /* CONFIG_RAETH_HW_LRO */

#if defined(CONFIG_RAETH_INT_DBG)
struct raeth_int_t{
	unsigned int RX_COHERENT_CNT;
	unsigned int RX_DLY_INT_CNT;
	unsigned int TX_COHERENT_CNT;
	unsigned int TX_DLY_INT_CNT;
	unsigned int RING3_RX_DLY_INT_CNT;
	unsigned int RING2_RX_DLY_INT_CNT;
	unsigned int RING1_RX_DLY_INT_CNT;
	unsigned int RXD_ERROR_CNT;
	unsigned int ALT_RPLC_INT3_CNT;
	unsigned int ALT_RPLC_INT2_CNT;
	unsigned int ALT_RPLC_INT1_CNT;
	unsigned int RX_DONE_INT3_CNT;
	unsigned int RX_DONE_INT2_CNT;
	unsigned int RX_DONE_INT1_CNT;
	unsigned int RX_DONE_INT0_CNT;
	unsigned int TX_DONE_INT3_CNT;
	unsigned int TX_DONE_INT2_CNT;
	unsigned int TX_DONE_INT1_CNT;
	unsigned int TX_DONE_INT0_CNT;
};
int IntStatsUpdate(unsigned int int_status);
#endif	/* CONFIG_RAETH_INT_DBG */

int getnext(const char *src, int separator, char *dest);
int str_to_ip(unsigned int *ip, const char *str);

#if defined(CONFIG_RAETH_PDMA_DVT)
typedef int (*PDMA_DBG_FUNC)(int par1, int par2);
#endif  //#if defined(CONFIG_RAETH_PDMA_DVT)
#endif
