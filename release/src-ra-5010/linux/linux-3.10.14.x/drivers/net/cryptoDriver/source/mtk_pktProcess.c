#include <linux/init.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/netdevice.h>
#include <linux/sched.h>

#include <linux/delay.h>
#include <linux/string.h>

#include <net/ip.h>
#include <asm/io.h>
#include <asm/mach-ralink/rt_mmap.h>
#include "mtk_cAdapter.h"
#include "mtk_baseDefs.h"         // uint32_t
#include "mtk_pecApi.h"            // PEC_* (the API we implement here)
#include "mtk_dmaBuf.h"         // DMABuf_*
#include "mtk_hwDmaAccess.h"      // HWPAL_Resource_*
#include "mtk_cLib.h"               // memcpy
#include "mtk_AdapterInternal.h"
#include "mtk_interrupts.h"
#include "mtk_arm.h"        // driver library API we will use
#include "mtk_descp.h" // for parsing result descriptor

#include <net/mtk_esp.h>
#include "mtk_ipsec.h"
#include <linux/skbuff.h>


extern unsigned int *pCmdRingBase, *pResRingBase; //uncached memory address
extern void mtk_interruptHandler_descriptorDone(void);

#define EIP93_RING_SIZE		((ADAPTER_EIP93_RINGSIZE_BYTES)>>5)
#define EIP93_REG_BASE		(RALINK_CRYPTO_ENGINE_BASE)
#define PE_CTRL_STAT			0x00000000
#define PE_CD_COUNT			0x00000090
#define	PE_RD_COUNT			0x00000094
#define PE_RING_PNTR		0x00000098
#define PE_CONFIG			0x00000100
#define PE_DMA_CONFIG			0x00000120
#define PE_ENDIAN_CONFIG	0x000001d0
//#define dma_cache_wback_inv(start, size) _dma_cache_wback_inv(start,size)
#define K1_TO_PHY(x)		(((unsigned int)x) & 0x1fffffff)
#define WORDSWAP(a)     	((((a)>>24)&0xff) | (((a)>>8)&0xff00) | (((a)<<8)&0xff0000) | (((a)<<24)&0xff000000))
#define DESCP_SIZE			32
#define EIP93_RING_BUFFER	24

static unsigned int cmdRingIdx, resRingIdx, cmdRingFrontIdx,resPrepRingIdx;
static uint32_t *pEip93RegBase = (uint32_t *)EIP93_REG_BASE;

//static spinlock_t 			putlock, getlock;

#ifdef WORKQUEUE_BH
static DECLARE_WORK(mtk_interrupt_BH_result_wq, mtk_BH_handler_resultGet);
#else
static DECLARE_TASKLET( \
mtk_interrupt_BH_result_tsk, mtk_BH_handler_resultGet, 0);
#endif
static void mtk_interruptHandler_done(void);

#ifdef CONFIG_L2TP
#define BUFFER_MEMCPY	1
#define SKB_HEAD_SHIFT	1
#endif

#ifdef MCRYPTO_DBG
#define ra_dbg 	printk
#else
#define ra_dbg(fmt, arg...) do {}while(0)
#endif

static int pskb_alloc_head(struct sk_buff *skb, u8* data, u32 size, int offset);
int copy_data_head(struct sk_buff *skb, int offset);
int copy_data_bottom(struct sk_buff *skb, int offset);
static int DMAAlign = 0;

#ifdef MCRYPTO_DBG
static void skb_dump(struct sk_buff* sk, const char* func,int line) {
        unsigned int i;

        ra_dbg("(%d)skb_dump: [%s] with len %d (%08X) headroom=%d tailroom=%d\n",
                line,func,sk->len,sk,
                skb_headroom(sk),skb_tailroom(sk));

        for(i=(unsigned int)sk->head;i<=(unsigned int)sk->data + sk->len;i++) {
                if((i % 16) == 0)
                        ra_dbg("\n");
                if(i==(unsigned int)sk->data) printk("{");
                //if(i==(unsigned int)sk->h.raw) printk("#");
                //if(i==(unsigned int)sk->nh.raw) printk("|");
                //if(i==(unsigned int)sk->mac.raw) printk("*");
                ra_dbg("%02x ",*((unsigned char*)i));
                if(i==(unsigned int)(sk->tail)-1) printk("}");
        }
        ra_dbg("\n");
}
#else
#define skb_dump(x,y,z) do {}while(0)
#endif
#if defined (CONFIG_HWCRYPTO_MEMPOOL)
/************************************************************************
*              P R I V A T E     F U N C T I O N S
*************************************************************************
*/
static void 
mtk_cmdHandler_free(
	eip93DescpHandler_t *cmdHandler
)
{
	saRecord_t *saRecord;
	saState_t *saState;
	dma_addr_t	saPhyAddr, statePhyAddr;
	
	saRecord = (saRecord_t *)cmdHandler->saAddr.addr;
	saPhyAddr = (dma_addr_t)cmdHandler->saAddr.phyAddr;
	saState = (saState_t *)cmdHandler->stateAddr.addr;
	statePhyAddr = (dma_addr_t)cmdHandler->stateAddr.phyAddr;	
	
	dma_free_coherent(NULL, sizeof(saRecord_t), saRecord, saPhyAddr);
	dma_free_coherent(NULL, sizeof(saState_t), saState, statePhyAddr);
	kfree(cmdHandler);
}

/*_______________________________________________________________________
**function name: ipsec_addrsDigestPreCompute_free
**
**description:
*   free those structions that are created for Hash Digest Pre-Compute!
*	Those sturctures won't be used anymore during encryption/decryption!
**parameters:
*   currAdapterPtr -- point to the structure that stores the addresses
*		for those structures for Hash Digest Pre-Compute.
**global:
*   none
**return:
*   none
**call:
*   none
**revision:
*   1.Trey 20120209
**_______________________________________________________________________*/
static void 
mtk_addrsDigestPreCompute_free(
	ipsecEip93Adapter_t *currAdapterPtr
)
{
	unsigned int *ipad, *opad, *hashKeyTank;
	unsigned int *pIDigest, *pODigest;
	unsigned int blkSize;
	saRecord_t *saRecord;
	saState_t *saState, *saState2;
	dma_addr_t	ipadPhyAddr, opadPhyAddr, saPhyAddr, statePhyAddr, statePhyAddr2;
	eip93DescpHandler_t *cmdHandler;
	addrsDigestPreCompute_t *addrsPreCompute;
	
	addrsPreCompute = currAdapterPtr->addrsPreCompute;
	
	if(addrsPreCompute == NULL)
		return;
	
	hashKeyTank = addrsPreCompute->hashKeyTank;
	ipad		= (unsigned int *)addrsPreCompute->ipadHandler.addr;
	ipadPhyAddr = addrsPreCompute->ipadHandler.phyAddr;
	opad		= (unsigned int *)addrsPreCompute->opadHandler.addr;
	opadPhyAddr = addrsPreCompute->opadHandler.phyAddr;
	blkSize 	= addrsPreCompute->blkSize;
	cmdHandler 	= addrsPreCompute->cmdHandler;
	saRecord 	= (saRecord_t *)addrsPreCompute->saHandler.addr;
	saPhyAddr 	= addrsPreCompute->saHandler.phyAddr;
	saState 	= (saState_t *)addrsPreCompute->stateHandler.addr;
	statePhyAddr = addrsPreCompute->stateHandler.phyAddr;
	saState2 	= (saState_t *)addrsPreCompute->stateHandler2.addr;
	statePhyAddr2 = addrsPreCompute->stateHandler2.phyAddr;
	pIDigest 	= addrsPreCompute->pIDigest;
	pODigest 	= addrsPreCompute->pODigest;		

	kfree(pODigest);
	kfree(pIDigest);
	dma_free_coherent(NULL, sizeof(saState_t), saState2, statePhyAddr2);
	dma_free_coherent(NULL, sizeof(saState_t), saState, statePhyAddr);
	dma_free_coherent(NULL, sizeof(saRecord_t), saRecord, saPhyAddr);
	kfree(cmdHandler);		
	dma_free_coherent(NULL, blkSize, opad, opadPhyAddr);		
	dma_free_coherent(NULL, blkSize, ipad, ipadPhyAddr);		
	kfree(hashKeyTank);
	kfree(addrsPreCompute);
	addrsPreCompute->pODigest = NULL;
	addrsPreCompute->pIDigest = NULL;
	currAdapterPtr->addrsPreCompute = NULL;
}
#endif
static void 
mtk_hashDigests_get(
	ipsecEip93Adapter_t *currAdapterPtr
)
{
	eip93DescpHandler_t *cmdHandler;
	saRecord_t *saRecord;
	addrsDigestPreCompute_t* addrsPreCompute;
	unsigned int i;
	
	cmdHandler = currAdapterPtr->cmdHandler;
	saRecord = (saRecord_t *)cmdHandler->saAddr.addr;
	addrsPreCompute = currAdapterPtr->addrsPreCompute;
	
	for (i = 0; i < (addrsPreCompute->digestWord); i++)
	{
		saRecord->saIDigest[i] = addrsPreCompute->pIDigest[i];
		saRecord->saODigest[i] = addrsPreCompute->pODigest[i];
	}
}

static void 
mtk_hashDigests_set(
	ipsecEip93Adapter_t *currAdapterPtr,
	unsigned int isInOrOut
)
{
//resDescpHandler only has physical addresses, so we have to get saState's virtual address from addrsPreCompute.

	addrsDigestPreCompute_t *addrsPreCompute;
	saState_t *stateHandler;
	unsigned int i, digestWord;
	
	
	addrsPreCompute = (addrsDigestPreCompute_t*) currAdapterPtr->addrsPreCompute;
	digestWord = addrsPreCompute->digestWord;

	if (isInOrOut == 1) //for Inner Digests
	{
		stateHandler = (saState_t *) addrsPreCompute->stateHandler.addr;
		
		for (i = 0; i < digestWord; i++)
		{
			addrsPreCompute->pIDigest[i] = stateHandler->stateIDigest[i];
		}
	}
	else if (isInOrOut == 2) //for Outer Digests
	{
		stateHandler = (saState_t *) addrsPreCompute->stateHandler2.addr;
		
		for (i = 0; i < digestWord; i++)
		{
			addrsPreCompute->pODigest[i] = stateHandler->stateIDigest[i];
		}		
	}
}

static unsigned int 
mtk_eip93UserId_get(
	eip93DescpHandler_t *resHandler
)
{
/* In our case, during hash digest pre-compute, the userId will be
 * currAdapterPtr; but during encryption/decryption, the userId
 * will be skb
 */
	return resHandler->userId;
}

static unsigned int 
mtk_eip93HashFinal_get(
	eip93DescpHandler_t *resHandler
)
{
/* In our case, during hash digest pre-compute, the hashFinal bit 
 * won't be set; but during encryption/decryption, the hashFinal
 * bit will be set
 */
	return resHandler->peCrtlStat.bits.hashFinal;
}

static unsigned int 
mtk_pktLength_get(
	eip93DescpHandler_t *resHandler
)
{
	return resHandler->peLength.bits.length;
}

static unsigned char 
mtk_espNextHeader_get(
	eip93DescpHandler_t *resHandler
)
{
	return resHandler->peCrtlStat.bits.padValue;
}

static void 
mtk_espNextHeader_set(
	eip93DescpHandler_t *cmdHandler, 
	unsigned char protocol	
)
{
	cmdHandler->peCrtlStat.bits.padValue = protocol; //ipsec esp's next-header which is IPPROTO_IPIP for tunnel or ICMP/TCP/UDP for transport mode
}

static int 
mtk_preComputeIn_cmdDescp_set(
	ipsecEip93Adapter_t *currAdapterPtr,
	unsigned int direction
)
{
	addrsDigestPreCompute_t* addrsPreCompute = currAdapterPtr->addrsPreCompute;
	eip93DescpHandler_t *cmdHandler;
	saRecord_t *saRecord;
	saState_t *saState;
	dma_addr_t	saPhyAddr, statePhyAddr;
	int errVal;
#if defined (CONFIG_HWCRYPTO_MEMPOOL)	
	addrsPreCompute->saHandler.addr = addrsPreCompute->RecPoolHandler.addr + SA_OFFSET;
	addrsPreCompute->saHandler.phyAddr = addrsPreCompute->RecPoolHandler.phyAddr + SA_OFFSET;
	addrsPreCompute->stateHandler.addr = addrsPreCompute->RecPoolHandler.addr + STATE_OFFSET;
	addrsPreCompute->stateHandler.phyAddr = addrsPreCompute->RecPoolHandler.phyAddr + STATE_OFFSET;
	addrsPreCompute->ipadHandler.addr = addrsPreCompute->RecPoolHandler.addr + IPAD_OFFSET;
	addrsPreCompute->ipadHandler.phyAddr =  addrsPreCompute->RecPoolHandler.phyAddr + IPAD_OFFSET;
	
	cmdHandler = addrsPreCompute->cmdHandler;
	saRecord = addrsPreCompute->saHandler.addr;
	saPhyAddr = addrsPreCompute->saHandler.phyAddr;
	saState = addrsPreCompute->stateHandler.addr;
	statePhyAddr = addrsPreCompute->stateHandler.phyAddr;	

	memset(saRecord, 0, sizeof(saRecord_t));
	memset(saState, 0, sizeof(saState_t));
#else
	cmdHandler = (eip93DescpHandler_t *) kzalloc(sizeof(eip93DescpHandler_t), GFP_KERNEL);
	if (unlikely(cmdHandler == NULL))
	{
		printk("\n\n !!kmalloc for cmdHandler failed!! \n\n");
		return -ENOMEM;
	}
	addrsPreCompute->cmdHandler = cmdHandler;
	
	saRecord = (saRecord_t *) dma_alloc_coherent(NULL, sizeof(saRecord_t), &saPhyAddr, GFP_KERNEL);
	if (unlikely(saRecord == NULL))
	{
		printk("\n\n !!dma_alloc for saRecord failed!! \n\n");
		errVal = -ENOMEM;
		goto free_cmdHandler;
	}
	memset(saRecord, 0, sizeof(saRecord_t));
	addrsPreCompute->saHandler.addr = (unsigned int)saRecord;
	addrsPreCompute->saHandler.phyAddr = saPhyAddr;
	
	saState = (saState_t *) dma_alloc_coherent(NULL, sizeof(saState_t), &statePhyAddr, GFP_KERNEL);
	if (unlikely(saState == NULL))
	{
		printk("\n\n !!dma_alloc for saState failed!! \n\n");
		errVal = -ENOMEM;
		goto free_saRecord;
	}
	memset(saState, 0, sizeof(saState_t));
	addrsPreCompute->stateHandler.addr = (unsigned int)saState;
	addrsPreCompute->stateHandler.phyAddr = statePhyAddr;	
#endif
	saRecord->saCmd0.bits.opCode = 0x3; //basic hash operation
	saRecord->saCmd0.bits.hashSource = 0x3; //no load HASH_SOURCE
	saRecord->saCmd0.bits.saveHash = 0x1;

	if (addrsPreCompute->digestWord == 4)
		saRecord->saCmd0.bits.hash = 0x0; //md5
	else if (addrsPreCompute->digestWord == 5)
		saRecord->saCmd0.bits.hash = 0x1; //sha1
	else if (addrsPreCompute->digestWord == 8)
		saRecord->saCmd0.bits.hash = 0x3; //sha256
	else if (addrsPreCompute->digestWord == 0)
		saRecord->saCmd0.bits.hash = 0xf; //null

	if (direction == HASH_DIGEST_OUT)
	{
		saRecord->saCmd0.bits.direction = 0x0; //outbound
	}
	else if (direction == HASH_DIGEST_IN)
	{
		saRecord->saCmd0.bits.direction = 0x1; //inbound
	}	

	//saRecord->saCmd0.bits.hash = hashAlg;
	
	cmdHandler->peCrtlStat.bits.hostReady = 0x1;
	cmdHandler->srcAddr.phyAddr = addrsPreCompute->ipadHandler.phyAddr;
	cmdHandler->saAddr.phyAddr = saPhyAddr;
	cmdHandler->stateAddr.phyAddr = statePhyAddr;
	cmdHandler->peLength.bits.hostReady = 0x1;
	cmdHandler->peLength.bits.length = (addrsPreCompute->blkSize) & (BIT_20 - 1);

	//save needed info in EIP93's userID, so the needed info can be used by the tasklet which is raised by interrupt.
	cmdHandler->userId = (unsigned int)currAdapterPtr;

	return 1;
	
#if !defined (CONFIG_HWCRYPTO_MEMPOOL)	
free_saRecord:
	dma_free_coherent(NULL, sizeof(saRecord_t), saRecord, saPhyAddr);
free_cmdHandler:
	kfree(cmdHandler);
	
	return errVal;
#endif
}

static int 
mtk_preComputeOut_cmdDescp_set(
	ipsecEip93Adapter_t *currAdapterPtr,
	unsigned int direction
)
{
	addrsDigestPreCompute_t* addrsPreCompute = currAdapterPtr->addrsPreCompute;	
	saState_t *saState2;
	dma_addr_t	statePhyAddr2;
	int errVal;
	eip93DescpHandler_t *cmdHandler = addrsPreCompute->cmdHandler;
#if defined (CONFIG_HWCRYPTO_MEMPOOL)	
	addrsPreCompute->stateHandler2.addr = addrsPreCompute->RecPoolHandler.addr + STATE2_OFFSET;
	addrsPreCompute->stateHandler2.phyAddr = addrsPreCompute->RecPoolHandler.phyAddr + STATE2_OFFSET;
	addrsPreCompute->opadHandler.addr = addrsPreCompute->RecPoolHandler.addr + OPAD_OFFSET;
	addrsPreCompute->opadHandler.phyAddr =  addrsPreCompute->RecPoolHandler.phyAddr + OPAD_OFFSET;
	
	saState2 = addrsPreCompute->stateHandler2.addr;
	statePhyAddr2 = addrsPreCompute->stateHandler2.phyAddr;	
	
	memset(saState2, 0, sizeof(saState_t));
#else
	saState2 = (saState_t *) dma_alloc_coherent(NULL, sizeof(saState_t), &statePhyAddr2, GFP_KERNEL);
	if (unlikely(saState2 == NULL))
	{
		printk("\n\n !!dma_alloc for saState2 failed!! \n\n");
		errVal = -ENOMEM;
		goto free_preComputeIn;
	}
	memset(saState2, 0, sizeof(saState_t));
	addrsPreCompute->stateHandler2.addr = (unsigned int)saState2;
	addrsPreCompute->stateHandler2.phyAddr = statePhyAddr2;	
#endif
	
	cmdHandler->srcAddr.phyAddr = addrsPreCompute->opadHandler.phyAddr;
	cmdHandler->stateAddr.phyAddr = statePhyAddr2;

	return 1;	
	
#if !defined (CONFIG_HWCRYPTO_MEMPOOL)	
free_preComputeIn:
	dma_free_coherent(NULL, sizeof(saState_t), (saState_t *)addrsPreCompute->stateHandler.addr, addrsPreCompute->stateHandler.phyAddr);
	dma_free_coherent(NULL, sizeof(saRecord_t), (saRecord_t *)addrsPreCompute->saHandler.addr, addrsPreCompute->saHandler.phyAddr);
	kfree(addrsPreCompute->cmdHandler);
	
	return errVal;
#endif
}

static int 
mtk_cmdHandler_cmdDescp_set(
	ipsecEip93Adapter_t *currAdapterPtr, 
	unsigned int direction,
	unsigned int cipherAlg, 
	unsigned int hashAlg, 
	unsigned int digestWord, 
	unsigned int cipherMode, 
	unsigned int enHmac, 
	unsigned int aesKeyLen, 
	unsigned int *cipherKey, 
	unsigned int keyLen, 
	unsigned int spi, 
	unsigned int padCrtlStat
)
{
	eip93DescpHandler_t *cmdHandler;
	saRecord_t *saRecord;
	saState_t *saState;
	dma_addr_t saPhyAddr, statePhyAddr;
	int errVal;
	unsigned int keyWord, i;
#if defined (CONFIG_HWCRYPTO_MEMPOOL)	
	cmdHandler = currAdapterPtr->cmdHandler;
	
	cmdHandler->stateAddr.addr = cmdHandler->saAddr.addr + SAPOOL_STATE_OFFSET;
	cmdHandler->stateAddr.phyAddr = cmdHandler->saAddr.phyAddr + SAPOOL_STATE_OFFSET;
	cmdHandler->arc4Addr.addr = cmdHandler->saAddr.addr + SAPOOL_ARC4STATE_OFFSET;
	cmdHandler->arc4Addr.phyAddr = cmdHandler->saAddr.phyAddr + SAPOOL_ARC4STATE_OFFSET;
	
	saRecord = cmdHandler->saAddr.addr;
	saPhyAddr = cmdHandler->saAddr.phyAddr;
	saState = cmdHandler->stateAddr.addr;
	statePhyAddr = cmdHandler->stateAddr.phyAddr;
	saState = cmdHandler->arc4Addr.addr;
	statePhyAddr = cmdHandler->arc4Addr.phyAddr; 

	memset(saRecord, 0, sizeof(saRecord_t));
	memset(saState, 0, sizeof(saState_t));
#else
	cmdHandler = (eip93DescpHandler_t *) kzalloc(sizeof(eip93DescpHandler_t), GFP_KERNEL);
	if (unlikely(cmdHandler == NULL))
	{
		printk("\n\n !!kmalloc for cmdHandler_prepare failed!! \n\n");
		return -ENOMEM;
	}
	
	saRecord = (saRecord_t *) dma_alloc_coherent(NULL, sizeof(saRecord_t), &saPhyAddr, GFP_KERNEL);
	if (unlikely(saRecord == NULL))
	{
		printk("\n\n !!dma_alloc for saRecord_prepare failed!! \n\n");
		errVal = -ENOMEM;
		goto free_cmdHandler;
	}
	memset(saRecord, 0, sizeof(saRecord_t));
	
	saState = (saState_t *) dma_alloc_coherent(NULL, sizeof(saState_t), &statePhyAddr, GFP_KERNEL);
	if (unlikely(saState == NULL))
	{
		printk("\n\n !!dma_alloc for saState_prepare failed!! \n\n");
		errVal = -ENOMEM;
		goto free_saRecord;
	}
	memset(saState, 0, sizeof(saState_t));
#endif	
	
	/* prepare SA */

	if (direction == HASH_DIGEST_OUT)
	{
		currAdapterPtr->isEncryptOrDecrypt = 1; //encrypt
		saRecord->saCmd0.bits.direction = 0x0; //outbound
		saRecord->saCmd0.bits.ivSource = 0x3;//0x3; //load IV from PRNG
		//if (cipherAlg == 0x3)
			//saRecord->saCmd1.bits.aesDecKey = 0;
	}
	else if (direction == HASH_DIGEST_IN)
	{
		currAdapterPtr->isEncryptOrDecrypt = 2; //decrypt
		saRecord->saCmd0.bits.direction = 0x1; //inbound
		//if (cipherAlg == 0x3)
			//saRecord->saCmd1.bits.aesDecKey = 1;
	}

	saRecord->saCmd0.bits.opGroup = 0x1;
	saRecord->saCmd0.bits.opCode = 0x0; //esp protocol
	saRecord->saCmd0.bits.cipher = cipherAlg;
	saRecord->saCmd0.bits.hash = hashAlg;
	saRecord->saCmd0.bits.hdrProc = 0x1; //header processing for esp
	saRecord->saCmd0.bits.digestLength = digestWord;	
	saRecord->saCmd1.bits.cipherMode = cipherMode;
	saRecord->saCmd1.bits.hmac = enHmac;
	if (cipherAlg == 0x3) //aes
		saRecord->saCmd1.bits.arc4KeyLen = aesKeyLen;
		//saRecord->saCmd1.bits.aesKeyLen = aesKeyLen;
	
	saRecord->saCmd1.bits.seqNumCheck = 1;	
			
	keyWord = keyLen >> 2;
	for (i = 0; i < keyWord; i++)
	{
		//saRecord->saKey[i] = WORDSWAP(cipherKey[i]);
		saRecord->saKey[i] = cipherKey[i];
	}

	saRecord->saSpi = WORDSWAP(spi); //esp spi

	saRecord->saSeqNumMask[0] = 0xFFFFFFFF;
	saRecord->saSeqNumMask[1] = 0x0;
			
	/* prepare command descriptor */
	
	cmdHandler->peCrtlStat.bits.hostReady = 0x1;
	cmdHandler->peCrtlStat.bits.hashFinal = 0x1;
	cmdHandler->peCrtlStat.bits.padCrtlStat = padCrtlStat; //pad boundary

#if !defined (CONFIG_HWCRYPTO_MEMPOOL)
	cmdHandler->saAddr.addr = (unsigned int)saRecord;
	cmdHandler->saAddr.phyAddr = saPhyAddr;
	cmdHandler->stateAddr.addr = (unsigned int)saState;
	cmdHandler->stateAddr.phyAddr = statePhyAddr;
	cmdHandler->arc4Addr.addr = (unsigned int)saState;
	cmdHandler->arc4Addr.phyAddr = statePhyAddr;
#endif
	cmdHandler->peLength.bits.hostReady = 0x1;
	cmdHandler->peCrtlStat.bits.peReady = 0;
#if !defined (CONFIG_HWCRYPTO_MEMPOOL)
	/* restore cmdHandler for later use */
	currAdapterPtr->cmdHandler = cmdHandler;
#endif	
	return 1;
	
#if !defined (CONFIG_HWCRYPTO_MEMPOOL)	
free_saRecord:
	dma_free_coherent(NULL, sizeof(saRecord_t), saRecord, saPhyAddr);
free_cmdHandler:
	kfree(cmdHandler);
	return errVal;
#endif
}

/*_______________________________________________________________________
**function name: mtk_packet_put
**
**description:
*   put command descriptor into EIP93's Command Descriptor Ring and
*	then kick off EIP93.
**parameters:
*   cmdDescp -- point to the command handler that stores the needed
*		info for the command descriptor.
*	skb -- the packet for encryption/decryption
**global:
*   none
**return:
*   0 -- success.
**call:
*   none
**revision:
*   1.Trey 20120209
**_______________________________________________________________________*/
static int 
mtk_packet_put(
	eip93DescpHandler_t *cmdDescp, 
	struct sk_buff *skb //skb == NULL when in digestPreCompute
)
{
	unsigned int *pCrd = pCmdRingBase;
	ipsecEip93Adapter_t *currAdapterPtr;
	unsigned int addedLen;
	unsigned int *addrCurrAdapter;
	unsigned long flags;
	u32* pData = NULL;
	dma_addr_t pDataPhy;
	
	if(cmdRingIdx == EIP93_RING_SIZE)
	{
		cmdRingIdx = 0;
	}
	if(resPrepRingIdx == EIP93_RING_SIZE)
	{
		resPrepRingIdx = 0;
	}
	pCrd += (cmdRingIdx << 3); //cmdRingIdx*8 (a cmdDescp has 8 words!)
	cmdRingIdx++;
	resPrepRingIdx++;

	pCrd[3] = cmdDescp->saAddr.phyAddr;
	pCrd[4] = cmdDescp->stateAddr.phyAddr;
	pCrd[5] = cmdDescp->arc4Addr.phyAddr;

	if(likely(skb != NULL))
	{
		addrCurrAdapter = (unsigned int *) &(skb->cb[36]);
		currAdapterPtr = (ipsecEip93Adapter_t *)(*addrCurrAdapter);
		addedLen = currAdapterPtr->addedLen;
spin_lock(&currAdapterPtr->lock);
		currAdapterPtr->packet_count++;
spin_unlock(&currAdapterPtr->lock);
		dma_cache_wback_inv((unsigned long)(skb->data), (skb->len + addedLen) & (BIT_20 - 1));
		
#if defined (BUFFER_MEMCPY)	
		if ((u32)(skb->data)%DMAAlign)
		{	
#if defined (SKB_HEAD_SHIFT)
			int offset, alloc_size;
			offset = DMAAlign-(u32)(skb->data)%DMAAlign;
			pData = NULL;
#else				
			pData = kmalloc(skb->len + addedLen, GFP_KERNEL);
#endif
			*(unsigned int *) &(skb->cb[40]) = (u32)pData;
			if(pData==NULL)
			{	
#if defined (SKB_HEAD_SHIFT)
				pCrd[1] = K1_TO_PHY(skb->data);
				pCrd[2] = K1_TO_PHY(skb->data+offset);
#else
				printk("mtk_packet_put allocate null\n");
				pCrd[1] = K1_TO_PHY(skb->data);
				pCrd[2] = K1_TO_PHY(skb->data);
#endif
			}
			else
			{					
				pDataPhy = dma_map_single(NULL, pData, skb->len + addedLen, PCI_DMA_FROMDEVICE);
				if (pDataPhy==NULL)
				{
					printk("dma_map_single pDataPhy NULL\n");
				}
				pCrd[1] = K1_TO_PHY(skb->data);
				pCrd[2] = pDataPhy;			
			}
		}
		else
#endif
		{	
			pCrd[1] = K1_TO_PHY(skb->data);
			pCrd[2] = K1_TO_PHY(skb->data);
		}

		pCrd[6] = (unsigned int)skb;
#if 0
	    /*When encryption, it is necessary to consider when the packet size is greater than 200, and in tunnel mode (l2tpeth0), 
 			before the packet length longer (+4), otherwise there will be untied.   */
		if ((currAdapterPtr->isEncryptOrDecrypt==1) && (skb->len > 200) && !(skb->dev->name == NULL))
		{
			if(strcmp(skb->dev->name,"l2tpeth0") == 0)
			{		
				pCrd[7] = ((skb->len+4) & (BIT_20 - 1)) | (cmdDescp->peLength.word & (~(BIT_22 - 1)));
			}else{
				pCrd[7] = ((skb->len) & (BIT_20 - 1)) | (cmdDescp->peLength.word & (~(BIT_22 - 1)));
			}
		}
		else
#endif		
		{
			pCrd[7] = ((skb->len) & (BIT_20 - 1)) | (cmdDescp->peLength.word & (~(BIT_22 - 1)));
		}

	}
	else
	{
		pCrd[1] = cmdDescp->srcAddr.phyAddr;
		pCrd[2] = cmdDescp->srcAddr.phyAddr;
		pCrd[6] = cmdDescp->userId;
		pCrd[7] = cmdDescp->peLength.word;
	}
	pCrd[0] = cmdDescp->peCrtlStat.word;

	//prevent from inconsistency of HW DMA and SW memory access
	wmb();
	iowrite32(1, pEip93RegBase + (PE_CD_COUNT >> 2)); //PE_CD_COUNT/4	
	
	return 0; //success
}

/*_______________________________________________________________________
**function name: mtk_packet_get
**
**description:
*   get result descriptor from EIP93's Result Descriptor Ring.
**parameters:
*   resDescp -- point to the result handler that stores the needed
*		info for the result descriptor.
**global:
*   none
**return:
*   0  -- EIP93 has no result yet.
*   1  -- EIP93 has results ready.
*   -1 -- the current result is wrong!
**call:
*   none
**revision:
*   1.Trey 20120209
**_______________________________________________________________________*/
static int 
mtk_packet_get(
	eip93DescpHandler_t *resDescp,
	unsigned int rdx
)
{
	unsigned int *pRrd = pResRingBase;
	unsigned int done1, done2, err_sts, PktCnt, timeCnt = 0;
	unsigned long flags;
	struct sk_buff *skb = NULL;
	int retVal;

	ipsecEip93Adapter_t *currAdapterPtr;
	unsigned int *addrCurrAdapter;

	PktCnt = ioread32(pEip93RegBase + (PE_RD_COUNT >> 2)) & (BIT_10 - 1); //PE_RD_COUNT/4

	//don't wait for Crypto Engine in order to speed up!
	if(PktCnt == 0)
	{
		return 0; //no result yet
	}
	if(resRingIdx == EIP93_RING_SIZE)
	{
		resRingIdx = 0;
	}	
	pRrd += (resRingIdx << 3); //resRingIdx*8 (a resDescp has 8 words!)

	while (1)
	{
		
		PktCnt = ioread32(pEip93RegBase + (PE_RD_COUNT >> 2)) & (BIT_10 - 1); //PE_RD_COUNT/4

		resDescp->peCrtlStat.word 	= pRrd[0];
		resDescp->userId		 	= pRrd[6];
		resDescp->peLength.word 	= pRrd[7];
		//the others are physical addresses, no need to be copied!
		done1 = resDescp->peCrtlStat.bits.peReady;
		done2 = resDescp->peLength.bits.peReady;
		err_sts = resDescp->peCrtlStat.bits.errStatus;
		resDescp->saAddr.phyAddr = pRrd[3];
		if ((done1 == 1) && (done2 == 1))
		{	
			if(unlikely(err_sts))
			{
				int cmdPktCnt = (ioread32(pEip93RegBase + (PE_CD_COUNT >> 2)) & (BIT_10 - 1));
				skb = (struct sk_buff *)resDescp->userId;
				addrCurrAdapter = (unsigned int *) &(skb->cb[36]);
				currAdapterPtr = (ipsecEip93Adapter_t *)(*addrCurrAdapter);
				printk("\n\n !PE Ring[%d] ErrCode=0x%x! status=%x rdn=%d cdn=%d encrypt=%d spi=%x index=%d qlen=%d\n\n", resRingIdx, err_sts, ioread32(pEip93RegBase + (PE_CTRL_STAT >> 2)), PktCnt,\
						cmdPktCnt,currAdapterPtr->isEncryptOrDecrypt, currAdapterPtr->spi,currAdapterPtr->idx,currAdapterPtr->skbQueue.qlen);
				//for encryption/decryption case

				spin_lock(&currAdapterPtr->lock);
				currAdapterPtr->packet_count--;	
				spin_unlock(&currAdapterPtr->lock);
				//if (resDescp->peCrtlStat.bits.hashFinal == 0x1)
				{
#if defined (MCRYPTO_DBG)
					if ((err_sts&0x1)==0x1)
					{	
						{
							int k;
							int offset, alloc_size;
							offset = DMAAlign-(u32)(skb->data)%DMAAlign;
							printk("ICV[[");
							for (k = 0; k < 12; k++)
								printk("%02X ",skb->data[resDescp->peLength.bits.length-12+k+offset]);
							printk("]]\n");	
						}
					}
#endif

					if ((resDescp->userId>>31)&0x1)
					{	
#if defined (BUFFER_MEMCPY)
						addrCurrAdapter = (unsigned int *) &(skb->cb[36]);
						currAdapterPtr = (ipsecEip93Adapter_t *)(*addrCurrAdapter);
						if (pRrd[2]!=pRrd[1])
						{
							int offset, alloc_size;	
							u8* pData =  *(unsigned int *)&(skb->cb[40]);
							offset = DMAAlign-(u32)(skb->data)%DMAAlign;
							alloc_size = skb->end-skb->head+sizeof(struct skb_shared_info)+offset;	
#if defined (SKB_HEAD_SHIFT)
#else						
							dma_unmap_single (NULL, pRrd[2], skb->len + currAdapterPtr->addedLen, PCI_DMA_FROMDEVICE);
							kfree(pData);	
#endif
						}
#endif
						kfree_skb(skb);
					}
					else
						printk("resDescp->userId = 0x%x\n", resDescp->userId);	
				}
				//else {

				//}	
					 
				retVal = -1;
				break;
			}
			skb = (struct sk_buff *)resDescp->userId;

			if((skb!=NULL)&&(resDescp->peCrtlStat.bits.hashFinal == 0x1))
			{
				addrCurrAdapter = (unsigned int *) &(skb->cb[36]);
				currAdapterPtr = (ipsecEip93Adapter_t *)(*addrCurrAdapter);	
				spin_lock(&currAdapterPtr->lock);
				currAdapterPtr->packet_count--;	
				spin_unlock(&currAdapterPtr->lock);
#if defined (BUFFER_MEMCPY)
				if (pRrd[2]!=pRrd[1])	
				{	
#if defined (SKB_HEAD_SHIFT)
					int offset, alloc_size;
					offset = DMAAlign-(u32)(skb->data)%DMAAlign;
					copy_data_head(skb, offset);
#else					
					u8* pData =  *(unsigned int *)&(skb->cb[40]);
					dma_cache_sync(NULL, pData, skb->len + currAdapterPtr->addedLen, DMA_FROM_DEVICE);
					memcpy(skb->data, (u32)pData, resDescp->peLength.bits.length);
					dma_unmap_single (NULL, pRrd[2], skb->len + currAdapterPtr->addedLen, PCI_DMA_FROMDEVICE);			
					kfree(pData);
#endif
				}
#endif
			}

			retVal = 1;
			break; 
		}
		else
		{
			//if eip93 is done but the result is not ready yet, just reCkeckResult one more time!
			if (timeCnt++ > 10)
			{
				printk("\n !wait eip93's result for too long! Drop it! \n");
				printk("resRingIdx=%d\n",resRingIdx);
				//if (resDescp->peCrtlStat.bits.hashFinal == 0x1)
				{
					skb = (struct sk_buff *)resDescp->userId;
					if ((resDescp->userId>>31)&0x1)
					{
#if defined (BUFFER_MEMCPY)
						addrCurrAdapter = (unsigned int *) &(skb->cb[36]);
						currAdapterPtr = (ipsecEip93Adapter_t *)(*addrCurrAdapter);	
						spin_lock(&currAdapterPtr->lock);
						currAdapterPtr->packet_count--;	
						spin_unlock(&currAdapterPtr->lock);
						if (pRrd[2]!=pRrd[1])
						{
							int offset, alloc_size;	
							u8* pData =  *(unsigned int *)&(skb->cb[40]);
							offset = DMAAlign-(u32)(skb->data)%DMAAlign;
							alloc_size = skb->end-skb->head+sizeof(struct skb_shared_info)+offset;	
#if defined (SKB_HEAD_SHIFT)
#else							
							dma_unmap_single (NULL, pRrd[2], skb->len + currAdapterPtr->addedLen, PCI_DMA_FROMDEVICE);
							kfree(pData);		
#endif
						}
#endif

						kfree_skb(skb);
					}
					else
						printk("resDescp->userId = 0x%x\n", resDescp->userId);		
				} 
			
				retVal = -1;
				break;
			}
		}
	} //end while(1)
	
	//clear the peCrtlStat of the currrent resRingDescp, in case eip93 can't put the current result in resRingDescp on time!
	pRrd[0] = 0;

	wmb();
	if(cmdRingFrontIdx == EIP93_RING_SIZE)
	{
		cmdRingFrontIdx = 0;
	}
	cmdRingFrontIdx++;
	resRingIdx++;
	iowrite32(1, pEip93RegBase + (PE_RD_COUNT >> 2)); //PE_RD_COUNT/4

	return retVal;
}


static bool 
mtk_eip93CmdResCnt_check(
	unsigned int rdx	
)
{
	unsigned int ret;
	int diff,diff2;

	if (cmdRingFrontIdx > cmdRingIdx)
		diff = (cmdRingFrontIdx-cmdRingIdx);
	else
		diff = EIP93_RING_SIZE - (cmdRingIdx-cmdRingFrontIdx);
	if (resRingIdx > resPrepRingIdx )
		diff2 = (resRingIdx-resPrepRingIdx);
	else
		diff2 = EIP93_RING_SIZE - (resPrepRingIdx-resRingIdx);
	
	return ((diff >= (2)) && (diff2 >= (2)) );
}


static unsigned int 
mtk_espSeqNum_get(
	eip93DescpHandler_t *resHandler
)
{
	saRecord_t *saRecord;
	resHandler->saAddr.addr = (resHandler->saAddr.phyAddr|(0x5<<29));
	saRecord = (saRecord_t *)resHandler->saAddr.addr;

	return saRecord->saSeqNum[0];
}
/************************************************************************
*              P U B L I C     F U N C T I O N S
*************************************************************************
*/
void 
mtk_ipsec_init(
	void
)
{
	DMAAlign = 4;//dma_get_cache_alignment();
	printk("== IPSEC Crypto Engine Driver : %s %s ==\n",__DATE__,__TIME__);
	//spin_lock_init(&putlock);
	//spin_lock_init(&getlock);
	write_c0_config7((read_c0_config7()|(1<<8)));
	
	ipsec_eip93_adapters_init();
	ipsec_cryptoLock_init();
	
	//function pointer init
	ipsec_packet_put = mtk_packet_put;
	ipsec_packet_get = mtk_packet_get;
	ipsec_eip93CmdResCnt_check = mtk_eip93CmdResCnt_check;
	ipsec_preComputeIn_cmdDescp_set = mtk_preComputeIn_cmdDescp_set;
	ipsec_preComputeOut_cmdDescp_set = mtk_preComputeOut_cmdDescp_set;
	ipsec_cmdHandler_cmdDescp_set = mtk_cmdHandler_cmdDescp_set;
	ipsec_espNextHeader_set = mtk_espNextHeader_set;
	ipsec_espNextHeader_get = mtk_espNextHeader_get;
	ipsec_pktLength_get = mtk_pktLength_get;
	ipsec_eip93HashFinal_get = mtk_eip93HashFinal_get;
	ipsec_eip93UserId_get = mtk_eip93UserId_get;
#if !defined (CONFIG_HWCRYPTO_MEMPOOL)
	ipsec_addrsDigestPreCompute_free = mtk_addrsDigestPreCompute_free;
	ipsec_cmdHandler_free = mtk_cmdHandler_free;
#endif
	ipsec_hashDigests_get = mtk_hashDigests_get;
	ipsec_hashDigests_set = mtk_hashDigests_set;
	
	ipsec_espSeqNum_get = mtk_espSeqNum_get;
	
	//eip93 info init
	cmdRingIdx = ioread32(pEip93RegBase + (PE_RING_PNTR >> 2)) & (BIT_10-1); 
	resRingIdx = (ioread32(pEip93RegBase + (PE_RING_PNTR >> 2)) >>16) & (BIT_10-1);

	cmdRingFrontIdx = cmdRingIdx;
	resPrepRingIdx = resRingIdx;
	printk("-- cmdRingFrontIdx=%d cmdRingIdx=%d --\n",cmdRingFrontIdx,cmdRingIdx);
	printk("-- resPrepRingIdx=%d resRingIdx=%d --\n",resPrepRingIdx,resRingIdx);

#ifdef WORKQUEUE_BH
	INIT_WORK(&mtk_interrupt_BH_result_wq, mtk_BH_handler_resultGet);
#else
	tasklet_init(&mtk_interrupt_BH_result_tsk, mtk_BH_handler_resultGet , 0);
#endif
	//eip93 interrupt mode init
	Adapter_Interrupt_ClearAndEnable(IRQ_RDR_THRESH_IRQ);
	Adapter_Interrupt_SetHandler(IRQ_RDR_THRESH_IRQ, mtk_interruptHandler_done);
	
	//EndianSwap Setting for C.L.'s new POF for fix no_word_alignment  (put right b4 kick CryptoEngine)
	iowrite32(0x00000700, pEip93RegBase + (PE_CONFIG >> 2));
	iowrite32(0x00e400e4, pEip93RegBase + (PE_ENDIAN_CONFIG >> 2));

}

void mtk_BH_handler_resultGet(
	unsigned long data
)
{
	ipsec_BH_handler_resultGet(0);
	Adapter_Interrupt_ClearAndEnable(IRQ_RDR_THRESH_IRQ);
}

static void mtk_interruptHandler_done(void)
{
#ifdef WORKQUEUE_BH
	schedule_work(&mtk_interrupt_BH_result_wq);
#else	
	tasklet_hi_schedule(&mtk_interrupt_BH_result_tsk);
#endif
}

int copy_data_head(struct sk_buff *skb, int offset)
{
	unsigned int i;

	if (skb_shinfo(skb)->nr_frags > 0)
	{
		printk("skb %08X has frags\n",skb);
		return -1;
	}	
	for(i=(unsigned int)(skb->data-1);i>=(unsigned int) skb->head;i--) {

		*((unsigned char*)(i+offset)) = *((unsigned char*)(i));

    }

	skb->data += offset;

	/* {transport,network,mac}_header and tail are relative to skb->head */
	skb->tail	      += offset;
	skb->transport_header += offset;
	skb->network_header   += offset;
	if (skb_mac_header_was_set(skb))
		skb->mac_header += offset;
	/* Only adjust this if it actually is csum_start rather than csum */
	if (skb->ip_summed == CHECKSUM_PARTIAL)
		skb->csum_start += 0;
	skb->cloned   = 0;
	skb->hdr_len  = 0;
	skb->nohdr    = 0;

	return 0;	
}
