#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/pci.h>
#include <linux/dma-mapping.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/memory.h>
#include <asm/rt2880/surfboardint.h>
#include <linux/version.h>

#include "rt_hsdma.h"
#include "rt_dma.h"


/************************ DMA engine API functions ****************************/

#define MEMCPY_DMA_CH	8
#define to_rt_dma_chan(chan)            \
	container_of(chan, struct rt_dma_chan, common)
#ifdef CONFIG_RT_DMA_HSDMA	
static int hsdma_rx_dma_owner_idx0;
static int hsdma_rx_calc_idx0;
static unsigned long hsdma_tx_cpu_owner_idx0=0;
static unsigned long updateCRX =0;
#endif
static dma_cookie_t rt_dma_tx_submit(struct dma_async_tx_descriptor *tx)
{
	dma_cookie_t cookie;
	
	//printk("%s\n",__FUNCTION__);
  if(tx->chan)
	cookie = tx->chan->cookie;

	return cookie;
}

struct HSdmaReqEntry HSDMA_Entry;

#define MIN_RTDMA_PKT_LEN	128
static struct dma_async_tx_descriptor *
rt_dma_prep_dma_memcpy(struct dma_chan *chan, dma_addr_t dest, dma_addr_t src,
		size_t len, unsigned long flags)
{
	struct rt_dma_chan *rt_chan = to_rt_dma_chan(chan);
	unsigned long mid_offset;
#ifdef CONFIG_RT_DMA_HSDMA
	unsigned long i;
#endif
 
	//printk("%x->%x len=%d ch=%d\n", src, dest, len, chan->chan_id);
	spin_lock_bh(&rt_chan->lock);

#ifdef CONFIG_RT_DMA_HSDMA
  if ((dest & 0x03)!=0){
  	memcpy(phys_to_virt(dest), phys_to_virt(src), len);	
	dma_async_tx_descriptor_init(&rt_chan->txd, chan);
  }
  else{
	hsdma_rx_dma_owner_idx0 = (hsdma_rx_calc_idx0 + 1) % NUM_HSDMA_RX_DESC;
	HSDMA_Entry.HSDMA_tx_ring0[hsdma_tx_cpu_owner_idx0].hsdma_txd_info1.SDP0 = (src & 0xFFFFFFFF);
	HSDMA_Entry.HSDMA_rx_ring0[hsdma_rx_dma_owner_idx0].hsdma_rxd_info1.PDP0 = (dest & 0xFFFFFFFF);
  
	HSDMA_Entry.HSDMA_tx_ring0[hsdma_tx_cpu_owner_idx0].hsdma_txd_info2.SDL0 = len;
	HSDMA_Entry.HSDMA_rx_ring0[hsdma_rx_dma_owner_idx0].hsdma_rxd_info2.PLEN0 = len;
	  
	HSDMA_Entry.HSDMA_tx_ring0[hsdma_tx_cpu_owner_idx0].hsdma_txd_info2.LS0_bit = 1;
	HSDMA_Entry.HSDMA_tx_ring0[hsdma_tx_cpu_owner_idx0].hsdma_txd_info2.DDONE_bit = 0;
		
	hsdma_tx_cpu_owner_idx0 = (hsdma_tx_cpu_owner_idx0+1) % NUM_HSDMA_TX_DESC;
	hsdma_rx_calc_idx0 = (hsdma_rx_calc_idx0 + 1) % NUM_HSDMA_RX_DESC;
	sysRegWrite(HSDMA_TX_CTX_IDX0, cpu_to_le32((u32)hsdma_tx_cpu_owner_idx0));
			
	dma_async_tx_descriptor_init(&rt_chan->txd, chan);
	}
#else
		mid_offset = len/2;
		RT_DMA_WRITE_REG(RT_DMA_SRC_REG(MEMCPY_DMA_CH), src);
		RT_DMA_WRITE_REG(RT_DMA_DST_REG(MEMCPY_DMA_CH), dest);
		RT_DMA_WRITE_REG(RT_DMA_CTRL_REG(MEMCPY_DMA_CH), (mid_offset << 16) | (3 << 3) | (3 << 0));

		memcpy(phys_to_virt(dest)+mid_offset, phys_to_virt(src)+mid_offset, len-mid_offset);	
		
		dma_async_tx_descriptor_init(&rt_chan->txd, chan);
		
		while((RT_DMA_READ_REG(RT_DMA_DONEINT) & (0x1<<MEMCPY_DMA_CH))==0);
		RT_DMA_WRITE_REG(RT_DMA_DONEINT, (1<<MEMCPY_DMA_CH));
#endif

	spin_unlock_bh(&rt_chan->lock);

	return &rt_chan->txd;
}
#ifdef CONFIG_RT_DMA_HSDMA	
void set_fe_HSDMA_glo_cfg(void)
{
	int HSDMA_glo_cfg=0;
	printk("%s\n",__FUNCTION__);
	HSDMA_glo_cfg = (HSDMA_TX_WB_DDONE | HSDMA_RX_DMA_EN | HSDMA_TX_DMA_EN | HSDMA_BT_SIZE_16DWORDS | HSDMA_MUTI_ISSUE );
	sysRegWrite(HSDMA_GLO_CFG, HSDMA_glo_cfg);
}

static int HSDMA_init(void)
{
	int		i;
	unsigned int	regVal;
	printk("%s\n",__FUNCTION__);
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
	printk("\n hsdma_phy_tx_ring0 = 0x%08x, hsdma_tx_ring0 = 0x%p\n", HSDMA_Entry.phy_hsdma_tx_ring0, HSDMA_Entry.HSDMA_tx_ring0);
	
		
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
		printk("\n hsdma_phy_rx_ring0 = 0x%08x, hsdma_rx_ring0 = 0x%p\n",HSDMA_Entry.phy_hsdma_rx_ring0,HSDMA_Entry.HSDMA_rx_ring0);
	
		// HSDMA_GLO_CFG
		regVal = sysRegRead(HSDMA_GLO_CFG);
		regVal &= 0x000000FF;
		sysRegWrite(HSDMA_GLO_CFG, regVal);
		regVal=sysRegRead(HSDMA_GLO_CFG);
		/* Tell the adapter where the TX/RX rings are located. */
		//TX0
    sysRegWrite(HSDMA_TX_BASE_PTR0, phys_to_bus((u32) HSDMA_Entry.phy_hsdma_tx_ring0));
		sysRegWrite(HSDMA_TX_MAX_CNT0, cpu_to_le32((u32) NUM_HSDMA_TX_DESC));
		sysRegWrite(HSDMA_TX_CTX_IDX0, 0);
		hsdma_tx_cpu_owner_idx0 = 0;
		sysRegWrite(HSDMA_RST_CFG, HSDMA_PST_DTX_IDX0);
		printk("TX_CTX_IDX0 = %x\n", sysRegRead(HSDMA_TX_CTX_IDX0));
	  printk("TX_DTX_IDX0 = %x\n", sysRegRead(HSDMA_TX_DTX_IDX0));

	    
		//RX0
		sysRegWrite(HSDMA_RX_BASE_PTR0, phys_to_bus((u32) HSDMA_Entry.phy_hsdma_rx_ring0));
		sysRegWrite(HSDMA_RX_MAX_CNT0,  cpu_to_le32((u32) NUM_HSDMA_RX_DESC));
		sysRegWrite(HSDMA_RX_CALC_IDX0, cpu_to_le32((u32) (NUM_HSDMA_RX_DESC - 1)));
		hsdma_rx_calc_idx0 = hsdma_rx_dma_owner_idx0 =  sysRegRead(HSDMA_RX_CALC_IDX0);
		sysRegWrite(HSDMA_RST_CFG, HSDMA_PST_DRX_IDX0);
		printk("RX_CRX_IDX0 = %x\n", sysRegRead(HSDMA_RX_CALC_IDX0));
		printk("RX_DRX_IDX0 = %x\n", sysRegRead(HSDMA_RX_DRX_IDX0));

		set_fe_HSDMA_glo_cfg();
		printk("HSDMA_GLO_CFG = %x\n", sysRegRead(HSDMA_GLO_CFG));
		return 1;
}
#endif

int hsdma_housekeeping(void)
{
	int i;

	i = (sysRegRead(HSDMA_RX_CALC_IDX0)+1)%NUM_HSDMA_RX_DESC;	   

	while (1) {
		if (HSDMA_Entry.HSDMA_rx_ring0[i].hsdma_rxd_info2.DDONE_bit == 1) { 
			HSDMA_Entry.HSDMA_rx_ring0[i].hsdma_rxd_info2.DDONE_bit = 0; // RX_Done_bit=1->0
			updateCRX=i;
			i = (i + 1)%NUM_HSDMA_RX_DESC;
		}	
		else {
			break;
		}
	} 
	sysRegWrite(HSDMA_RX_CALC_IDX0, cpu_to_le32((u32)updateCRX)); //update RX CPU IDX 

	return 0;
}

/**
 * rt_dma_status - poll the status of an XOR transaction
 * @chan: XOR channel handle
 * @cookie: XOR transaction identifier
 * @txstate: XOR transactions state holder (or NULL)
 */
static enum dma_status rt_dma_status(struct dma_chan *chan,
					  dma_cookie_t cookie,
					  struct dma_tx_state *txstate)
{
	//printk("%s\n",__FUNCTION__);
	hsdma_housekeeping();

	return 0;
}


static irqreturn_t rt_dma_interrupt_handler(int irq, void *data)
{
	//printk("%s\n",__FUNCTION__);

	return IRQ_HANDLED;
}

static void rt_dma_issue_pending(struct dma_chan *chan)
{
	//printk("%s\n",__FUNCTION__);
}


static int rt_dma_alloc_chan_resources(struct dma_chan *chan)
{
	//("%s\n",__FUNCTION__);

	return 0;
}

static void rt_dma_free_chan_resources(struct dma_chan *chan)
{
	//printk("%s\n",__FUNCTION__);

}

#if LINUX_VERSION_CODE > KERNEL_VERSION(3,10,0)
static int rt_dma_probe(struct platform_device *pdev)
#else
static int __devinit rt_dma_probe(struct platform_device *pdev)
#endif
{
	struct dma_device *dma_dev;
	struct rt_dma_chan *rt_chan;
	int err;
	int ret;
#ifdef CONFIG_RT_DMA_HSDMA
	unsigned long reg_int_mask=0;
#else
	int reg;
#endif

	//printk("%s\n",__FUNCTION__);
	
	dma_dev = devm_kzalloc(&pdev->dev, sizeof(*dma_dev), GFP_KERNEL);
	if (!dma_dev)
		return -ENOMEM;


	INIT_LIST_HEAD(&dma_dev->channels);
	dma_cap_zero(dma_dev->cap_mask);
	dma_cap_set(DMA_MEMCPY, dma_dev->cap_mask);
	//dma_cap_set(DMA_SLAVE, dma_dev->cap_mask);
	dma_dev->device_alloc_chan_resources = rt_dma_alloc_chan_resources;
	dma_dev->device_free_chan_resources = rt_dma_free_chan_resources;
	dma_dev->device_tx_status = rt_dma_status;
	dma_dev->device_issue_pending = rt_dma_issue_pending;
	dma_dev->device_prep_dma_memcpy = rt_dma_prep_dma_memcpy;
	dma_dev->dev = &pdev->dev;

	rt_chan = devm_kzalloc(&pdev->dev, sizeof(*rt_chan), GFP_KERNEL);
        if (!rt_chan) {
		return -ENOMEM;
	}

	spin_lock_init(&rt_chan->lock);	
  INIT_LIST_HEAD(&rt_chan->chain);
	INIT_LIST_HEAD(&rt_chan->completed_slots);
	INIT_LIST_HEAD(&rt_chan->all_slots);
	rt_chan->common.device = dma_dev;
	rt_chan->txd.tx_submit = rt_dma_tx_submit;

	list_add_tail(&rt_chan->common.device_node, &dma_dev->channels);

	err = dma_async_device_register(dma_dev);

	if (0 != err) {
		pr_err("ERR_MDMA:device_register failed: %d\n", err);
		return 1;
	}
	
#ifdef CONFIG_RT_DMA_HSDMA
	ret = request_irq(SURFBOARDINT_HSGDMA, rt_dma_interrupt_handler, IRQF_DISABLED, "HS_DMA", NULL);
#else
	ret = request_irq(SURFBOARDINT_DMA, rt_dma_interrupt_handler, IRQF_DISABLED, "GDMA", NULL);
#endif
	if(ret){
		pr_err("IRQ %d is not free.\n", SURFBOARDINT_DMA);
		return 1;
	}

#ifdef CONFIG_RT_DMA_HSDMA
		sysRegWrite(HSDMA_INT_MASK, reg_int_mask  & ~(HSDMA_FE_INT_TX));  // disable int TX DONE
		sysRegWrite(HSDMA_INT_MASK, reg_int_mask  & ~(HSDMA_FE_INT_RX) );  // disable int RX DONE		
		printk("reg_int_mask=%lu, INT_MASK= %x \n", reg_int_mask, sysRegRead(HSDMA_INT_MASK));
  	HSDMA_init();	
#else
	//set GDMA register in advance.
	reg = (32 << 16) | (32 << 8) | (MEMCPY_DMA_CH << 3);
	RT_DMA_WRITE_REG(RT_DMA_CTRL_REG1(MEMCPY_DMA_CH), reg);
#endif

	return 0;
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(3,10,0)
static int  rt_dma_remove(struct platform_device *dev)
#else
static int __devexit rt_dma_remove(struct platform_device *dev)
#endif
{
	struct dma_device *dma_dev = platform_get_drvdata(dev);

	//printk("%s\n",__FUNCTION__);

	dma_async_device_unregister(dma_dev);

	return 0;
}

static struct platform_driver rt_dma_driver = {
	.probe		= rt_dma_probe,
#if LINUX_VERSION_CODE > KERNEL_VERSION(3,10,0)
	.remove		= rt_dma_remove,
#else
        .remove         = __devexit_p(rt_dma_remove),
#endif
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= RT_DMA_NAME,
	},
};

static int __init rt_dma_init(void)
{
	int rc;
 // printk("%s\n",__FUNCTION__);
	rc = platform_driver_register(&rt_dma_driver);
	return rc;
}
module_init(rt_dma_init);


MODULE_AUTHOR("Steven Liu <steven_liu@mediatek.com>");
MODULE_DESCRIPTION("DMA engine driver for Ralink DMA engine");
MODULE_LICENSE("GPL");
