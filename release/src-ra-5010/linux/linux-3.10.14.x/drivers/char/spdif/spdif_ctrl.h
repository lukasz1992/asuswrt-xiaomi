#ifndef __RALINK_SPDIF_H_
#define __RALINK_SPDIF_H_

#ifdef __KERNEL__
#include <asm/rt2880/rt_mmap.h>
#endif

/****************************/
/* SPDIF related definition */
/****************************/
#define SPDIF_MAX_DEV          		1
#define SPDIF_MOD_VERSION               "0.1"
#define phys_to_bus(a) 			(a & 0x1FFFFFFF)

#define RALINK_SPDIF_VERSION      	"1.0"
#define SPDIFDRV_DEVNAME          	"spdif0"

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
#define REGBIT(x, n)            	(x << n)
#endif

#define Virtual2Physical(x)             (((int)x) & 0x1fffffff)
#define Physical2Virtual(x)             (((int)x) | 0x80000000)
#define Virtual2NonCache(x)             (((int)x) | 0x20000000)
#define Physical2NonCache(x)            (((int)x) | 0xa0000000)
#define NonCache2Virtual(x)             (((int)x) & 0xDFFFFFFF)

//#define SPDIF_DEBUG_PRN
#ifdef SPDIF_DEBUG_PRN
#define MSG(fmt, args...) 		printk("SPDIF: " fmt, ## args)
#else
#define MSG(fmt, args...) 		{ }
#endif

#ifdef SPDIF_DEBUG_PRN
#define spdif_outw(address, value)      do{printk("0x%08X = 0x%08X\n",(u32)address,(u32)value);*((volatile uint32_t *)(address)) = cpu_to_le32(value);}while(0)
#else
#define spdif_outw(address, value)    	*((volatile uint32_t *)(address)) = cpu_to_le32(value)
#endif
#define spdif_inw(address)           	le32_to_cpu(*(volatile u32 *)(address))

/* Configuration enable */
#define SPDIF_STATISTIC
//#define SPDIF_AC3_DEBUG
//#define CONFIG_SPDIF_MMAP	

#if defined(CONFIG_SND_ALSA_SPDIF)
/* Definition for ALSA */
#define SUBSTREAM_PLAYBACK	0
#define SUBSTREAM_CAPTURE	1
#endif

/*******************************************/ 
/* Register Map, Ref. to MT7621 Data Sheet */
/*******************************************/

/* Register Map Detail - SPDIF*/
#define IEC_CTRL                  	(RALINK_SPDIF_BASE+0x0000)
#define IEC_BUF0_BS_SBLK               	(RALINK_SPDIF_BASE+0x0004)
#define IEC_BUF0_BS_EBLK               	(RALINK_SPDIF_BASE+0x0008)
#define IEC_BUF0_NSADR                 	(RALINK_SPDIF_BASE+0x000c)
#define IEC_BUF0_NEXT_UADR             	(RALINK_SPDIF_BASE+0x0010)
#define IEC_BUF0_INTR_NSNUM            	(RALINK_SPDIF_BASE+0x0014)
#define IEC_BUF0_PCPD_PACK             	(RALINK_SPDIF_BASE+0x0018)
#define IEC_BUF0_CH_CFG_TRIG           	(RALINK_SPDIF_BASE+0x001c)
#define IEC_BUF0_CH_CFG0               	(RALINK_SPDIF_BASE+0x0020)
#define IEC_BUF0_CH_CFG1               	(RALINK_SPDIF_BASE+0x0024)
#define IEC_BUF0_CH_CFG2               	(RALINK_SPDIF_BASE+0x0028)
#define IEC_BUF0_CH_CFG3               	(RALINK_SPDIF_BASE+0x002c)
#define IEC_BUF0_CH_CFG4               	(RALINK_SPDIF_BASE+0x0030)
#define IEC_BUF0_CH_CFG5               	(RALINK_SPDIF_BASE+0x0034)
#define IEC_BUF0_PFT_STS               	(RALINK_SPDIF_BASE+0x0038)
#define IEC_ACLK_DIV              	(RALINK_SPDIF_BASE+0x003c)
#define IEC_APLL_CFG0             	(RALINK_SPDIF_BASE+0x0040)
#define IEC_APLL_CFG1             	(RALINK_SPDIF_BASE+0x0044)
#define IEC_APLL_CFG2             	(RALINK_SPDIF_BASE+0x0048)
#define IEC_APLL_CFG3             	(RALINK_SPDIF_BASE+0x004c)
#define IEC_APLL_DEBUG            	(RALINK_SPDIF_BASE+0x0050)
#define IEC_BUF1_BS_SBLK               	(RALINK_SPDIF_BASE+0x0054)
#define IEC_BUF1_BS_EBLK               	(RALINK_SPDIF_BASE+0x0058)
#define IEC_BUF1_NSADR                 	(RALINK_SPDIF_BASE+0x005c)
#define IEC_BUF1_NEXT_UADR             	(RALINK_SPDIF_BASE+0x0060)
#define IEC_BUF1_INTR_NSNUM            	(RALINK_SPDIF_BASE+0x0064)
#define IEC_BUF1_PCPD_PACK             	(RALINK_SPDIF_BASE+0x0068)
#define IEC_BUF1_CH_CFG_TRIG           	(RALINK_SPDIF_BASE+0x006c)
#define IEC_BUF1_CH_CFG0               	(RALINK_SPDIF_BASE+0x0070)
#define IEC_BUF1_CH_CFG1               	(RALINK_SPDIF_BASE+0x0074)
#define IEC_BUF1_CH_CFG2               	(RALINK_SPDIF_BASE+0x0078)
#define IEC_BUF1_CH_CFG3               	(RALINK_SPDIF_BASE+0x007c)
#define IEC_BUF1_CH_CFG4               	(RALINK_SPDIF_BASE+0x0080)
#define IEC_BUF1_CH_CFG5               	(RALINK_SPDIF_BASE+0x0084)
#define IEC_BUF1_PFT_STS               	(RALINK_SPDIF_BASE+0x0088)


/* IEC_CTRL bit field */
#define RAW_EN				31
#define RAW_EN_OPT			30
#define BYPS_PFT			24
#define USER_PFT_RST			22
#define RAW_PFT_RST			20
#define INTR_EN				16
#define IEC_INTR_STATUS			15
#define DBUF_SEL			12
#define DBUF_DISABLE			11
#define DOWN_SAMPLE			8
#define MUTE_SPDF			7
#define BYTE_SWAP			6
#define RAW_SWAP			5
#define RAW_24				4
#define MUTE_SAMPLE			3
#define UDATA_EN  			2
#define DATA_FMT			1
#define DATA_SRC			0


/* IEC_BUF0_INTR_NSNUM & IEC_BUF1_INTR_NSNUM bit field */
#define INTR_SIZE			16
#define BURST_NSNUM			0

/* IEC_BUF0_PCPD_PACK & IEC_BUF1_PCPD_PACK bit field */
#define NB_LEN				16
#define BITSTREAM_NUM			13
#define DEP_INFO			8
#define ERR_FLAG			7
#define SUBDATA_TYPE			5
#define DATA_TYPE			0

/* IEC_BUF0_CH_CFG_TRIG & IEC_BUF1_CH_CFG_TRIG bit field */
#define CH_CFG_TRIG			31
#define CH2_NUM				20

/* IEC_BUF0_CH_CFG0 & IEC_BUF1_CH_CFG0 bit field */
#define CLK_ACCURACY			28
#define SAM_FREQ			24
#define CH1_NUM				20
#define SRC_NUM				16
#define CATEGORY_CODE			8
#define CH_MODE			 	6
#define ADD_INFO			3
#define CP_RIGHT			2
#define	CH_DIGITAL			1
#define CH_CONSUMER			0

/* IEC_BUF0_CH_CFG1 & IEC_BUF0_CH_CFG1 bit field */
#define CGMS_A				8
#define ORIGINAL_FS			4
#define WORD_LEN			0

/* IEC_ACLK_DIV bit field */
#define MAS_DIV				16
#define IEC_DIV				12
#define BIT_DIV				8
#define LRC_DIV				0


/* IEC_APLL_CFG0 bit field */
#define APLL_SEL			0

/***********************/
/* Constant definition */
/***********************/
#define MAX_SPDIF_PAGE           	10
#define PCM_BURST_SAMPLE		1536
#define NULL_BURST_SAMPLE		500    // FIXME
#define AC3_BURST_SAMPLE		1536
#define PAUSE_BURST_SAMPLE		1536

#define SPDIF_MIN_PAGE_SIZE		4096

#define BIT_MODE_16			2
#define BIT_MODE_24			3

#define MAX_SFREQ_HZ			192000
#define MIN_SFREQ_HZ			22050

#define MAX_WORD_LEN			24
#define MIN_WORD_LEN			16

#define MAX_DATA_TYPE			3
#define MIN_DATA_TYPE			0
#define RESERVE_DATA_TYPE		2

#define SPDIF_DMA_BUF0			0
#define SPDIF_DMA_BUF1			1

#define SPDIF_BUF_SIZE      		(1536*4*2)
/**************************/
/* SPDIF I/O ctrl command */
/**************************/
#define SPDIF_SAMPLE_FREQ		0
#define SPDIF_WORD_LEN			1
#define SPDIF_RAW_DATA_TYPE		2
#define SPDIF_NB_LEN			3
#define SPDIF_TX_PCM_ENABLE		4
#define SPDIF_TX_RAW_ENABLE		5
#define SPDIF_TX_DISABLE		6
#define SPDIF_PCM_PUT_AUDIO		7	
#define SPDIF_DOWN_SAMPLE		8
#define SPDIF_RAW_PUT_AUDIO		9
#define SPDIF_BYTE_SWAP_SET		10
#define SPDIF_INIT_PARAMETER		11
#define SPDIF_TX_STOP			12
#define SPDIF_TX_PAUSE			13
#define SPDIF_TX_RESUME			14

/***************/
/* Definition */
/***************/



/****************************/
/* Driver status definition */
/****************************/
#define SPDIF_OK                	0


/************/
/* IEC_CTRL */
/************/
//intr_status
#define NO_INTERRUPT			0
#define INTERRUPT_OCCUR			1

// down_sample
#define SPDIF_NO_DOWN_SAMPLE		0
#define SPDIF_2X_DOWN_SAMPLE		1
#define SPDIF_4X_DOWN_SAMPLE		3

// data_fmt
#define PCM_DATA			0
#define ENCODED_DATA			1


/******************************************/
/* IEC_PCPD_PACK -- burst info definition */
/******************************************/
// data_type
#define NULL_DATA			0
#define	AC3_DATA			1
#define PAUSE_DATA			3

// error flag
#define VALID_BURST_PAYLOAD		0
#define ERR_BURST_PAYLOAD		1

// dependent info
#define GENERAL_DATA			0
#define SEQUENCE_DISCONNECT		1

/************************************/
/* Channel Status Config defination */
/************************************/
// clk_accuracy
#define LEVEL_II			0
#define LEVEL_I				1
#define LEVEL_III			2
#define NOT_MATCH			3

// digital_ch_stat (bit 1 of channel status)
#define LINEAR_PCM			0
#define OTHER_PURPOSE			1

// Sam_freq
#define SFREQ_44100HZ			0	// 44.1K
#define SFREQ_NOT_INDICATED		1	// not indicated
#define	SFREQ_48000HZ			2	// 48K
#define SFREQ_32000HZ			3	// 32K
#define SFREQ_22050HZ			4	// 22.05K
#define	SFREQ_24000HZ			6	// 24K
#define	SFREQ_88200HZ			8	// 88.2K
#define	SFREQ_96000HZ			10	// 96K
#define	SFREQ_176400HZ			12	// 176.4K
#define	SFREQ_192000HZ			14	// 192K

// digital_samp
#define LINEAR_PCM			0
#define OTHER_PURPOSE			1

// original_sf
#define ORIG_SFREQ_NOT_INDICATED	0
#define ORIG_SFREQ_192000HZ		1
#define ORIG_SFREQ_12000HZ		2
#define ORIG_SFREQ_176400HZ		3
#define ORIG_SFREQ_96000HZ		5
#define ORIG_SFREQ_8000HZ		6
#define ORIG_SFREQ_88200HZ		7
#define ORGI_SFREQ_16000HZ		8
#define ORIG_SFREQ_24000HZ		9
#define ORIG_SFREQ_11025HZ		10
#define ORIG_SFREQ_22050HZ		11
#define ORIG_SFREQ_32000HZ		12
#define ORIG_SFREQ_48000HZ		13
#define ORIG_SFREQ_44100HZ		15

// word_len
#define WLEN_NOT_INDICATED_20BIT	0		
#define WLEN_16_20BIT			2
#define WLEN_18_20BIT			4
#define WLEN_19_20BIT			8
#define WLEN_20_20BIT			10
#define WLEN_17_20BIT			12

#define WLEN_NOT_INDICATED_24BIT	1
#define WLEN_20_24BIT			3
#define WLEN_22_24BIT			5
#define WLEN_23_24BIT			9
#define WLEN_24_24BIT			11
#define	WLEN_21_24BIT			13

/************************/
/* Structure definition */
/************************/
typedef struct spdif_status_t
{
        u32 txdmafault;
        u32 txovrun;
        u32 txunrun;
        u32 txthres;
        int txbuffer_unrun;
        int txbuffer_ovrun;
        int txbuffer_len;
}spdif_status_type;

typedef struct spdif_config_t
{
        u32 pos;
        u32 tx_isr_cnt;
#ifdef __KERNEL__
        spinlock_t lock;
        wait_queue_head_t spdif_tx_qh;
#if defined(CONFIG_SND_ALSA_SPDIF)
	struct snd_pcm_substream *substream[2];
#endif	
#endif
        u32 hw_buf;
	u32 dma_buf;
	u32 buff_chain_run;
        u32 dma_unmask_status;

        int tx_w_idx;
        int tx_r_idx;
	int mmap_index;
	int interrupt_cnt;

	int buf_undflow;
	u32 data_type_tmp;
	u32 subdata_type_tmp;
	u32 srate;
	u32 nb_len;
	int bTxDMAEnable;

	/* IEC_CTRL configuration */
	u32 raw_en;
	u32 intr_en;
	u32 intr_status;  	// interrupt flag
	u32 dbuf_sel;		// dram ping-pong buffer indicator
	u32 down_sample;	// IEC958 down sample control
	u32 mute_spdif;		// mute IEC output SPDF signal ---> 0: normal; 1: mute output SPDF 
	u32 byte_swap;		// IEC dram word data bytes switch mode
	u32 raw_swap;		// IEC 24bit raw bytes switch mode  
	u32 raw_24;		// IEC raw data 24bit mode
	u32 mute_sample;	// mute IEC output sample data ---> 0: normal; 1: mute output sample data
	u32 udata_en;		
	u32 data_fmt;		// output data format selection
	u32 data_src;		// data source selection ---> 0: cooked data; 1: raw data (from DRAM 61937 encoded audio data or 60958 plain PCM data) 	

	/* IEC_BUF0_NSADR & IEC_BUF1_NSADR */
	u32 nsadr_buf0;
	u32 nsadr_buf1;

	/* IEC_BUF0_NEXT_UADR & IEC_BUF1_NEXT_UADR */
	u32 nusadr_buf0;
	u32 nusadr_buf1;

	/* IEC_BUF0_INTR_NSNUM & IEC_BUF1_INTR_NSNUM*/
	u32 intr_size;      // buf0 & buf1 share
	u32 nsnum; 	    // buf0 & buf1 share

	/* IEC_BUF0_PCPD_PACK */
	u32 nb_len_buf0;
	u32 data_type;      // buf0 & buf1 share
	u32 subdata_type;   // buf0 & buf1 share 
	u32 err_flag_buf0;
	u32 dep_info_buf0;
	u32 bstream_num_buf0;

	/* IEC_BUF1_PCPD_PACK */
	u32 nb_len_buf1;
	u32 err_flag_buf1;
	u32 dep_info_buf1;
	u32 bstream_num_buf1;

	/* for channel status configuration */
	u32 ch_cfg_trig;
	u32 clk_accuracy;
	u32 sam_freq;		// sampling frequency
	u32 ch1_num;		// 0: Do not take into account; General be "0"
	u32 ch2_num;		// 0: Do not take into account; General be "0"
	u32 src_num;		// 0: Do not take into account; General be "0"
	u32 category_code;	// General. Use temporarily
	u32 ch_status_mode;	// default is mode0
	u32 add_info;
	u32 cp_right; 		// copy right: 0: cp right is asserted; 1: cp right is not asserted
        u32 digital_ch_stat;	// 0: linear PCM samples; 1: other purpose 
	u32 consumer_use; 	// 0: consumer use; 1: professional use
	u32 cgms_a;		// for DTS format
	u32 original_fs;   	// original sample frequency
	u32 word_len;		// word length


	/* SPDIF other setting*/
	u32 buf_size;
	u32 burst_sample;
	u32 byte_num;
	u32 copy_size;
	u32 pause_en;

        u8* buf8ptr;
        //char* pMMAPBufPtr[MAX_SPDIF_PAGE*2];
	char* pMMAPBufPtr[MAX_SPDIF_PAGE+1];
        //char* pMMAPTxBufPtr[MAX_SPDIF_PAGE+1];
        //char* pMMAPRxBufPtr[MAX_SPDIF_PAGE];

	u8* pPageTxBuf8ptr[MAX_SPDIF_PAGE+1];

	u32 txdma_addr_buf[MAX_SPDIF_PAGE+1];
	int rdone_bit[MAX_SPDIF_PAGE];
	int page0_start;
	int page1_start;

}spdif_config_type;




/***********************/
/* Funtion declaration */
/***********************/
#if defined(CONFIG_SPDIF_MMAP)
int spdif_mem_unmap(spdif_config_type* pspdif_config);
int spdif_mmap_txbuf_alloc(spdif_config_type* pspdif_config);
int spdif_mmap_txbuf_free(spdif_config_type* pspdif_config);
#endif
int spdif_clock_enable(spdif_config_type* pspdif_config);
int spdif_clock_disable(spdif_config_type* pspdif_config);
int spdif_txPagebuf_alloc(spdif_config_type* pspdif_config);
int spdif_ch_status_init(spdif_config_type* pspdif_config);
int spdif_nsadr_set(spdif_config_type* pspdif_config, u32 dma_ch, u32 r_idx);
int spdif_ch_status_config(spdif_config_type* pspdif_config, u32 dma_ch);
int spdif_ch_cfg_trig(spdif_config_type* pspdif_config, u32 dma_ch);
int spdif_intr_status_config(spdif_config_type* pspdif_config);
int spdif_next_uadr_config(spdif_config_type* pspdif_config, u32 dma_ch);
int spdif_nsadr_config(spdif_config_type* pspdif_config, u32 dma_ch);
int spdif_intr_nsnum_config(spdif_config_type* pspdif_config, u32 dma_ch);
int spdif_pcpd_pack_config(spdif_config_type* pspdif_config, u32 dma_ch);
int spdif_reset_tx_config(spdif_config_type* pspdif_config);
int spdif_tx_hw_config(spdif_config_type* pspdif_config);
int spdif_tx_enable(spdif_config_type* pspdif_config);
int spdif_tx_disable(spdif_config_type* pspdif_config);
int spdif_txPagebuf_free(spdif_config_type* pspdif_config);
int spdif_clock_divider_set(spdif_config_type* pspdif_config, unsigned long index);
int spdif_pcm_bufsize_init(spdif_config_type* pspdif_config);
int spdif_raw_bufsize_init(spdif_config_type* pspdif_config);
int spdif_pcm_intr_nsnum_init(spdif_config_type* pspdif_config);
int spdif_raw_intr_nsnum_init(spdif_config_type* pspdif_config);
int spdif_pcpd_pack_init(spdif_config_type* pspdif_config);
int spdif_tx_transf_zero_set(spdif_config_type* pspdif_config, u32 dma_ch);
int spdif_tx_transf_data_set(spdif_config_type* pspdif_config, u32 dma_ch);
int spdif_tx_transf_zero_handle(spdif_config_type* pspdif_config);
int spdif_tx_transf_data_handle(spdif_config_type* pspdif_config);
int spdif_buffer_underflow_occur(spdif_config_type* pspdif_config);
int spdif_buffer_underflow_recover(spdif_config_type* pspdif_config);
int spdif_rdone_bit_handle(spdif_config_type* pspdif_config, u32 dma_ch);
int spdif_rdone_bit_init(spdif_config_type* pspdif_config);
int spdif_pcm_put_audio(spdif_config_type* pspdif_config, unsigned long arg);
int spdif_raw_put_audio(spdif_config_type* pspdif_config, unsigned long arg);
int spdif_irq_request(void);
int spdif_irq_free(void);
void spdif_reset(void);
void spdif_share_pin_config(void);
void spdif_tx_clock_config(void);
void spdif_mute_config(void);
void spdif_demute_config(void);
void spdif_nb_len_config(spdif_config_type* pspdif_config, u32 dma_ch);

#if defined(CONFIG_RALINK_MT7621)
int spdif_pll_config(unsigned long index);
#endif

#if defined(SPDIF_AC3_DEBUG)
void spdif_ac3_header_check(spdif_config_type* pspdif_config);
#endif

/**********************
 * SPDIF API for ALSA *
 **********************/
#if defined(CONFIG_SND_ALSA_SPDIF)
char* spdif_tx_dma_alloc(spdif_config_type* pspdif_config);
int spdif_tx_dma_free(spdif_config_type* pspdif_config);
int spdif_bufsize_init(spdif_config_type* pspdif_config);
int spdif_startup(void);
int spdif_shutdown(void);
int spdif_tx_idx_update(spdif_config_type* pspdif_config);
int spdif_tx_dma_ctrl_enable(spdif_config_type* pspdif_config);
int spdif_tx_dma_ctrl_disable(spdif_config_type* pspdif_config);
int spdif_irq_init(void);
int spdif_irq_release(void);
#endif 
#endif
