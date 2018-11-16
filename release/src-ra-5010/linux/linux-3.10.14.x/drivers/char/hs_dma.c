#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/if_vlan.h>
#include <linux/if_ether.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <asm/uaccess.h>
#include <asm/rt2880/surfboardint.h>


#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
#include <asm/rt2880/rt_mmap.h>
#else
#include <linux/libata-compat.h>
#endif

#include "hs_dma.h"

static int hsdma_rx_dma_owner_idx0;
static int hsdma_rx_calc_idx0;
static unsigned long hsdma_tx_cpu_owner_idx0=0;
static unsigned long hsdma_tx_cpu_idx=0;
//static unsigned long hsdma_rx_cpu_owner_idx0 =0; //for chain mode
static unsigned long updateCRX =0;
static unsigned long TransCountLength;
int (*HSdmaDoneIntCallback[ADDRESS_VLAUE])(uint32_t ,uint32_t); 

struct HSdmaReqEntry HSDMA_Entry;
EXPORT_SYMBOL(HSDMA_Entry);

int HS_DmaMem2MemScatterGather( 
	uint32_t Src0,
	uint32_t Dst0,
	uint32_t TransCount,
	uint16_t DescNum,
	uint16_t LastDescLSO0,
	uint16_t LastDescLSO1,
	int (*DoneIntCallback)(uint32_t transcount, uint32_t data)
	)
{
	int i=0;
	int j=0;
	int DescRx=0;
	unsigned int HSDMA_SDL0_Addr=0 ;
	unsigned int HSDMA_SDL1_Addr=0;
	unsigned int HSDMA_SDL0_Len=0;
	unsigned int HSDMA_SDL1_Len=0;
	unsigned int HSDMA_PDP0_Addr=0 ;
	unsigned int HSDMA_PLEN0_Len=0;
	unsigned int hsdma_tx_scatter=0;
	unsigned int hsdma_rx_gather=0;
	unsigned long flags;
	HSDMA_Entry.DoneIntCallback = DoneIntCallback;
	//spin_lock_irqsave(&(HSDMA_Entry.page_lock_mem), flags);
	while(i<DescNum){
		hsdma_tx_scatter = (hsdma_tx_cpu_owner_idx0 + i) % NUM_HSDMA_TX_DESC;
		
		if (i==0){
			HSDMA_Entry.HSDMA_tx_ring0[hsdma_tx_scatter].hsdma_txd_info1.SDP0 = ((Src0) & 0xFFFFFFFF);
		}
		else{
			HSDMA_Entry.HSDMA_tx_ring0[hsdma_tx_scatter].hsdma_txd_info1.SDP0 = ((HSDMA_SDL1_Addr+HSDMA_SDL1_Len) & 0xFFFFFFFF);	
		}
		HSDMA_Entry.HSDMA_tx_ring0[hsdma_tx_scatter].hsdma_txd_info2.SDL0 = 16000;  //TX0 length, 
		HSDMA_SDL0_Addr = HSDMA_Entry.HSDMA_tx_ring0[hsdma_tx_scatter].hsdma_txd_info1.SDP0;
		HSDMA_SDL0_Len =  HSDMA_Entry.HSDMA_tx_ring0[hsdma_tx_scatter].hsdma_txd_info2.SDL0;	
		HSDMA_Entry.HSDMA_tx_ring0[hsdma_tx_scatter].hsdma_txd_info2.LS0_bit = 0;
		HSDMA_Entry.HSDMA_tx_ring0[hsdma_tx_scatter].hsdma_txd_info2.DDONE_bit = 0;
		
		
		if (LastDescLSO0 == 0 && LastDescLSO1 == 1){ // used i^th SDPL1
			HSDMA_Entry.HSDMA_tx_ring0[hsdma_tx_scatter].hsdma_txd_info3.SDP1 = ((HSDMA_SDL0_Addr+HSDMA_SDL0_Len) & 0xFFFFFFFF);	
			if ( i < (DescNum-1) ){
				HSDMA_Entry.HSDMA_tx_ring0[hsdma_tx_scatter].hsdma_txd_info2.SDL1= 16000;  //TX1 length, 
			}
		else if(i==DescNum-1)
		{
			HSDMA_Entry.HSDMA_tx_ring0[hsdma_tx_scatter].hsdma_txd_info2.SDL1 = TransCount - (DescNum-1)*2*16000-16000;  //TX1 length, 
 
		}
		HSDMA_Entry.HSDMA_tx_ring0[hsdma_tx_scatter].hsdma_txd_info2.LS0_bit = 0;
		HSDMA_SDL1_Len = HSDMA_Entry.HSDMA_tx_ring0[hsdma_tx_scatter].hsdma_txd_info2.SDL1;
		HSDMA_SDL1_Addr = HSDMA_Entry.HSDMA_tx_ring0[hsdma_tx_scatter].hsdma_txd_info3.SDP1;
			
		if ((i == (DescNum-1)) | (DescNum == 1)){ 
			HSDMA_Entry.HSDMA_tx_ring0[hsdma_tx_scatter].hsdma_txd_info2.LS1_bit = 1;
		}
		}
		else if(LastDescLSO0 == 1 && LastDescLSO1 == 0){//used i^th SDPL0
		
			if (i<(DescNum-1)){
				HSDMA_Entry.HSDMA_tx_ring0[hsdma_tx_scatter].hsdma_txd_info3.SDP1 = ((HSDMA_SDL0_Addr+HSDMA_SDL0_Len) & 0xFFFFFFFF);	
				HSDMA_Entry.HSDMA_tx_ring0[hsdma_tx_scatter].hsdma_txd_info2.SDL1 = 16000;  //TX1 length, 
				HSDMA_Entry.HSDMA_tx_ring0[hsdma_tx_scatter].hsdma_txd_info2.LS0_bit = 0;
			}
			else if(i==DescNum-1){
				HSDMA_Entry.HSDMA_tx_ring0[hsdma_tx_scatter].hsdma_txd_info2.SDL0 = TransCount - (DescNum-1)*16000*2;  //TX1 length, 	  
				HSDMA_Entry.HSDMA_tx_ring0[hsdma_tx_scatter].hsdma_txd_info2.LS0_bit = 1;
			}
			HSDMA_SDL1_Len = HSDMA_Entry.HSDMA_tx_ring0[hsdma_tx_scatter].hsdma_txd_info2.SDL1; 
			HSDMA_SDL1_Addr = HSDMA_Entry.HSDMA_tx_ring0[hsdma_tx_scatter].hsdma_txd_info3.SDP1;
		}

		i++;			
	}	
	hsdma_rx_dma_owner_idx0 = (hsdma_rx_calc_idx0 + 1) % NUM_HSDMA_RX_DESC;
  //RX descriptor
	if(LastDescLSO0 == 1 && LastDescLSO1 == 0){
		DescRx = DescNum*2-1;
	}
	else{
		DescRx = DescNum*2;
	}
	while(j<DescRx)
	{
		hsdma_rx_gather  = (hsdma_rx_dma_owner_idx0 + j) % NUM_HSDMA_RX_DESC;
		if (j==0){
			HSDMA_Entry.HSDMA_rx_ring0[hsdma_rx_gather].hsdma_rxd_info1.PDP0 = ((Dst0) & 0xFFFFFFFF);
		}
	else{
	  	HSDMA_Entry.HSDMA_rx_ring0[hsdma_rx_gather].hsdma_rxd_info1.PDP0 = ((HSDMA_PDP0_Addr+HSDMA_PLEN0_Len) & 0xFFFFFFFF);	
	}
	if (j < (DescNum*2-1))
	{	
		HSDMA_Entry.HSDMA_rx_ring0[hsdma_rx_gather].hsdma_rxd_info2.PLEN0 = 16000;  //RX0 Length,
	}
	else if (j==(DescNum*2-1)){
		HSDMA_Entry.HSDMA_rx_ring0[hsdma_rx_gather].hsdma_rxd_info2.PLEN0 = TransCount - (DescNum*2-1)*16000;
	} 
	HSDMA_PDP0_Addr = HSDMA_Entry.HSDMA_rx_ring0[hsdma_rx_gather].hsdma_rxd_info1.PDP0;
	HSDMA_PLEN0_Len = HSDMA_Entry.HSDMA_rx_ring0[hsdma_rx_gather].hsdma_rxd_info2.PLEN0;
	j++;
	}	  
  
	hsdma_tx_cpu_idx = hsdma_tx_cpu_owner_idx0;
	hsdma_tx_cpu_owner_idx0 = (hsdma_tx_cpu_owner_idx0 + DescNum) % NUM_HSDMA_TX_DESC;
	hsdma_rx_calc_idx0 = (hsdma_rx_calc_idx0 + DescRx) % NUM_HSDMA_RX_DESC;
	if(HSDMA_Entry.DoneIntCallback!=NULL) {
		HSdmaDoneIntCallback[hsdma_tx_cpu_owner_idx0] = HSDMA_Entry.DoneIntCallback;
	}

	if (DescNum == NUM_HSDMA_TX_DESC){
		sysRegWrite(HSDMA_TX_CTX_IDX0, cpu_to_le32((u32)(1)));
	}
	sysRegWrite(HSDMA_TX_CTX_IDX0, cpu_to_le32((u32)hsdma_tx_cpu_owner_idx0));//tx start to move, 1->2->255->0->1

	if(HSDMA_Entry.DoneIntCallback==NULL) {
    //printk("hsdma_rx_calc_idx0 = %d\n",hsdma_rx_calc_idx0 ); 
		while(HSDMA_Entry.HSDMA_rx_ring0[hsdma_rx_calc_idx0].hsdma_rxd_info2.DDONE_bit ==0);
		
		
	}
	//spin_unlock_irqrestore(&(HSDMA_Entry.page_lock_mem), flags);
	return 0;
}
	struct timeval begin, end;
int HS_DmaMem2Mem( 
	uint32_t Src, 
	uint32_t Dst, 
	uint16_t TransCount,
	int (*DoneIntCallback)(uint32_t transcount, uint32_t data)
	)
{
	unsigned long flags;
	spin_lock_irqsave(&(HSDMA_Entry.page_lock), flags);
	HSDMA_Entry.DoneIntCallback = DoneIntCallback;
  hsdma_tx_cpu_owner_idx0 = sysRegRead(HSDMA_TX_CTX_IDX0);
  //while(HSDMA_Entry.HSDMA_tx_ring0[hsdma_tx_cpu_owner_idx0].hsdma_txd_info2.DDONE_bit  ==0);
 	//while(HSDMA_Entry.HSDMA_rx_ring0[hsdma_rx_dma_owner_idx0].hsdma_rxd_info2.DDONE_bit ==1);
  
	hsdma_rx_dma_owner_idx0 = (hsdma_rx_calc_idx0 + 1) % NUM_HSDMA_RX_DESC;
	HSDMA_Entry.HSDMA_tx_ring0[hsdma_tx_cpu_owner_idx0].hsdma_txd_info1.SDP0 = (Src & 0xFFFFFFFF);
	HSDMA_Entry.HSDMA_rx_ring0[hsdma_rx_dma_owner_idx0].hsdma_rxd_info1.PDP0 = (Dst & 0xFFFFFFFF);
  
	HSDMA_Entry.HSDMA_tx_ring0[hsdma_tx_cpu_owner_idx0].hsdma_txd_info2.SDL0 = TransCount;
	HSDMA_Entry.HSDMA_rx_ring0[hsdma_rx_dma_owner_idx0].hsdma_rxd_info2.PLEN0 = TransCount;
  
	HSDMA_Entry.HSDMA_tx_ring0[hsdma_tx_cpu_owner_idx0].hsdma_txd_info2.LS0_bit = 1;
	HSDMA_Entry.HSDMA_tx_ring0[hsdma_tx_cpu_owner_idx0].hsdma_txd_info2.DDONE_bit = 0;
	HSDMA_Entry.HSDMA_rx_ring0[hsdma_rx_dma_owner_idx0].hsdma_rxd_info2.DDONE_bit = 0;

	hsdma_tx_cpu_idx = hsdma_tx_cpu_owner_idx0;
	hsdma_tx_cpu_owner_idx0 = (hsdma_tx_cpu_owner_idx0+1) % NUM_HSDMA_TX_DESC;
	hsdma_rx_calc_idx0 = (hsdma_rx_calc_idx0 + 1) % NUM_HSDMA_RX_DESC;


	sysRegWrite(HSDMA_TX_CTX_IDX0, cpu_to_le32((u32)hsdma_tx_cpu_owner_idx0));//tx start to move, 1->2->255->0->1
  	if(HSDMA_Entry.DoneIntCallback!=NULL) {
	   HSdmaDoneIntCallback[hsdma_rx_dma_owner_idx0%ADDRESS_VLAUE] = HSDMA_Entry.DoneIntCallback;
	}
	
	if(HSDMA_Entry.DoneIntCallback==NULL) {
 
	while(HSDMA_Entry.HSDMA_rx_ring0[hsdma_rx_calc_idx0].hsdma_rxd_info2.DDONE_bit ==0);
	
	sysRegWrite(HSDMA_RX_CALC_IDX0, cpu_to_le32((u32)hsdma_rx_calc_idx0)); //update RX CPU IDX
	sysRegWrite(HSDMA_INT_STATUS, HSDMA_FE_INT_ALL);  //Write one clear INT_status	
	spin_unlock_irqrestore(&(HSDMA_Entry.page_lock), flags);

	}	
  

	return 0;
}
/*
int HS_DmaMem2MemScatterGather( 
	uint32_t Src0,
	uint32_t Dst0,
	uint32_t TransCount,
	uint16_t DescNum,
	uint16_t LastDescLSO0,
	uint16_t LastDescLSO1,
*/
int HS_DmaMem2MemGenUse( 
	uint32_t Src0,
	uint32_t Dst0,
	uint32_t TransCount,
	int (*DoneIntCallback)(uint32_t transcount, uint32_t data)
	)
{   
	int DesNum;

	if (TransCount <= 16383){                                                // one descriptor
		return HS_DmaMem2Mem(Src0, Dst0, TransCount, DoneIntCallback);
	}
	
	DesNum =(TransCount/(16000*2));                                            //TX need descriptor
	
	if (TransCount >16000 && TransCount <32000){
		
		return HS_DmaMem2MemScatterGather(Src0, Dst0, TransCount, 1 ,0 , 1, DoneIntCallback);
	}
	
	if ((DesNum*16000*2)==TransCount ){                                      //full descriptor use Sd1

		return HS_DmaMem2MemScatterGather(Src0, Dst0, TransCount, DesNum ,0 , 1, DoneIntCallback);
	}	
	if ((DesNum*16000*2 < TransCount)&& ((DesNum+1)*16000*2-16000) < TransCount){ //not full descriptor use Sd0 Sd1

		return HS_DmaMem2MemScatterGather(Src0, Dst0, TransCount, (DesNum+1) ,0 , 1, DoneIntCallback);
	}	
	if ((DesNum*16000*2 < TransCount)&& ((DesNum+1)*16000*2-16000) > TransCount){ //not full descriptor use Sd0 
		
		return HS_DmaMem2MemScatterGather(Src0, Dst0, TransCount, (DesNum+1) ,1 , 0, DoneIntCallback);
	}	
	printk("something error\n");
	return -1;
}
  unsigned long value;
irqreturn_t HSdmaIrqHandler(
	int irq, 
	void *irqaction
	)
{
	unsigned long flags;
	int i;
	spin_lock_irqsave(&(HSDMA_Entry.page_lock), flags);

if(HSdmaDoneIntCallback[updateCRX%ADDRESS_VLAUE]!= NULL) 
{	
	sysRegWrite(HSDMA_INT_STATUS, HSDMA_FE_INT_ALL);  //Write one clear INT_status	
	for (i=0;i<NUM_HSDMA_RX_DESC;i++){
		if (HSDMA_Entry.HSDMA_rx_ring0[i].hsdma_rxd_info2.DDONE_bit == 1) { 
			  updateCRX = i; 

			  HSDMA_Entry.HSDMA_rx_ring0[updateCRX].hsdma_rxd_info2.DDONE_bit = 0; // RX_Done_bit=1->0
			  TransCountLength=HSDMA_Entry.HSDMA_rx_ring0[updateCRX].hsdma_rxd_info2.PLEN0;
			  sysRegWrite(HSDMA_RX_CALC_IDX0, updateCRX); //update RX CPU IDX
				HSdmaDoneIntCallback[updateCRX%ADDRESS_VLAUE](TransCountLength, (updateCRX%ADDRESS_VLAUE)); 
		}
	}
}	
else{
	for (i=0;i<NUM_HSDMA_RX_DESC;i++){
		if (HSDMA_Entry.HSDMA_rx_ring0[i].hsdma_rxd_info2.DDONE_bit == 1) { 
			  updateCRX = i; 
				HSDMA_Entry.HSDMA_rx_ring0[i].hsdma_rxd_info2.DDONE_bit = 0; // RX_Done_bit=1->0
		}	
	}
	sysRegWrite(HSDMA_RX_CALC_IDX0, cpu_to_le32((u32)updateCRX)); //update RX CPU IDX
	sysRegWrite(HSDMA_INT_STATUS, HSDMA_FE_INT_ALL);  //Write one clear INT_status	
}


	
	
	
	spin_unlock_irqrestore(&(HSDMA_Entry.page_lock), flags);
	return IRQ_HANDLED;	
}
void set_HSDMA_glo_cfg(void)
{
	int HSDMA_glo_cfg=0;
	HSDMA_glo_cfg = (HSDMA_TX_WB_DDONE | HSDMA_RX_DMA_EN | HSDMA_TX_DMA_EN | HSDMA_BT_SIZE_16DWORDS | HSDMA_MUTI_ISSUE);
	sysRegWrite(HSDMA_GLO_CFG, HSDMA_glo_cfg);
}

static int HSDMA_init(void)
{
	int		i;
	unsigned int	regVal;
	printk("HSDMA_init function \n");
	while(1)
	{
		regVal = sysRegRead(HSDMA_GLO_CFG);
		if((regVal & HSDMA_RX_DMA_BUSY))
		{
			printk("\n  RX_DMA_BUSY !!! ");
			continue;
		}
		if((regVal & HSDMA_TX_DMA_BUSY))
		{
			printk("\n  TX_DMA_BUSY !!! ");
			continue;
		}
		break;
	}
	//initial TX ring0
	HSDMA_Entry.HSDMA_tx_ring0 = pci_alloc_consistent(NULL, NUM_HSDMA_TX_DESC * sizeof(struct HSDMA_txdesc), &HSDMA_Entry.phy_hsdma_tx_ring0);
	printk("\nphy_tx_ring0 = 0x%08x, tx_ring0 = 0x%p\n", HSDMA_Entry.phy_hsdma_tx_ring0, HSDMA_Entry.HSDMA_tx_ring0);
	
		
	for (i=0; i < NUM_HSDMA_TX_DESC; i++) {
		memset(&HSDMA_Entry.HSDMA_tx_ring0[i],0,sizeof(struct HSDMA_txdesc));
		HSDMA_Entry.HSDMA_tx_ring0[i].hsdma_txd_info2.LS0_bit = 1;
		HSDMA_Entry.HSDMA_tx_ring0[i].hsdma_txd_info2.DDONE_bit = 1;
	}

	//initial RX ring0
	HSDMA_Entry.HSDMA_rx_ring0 = pci_alloc_consistent(NULL, NUM_HSDMA_RX_DESC * sizeof(struct HSDMA_rxdesc), &HSDMA_Entry.phy_hsdma_rx_ring0);
	
	
	for (i = 0; i < NUM_HSDMA_RX_DESC; i++) {
		memset(&HSDMA_Entry.HSDMA_rx_ring0[i],0,sizeof(struct HSDMA_rxdesc));
		HSDMA_Entry.HSDMA_rx_ring0[i].hsdma_rxd_info2.DDONE_bit = 0;
		HSDMA_Entry.HSDMA_rx_ring0[i].hsdma_rxd_info2.LS0 = 0;
	}	
	printk("\nphy_rx_ring0 = 0x%08x, rx_ring0 = 0x%p\n",HSDMA_Entry.phy_hsdma_rx_ring0,HSDMA_Entry.HSDMA_rx_ring0);
	  
	// HSDMA_GLO_CFG
	regVal = sysRegRead(HSDMA_GLO_CFG);
	regVal &= 0x000000FF;
	sysRegWrite(HSDMA_GLO_CFG, regVal);
	regVal=sysRegRead(HSDMA_GLO_CFG);
	/* Tell the adapter where the TX/RX rings are located. */
	//TX0
	//sysRegWrite(HSDMA_TX_BASE_PTR0, phys_to_bus((u32) HSDMA_Entry.phy_hsdma_tx_ring0));
	sysRegWrite(HSDMA_TX_BASE_PTR0, HSDMA_Entry.phy_hsdma_tx_ring0);
	sysRegWrite(HSDMA_TX_MAX_CNT0, cpu_to_le32((u32) NUM_HSDMA_TX_DESC));
	sysRegWrite(HSDMA_TX_CTX_IDX0, 0);
	hsdma_tx_cpu_owner_idx0 = 0;
	sysRegWrite(HSDMA_RST_CFG, HSDMA_PST_DTX_IDX0);
	printk("TX_CTX_IDX0 = %x\n", sysRegRead(HSDMA_TX_CTX_IDX0));
	printk("TX_DTX_IDX0 = %x\n", sysRegRead(HSDMA_TX_DTX_IDX0));

	    
	//RX0
	//sysRegWrite(HSDMA_RX_BASE_PTR0, phys_to_bus((u32) HSDMA_Entry.phy_hsdma_rx_ring0));
	sysRegWrite(HSDMA_RX_BASE_PTR0, HSDMA_Entry.phy_hsdma_rx_ring0);
	sysRegWrite(HSDMA_RX_MAX_CNT0,  cpu_to_le32((u32) NUM_HSDMA_RX_DESC));
	sysRegWrite(HSDMA_RX_CALC_IDX0, cpu_to_le32((u32) (NUM_HSDMA_RX_DESC - 1)));
	hsdma_rx_calc_idx0 = hsdma_rx_dma_owner_idx0 =  sysRegRead(HSDMA_RX_CALC_IDX0);
	sysRegWrite(HSDMA_RST_CFG, HSDMA_PST_DRX_IDX0);
	printk("RX_CRX_IDX0 = %x\n", sysRegRead(HSDMA_RX_CALC_IDX0));
	printk("RX_DRX_IDX0 = %x\n", sysRegRead(HSDMA_RX_DRX_IDX0));

	set_HSDMA_glo_cfg();
	printk("HSDMA_GLO_CFG = %x\n", sysRegRead(HSDMA_GLO_CFG));
	return 1;
}

static int RalinkHSdmaInit(void)
{
	uint32_t Ret=0;
	unsigned long reg_int_mask=0;
	printk("Enable Ralink HS_DMA Controller Module \n");

	Ret = request_irq(SURFBOARDINT_HSGDMA, HSdmaIrqHandler, \
	    IRQF_TRIGGER_LOW, "HS_DMA", NULL);

	if(Ret){
		printk("IRQ %d is not free.\n", SURFBOARDINT_DMA);
	return 1;
	}
	spin_lock_init(&(HSDMA_Entry.page_lock));
	spin_lock_init(&(HSDMA_Entry.page_lock_mem));
	sysRegWrite(HSDMA_INT_MASK, reg_int_mask  & ~(HSDMA_FE_INT_TX));  // disable int TX DONE
	sysRegWrite(HSDMA_INT_MASK, reg_int_mask  & ~(HSDMA_FE_INT_RX));  // disable int RX DONE
		
	spin_lock_init(&(HSDMA_Entry.page_lock));
	printk("reg_int_mask=%lu, INT_MASK= %x \n", reg_int_mask, sysRegRead(HSDMA_INT_MASK));
  HSDMA_init();	
  return Ret;
}

void hsdmastop(void)
{
	unsigned int regValue;

	regValue = sysRegRead(HSDMA_GLO_CFG);
	regValue &= ~(HSDMA_TX_WB_DDONE | HSDMA_RX_DMA_EN | HSDMA_TX_DMA_EN  );
	sysRegWrite(HSDMA_GLO_CFG, regValue);
	printk("Done\n");	
}

static void __exit RalinkHSdmaExit(void)
{
	unsigned long reg_int_mask=0;
	unsigned long flags;
	spin_lock_irqsave(&(HSDMA_Entry.page_lock), flags);

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
	cancel_work_sync(&HSDMA_Entry.reset_task);
#endif
	hsdmastop();
	msleep(10);

	if (HSDMA_Entry.HSDMA_tx_ring0 != NULL) {
		pci_free_consistent(NULL, NUM_HSDMA_TX_DESC*sizeof(struct HSDMA_txdesc), HSDMA_Entry.HSDMA_tx_ring0, HSDMA_Entry.phy_hsdma_tx_ring0);
	}	
													
	pci_free_consistent(NULL, NUM_HSDMA_RX_DESC*sizeof(struct HSDMA_rxdesc), HSDMA_Entry.HSDMA_rx_ring0, HSDMA_Entry.phy_hsdma_rx_ring0);
	printk("Free TX/RX Ring Memory!\n");
	printk("Disable HSDMA Controller Module\n");
	//Disable HS_DMA interrupt
	reg_int_mask=sysRegRead(HSDMA_INT_MASK);
	sysRegWrite(HSDMA_INT_MASK,reg_int_mask & ~(HSDMA_FE_INT_ALL) );
	free_irq(SURFBOARDINT_HSGDMA, NULL);
	spin_unlock_irqrestore(&(HSDMA_Entry.page_lock), flags);

}


module_init(RalinkHSdmaInit);
module_exit(RalinkHSdmaExit);

EXPORT_SYMBOL(HS_DmaMem2Mem);
EXPORT_SYMBOL(HS_DmaMem2MemGenUse);
EXPORT_SYMBOL(HS_DmaMem2MemScatterGather);
MODULE_DESCRIPTION("Ralink SoC HS_GDMA Controller API Module");
MODULE_AUTHOR("Harry");
MODULE_LICENSE("GPL");
MODULE_VERSION(MOD_VERSION_HS_DMA);

