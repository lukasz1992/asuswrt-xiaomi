#ifndef _RALINK_HSDMA
#define _RALINK_HSDMA
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
#include <asm/rt2880/rt_mmap.h>
#endif


#define MOD_VERSION_HS_DMA 			"0.1"
#define NUM_HSDMA_RX_DESC     256
#define NUM_HSDMA_TX_DESC     256
#define ADDRESS_VLAUE         16
#define phys_to_bus(a) (a & 0x1FFFFFFF)

#if defined (CONFIG_ARCH_MT7623)
#include <mach/sync_write.h>
#define sysRegRead(phys)            (*(volatile unsigned int *)((phys)))
#define sysRegWrite(phys, val)      mt65xx_reg_sync_writel((val), (phys))
#else
#define PHYS_TO_K1(physaddr) KSEG1ADDR(physaddr)
#define sysRegRead(phys)        (*(volatile unsigned int *)PHYS_TO_K1(phys))
#define sysRegWrite(phys, val)  ((*(volatile unsigned int *)PHYS_TO_K1(phys)) = (val))
#endif



#define u_long	unsigned long
#define u32	unsigned int
#define u16	unsigned short


#define HSDMA_RELATED            0x0800
/* 1. HSDMA */
#define HSDMA_TX_BASE_PTR0            (RALINK_HS_DMA_BASE + HSDMA_RELATED+0x000)
#define HSDMA_TX_MAX_CNT0             (RALINK_HS_DMA_BASE + HSDMA_RELATED+0x004)
#define HSDMA_TX_CTX_IDX0             (RALINK_HS_DMA_BASE + HSDMA_RELATED+0x008)
#define HSDMA_TX_DTX_IDX0             (RALINK_HS_DMA_BASE + HSDMA_RELATED+0x00C)

#define HSDMA_RX_BASE_PTR0            (RALINK_HS_DMA_BASE + HSDMA_RELATED+0x100)
#define HSDMA_RX_MAX_CNT0             (RALINK_HS_DMA_BASE + HSDMA_RELATED+0x104)
#define HSDMA_RX_CALC_IDX0            (RALINK_HS_DMA_BASE + HSDMA_RELATED+0x108)
#define HSDMA_RX_DRX_IDX0             (RALINK_HS_DMA_BASE + HSDMA_RELATED+0x10C)

#define HSDMA_INFO                    (RALINK_HS_DMA_BASE + HSDMA_RELATED+0x200)
#define HSDMA_GLO_CFG 								(RALINK_HS_DMA_BASE + HSDMA_RELATED+0x204)
#define HSDMA_RST_IDX            			(RALINK_HS_DMA_BASE + HSDMA_RELATED+0x208)
#define HSDMA_RST_CFG            			(HSDMA_RST_IDX)
#define HSDMA_DLY_INT_CFG             (RALINK_HS_DMA_BASE + HSDMA_RELATED+0x20C)
#define HSDMA_FREEQ_THRES             (RALINK_HS_DMA_BASE + HSDMA_RELATED+0x210)
#define HSDMA_INT_STATUS              (RALINK_HS_DMA_BASE + HSDMA_RELATED+0x220)
#define HSDMA_FE_INT_STATUS		        (HSDMA_INT_STATUS)
#define HSDMA_INT_MASK                (RALINK_HS_DMA_BASE + HSDMA_RELATED+0x228)
//#define FE_INT_ENABLE		(INT_MASK)
#define SCH_Q01_CFG		(RALINK_HS_DMA_BASE+RAHSDMA_OFFSET+0x280)
#define SCH_Q23_CFG		(RALINK_HS_DMA_BASE+RAHSDMA_OFFSET+0x284)
/*BUS MATRIX*/
#define OCP_CFG0			(RALINK_RBUS_MATRIXCTL_BASE+0x000);
#define OCP_CFG1      (RALINK_RBUS_MATRIXCTL_BASE+0x004);
#define DYN_CFG0      (RALINK_RBUS_MATRIXCTL_BASE+0x010);
#define DYN_CFG1			(RALINK_RBUS_MATRIXCTL_BASE+0x014);
#define DYN_CFG2      (RALINK_RBUS_MATRIXCTL_BASE+0x018);
#define DYN_CFG3      (RALINK_RBUS_MATRIXCTL_BASE+0x01C);
#define IOCU_CFG      (RALINK_RBUS_MATRIXCTL_BASE+0x020);


/* ====================================== */
/* ====================================== */
#define GP1_LNK_DWN     (1<<9)
#define GP1_AN_FAIL     (1<<8) 
/* ====================================== */
/* ====================================== */
#define PSE_RESET       (1<<0)
/* ====================================== */
#define HSDMA_PST_DRX_IDX1       (1<<17)
#define HSDMA_PST_DRX_IDX0       (1<<16)
#define HSDMA_PST_DTX_IDX3       (1<<3)
#define HSDMA_PST_DTX_IDX2       (1<<2)
#define HSDMA_PST_DTX_IDX1       (1<<1)
#define HSDMA_PST_DTX_IDX0       (1<<0)

#define HSDMA_RX_2B_OFFSET	    (1<<31)
#define HSDMA_MUTI_ISSUE        (1<<10)
#define HSDMA_TWO_BUFFER        (1<<9)
#define HSDMA_TX_WB_DDONE       (1<<6)
#define HSDMA_RX_DMA_BUSY       (1<<3)
#define HSDMA_TX_DMA_BUSY       (1<<1)
#define HSDMA_RX_DMA_EN         (1<<2)
#define HSDMA_TX_DMA_EN         (1<<0)

#define HSDMA_BT_SIZE_4DWORDS     (0<<4)
#define HSDMA_BT_SIZE_8DWORDS     (1<<4)
#define HSDMA_BT_SIZE_16DWORDS    (2<<4)
#define HSDMA_BT_SIZE_32DWORDS    (3<<4)

#define HSDMA_RX_COHERENT      (1<<31)
#define HSDMA_RX_DLY_INT       (1<<30)
#define HSDMA_TX_COHERENT      (1<<29)
#define HSDMA_TX_DLY_INT       (1<<28)
#define HSDMA_RX_DONE_INT0     (1<<16)
#define HSDMA_TX_DONE_INT0     (1)
#define HSDMA_TX_DONE_BIT      (1<<31)
#define HSDMA_TX_LS0           (1<<30)
/*
#define FE_INT_ALL		(HSDMA_TX_DONE_INT3 | HSDMA_TX_DONE_INT2 | \
			         HSDMA_TX_DONE_INT1 | HSDMA_TX_DONE_INT0 | \
	                         HSDMA_RX_DONE_INT0 | HSDMA_RX_DONE_INT1)*/
#define HSDMA_FE_INT_ALL    (HSDMA_TX_DONE_INT0 | HSDMA_RX_DONE_INT0)                    
#define HSDMA_FE_INT_TX			(HSDMA_TX_DONE_INT0)
#define HSDMA_FE_INT_RX     (HSDMA_RX_DONE_INT0)
#define HSDMA_FE_INT_STATUS_REG (*(volatile unsigned long *)(FE_INT_STATUS))
#define HSDMA_FE_INT_STATUS_CLEAN(reg) (*(volatile unsigned long *)(FE_INT_STATUS)) = reg

// Define Whole FE Reset Register
#define RSTCTRL         (RALINK_SYSCTL_BASE + 0x34)
/*=========================================
      HSDMA HSDMA_RX Descriptor Format define
=========================================*/

//-------------------------------------------------
typedef struct _HSDMA_RXD_INFO1_  HSDMA_RXD_INFO1_T;

struct _HSDMA_RXD_INFO1_
{
    volatile unsigned int    PDP0;
};
//-------------------------------------------------
typedef struct _HSDMA_RXD_INFO2_    HSDMA_RXD_INFO2_T;

struct _HSDMA_RXD_INFO2_
{
    volatile unsigned int    PLEN1                 : 14;
    volatile unsigned int    LS1                   : 1;
    volatile unsigned int    TAG                   : 1;
    volatile unsigned int    PLEN0                 : 14;
    volatile unsigned int    LS0                   : 1;
    volatile unsigned int    DDONE_bit             : 1;
};
//-------------------------------------------------
typedef struct _HSDMA_RXD_INFO3_  HSDMA_RXD_INFO3_T;

struct _HSDMA_RXD_INFO3_
{
    volatile unsigned int    PDP1;
};
//-------------------------------------------------
typedef struct _HSDMA_RXD_INFO4_    HSDMA_RXD_INFO4_T;

struct _HSDMA_RXD_INFO4_
{
	 unsigned int	unused : 32;
};


struct HSDMA_rxdesc {
	HSDMA_RXD_INFO1_T hsdma_rxd_info1;
	HSDMA_RXD_INFO2_T hsdma_rxd_info2;
	HSDMA_RXD_INFO3_T hsdma_rxd_info3;
	HSDMA_RXD_INFO4_T hsdma_rxd_info4;
};

/*=========================================
      HSDMA HSDMA_TX Descriptor Format define
=========================================*/
//-------------------------------------------------
typedef struct _HSDMA_TXD_INFO1_  HSDMA_TXD_INFO1_T;

struct _HSDMA_TXD_INFO1_
{
    volatile unsigned int    SDP0;
};
//-------------------------------------------------
typedef struct _HSDMA_TXD_INFO2_    HSDMA_TXD_INFO2_T;

struct _HSDMA_TXD_INFO2_
{
	
    volatile unsigned int    SDL1                  : 14;
    volatile unsigned int    LS1_bit               : 1;
    volatile unsigned int    BURST_bit             : 1;
    volatile unsigned int    SDL0                  : 14;
    volatile unsigned int    LS0_bit               : 1;
    volatile unsigned int    DDONE_bit             : 1;
    //volatile unsigned int    All                   : 32;
};
//-------------------------------------------------
typedef struct _HSDMA_TXD_INFO3_  HSDMA_TXD_INFO3_T;

struct _HSDMA_TXD_INFO3_
{
    volatile unsigned int    SDP1;
};
//-------------------------------------------------
typedef struct _HSDMA_TXD_INFO4_    HSDMA_TXD_INFO4_T;

struct _HSDMA_TXD_INFO4_
{
		unsigned int	unused : 32;
};


struct HSDMA_txdesc {
	HSDMA_TXD_INFO1_T hsdma_txd_info1;
	HSDMA_TXD_INFO2_T hsdma_txd_info2;
	HSDMA_TXD_INFO3_T hsdma_txd_info3;
	HSDMA_TXD_INFO4_T hsdma_txd_info4;
};


struct HSdmaReqEntry {

    unsigned int  hsdma_tx_full;
    unsigned int	phy_hsdma_tx_ring0;
    unsigned int	phy_hsdma_rx_ring0;
    spinlock_t          page_lock;
    spinlock_t          page_lock_mem;               /* Page register locks */
    struct HSDMA_txdesc *HSDMA_tx_ring0;

    struct HSDMA_rxdesc *HSDMA_rx_ring0;
		int (*DoneIntCallback)(uint32_t, uint32_t);
		struct work_struct  reset_task;
};

int HS_DmaMem2Mem(uint32_t Src, uint32_t Dst, uint16_t TransCount, int (*DoneIntCallback)(uint32_t transcount, uint32_t data)); 		
int HS_DmaMem2MemScatterGather(uint32_t Src0, uint32_t Dst0, uint32_t TransCount, uint16_t DescNum, uint16_t LastDescLSO0, uint16_t LastDescLSO1, int (*DoneIntCallback)(uint32_t transcount, uint32_t data));
int HS_DmaMem2MemGenUse(uint32_t Src0, uint32_t Dst0, uint32_t TransCount, int (*DoneIntCallback)(uint32_t transcount, uint32_t data));
#endif
