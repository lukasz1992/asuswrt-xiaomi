#ifndef __RALINK_I2S_H_
#define __RALINK_I2S_H_

#ifdef __KERNEL__
#include <asm/rt2880/rt_mmap.h>
#endif

#if defined(CONFIG_I2S_WITH_AEC)
#include "aec/aec_api.h"
#endif

#define I2S_MAX_DEV			1
#define I2S_MOD_VERSION			"0.1"
#define phys_to_bus(a) (a & 0x1FFFFFFF)

#ifndef u32
#define u32 unsigned int
#endif

#ifndef u16
#define u16 unsigned short
#endif

#ifndef u8
#define u8 unsigned char
#endif

#ifndef REGBIT
#define REGBIT(x, n)		(x << n)
#endif

#define Virtual2Physical(x)             (((int)x) & 0x1fffffff)
#define Physical2Virtual(x)             (((int)x) | 0x80000000)
#define Virtual2NonCache(x)             (((int)x) | 0x20000000)
#define Physical2NonCache(x)            (((int)x) | 0xa0000000)
#define NonCache2Virtual(x)             (((int)x) & 0xDFFFFFFF)

#if defined(CONFIG_I2S_MCLK_12MHZ)
#define CONFIG_I2S_CODEC_PLL_EN		1
#else
#define CONFIG_I2S_CODEC_PLL_EN		0
#endif

//#define CONFIG_I2S_MS_CTRL		
//#define CONFIG_I2S_MS_MODE
//#define memory_test

#if defined (CONFIG_ARCH_MT7623)
#define MT7623_ASIC_BOARD
#define ARM_ARCH
#endif

#if defined (CONFIG_RALINK_MT7621)
#define MT7621_ASIC_BOARD
#endif

#if defined (CONFIG_RALINK_MT7628)
#define MT7628_ASIC_BOARD
#endif

//#define I2S_DEBUG_PRN
#ifdef I2S_DEBUG_PRN
#define MSG(fmt, args...) printk("I2S: " fmt, ## args)
#else
#define MSG(fmt, args...) { }
#endif

#ifdef I2S_DEBUG_PRN
#define i2s_outw(address, value)	do{printk("0x%08X = 0x%08X\n",(u32)address,(u32)value);*((volatile uint32_t *)(address)) = cpu_to_le32(value);}while(0)
#else
#define i2s_outw(address, value)    	*((volatile uint32_t *)(address)) = cpu_to_le32(value)
#endif
#define i2s_inw(address)		le32_to_cpu(*(volatile u32 *)(address))

/* HW feature definiations */
#if defined(CONFIG_RALINK_RT3883)
#define CONFIG_I2S_TXRX			1
#define CONFIG_I2S_IN_MCLK		1
//#define CONFIG_I2S_WS_EDGE		1
#define CONFIG_I2S_FRAC_DIV		1
#define CONFIG_I2S_IN_CLK		1
#define CONFIG_I2S_MS_MODE		1
#endif

#if defined(CONFIG_RALINK_RT3352)||defined(CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_RT6855) \
	|| defined(CONFIG_RALINK_RT6855A) || defined(CONFIG_RALINK_MT7620) || defined(CONFIG_RALINK_MT7621) \
	|| defined (CONFIG_RALINK_MT7628) || defined(CONFIG_ARCH_MT7623)
#define CONFIG_I2S_TXRX			1
//#define CONFIG_I2S_WS_EDGE		1
#define CONFIG_I2S_FRAC_DIV		1
#define CONFIG_I2S_IN_CLK		1
#endif

#if defined(CONFIG_RALINK_RT3350)
#define CONFIG_I2S_IN_MCLK		1
#endif

#if defined(CONFIG_RALINK_RT3052)
#define CONFIG_I2S_MS_MODE		1
#endif

/* This is decided in menuconfig */
#define CONFIG_I2S_MMAP           	1

/* For MT7623 ASIC PLL Setting */
#if defined(CONFIG_ARCH_MT7623)
#define AUD1PLL_CON0		(0xF0209270)
#define AUD1PLL_CON1		(0xF0209274)
#define AUD1PLL_CON2		(0xF0209278)
#define AUD1PLL_PWR_CON0	(0xF020927C)
#define AUD2PLL_CON0		(0xF02092C0)
#define AUD2PLL_CON1		(0xF02092C4)
#define AUD2PLL_CON2		(0xF02092C8)
#define AUD2PLL_PWR_CON0	(0xF02092CC)
#endif

/* Register Map, Ref to RT3052 Data Sheet */

/* Register Map Detail */
#if defined(CONFIG_ARCH_MT7623)
#define I2S_I2SCFG			(ETHDMASYS_I2S_BASE+0x0000)
#define I2S_INT_STATUS			(ETHDMASYS_I2S_BASE+0x0004)
#define I2S_INT_EN			(ETHDMASYS_I2S_BASE+0x0008)
#define I2S_FF_STATUS			(ETHDMASYS_I2S_BASE+0x000c)
#define I2S_FIFO_WREG			(ETHDMASYS_I2S_BASE+0x0010)
#define I2S_TX_FIFO_WREG		I2S_FIFO_WREG
#define I2S_RX_FIFO_RREG		(ETHDMASYS_I2S_BASE+0x0014)
#define I2S_I2SCFG1			(ETHDMASYS_I2S_BASE+0x0018)
#define I2S_DIVINT_CFG			(ETHDMASYS_I2S_BASE+0x0024)
#define I2S_DIVCOMP_CFG			(ETHDMASYS_I2S_BASE+0x0020)
#else
#define I2S_I2SCFG			(RALINK_I2S_BASE+0x0000)
#define I2S_INT_STATUS			(RALINK_I2S_BASE+0x0004)
#define I2S_INT_EN			(RALINK_I2S_BASE+0x0008)
#define I2S_FF_STATUS			(RALINK_I2S_BASE+0x000c)
#define I2S_FIFO_WREG			(RALINK_I2S_BASE+0x0010)
#define I2S_TX_FIFO_WREG		I2S_FIFO_WREG
#define I2S_RX_FIFO_RREG		(RALINK_I2S_BASE+0x0014)
#define I2S_I2SCFG1			(RALINK_I2S_BASE+0x0018)
#define I2S_DIVINT_CFG			(RALINK_I2S_BASE+0x0024)
#define I2S_DIVCOMP_CFG			(RALINK_I2S_BASE+0x0020)
#endif


/* I2SCFG bit field */
#define I2S_EN			31
#define I2S_DMA_EN		30
#if defined(CONFIG_RALINK_MT7628) || defined(CONFIG_ARCH_MT7623)
#define I2S_LITTLE_ENDIAN	29
#define I2S_SYS_ENDIAN		28
#elif defined(CONFIG_RALINK_RT6855A)
#define I2S_BYTE_SWAP		28
#endif
#define I2S_TX_EN		24
#define I2S_RX_EN		20
#if defined(CONFIG_RALINK_MT7628) || defined(CONFIG_ARCH_MT7623)
#define I2S_NORM_24BIT		18
#define I2S_DATA_24BIT		17
#endif
#define I2S_SLAVE_MODE		16
#define I2S_RX_FF_THRES		12
#define I2S_RX_CH_SWAP		11
#define I2S_RX_CH1_OFF		10
#define I2S_RX_CH0_OFF		9
#if defined(CONFIG_RALINK_RT3052)
#define I2S_CLK_OUT_DIS		8
#endif
#define I2S_TX_FF_THRES		4
#define I2S_TX_CH_SWAP		3
#define I2S_TX_CH1_OFF		2
#define I2S_TX_CH0_OFF		1
#if defined(CONFIG_RALINK_RT3052)
#define I2S_SLAVE_EN            0
#else
#define I2S_WS_INV		0
#endif
/* INT_EN bit field */
#define I2S_RX_INT3_EN		7
#define I2S_RX_INT2_EN		6
#define I2S_RX_INT1_EN		5
#define I2S_RX_INT0_EN		4
#define I2S_TX_INT3_EN		3
#define I2S_TX_INT2_EN		2
#define I2S_TX_INT1_EN		1
#define I2S_TX_INT0_EN		0

/* INT_STATUS bit field */
#define I2S_RX_DMA_FAULT	7
#define I2S_RX_OVRUN		6
#define I2S_RX_UNRUN		5
#define I2S_RX_THRES		4
#define I2S_TX_DMA_FAULT	3
#define I2S_TX_OVRUN		2
#define I2S_TX_UNRUN		1
#define I2S_TX_THRES		0

/* FF_STATUS bit field */
#define I2S_RX_EPCNT		4
#define I2S_TX_EPCNT		0
/* I2S_DIVCOMP_CFG bit field */
#define I2S_CLKDIV_EN		31

/* I2S_CFG1 bit field */
#define I2S_LBK_EN		31
#define I2S_EXT_LBK_EN		30
#if defined(CONFIG_RALINK_MT7628) || defined(CONFIG_ARCH_MT7623)
#define I2S_DATA_FMT		0
#endif

/* FIFO_WREG bit field */
#define I2S_FIFO_WDATA		0

/* Constant definition */
#define NFF_THRES		4
#define I2S_PAGE_SIZE		3072//(3*4096)//(1152*2*2*2)
#define I2S_MIN_PAGE_SIZE	4096
#define MAX_I2S_PAGE		8
#define I2S_TOTAL_PAGE_SIZE 	(I2S_PAGE_SIZE*MAX_I2S_PAGE)

#if defined(CONFIG_I2S_WM8960)
#define MAX_SRATE_HZ            48000
#define MIN_SRATE_HZ            8000
#elif defined(CONFIG_I2S_WM8750)
#define MAX_SRATE_HZ		96000
#define MIN_SRATE_HZ		8000
#endif

#define MAX_VOL_DB		+0			
#define MIN_VOL_DB		-127

#define ALSA_MMAP_IDX_SHIFT	2
#if defined(CONFIG_SND_MT76XX_SOC)
#define STREAM_PLAYBACK		SNDRV_PCM_STREAM_PLAYBACK 
#define STREAM_CAPTURE		SNDRV_PCM_STREAM_CAPTURE
#else
#define STREAM_PLAYBACK		0
#define STREAM_CAPTURE		1
#endif

/* I2S I/O command */
#define I2S_SRATE		0
#define I2S_VOL			1
#define I2S_ENABLE		2
#define I2S_DISABLE		3
#define I2S_TX_ENABLE		27
#define I2S_TX_DISABLE		3
#define I2S_GET_WBUF	 	4
#define I2S_PUT_WBUF		5
#define I2S_RX_ENABLE		6
#define I2S_RX_DISABLE		7
#define I2S_PUT_AUDIO		4
#define I2S_GET_AUDIO		5
#define I2S_TX_VOL		1
#define I2S_RX_VOL		8
#define I2S_WORD_LEN		9
#define I2S_ENDIAN_FMT		10
#define I2S_INTERNAL_LBK	11
#define I2S_TX_STOP             12
#define I2S_DEBUG_CODEC		13
#define I2S_MS_MODE_CTRL	14
#define I2S_TX_PAUSE		15
#define I2S_TX_RESUME		16
#define I2S_RESET		17
#define I2S_RX_STOP		18
#define I2S_EXTERNAL_LBK	19
#define I2S_TXRX_COEXIST	20
#define I2S_RX_PAUSE		21
#define I2S_RX_RESUME		22
#define I2S_CODEC_MIC_BOOST	23
#define I2S_CODEC_MIC_IN	24
#define I2S_CLOCK_ENABLE	25
#define I2S_TEST_TEST		26

#define I2S_DEBUG		30
#define I2S_DEBUG_CLKGEN	30
#define I2S_DEBUG_INLBK		31
#define I2S_DEBUG_EXLBK		32
#define I2S_DEBUG_FMT		33
#define I2S_DEBUG_RESET		34
#define I2S_DEBUG_CODECBYPASS	35
#if defined(CONFIG_I2S_WM8960)
#define I2S_DEBUG_CODEC_EXLBK	36
#endif

/* configuration */
#define CONFIG_I2S_TFF_THRES	NFF_THRES
#define CONFIG_I2S_CH_SWAP	0
#if defined(CONFIG_I2S_MS_MODE)    
#define CONFIG_I2S_SLAVE_EN	0
#else
#define CONFIG_I2S_SLAVE_EN	1
#endif

/* driver status definition */
#define I2S_OK			0
#define I2S_OUTOFMEM		0x01
#define I2S_GDMAFAILED		0x02
#define I2S_REQUEST_IRQ_FAILED	0x04
#define I2S_REG_SETUP_FAILED	0x08

#define I2S_STATISTIC
//#define I2S_HW_INTERRUPT_EN
//#define I2S_SW_IRQ_EN
#define I2S_MAJOR		234

/* parameter for ALSA */
/*GDMA for I2S Status*/
#define GDMA_I2S_DIS (0)
#define GDMA_I2S_EN (1)


typedef struct i2s_status_t
{
	u32 txdmafault;
	u32 txovrun;
	u32 txunrun;
	u32 txthres;
	int txbuffer_unrun;
	int txbuffer_ovrun;
	int txbuffer_len;
	
	u32 rxdmafault;
	u32 rxovrun;
	u32 rxunrun;
	u32 rxthres;
	int rxbuffer_unrun;
	int rxbuffer_ovrun;
	int rxbuffer_len;
}i2s_status_type;


typedef struct i2s_config_t
{

	int srate;
	int txvol;
	int rxvol;
	u32 pos;
	u32 tx_isr_cnt;
	u32 rx_isr_cnt;
	int bSleep;
	int bTxDMAEnable;
	int bRxDMAEnable;
	int enLable;
	int micboost;
	int micin;
	
	/* parameters fo ALSA */
	int bALSAEnable;
	int bALSAMMAPEnable;
	unsigned char bTrigger[2];
	unsigned char bPreTrigger[2];
	unsigned char dmaStat[2];
	unsigned char i2sStat[2];
	unsigned int hw_base_frame[2];
	struct snd_pcm_substream *pss[2];

#ifdef __KERNEL__		
	spinlock_t lock;
	wait_queue_head_t i2s_tx_qh, i2s_rx_qh;
#endif
	u32 dmach;
	u32 tx_unmask_ch;
	u32 rx_unmask_ch;
	u32 dma_unmask_status;
	u32 dma_done_status;   
	u32 tx_ff_thres;
	u32 tx_ch_swap;
	u32 rx_ff_thres;
	u32 rx_ch_swap;
	u32 slave_en;

	u32 dis_match;
	int start_cnt;
#if defined (CONFIG_RALINK_MT7628) || defined(CONFIG_ARCH_MT7623)
	int little_edn;  /* test file's fmt: little endian->1; big endian->0 */
        int sys_endian;  /* kernal' system fmt: little endian->0; big endian->1 */	
#endif
	int wordlen_24b;
	int codec_pll_en;
	int codec_num;
	int tx_pause_en;
	int rx_pause_en;
	int end_cnt;
	int txrx_coexist;
	int tx_stop_cnt;
	int rx_stop_cnt;
	/* for I2S_CFG1 */
	u32 lbk;
	u32 extlbk;
	u32 fmt;
	
	int w_idx;
	int r_idx;
	
	int tx_w_idx;
	int tx_r_idx;
	int rx_w_idx;
	int rx_r_idx;
	int mmap_index;
	int next_p0_idx;
	int next_p1_idx;
	
	u8* buf8ptr;	
	char* pMMAPBufPtr[MAX_I2S_PAGE*2];	
	char* pMMAPTxBufPtr[MAX_I2S_PAGE];
	char* pMMAPRxBufPtr[MAX_I2S_PAGE];
	
	union {
		u16* pPage0TxBuf16Ptr;	
		u8* pPage0TxBuf8ptr;	
	};
	union {
		u16* pPage1TxBuf16Ptr;	
		u8* pPage1TxBuf8ptr;	
	};
		
	union {
		u16* pPage0RxBuf16Ptr;	
		u8* pPage0RxBuf8ptr;	
	};
	union {
		u16* pPage1RxBuf16Ptr;	
		u8* pPage1RxBuf8ptr;	
	};

}i2s_config_type;


void i2s_gen_test_pattern(void);
int i2s_mem_unmap(i2s_config_type* ptri2s_config);
int i2s_param_init(i2s_config_type* ptri2s_config);
int i2s_txbuf_alloc(i2s_config_type* ptri2s_config);
int i2s_rxbuf_alloc(i2s_config_type* ptri2s_config);
int i2s_txPagebuf_alloc(i2s_config_type* ptri2s_config);
int i2s_rxPagebuf_alloc(i2s_config_type* ptri2s_config);
int i2s_txbuf_free(i2s_config_type* ptri2s_config);
int i2s_rxbuf_free(i2s_config_type* ptri2s_config);
int i2s_txPagebuf_free(i2s_config_type* ptri2s_config);
int i2s_rxPagebuf_free(i2s_config_type* ptri2s_config);
int i2s_reset_tx_param(i2s_config_type* ptri2s_config);
int i2s_reset_rx_param(i2s_config_type* ptri2s_config);
int i2s_tx_config(i2s_config_type* ptri2s_config);
int i2s_rx_config(i2s_config_type* ptri2s_config);
int i2s_tx_enable(i2s_config_type* ptri2s_config);
int i2s_tx_disable(i2s_config_type* ptri2s_config);
int i2s_rx_enable(i2s_config_type* ptri2s_config);
int i2s_rx_disable(i2s_config_type* ptri2s_config);
int i2s_codec_enable(i2s_config_type* ptri2s_config);
int i2s_codec_disable(i2s_config_type* ptri2s_config);
int i2s_clock_enable(i2s_config_type* ptri2s_config);
int i2s_clock_disable(i2s_config_type* ptri2s_config);
int i2s_reset_config(i2s_config_type* ptri2s_config);
int i2s_refclk_disable(void);
int i2s_refclk_gpio_out_config(void);
int i2s_refclk_gpio_in_config(void);
int i2s_share_pin_config(i2s_config_type* ptri2s_config);
int i2s_share_pin_mt7623(i2s_config_type* ptri2s_config);
int i2s_master_clock_gpio_out_mt7623(void);
int i2s_slave_clock_gpio_in_mt7623(void);
int i2s_ws_config(i2s_config_type* ptri2s_config, unsigned long index);
int i2s_mode_config(u32 slave_en);
int i2s_codec_frequency_config(i2s_config_type* ptri2s_config, unsigned long index);
void i2s_tx_end_sleep_on(i2s_config_type* ptri2s_config);
void i2s_rx_end_sleep_on(i2s_config_type* ptri2s_config);

#if defined(CONFIG_I2S_MCLK_12MHZ)
int i2s_refclk_12m_enable(void);
#endif
#if defined(CONFIG_I2S_MCLK_12P288MHZ)
int i2s_refclk_12p288m_enable(void);
#endif

#if defined(MT7621_ASIC_BOARD)
int i2s_pll_config_mt7621(unsigned long index);
int i2s_pll_refclk_set(void);
#endif
#if defined(MT7623_ASIC_BOARD)
int i2s_pll_config_mt7623(unsigned long index);
#endif
#if defined(MT7628_ASIC_BOARD) || defined(CONFIG_ARCH_MT7623)
int i2s_driving_strength_adjust(void);
#endif
#if defined(I2S_STATISTIC)
void i2s_int_status(u32 dma_ch);
#endif
void i2s_dma_tx_handler(u32 dma_ch);
void i2s_dma_rx_handler(u32 dma_ch);
void i2s_dma_unmask_handler(u32 dma_ch);
void i2s_dma_mask_handler(u32 dma_ch);
void i2s_dma_tx_init(i2s_config_type* ptri2s_config);
void i2s_dma_rx_init(i2s_config_type* ptri2s_config);
void i2s_tx_task(unsigned long pData);
void i2s_rx_task(unsigned long pData);
void i2s_dma_tx_unmask_handler(u32 dma_ch);
void i2s_dma_rx_unmask_handler(u32 dma_ch);
int i2s_dma_tx_transf_data(i2s_config_type* ptri2s_config, u32 dma_ch);
int i2s_dma_tx_transf_zero(i2s_config_type* ptri2s_config, u32 dma_ch);
int i2s_dma_rx_transf_data(i2s_config_type* ptri2s_config, u32 dma_ch);
int i2s_dma_rx_transf_zero(i2s_config_type* ptri2s_config, u32 dma_ch);
void i2s_dma_tx_end_handle(i2s_config_type* ptri2s_config);
int i2s_dma_tx_soft_stop(i2s_config_type* ptri2s_config, u32 dma_ch);
int i2s_dma_rx_soft_stop(i2s_config_type* ptri2s_config, u32 dma_ch);

int i2s_page_prepare(i2s_config_type* ptri2s_config,int dir);
int i2s_page_release(i2s_config_type* ptri2s_config,int dir);
int gdma_En_Switch(i2s_config_type* ptri2s_config,int dir,int enabled);
int i2s_startup(void);
int i2s_audio_exchange(i2s_config_type* ptri2s_config,int dir,unsigned long arg);
void gdma_unmask_handler(u32 dma_ch);
char* i2s_memPool_Alloc(i2s_config_type* ptri2s_config,int dir);
void i2s_memPool_free(i2s_config_type* ptri2s_config,int dir);
u32 i2s_mmap_phys_addr(i2s_config_type* ptri2s_config);

#if !defined(CONFIG_I2S_TXRX)
#define GdmaI2sRx	//GdmaI2sRx
#endif

#define RALINK_I2S_VERSION	"1.0"
#define I2SDRV_DEVNAME		"i2s0"

#endif /* __RALINK_I2S_H_ */

