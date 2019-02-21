 /************************************************************************
 *
 *	Copyright (C) 2012 MediaTek Technologies, Corp.
 *	All Rights Reserved.
 *
 * MediaTek Confidential; Need to Know only.
 * Protected as an unpublished work.
 *
 * The computer program listings, specifications and documentation
 * herein are the property of MediaTek Technologies, Co. and shall
 * not be reproduced, copied, disclosed, or used in whole or in part
 * for any reason without the prior express written permission of
 * MediaTek Technologeis, Co.
 *
 *************************************************************************/


#include <linux/err.h>
#include <linux/module.h>
#include <linux/version.h>
#include <net/ip.h>
#include <net/xfrm.h>
#include <net/esp.h>
#include <asm/scatterlist.h>
#include <linux/crypto.h>
#include <linux/kernel.h>
#include <linux/pfkeyv2.h>
#include <linux/random.h>
#include <net/icmp.h>
#include <net/protocol.h>
#include <net/udp.h>

#include <net/mtk_esp.h>
#include <linux/netfilter_ipv4.h>

#ifdef MTK_EIP97_IPI
#include <linux/smp.h>		
#endif		
/************************************************************************
*                          C O N S T A N T S
*************************************************************************
*/

#define HASH_MD5_HMAC			"hmac(md5)"
#define HASH_SHA1_HMAC			"hmac(sha1)"
#define HASH_SHA256_HMAC		"hmac(sha256)"
#define HASH_NULL_HMAC 			"hmac(digest_null)"
#define HASH_IPAD				0x36363636
#define HASH_OPAD				0x5c5c5c5c
#define CIPHER_DES_CBC			"cbc(des)"
#define CIPHER_3DES_CBC			"cbc(des3_ede)"
#define CIPHER_AES_CBC			"cbc(aes)"
#define CIPHER_NULL_ECB			"ecb(cipher_null)"
#define SKB_QUEUE_MAX_SIZE		2048

#define RALINK_HWCRYPTO_NAT_T	1
#define FEATURE_AVOID_QUEUE_PACKET	1

/************************************************************************
*      P R I V A T E    S T R U C T U R E    D E F I N I T I O N
*************************************************************************
*/


/************************************************************************
*              P R I V A T E     D A T A
*************************************************************************
*/
ipsecEip93Adapter_t 	*ipsecEip93AdapterListOut[IPESC_EIP93_ADAPTERS];
ipsecEip93Adapter_t 	*ipsecEip93AdapterListIn[IPESC_EIP93_ADAPTERS];
#if defined (CONFIG_HWCRYPTO_MEMPOOL)
static unsigned int spi_inbound_tbl[IPESC_EIP93_ADAPTERS]  __attribute__((aligned(32)));
static unsigned int spi_outbound_tbl[IPESC_EIP93_ADAPTERS] __attribute__((aligned(32)));
#endif
static spinlock_t 			cryptoLock[NUM_CMD_RING];
static spinlock_t				ipsec_adapters_outlock, ipsec_adapters_inlock;
#ifdef MTK_EIP97_DRIVER
static eip97DescpHandler_t 	resDescpHandler[NUM_RESULT_RING];
#else
static eip93DescpHandler_t 	resDescpHandler[NUM_RESULT_RING];
#endif
mcrypto_proc_type 			mcrypto_proc;
EXPORT_SYMBOL(mcrypto_proc);
EXPORT_SYMBOL(ipsecEip93AdapterListOut);
EXPORT_SYMBOL(ipsecEip93AdapterListIn);
/************************************************************************
*              E X T E R N A L     D A T A
*************************************************************************
*/
#ifdef MTK_EIP97_DRIVER
int 
(*ipsec_packet_put)(
	void *descpHandler, 
	struct sk_buff *skb,
	unsigned int rdx
);
#else
int 
(*ipsec_packet_put)(
	void *descpHandler, 
	struct sk_buff *skb
);
#endif
int 
(*ipsec_packet_get)(
	void *descpHandler,
	unsigned int rdx
);
bool 
(*ipsec_eip93CmdResCnt_check)(
	unsigned int rdx
);
int 
(*ipsec_preComputeIn_cmdDescp_set)(
	ipsecEip93Adapter_t *currAdapterPtr,
	//unsigned int hashAlg, 
	unsigned int direction
);
int 
(*ipsec_preComputeOut_cmdDescp_set)(
	ipsecEip93Adapter_t *currAdapterPtr, 
	//unsigned int hashAlg,
	unsigned int direction
);
int 
(*ipsec_cmdHandler_cmdDescp_set)(
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
);
void 
(*ipsec_espNextHeader_set)(
	void *cmdHandler, 
	unsigned char protocol	
);
unsigned char 
(*ipsec_espNextHeader_get)(
	void *resHandler
);
unsigned int 
(*ipsec_pktLength_get)(
	void *resHandler
);
unsigned int 
(*ipsec_eip93HashFinal_get)(
	void *resHandler
);
unsigned int 
(*ipsec_eip93UserId_get)(
	void *resHandler
);

void 
(*ipsec_addrsDigestPreCompute_free)(
	ipsecEip93Adapter_t *currAdapterPtr
);

void 
(*ipsec_cmdHandler_free)(
	void *cmdHandler
);

void 
(*ipsec_hashDigests_get)(
	ipsecEip93Adapter_t *currAdapterPtr
);

void 
(*ipsec_hashDigests_set)(
	ipsecEip93Adapter_t *currAdapterPtr,
	unsigned int isInOrOut
);

unsigned int 
(*ipsec_espSeqNum_get)(
	void *resHandler
);

EXPORT_SYMBOL(ipsec_packet_put);
EXPORT_SYMBOL(ipsec_packet_get);
EXPORT_SYMBOL(ipsec_eip93CmdResCnt_check);
EXPORT_SYMBOL(ipsec_preComputeIn_cmdDescp_set);
EXPORT_SYMBOL(ipsec_preComputeOut_cmdDescp_set);
EXPORT_SYMBOL(ipsec_cmdHandler_cmdDescp_set);
EXPORT_SYMBOL(ipsec_espNextHeader_set);
EXPORT_SYMBOL(ipsec_espNextHeader_get);
EXPORT_SYMBOL(ipsec_pktLength_get);
EXPORT_SYMBOL(ipsec_eip93HashFinal_get);
EXPORT_SYMBOL(ipsec_eip93UserId_get);
#if !defined (CONFIG_HWCRYPTO_MEMPOOL)
EXPORT_SYMBOL(ipsec_addrsDigestPreCompute_free);
EXPORT_SYMBOL(ipsec_cmdHandler_free);
#endif
EXPORT_SYMBOL(ipsec_hashDigests_get);
EXPORT_SYMBOL(ipsec_hashDigests_set);
EXPORT_SYMBOL(ipsec_espSeqNum_get);

#ifdef MTK_EIP97_IPI
static void	smp_func_call_BH_handler(unsigned long data);
static DECLARE_TASKLET( \
	smp_func_call_tsk, smp_func_call_BH_handler, 0);
static void smp_func_call(void *info);
#endif
//#define MCRYPTO_DBG

#ifdef MCRYPTO_DBG
#define ra_dbg 	printk
#else
#define ra_dbg(fmt, arg...) do {}while(0)
#endif

#ifdef MCRYPTO_DBG
static void skb_dump(struct sk_buff* sk, const char* func,int line) {
        unsigned int i;

        printk("(%d)skb_dump: [%s] with len %d (%08X) headroom=%d tailroom=%d\n",
                line,func,sk->len,(unsigned int)sk,
                skb_headroom(sk),skb_tailroom(sk));

        for(i=(unsigned int)sk->head;i<=(unsigned int)sk->data + 160;i++) {
                if((i % 16) == 0)
                        printk("\n");
                if(i==(unsigned int)sk->data) printk("{");
                //if(i==(unsigned int)sk->h.raw) ra_dbg("#");
                //if(i==(unsigned int)sk->nh.raw) ra_dbg("|");
                //if(i==(unsigned int)sk->mac.raw) ra_dbg("*");
                printk("%02x ",*((unsigned char*)i));
                if(i==(unsigned int)(sk->tail)-1) printk("}");
        }
        printk("\n");
}
#else
#define skb_dump //skb_dump
#endif
/************************************************************************
*              P R I V A T E     F U N C T I O N S
*************************************************************************
*/
/*_______________________________________________________________________
**function name: ipsec_hashDigest_preCompute
**
**description:
*   EIP93 can only use Hash Digests (not Hash keys) to do authentication!
*	This funtion is to use EIP93 to generate Hash Digests from Hash keys!
*	Only the first packet for a IPSec flow need to do this!
**parameters:
*   x -- point to the structure that stores IPSec SA information
*	currAdapterPtr -- point to the structure that stores needed info
*		for Hash Digest Pre-Compute. After Pre-Compute is done,
*		currAdapterPtr->addrsPreCompute is used to free resource.
*	digestPreComputeDir -- indicate direction for encryption or
*		decryption.
**global:
*   none
**return:
*   -EPERM, -ENOMEM -- failed: the pakcet will be dropped!
*	1 -- success
**call:
*   none
**revision:
*   1.Trey 20120209
**_______________________________________________________________________*/
static int 
ipsec_hashDigest_preCompute(
	struct xfrm_state *x, 
	ipsecEip93Adapter_t *currAdapterPtr, 
	unsigned int digestPreComputeDir
)
{
	char hashKeyName[32];
	unsigned int blkSize, blkWord, digestWord, hashKeyLen, hashKeyWord;
	unsigned int *ipad, *opad, *hashKey, *hashKeyTank;
	dma_addr_t	ipadPhyAddr, opadPhyAddr;
	unsigned int *pIDigest, *pODigest;
	unsigned int i, j;
	int errVal;
	unsigned int flags = 0;
	addrsDigestPreCompute_t* addrsPreCompute;
	int rdx = 0;
	int nTry;	
	if (x->aalg)
	{	
	strcpy(hashKeyName, x->aalg->alg_name);
	hashKeyLen = (x->aalg->alg_key_len+7)/8;
	}
	else
	{
		strcpy(hashKeyName, HASH_NULL_HMAC);
		hashKeyLen = 0;
		currAdapterPtr->isHashPreCompute = 3;
		return 1;
	}
	
	hashKeyWord = hashKeyLen >> 2;

	if (strcmp(hashKeyName, HASH_MD5_HMAC) == 0)
	{
		blkSize = 64; //bytes
		digestWord = 4; //words
	}
	else if (strcmp(hashKeyName, HASH_SHA1_HMAC) == 0)
	{
		blkSize = 64; //bytes
		digestWord = 5; //words	
	}
	else if (strcmp(hashKeyName, HASH_SHA256_HMAC) == 0)
	{
		blkSize = 64; //bytes
		digestWord = 8; //words	
	}
	else if (strcmp(hashKeyName, HASH_NULL_HMAC) == 0)
	{
		blkSize = 64; //bytes
		digestWord = 0; //words	
	}
	else
	{
		printk("\n !Unsupported Hash Algorithm by EIP93: %s! \n", hashKeyName);
		return -EPERM;
	}

#if defined (CONFIG_HWCRYPTO_MEMPOOL)
	addrsPreCompute = currAdapterPtr->addrsPreCompute;
	hashKeyTank = addrsPreCompute->hashKeyTank;
	ipad = addrsPreCompute->RecPoolHandler.addr + IPAD_OFFSET;
	ipadPhyAddr = addrsPreCompute->RecPoolHandler.phyAddr + IPAD_OFFSET;
	opad = addrsPreCompute->RecPoolHandler.addr + OPAD_OFFSET;
	opadPhyAddr = addrsPreCompute->RecPoolHandler.phyAddr + OPAD_OFFSET;
#else
	addrsPreCompute = (addrsDigestPreCompute_t *) kzalloc(sizeof(addrsDigestPreCompute_t), GFP_KERNEL);
	if (unlikely(addrsPreCompute == NULL))
	{
		printk("\n\n !!kmalloc for addrsPreCompute failed!! \n\n");
		return -ENOMEM;
	}
	currAdapterPtr->addrsPreCompute = addrsPreCompute;
	
	hashKeyTank = (unsigned int *) kzalloc(blkSize, GFP_KERNEL);
	if (unlikely(hashKeyTank == NULL))
	{
		printk("\n\n !!kmalloc for hashKeyTank failed!! \n\n");
		errVal = -ENOMEM;
		goto free_addrsPreCompute;
	}
	addrsPreCompute->hashKeyTank = hashKeyTank;
	if (in_atomic())
		flags |= GFP_ATOMIC;    // non-sleepable
	else
		flags |= GFP_KERNEL;    // sleepable
#endif
#ifdef MTK_EIP97_DRIVER
#if !defined (CONFIG_HWCRYPTO_MEMPOOL)
	addrsPreCompute->RecPoolHandler.size = RECPOOLSIZE;
	addrsPreCompute->RecPoolHandler.addr = (unsigned int *) dma_alloc_coherent(NULL, addrsPreCompute->RecPoolHandler.size, &addrsPreCompute->RecPoolHandler.phyAddr, flags);
	
	if (unlikely(addrsPreCompute->RecPoolHandler.addr == NULL))
	{
		printk("\n\n !!dma_alloc for RecPoolHandler failed!! \n\n");
		errVal = -ENOMEM;
		goto free_hashKeyTank;
	}

	ipad = addrsPreCompute->RecPoolHandler.addr + IPAD_OFFSET;
	ipadPhyAddr = addrsPreCompute->RecPoolHandler.phyAddr + IPAD_OFFSET;
	opad = addrsPreCompute->RecPoolHandler.addr + OPAD_OFFSET;
	opadPhyAddr = addrsPreCompute->RecPoolHandler.phyAddr + OPAD_OFFSET;
#endif
#endif

#ifndef MTK_EIP97_DRIVER
#if !defined (CONFIG_HWCRYPTO_MEMPOOL)	
	ipad = (unsigned int *) dma_alloc_coherent(NULL, blkSize, &ipadPhyAddr, flags);
	if (unlikely(ipad == NULL))
	{
		printk("\n\n !!dma_alloc for ipad failed!! \n\n");
		errVal = -ENOMEM;
		goto free_hashKeyTank;
	}
#endif
#endif	
	addrsPreCompute->ipadHandler.addr = (unsigned int)ipad;
	addrsPreCompute->ipadHandler.phyAddr = ipadPhyAddr;
	addrsPreCompute->blkSize = blkSize;
#ifndef MTK_EIP97_DRIVER
#if !defined (CONFIG_HWCRYPTO_MEMPOOL)
	opad = (unsigned int *) dma_alloc_coherent(NULL, blkSize, &opadPhyAddr, flags);
	if (unlikely(opad == NULL))
	{
		printk("\n\n !!dma_alloc for opad failed!! \n\n");
		errVal = -ENOMEM;
		goto free_ipad;
	}
#endif
#endif	
	addrsPreCompute->opadHandler.addr = (unsigned int)opad;
	addrsPreCompute->opadHandler.phyAddr = opadPhyAddr;	

	blkWord = blkSize >> 2;
	if (x->aalg)
	{	
	hashKey = (unsigned int *)x->aalg->alg_key;
	                                     
	if(hashKeyLen <= blkSize)
	{
		for(i = 0; i < hashKeyWord; i++)
		{
			hashKeyTank[i] = hashKey[i];
		}
		for(j = i; j < blkWord; j++)
		{
			hashKeyTank[j] = 0x0;
		}
	}
	else
	{
		// EIP93 supports md5, sha1, sha256. Their hash key length and their function output length should be the same, which are 128, 160, and 256 bits respectively! Their block size are 64 bytes which are always larger than all of their hash key length! 
		printk("\n !Unsupported hashKeyLen:%d by EIP93! \n", hashKeyLen);
		errVal = -EPERM;
		goto free_opad;
	}
	}
	else
		memset(hashKeyTank, 0, blkSize);
	
	for(i=0; i<blkWord; i++)
	{
		ipad[i] = HASH_IPAD;
		opad[i] = HASH_OPAD;
		ipad[i] ^= hashKeyTank[i];
		opad[i] ^= hashKeyTank[i];			
	}
#if defined (CONFIG_HWCRYPTO_MEMPOOL)
	pIDigest = addrsPreCompute->pIDigest;
	pODigest = addrsPreCompute->pODigest;
#else
	pIDigest = (unsigned int *) kzalloc(sizeof(unsigned int) << 3, GFP_KERNEL);
	if(pIDigest == NULL)
	{
		printk("\n\n !!kmalloc for Hash Inner Digests failed!! \n\n");
		errVal = -ENOMEM;
		goto free_opad;
	}
	addrsPreCompute->pIDigest = pIDigest;
	
	pODigest = (unsigned int *) kzalloc(sizeof(unsigned int) << 3, GFP_KERNEL);
	if(pODigest == NULL)
	{
		printk("\n\n !!kmalloc for Hash Outer Digests failed!! \n\n");
		errVal = -ENOMEM;
		goto free_pIDigest;
	}
	addrsPreCompute->pODigest = pODigest;
#endif		
	addrsPreCompute->digestWord = digestWord;

	currAdapterPtr->isHashPreCompute = 0; //pre-compute init	

	/* start pre-compute for Hash Inner Digests */
	errVal = ipsec_preComputeIn_cmdDescp_set(currAdapterPtr, digestPreComputeDir);
	if (errVal < 0)
	{
		goto free_pODigest;
	}
#ifdef MTK_EIP97_DRIVER
		if (currAdapterPtr->isEncryptOrDecrypt==CRYPTO_ENCRYPTION)
		{	
			if ((currAdapterPtr->idx&0x2)==0)
			{	
			rdx = 0;
				mcrypto_proc.dbg_pt[0]++;
			}
		else
			{
				mcrypto_proc.dbg_pt[1]++;	
				rdx = 2;
			}	
		}
		else
		{
			if ((currAdapterPtr->idx&0x2)==0)
			{	
				mcrypto_proc.dbg_pt[2]++;
			rdx = 1;	
			}
			else
			{
				mcrypto_proc.dbg_pt[3]++;
				rdx = 3;
			}	
		}	
#endif

	spin_lock(&cryptoLock[rdx]);
	nTry = 0;
	while (nTry < 3)
	{	
		if (ipsec_eip93CmdResCnt_check(rdx)==false)
			nTry++;
		else
			break;
	}
	if (nTry >= 3)
	{	
		spin_unlock(&cryptoLock[rdx]);
		return HWCRYPTO_PREPROCESS_DROP;
	}	
	{	
#ifdef MTK_EIP97_DRIVER
		ipsec_packet_put(addrsPreCompute->cmdHandler, NULL, rdx); //mtk_packet_put()
#else
		ipsec_packet_put(addrsPreCompute->cmdHandler, NULL); //mtk_packet_put()
#endif
	}
	spin_unlock(&cryptoLock[rdx]);
	
	/* start pre-compute for Hash Outer Digests */	
	errVal = ipsec_preComputeOut_cmdDescp_set(currAdapterPtr, digestPreComputeDir);
	if (errVal < 0)
	{
		goto free_pODigest;
	}
	
	spin_lock(&cryptoLock[rdx]);
	nTry = 0;
	while (nTry < 3)
	{	
		if (ipsec_eip93CmdResCnt_check(rdx)==false)
			nTry++;
		else
			break;
	}	
	if (nTry >=3)
	{
		spin_unlock(&cryptoLock[rdx]);
		return HWCRYPTO_PREPROCESS_DROP;
	}
	{		
#ifdef MTK_EIP97_DRIVER
		ipsec_packet_put(addrsPreCompute->cmdHandler, NULL, rdx); //mtk_packet_put()
#else
		ipsec_packet_put(addrsPreCompute->cmdHandler, NULL); //mtk_packet_put()
#endif
	}
	
	spin_unlock(&cryptoLock[rdx]);

	return 1; //success
	
#if defined (CONFIG_HWCRYPTO_MEMPOOL)
free_pODigest:
free_pIDigest:
free_opad:
free_ipad:
free_hashKeyTank:
free_addrsPreCompute:
#else
free_pODigest:
	kfree(pODigest);
free_pIDigest:
	kfree(pIDigest);
#ifndef MTK_EIP97_DRIVER		
free_opad:
	dma_free_coherent(NULL, blkSize, opad, opadPhyAddr);		
free_ipad:
	dma_free_coherent(NULL, blkSize, ipad, ipadPhyAddr);
#endif	
free_hashKeyTank:
	kfree(hashKeyTank);
#ifdef MTK_EIP97_DRIVER		
free_ipad:
free_opad:
#endif	
free_addrsPreCompute:
#ifdef MTK_EIP97_DRIVER	
	if (addrsPreCompute->RecPoolHandler.addr)
		dma_free_coherent(NULL, addrsPreCompute->RecPoolHandler.size, addrsPreCompute->RecPoolHandler.addr, addrsPreCompute->RecPoolHandler.phyAddr);
#endif
	kfree(addrsPreCompute);
	currAdapterPtr->addrsPreCompute = NULL;	
#endif /* CONFIG_HWCRYPTO_MEMPOOL */
	return errVal;	
}

/*_______________________________________________________________________
**function name: ipsec_cmdHandler_prepare
**
**description:
*   Prepare a command handler for a IPSec flow. This handler includes 
*	all needed information for EIP93 to do encryption/decryption.
*	Only the first packet for a IPSec flow need to do this!
**parameters:
*   x -- point to the structure that stores IPSec SA information
*	currAdapterPtr -- point to the structure that will stores the
*		command handler
*	cmdHandlerDir -- indicate direction for encryption or decryption.
**global:
*   none
**return:
*   -EPERM, -ENOMEM -- failed: the pakcet will be dropped!
*	1 -- success
**call:
*   none
**revision:
*   1.Trey 20120209
**_______________________________________________________________________*/
static int 
ipsec_cmdHandler_prepare(
	struct xfrm_state *x, 
	ipsecEip93Adapter_t *currAdapterPtr,
	unsigned int cmdHandlerDir
)
{
	int errVal;
	struct esp_data *esp = x->data;
	int padBoundary = ALIGN(crypto_aead_blocksize(esp->aead), 4);
	unsigned int padCrtlStat, keyLen;
	char nameString[32];
	unsigned int cipherAlg, cipherMode, aesKeyLen = 0, hashAlg, enHmac;
	unsigned int *cipherKey;
	unsigned int addedLen = 0;

	addedLen += 8; //for esp header	

	/* decide pad boundary */
	switch(padBoundary){
		case 1:
			padCrtlStat = 0x1;
			addedLen += 1;
			break;
		case 4:
			padCrtlStat = 0x1 << 1;
			addedLen += 4;
			break;
		case 8:
			padCrtlStat = 0x1 << 2;
			addedLen += 8;
			break;
		case 16:
			padCrtlStat = 0x1 << 3;
			addedLen += 16;
			break;
		case 32:
			padCrtlStat = 0x1 << 4;
			addedLen += 32;
			break;
		case 64:
			padCrtlStat = 0x1 << 5;
			addedLen += 64;
			break;
		case 128:
			padCrtlStat = 0x1 << 6;
			addedLen += 128;
			break;
		case 256:
			padCrtlStat = 0x1 << 7;
			addedLen += 256;
			break;
		default:
			printk("\n !Unsupported pad boundary (%d) by EIP93! \n", padBoundary);
			errVal = -EPERM;
			goto free_addrsPreComputes;
	}
	
	
	/* decide cipher */
	strcpy(nameString, x->ealg->alg_name);

	keyLen = (x->ealg->alg_key_len+7)/8;
	if(strcmp(nameString, CIPHER_DES_CBC) == 0)
	{
		cipherAlg = 0x0; //des
		cipherMode = 0x1; //cbc
		addedLen += (8 + (8 + 1)); //iv + (esp trailer + padding)
	}
	else if(strcmp(nameString, CIPHER_3DES_CBC) == 0)
	{
		cipherAlg = 0x1; //3des
		cipherMode = 0x1; //cbc
		addedLen += (8 + (8 + 1)); //iv + (esp trailer + padding)
	}
	else if(strcmp(nameString, CIPHER_AES_CBC) == 0)
	{
		cipherAlg = 0x3; //aes
		cipherMode = 0x1; //cbc
		addedLen += (16 + (16 + 1)); //iv + (esp trailer + padding)

		switch(keyLen << 3) //keyLen*8
		{ 
			case 128:
				aesKeyLen = 0x2;
				break;
			case 192:
				aesKeyLen = 0x3;
				break;
			case 256:
				aesKeyLen = 0x4;
				break;
			default:
				printk("\n !Unsupported AES key length (%d) by EIP93! \n", keyLen << 3);
				errVal = -EPERM;
				goto free_addrsPreComputes;
		}
	}
	else if(strcmp(nameString, CIPHER_NULL_ECB) == 0)
	{
		cipherAlg = 0xf; //null
		cipherMode = 0x0; //ecb
		addedLen += (8 + (16 + 1) + 16); //iv + (esp trailer + padding) + ICV
	}
	else
	{
		printk("\n !Unsupported Cipher Algorithm (%s) by EIP93! \n", nameString);
		errVal = -EPERM;
		goto free_addrsPreComputes;
	}

	
	/* decide hash */
	if (x->aalg==NULL)
		strcpy(nameString, HASH_NULL_HMAC);	
	else
	strcpy(nameString, x->aalg->alg_name);

	if(strcmp(nameString, HASH_MD5_HMAC) == 0)
	{
		hashAlg = 0x0; //md5
		enHmac = 0x1; //hmac
		addedLen += 12; //ICV
	}
	else if(strcmp(nameString, HASH_SHA1_HMAC) == 0)
	{
		hashAlg = 0x1; //sha1
		enHmac = 0x1; //hmac
		addedLen += 12; //ICV
	}
	else if(strcmp(nameString, HASH_SHA256_HMAC) == 0)
	{
		hashAlg = 0x3; //sha256
		enHmac = 0x1; //hmac
		addedLen += 16; //ICV
	}
	else if(strcmp(nameString, HASH_NULL_HMAC) == 0)
	{
		hashAlg = 0xf; //null
		enHmac = 0x0;//0x1; //hmac
	}
	else
	{
		printk("\n !Unsupported! Hash Algorithm (%s) by EIP93! \n", nameString);
		errVal = -EPERM;
		goto free_addrsPreComputes;
	}

	cipherKey =	(unsigned int *)x->ealg->alg_key;
	currAdapterPtr->addedLen = addedLen;
	errVal = ipsec_cmdHandler_cmdDescp_set(currAdapterPtr, cmdHandlerDir, cipherAlg, hashAlg, \
			crypto_aead_authsize(esp->aead)/sizeof(unsigned int), cipherMode, enHmac, aesKeyLen, \
			cipherKey, keyLen, x->id.spi, padCrtlStat);
	if (errVal < 0)
	{
		goto free_addrsPreComputes;
	}

	return 1; //success

free_addrsPreComputes:
#if !defined (CONFIG_HWCRYPTO_MEMPOOL)
	ipsec_addrsDigestPreCompute_free(currAdapterPtr);
#endif
	return errVal;
}

static int 
ipsec_esp_preProcess(
	struct xfrm_state *x, 
	struct sk_buff *skb,
	unsigned int direction
)
{
	ipsecEip93Adapter_t **ipsecEip93AdapterList;
	unsigned int i, usedEntryNum = 0;
	ipsecEip93Adapter_t *currAdapterPtr;
	unsigned int spi = x->id.spi;
	int currAdapterIdx = -1;
	int err = 1;
	unsigned int *addrCurrAdapter;
	unsigned long flags;
#if defined (CONFIG_HWCRYPTO_MEMPOOL)
	unsigned int* spi_tbl;
#endif

	if (direction == HASH_DIGEST_OUT)
	{
		spin_lock(&ipsec_adapters_outlock);
		ipsecEip93AdapterList = &ipsecEip93AdapterListOut[0];
#if defined (CONFIG_HWCRYPTO_MEMPOOL)
		spi_tbl = spi_outbound_tbl;
#endif
	}
	else
	{
		spin_lock(&ipsec_adapters_inlock);
		ipsecEip93AdapterList = &ipsecEip93AdapterListIn[0];
#if defined (CONFIG_HWCRYPTO_MEMPOOL)
		spi_tbl = spi_inbound_tbl;
#endif
	}

	//try to find the matched ipsecEip93Adapter for the ipsec flow
	for (i = 0; i < IPESC_EIP93_ADAPTERS; i++)
	{
#if defined (CONFIG_HWCRYPTO_MEMPOOL)
		if (spi_tbl[i]!=0xFFFFFFFF)
		{
			if (spi_tbl[i]==spi)
			{
				currAdapterPtr = ipsecEip93AdapterList[i];
				if (currAdapterPtr->status != TBL_ACTIVE)
				{
					printk("Drop packet for Conn[%d] status=%x\n",i, currAdapterPtr->status);
					kfree_skb(skb);
					if (direction == HASH_DIGEST_OUT)
						spin_unlock(&ipsec_adapters_outlock);
					else
						spin_unlock(&ipsec_adapters_inlock);
					err = HWCRYPTO_PREPROCESS_DROP;	
					goto EXIT;
				}	
				currAdapterIdx = i;
				break;
			}
			usedEntryNum++;
		}
#else
		if ((currAdapterPtr = ipsecEip93AdapterList[i]) != NULL)
		{
			if (currAdapterPtr->spi == spi)
			{
				currAdapterIdx = i;
				break;
			}
			usedEntryNum++;
		}
#endif
		else
		{	//try to record the first unused entry in ipsecEip93AdapterList
			if (currAdapterIdx == -1)
			{
				currAdapterIdx = i;
			}
		}
	}
	
	if (usedEntryNum == IPESC_EIP93_ADAPTERS)
	{
		printk("\n\n !The ipsecEip93AdapterList (dir=%d) table is full!(%d) (spi=%08X)\n\n",direction,usedEntryNum,spi);
		err = -EPERM;
		if (direction == HASH_DIGEST_OUT)
			spin_unlock(&ipsec_adapters_outlock);
		else
			spin_unlock(&ipsec_adapters_inlock);
		goto EXIT;
	}

	//no ipsecEip93Adapter matched, so create a new one for the ipsec flow. \
	//Only the first packet of a ipsec flow will encounter this.
	if (i == IPESC_EIP93_ADAPTERS)
	{
		if (x->aalg == NULL)
		{
			//printk("\n\n !please set a hash algorithm! \n\n");
			//err = -EPERM;
			//goto EXIT;
		}
		else if (x->ealg == NULL)
		{
			printk("\n\n !please set a cipher algorithm! \n\n");
			if (direction == HASH_DIGEST_OUT)
				spin_unlock(&ipsec_adapters_outlock);
			else
				spin_unlock(&ipsec_adapters_inlock);
			err = -EPERM;
			goto EXIT;
		}
#if defined (CONFIG_HWCRYPTO_MEMPOOL)
		if ((currAdapterIdx >=0) && (currAdapterIdx<IPESC_EIP93_ADAPTERS))
			currAdapterPtr = ipsecEip93AdapterList[currAdapterIdx];
		else	
			currAdapterPtr = NULL;

		if ((currAdapterPtr == NULL) || (currAdapterIdx==-1))
#else	
		currAdapterPtr = (ipsecEip93Adapter_t *) kzalloc(sizeof(ipsecEip93Adapter_t), GFP_KERNEL);	
		if(currAdapterPtr == NULL)
#endif
		{
			printk("\n\n !!kmalloc for new ipsecEip93Adapter failed index=%d, used entry=%d %08X!! \n\n", currAdapterIdx,usedEntryNum,ipsecEip93AdapterList[currAdapterIdx]);
			if (direction == HASH_DIGEST_OUT)
				spin_unlock(&ipsec_adapters_outlock);
			else
				spin_unlock(&ipsec_adapters_inlock);
			err = -ENOMEM;
			goto EXIT;
		}
		
		spin_lock_init(&currAdapterPtr->lock);
		spin_lock_init(&currAdapterPtr->seqlock);
		skb_queue_head_init(&currAdapterPtr->skbQueue);
#ifdef MTK_EIP97_IPI
		skb_queue_head_init(&currAdapterPtr->skbIPIQueue);		
#endif	
#if defined (CONFIG_HWCRYPTO_MEMPOOL)
		spi_tbl[currAdapterIdx] = spi;	
#endif
		currAdapterPtr->status = TBL_ACTIVE;
		currAdapterPtr->spi = spi;
		currAdapterPtr->x = x;
		currAdapterPtr->dst = skb_dst(skb);
		currAdapterPtr->idx = currAdapterIdx;
		if (x->props.mode == XFRM_MODE_TUNNEL)
			currAdapterPtr->tunnel = 1;
		else
			currAdapterPtr->tunnel = 0;

		if (direction == HASH_DIGEST_IN)
		{	
				currAdapterPtr->seqno_in = 0;
				currAdapterPtr->isEncryptOrDecrypt = CRYPTO_DECRYPTION;
		}
		else
		{
				currAdapterPtr->seqno_out = 0;	
				currAdapterPtr->isEncryptOrDecrypt = CRYPTO_ENCRYPTION;
		}
		err = ipsec_hashDigest_preCompute(x, currAdapterPtr, direction);
		if (err < 0)
		{
			ra_dbg("\n\n !ipsec_hashDigest_preCompute for direction:%d failed! \n\n", direction);
#if !defined (CONFIG_HWCRYPTO_MEMPOOL)			
			kfree(currAdapterPtr);
#endif
			if (direction == HASH_DIGEST_OUT)
				spin_unlock(&ipsec_adapters_outlock);
			else
				spin_unlock(&ipsec_adapters_inlock);
			goto EXIT;
		}
		if (err == HWCRYPTO_PREPROCESS_DROP)
		{
			if (direction == HASH_DIGEST_OUT)
				spin_unlock(&ipsec_adapters_outlock);
			else
				spin_unlock(&ipsec_adapters_inlock);
			goto EXIT;
		}
		err = ipsec_cmdHandler_prepare(x, currAdapterPtr, direction);
		if (err < 0)
		{
			printk("\n\n !ipsec_cmdHandler_prepare for direction:%d failed! \n\n", direction);
#if !defined (CONFIG_HWCRYPTO_MEMPOOL)			
			kfree(currAdapterPtr);
#endif			
			if (direction == HASH_DIGEST_OUT)
				spin_unlock(&ipsec_adapters_outlock);
			else
				spin_unlock(&ipsec_adapters_inlock);
			goto EXIT;
		}
#if !defined (CONFIG_HWCRYPTO_MEMPOOL)
		ipsecEip93AdapterList[currAdapterIdx] = currAdapterPtr;	
#endif
	}

	if (direction == HASH_DIGEST_OUT)
		spin_unlock(&ipsec_adapters_outlock);
	else
		spin_unlock(&ipsec_adapters_inlock);
	
	currAdapterPtr = ipsecEip93AdapterList[currAdapterIdx];

#if !defined (FEATURE_AVOID_QUEUE_PACKET)
	//Hash Digests are ready
	spin_lock(&currAdapterPtr->lock);
	if (currAdapterPtr->isHashPreCompute == 2)
	{	 		
		ipsec_hashDigests_get(currAdapterPtr);
		currAdapterPtr->isHashPreCompute = 3; //pre-compute done
#if !defined (CONFIG_HWCRYPTO_MEMPOOL)
		ipsec_addrsDigestPreCompute_free(currAdapterPtr);	
#endif
	}
	spin_unlock(&currAdapterPtr->lock);
#endif
	//save needed info skb (cryptoDriver will save skb in EIP93's userID), so the needed \
	//info can be used by the tasklet which is raised by interrupt.
	addrCurrAdapter = (unsigned int *) &(skb->cb[36]);
	*addrCurrAdapter = (unsigned int)currAdapterPtr;

EXIT:
	return err;

	
}

static int 
ipsec_esp_pktPut(
	ipsecEip93Adapter_t *currAdapterPtr,
	struct sk_buff *skb
)
{
#ifdef MTK_EIP97_DRIVER
	eip97DescpHandler_t *cmdHandler;
#else	
	eip93DescpHandler_t *cmdHandler;
#endif	
	struct sk_buff *pSkb;
	unsigned int isQueueFull = 0;
	unsigned int addedLen;
	struct sk_buff *skb2 = NULL;
	struct dst_entry *dst;
	unsigned int *addrCurrAdapter;
	unsigned int flags;
	
	if (currAdapterPtr!=NULL)
	{
		//spin_lock(&currAdapterPtr->lock);
		cmdHandler = currAdapterPtr->cmdHandler;
		addedLen = currAdapterPtr->addedLen;
		goto DEQUEUE;
	}		

	dst = skb_dst(skb);
	addrCurrAdapter = (unsigned int *) &(skb->cb[36]);
	currAdapterPtr = (ipsecEip93Adapter_t *)(*addrCurrAdapter);

	//spin_lock(&currAdapterPtr->lock);
	cmdHandler = currAdapterPtr->cmdHandler;
	addedLen = currAdapterPtr->addedLen;

	//resemble paged packets if needed
	if (skb_is_nonlinear(skb)) 
	{
		ra_dbg("skb should linearize\n");
		mcrypto_proc.nolinear_count++;
		if (skb_linearize(skb) != 0)
		{
			printk("\n !resembling paged packets failed! \n");
			spin_unlock(&currAdapterPtr->lock);
			return -EPERM;
		}
		
		//skb_linearize() may return a new skb, so insert currAdapterPtr back to skb!
		addrCurrAdapter = (unsigned int *) &(skb->cb[36]);
		*addrCurrAdapter = (unsigned int)currAdapterPtr;
	}

	//make sure that tailroom is enough for the added length due to encryption
	if (currAdapterPtr->isEncryptOrDecrypt==CRYPTO_ENCRYPTION)
	{	
		if (skb_tailroom(skb) < addedLen)
		{
		skb2 = skb_copy_expand(skb, skb_headroom(skb), addedLen, GFP_ATOMIC);
	
			kfree_skb(skb); //free old skb
	
			if (skb2 == NULL)
			{
				printk("\n !skb_copy_expand failed! \n");
				//spin_unlock(&currAdapterPtr->lock);
				return -EPERM;
			}
			
			skb = skb2; //the new skb
			skb_dst_set(skb, dst_clone(dst));
			//skb_dst_set(skb, dst);
			addrCurrAdapter = (unsigned int *) &(skb->cb[36]);
			*addrCurrAdapter = (unsigned int)currAdapterPtr;
			
			mcrypto_proc.copy_expand_count++;
		}
	}

	if (currAdapterPtr->skbQueue.qlen < SKB_QUEUE_MAX_SIZE)
	{
		skb_queue_tail(&currAdapterPtr->skbQueue, skb);	
	}
	else
	{
		isQueueFull = 1;
	}
DEQUEUE:
	//ipsec_BH_handler_resultGet has no chance to set isHashPreCompute as 3, \
	//so currAdapterPtr->lock is not needed here!
	if (currAdapterPtr->isHashPreCompute == 3) //pre-compute done	
	{		
		int rdx = 0;
	
#ifdef MTK_EIP97_DRIVER
		if (currAdapterPtr->isEncryptOrDecrypt==CRYPTO_ENCRYPTION)
		{	
			if ((currAdapterPtr->idx&0x2)==0)
			{	
			rdx = 0;
				mcrypto_proc.dbg_pt[0]++;
			}
		else
			{
				mcrypto_proc.dbg_pt[1]++;	
				rdx = 2;
			}	
		}
		else
		{
			if ((currAdapterPtr->idx&0x2)==0)
			{	
				mcrypto_proc.dbg_pt[2]++;
			rdx = 1;	
			}
			else
			{
				mcrypto_proc.dbg_pt[3]++;
				rdx = 3;
			}	
		}	
#endif
		do
		{
			spin_lock(&cryptoLock[rdx]);
			if (ipsec_eip93CmdResCnt_check(rdx)==false) {
				spin_unlock(&cryptoLock[rdx]);
				break;
			}
			pSkb = skb_dequeue(&currAdapterPtr->skbQueue);
			if (pSkb==NULL)	{
				spin_unlock(&cryptoLock[rdx]);
				break;
			}
#ifdef MTK_EIP97_DRIVER
			ipsec_packet_put(cmdHandler, pSkb, rdx); //mtk_packet_put
#else
			ipsec_packet_put(cmdHandler, pSkb); //mtk_packet_put
#endif
			spin_unlock(&cryptoLock[rdx]);

		}while (1);

		if (isQueueFull && (currAdapterPtr->skbQueue.qlen < SKB_QUEUE_MAX_SIZE))
		{
			isQueueFull = 0;
			if (skb)
				skb_queue_tail(&currAdapterPtr->skbQueue, skb);
		}
	}

	if (isQueueFull == 0)
	{
		//spin_unlock(&currAdapterPtr->lock);

		return HWCRYPTO_OK; //success
	}
	else
	{
		ra_dbg("-ENOMEM qlen=%d\n",currAdapterPtr->skbQueue.qlen);
		mcrypto_proc.oom_in_put++;
		if(skb2)
		{	
			kfree_skb(skb2);
			//spin_unlock(&currAdapterPtr->lock);
			return HWCRYPTO_NOMEM;
		}
		else
		{
			//spin_unlock(&currAdapterPtr->lock);
			return -ENOMEM; //drop the packet
		}
}
}

/*_______________________________________________________________________
**function name: ipsec_esp_output_finish
**
**description:
*   Deal with the rest of Linux Kernel's esp_output(). Then,
*	the encrypted packet can do the correct post-routing.
**parameters:
*   resHandler -- point to the result descriptor handler that stores
*		the needed info comming from EIP93's Result Descriptor Ring.
**global:
*   none
**return:
*   none
**call:
*   ip_output()
**revision:
*   1.Trey 20120209
**_______________________________________________________________________*/
static void 
ipsec_esp_output_finish(
	void *resHandler_ptr
)
{
#if defined (MTK_EIP97_DRIVER)	
	eip97DescpHandler_t *resHandler = resHandler_ptr;
#else
	eip93DescpHandler_t *resHandler = resHandler_ptr;
#endif
	struct sk_buff *skb;
	struct iphdr *top_iph;
	unsigned int length;
	struct dst_entry *dst;
	struct xfrm_state *x;
	ipsecEip93Adapter_t *currAdapterPtr;
	unsigned int *addrCurrAdapter;
	struct net *net;
	int err;
	struct ip_esp_hdr *esph;
		
	skb = (struct sk_buff *) ipsec_eip93UserId_get(resHandler);
	if (skb==NULL)
	{	
		printk("UserId got NULL skb in %s\n",__func__);
		return;
	}
	addrCurrAdapter = (unsigned int *) &(skb->cb[36]);
	currAdapterPtr = (ipsecEip93Adapter_t *)(*addrCurrAdapter);
#if defined (CONFIG_HWCRYPTO_MEMPOOL)	
	if (currAdapterPtr->status==TBL_DEL)
	{	
		kfree_skb(skb);
		return;
	}
#endif
	top_iph = ip_hdr(skb);
	dst = skb_dst(skb);
	if (dst==NULL)
	{	
		printk("dst got NULL in %s\n",__func__);
		dst = currAdapterPtr->dst;
		if (dst==NULL)
		{	
			printk("currAdapterPtr->dst got NULL skb in %s\n",__func__);
			kfree_skb(skb);
			return;
		}	
		x = currAdapterPtr->x;
	}
	else
		x = dst->xfrm;
	if (x==NULL)
	{	
		
		printk("xfrm got NULL x in %s\n",__func__);
		x = currAdapterPtr->x;
		if (x==NULL)
		{	
			printk("currAdapterPtr->xfrm got NULL x in %s\n",__func__);
			kfree_skb(skb);
			return;
		}	
	}
	length = ipsec_pktLength_get(resHandler);
	{
		struct ip_esp_hdr *esph_seq = skb->data;
		static struct sk_buff *skb_old = NULL;
		static unsigned char* ptr = NULL;
		//spin_lock(&currAdapterPtr->lock);
		if ((currAdapterPtr->seqno_out+1) != ntohl(esph_seq->seq_no))
		{	
			if (ptr)
				printk("seqno_out[%d]: length=%d %08X %08X [%08X %08X][%08X %08X],txpacket=%d\n", \
						currAdapterPtr->idx, length, currAdapterPtr->seqno_out, \
						ntohl(esph_seq->seq_no),skb_old, skb,ptr,skb->data,mcrypto_proc.dbg_pt[8]);	
			else
				printk("seqno_out[%d]: length=%d %08X %08X, txpacket=%d\n",	\
						currAdapterPtr->idx, length, currAdapterPtr->seqno_out, \
						ntohl(esph_seq->seq_no),mcrypto_proc.dbg_pt[8]);

			mcrypto_proc.dbg_pt[15]++;
		}
		currAdapterPtr->seqno_out = ntohl(esph_seq->seq_no);	
		skb_old = skb;
		ptr = skb->data;	
		//spin_unlock(&currAdapterPtr->lock);
	}		

	net = xs_net(x);
	esph = ip_esp_hdr(skb);
	
	
	skb_put(skb, length - skb->len); //adjust skb->tail

	length += skb->data - skb_network_header(skb); //IP total length
	
	
	__skb_push(skb, -skb_network_offset(skb));
#ifdef RALINK_HWCRYPTO_NAT_T
	//if (x->encap)
	//	skb_push(skb, 8);
#endif		

	esph = ip_esp_hdr(skb);
	*skb_mac_header(skb) = IPPROTO_ESP;	      
#ifdef RALINK_HWCRYPTO_NAT_T
	if (x->encap) {
		struct xfrm_encap_tmpl *encap = x->encap;
		struct udphdr *uh;
		__be32 *udpdata32;
		__be16 sport, dport;
		int encap_type;

		sport = encap->encap_sport;
		dport = encap->encap_dport;
		encap_type = encap->encap_type;

		uh = (struct udphdr *)esph;
		uh->source = sport;
		uh->dest = dport;
		uh->len = htons(skb->len - skb_transport_offset(skb));
		uh->check = 0;
	
		switch (encap_type) {
		default:
		case UDP_ENCAP_ESPINUDP:
			esph = (struct ip_esp_hdr *)(uh + 1);
			break;
		case UDP_ENCAP_ESPINUDP_NON_IKE:
			udpdata32 = (__be32 *)(uh + 1);
			udpdata32[0] = udpdata32[1] = 0;
			esph = (struct ip_esp_hdr *)(udpdata32 + 2);
			break;
		}

		*skb_mac_header(skb) = IPPROTO_UDP;
		//__skb_push(skb, -skb_network_offset(skb));
	}
#endif

	top_iph->tot_len = htons(length);
	ip_send_check(top_iph);
#ifdef 	RALINK_ESP_OUTPUT_POLLING	
	goto out;
#endif	
	/* adjust for IPSec post-routing */
	dst = skb_dst_pop(skb);
	if (!dst) {
		XFRM_INC_STATS(net, LINUX_MIB_XFRMOUTERROR);
		err = -EHOSTUNREACH;
		printk("(%d)ipsec_esp_output_finish EHOSTUNREACH\n",__LINE__);
		kfree_skb(skb);
		return;
	}
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,36)	
	skb_dst_set(skb, dst_clone(dst));
#else
	skb_dst_set(skb, dst);
#endif	

	if (skb_dst(skb)->xfrm)
	{
		x = dst->xfrm;
		if (x->type->proto==IPPROTO_AH)
		{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,36)
			extern int xfrm_skb_check_space(struct sk_buff *skb);
			err = xfrm_skb_check_space(skb);
#else
			extern int xfrm_state_check_space(struct xfrm_state *x, struct sk_buff *skb);
			err = xfrm_state_check_space(x, skb);
#endif			
			if (err) {
				XFRM_INC_STATS(net, LINUX_MIB_XFRMOUTERROR);
				printk("(%d)ipsec_esp_output_finish LINUX_MIB_XFRMOUTERROR\n",__LINE__);
				kfree_skb(skb);
				return;	
			}
	
			err = x->outer_mode->output(x, skb);
			if (err) {
				XFRM_INC_STATS(net, LINUX_MIB_XFRMOUTSTATEMODEERROR);
				printk("(%d)ipsec_esp_output_finish LINUX_MIB_XFRMOUTSTATEMODEERROR\n",__LINE__);
				kfree_skb(skb);
				return;	
			}
	
			spin_lock_bh(&x->lock);
			
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,36)
			if (unlikely(x->km.state != XFRM_STATE_VALID)) {
				XFRM_INC_STATS(net, LINUX_MIB_XFRMOUTSTATEINVALID);
				spin_unlock_bh(&x->lock);
				printk("(%d)ipsec_esp_output_finish LINUX_MIB_XFRMOUTSTATEINVALID\n",__LINE__);
				kfree_skb(skb);
				return;	
			}
#endif
			err = xfrm_state_check_expire(x);
			if (err) {
				XFRM_INC_STATS(net, LINUX_MIB_XFRMOUTSTATEEXPIRED);
				spin_unlock_bh(&x->lock);
				printk("(%d)ipsec_esp_output_finish LINUX_MIB_XFRMOUTSTATEEXPIRED\n",__LINE__);
				kfree_skb(skb);
				return;	
			}

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,36)
			err = x->repl->overflow(x, skb);
			if (err) {
				XFRM_INC_STATS(net, LINUX_MIB_XFRMOUTSTATESEQERROR);
				printk("(%d)ipsec_esp_output_finish LINUX_MIB_XFRMOUTSTATESEQERROR\n",__LINE__);
				kfree_skb(skb);
				return;
			}
#else	
			if (x->type->flags & XFRM_TYPE_REPLAY_PROT) {
				XFRM_SKB_CB(skb)->seq.output = ++x->replay.oseq;
				if (unlikely(x->replay.oseq == 0)) {
					XFRM_INC_STATS(net, LINUX_MIB_XFRMOUTSTATESEQERROR);
					x->replay.oseq--;
					xfrm_audit_state_replay_overflow(x, skb);
					err = -EOVERFLOW;
					spin_unlock_bh(&x->lock);
					printk("(%d)ipsec_esp_output_finish -EOVERFLOW\n",__LINE__);
					kfree_skb(skb);
					return;
				}
				if (xfrm_aevent_is_on(net))
					xfrm_replay_notify(x, XFRM_REPLAY_UPDATE);
			}
#endif	
			x->curlft.bytes += skb->len;
			x->curlft.packets++;
			spin_unlock_bh(&x->lock);
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,36)			
			skb_dst_force(skb);
#endif			
			err = x->type->output(x, skb);
			top_iph = ip_hdr(skb);
			ip_send_check(top_iph);
			dst = skb_dst_pop(skb);
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,36)	
			skb_dst_set(skb, dst_clone(dst));
#else
			skb_dst_set(skb, dst);
#endif
		}	
	}

	nf_reset(skb);

	if (!skb_dst(skb)->xfrm)
	{
		mcrypto_proc.dbg_pt[8]++;
		ip_output(skb);
		return;
	}
		      
	return;
}

static void 
ipsec_esp6_output_finish(
	eip93DescpHandler_t *resHandler
)
{
	struct sk_buff *skb = (struct sk_buff *) ipsec_eip93UserId_get(resHandler);
	struct ipv6hdr *top_iph = ipv6_hdr(skb);
	unsigned int length;
	struct dst_entry *dst = skb_dst(skb);
	struct xfrm_state *x = dst->xfrm;
	struct net *net = xs_net(x);
	int err;

	length = ipsec_pktLength_get(resHandler);

	skb_put(skb, length - skb->len); //adjust skb->tail

	__skb_push(skb, -skb_network_offset(skb));

	*skb_mac_header(skb) = IPPROTO_ESP;	      

	top_iph->payload_len = htons(length);

	/* adjust for IPSec post-routing */
	dst = skb_dst_pop(skb);
	
	if (!dst) {
		XFRM_INC_STATS(net, LINUX_MIB_XFRMOUTERROR);
		err = -EHOSTUNREACH;
		printk("(%d)ipsec_esp_output_finish EHOSTUNREACH\n",__LINE__);
		kfree_skb(skb);
		return;
	}
	
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,36)	
	skb_dst_set(skb, dst_clone(dst));
#else
	skb_dst_set(skb, dst);
#endif

	if (skb_dst(skb)->xfrm)
	{
		x = dst->xfrm;
		if (x->type->proto==IPPROTO_AH)
		{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,36)
			extern int xfrm_skb_check_space(struct sk_buff *skb);
			err = xfrm_skb_check_space(skb);
#else
			extern int xfrm_state_check_space(struct xfrm_state *x, struct sk_buff *skb);
			err = xfrm_state_check_space(x, skb);
#endif
			if (err) {
				XFRM_INC_STATS(net, LINUX_MIB_XFRMOUTERROR);
				printk("(%d)ipsec_esp_output_finish LINUX_MIB_XFRMOUTERROR\n",__LINE__);
				kfree_skb(skb);
				return;	
			}
	
			err = x->outer_mode->output(x, skb);
			if (err) {
				XFRM_INC_STATS(net, LINUX_MIB_XFRMOUTSTATEMODEERROR);
				printk("(%d)ipsec_esp_output_finish LINUX_MIB_XFRMOUTSTATEMODEERROR\n",__LINE__);
				kfree_skb(skb);
				return;	
			}
	
			spin_lock_bh(&x->lock);
			err = xfrm_state_check_expire(x);
			if (err) {
				XFRM_INC_STATS(net, LINUX_MIB_XFRMOUTSTATEEXPIRED);
				spin_unlock_bh(&x->lock);
				printk("(%d)ipsec_esp_output_finish LINUX_MIB_XFRMOUTSTATEEXPIRED\n",__LINE__);
				kfree_skb(skb);
				return;	
			}

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,36)
			err = x->repl->overflow(x, skb);
			if (err) {
				XFRM_INC_STATS(net, LINUX_MIB_XFRMOUTSTATESEQERROR);
				printk("(%d)ipsec_esp_output_finish LINUX_MIB_XFRMOUTSTATESEQERROR\n",__LINE__);
				kfree_skb(skb);
				return;
			}
#else	
			if (x->type->flags & XFRM_TYPE_REPLAY_PROT) {
				XFRM_SKB_CB(skb)->seq.output = ++x->replay.oseq;
				if (unlikely(x->replay.oseq == 0)) {
					XFRM_INC_STATS(net, LINUX_MIB_XFRMOUTSTATESEQERROR);
					x->replay.oseq--;
					xfrm_audit_state_replay_overflow(x, skb);
					err = -EOVERFLOW;
					spin_unlock_bh(&x->lock);
					printk("(%d)ipsec_esp_output_finish -EOVERFLOW\n",__LINE__);
					kfree_skb(skb);
					return;
				}
				if (xfrm_aevent_is_on(net))
					xfrm_replay_notify(x, XFRM_REPLAY_UPDATE);
			}
#endif
	
			x->curlft.bytes += skb->len;
			x->curlft.packets++;
			spin_unlock_bh(&x->lock);
			err = x->type->output(x, skb);
			dst = skb_dst_pop(skb);
	#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,36)	
			skb_dst_set(skb, dst_clone(dst));
	#else
			skb_dst_set(skb, dst);
	#endif
		}
	}

	nf_reset(skb);

	if (!skb_dst(skb)->xfrm)
	{
		mcrypto_proc.dbg_pt[8]++;
		dst_output(skb);
		return;
	}

	return;
}

/*_______________________________________________________________________
**function name: ipsec_esp_input_finish
**
**description:
*   Deal with the rest of Linux Kernel's esp_input(). Then,
*	the decrypted packet can do the correct post-routing.
**parameters:
*   resHandler -- point to the result descriptor handler that stores
*		the needed info comming from EIP93's Result Descriptor Ring.
*   x -- point to the structure that stores IPSec SA information
**global:
*   none
**return:
*   none
**call:
*   netif_rx() for tunnel mode, or xfrm4_rcv_encap_finish() for transport
*		mode.
**revision:
*   1.Trey 20120209
**_______________________________________________________________________*/
static void 
ipsec_esp_input_finish(
	void *resHandler_ptr, 
	struct xfrm_state *x
)
{
#if defined (MTK_EIP97_DRIVER)	
	eip97DescpHandler_t *resHandler = resHandler_ptr;
#else
	eip93DescpHandler_t *resHandler = resHandler_ptr;
#endif	
	struct sk_buff *skb = (struct sk_buff *) ipsec_eip93UserId_get(resHandler);
	struct iphdr *iph;
	unsigned int ihl, pktLen;
	struct esp_data *esp = x->data;
	int decaps = 0;
	__be32 spi, seq;
	int err;
	struct net *net = dev_net(skb->dev);
	int nexthdr = 0;
	struct xfrm_mode *inner_mode = x->inner_mode;
	int async = 0;
	struct ip_esp_hdr *esph = (struct ip_esp_hdr *)skb->data;	
	ipsecEip93Adapter_t *currAdapterPtr;
	unsigned int *addrCurrAdapter;

	addrCurrAdapter = (unsigned int *) &(skb->cb[36]);
	currAdapterPtr = (ipsecEip93Adapter_t *)(*addrCurrAdapter);
#if defined (CONFIG_HWCRYPTO_MEMPOOL)
	if (currAdapterPtr->status==TBL_DEL)
	{	
		kfree_skb(skb);
		return;
	}
#endif
	spi = currAdapterPtr->spi;

#if defined (MTK_EIP97_DRIVER)
	seq = resHandler->seq_no;
#else
	esph->seq_no = htonl(ipsec_espSeqNum_get(resHandler));
	seq = esph->seq_no;
	esph->spi = spi;
#endif
	skb->ip_summed = CHECKSUM_NONE;	
	iph = ip_hdr(skb);
	ihl = iph->ihl << 2;
	if (x->props.mode == XFRM_MODE_TRANSPORT)
	{	
		iph->protocol = ipsec_espNextHeader_get(resHandler);
		nexthdr = iph->protocol;
	}
	else	
		nexthdr	= ipsec_espNextHeader_get(resHandler);

	//adjest skb->tail & skb->len
	pktLen = ipsec_pktLength_get(resHandler);
	//*(skb->data-20+9) = 0x32;
#ifdef RALINK_HWCRYPTO_NAT_T	
	if (x->encap) {
		struct xfrm_encap_tmpl *encap = x->encap;
		struct udphdr *uh = (void *)(skb_network_header(skb) + ihl);

		/*
		 * 1) if the NAT-T peer's IP or port changed then
		 *    advertize the change to the keying daemon.
		 *    This is an inbound SA, so just compare
		 *    SRC ports.
		 */
		if (iph->saddr != x->props.saddr.a4 ||
		    uh->source != encap->encap_sport) {
			xfrm_address_t ipaddr;

			ipaddr.a4 = iph->saddr;
			km_new_mapping(x, &ipaddr, uh->source);

			/* XXX: perhaps add an extra
			 * policy check here, to see
			 * if we should allow or
			 * reject a packet from a
			 * different source
			 * address/port.
			 */
		}

		/*
		 * 2) ignore UDP/TCP checksums in case
		 *    of NAT-T in Transport Mode, or
		 *    perform other post-processing fixes
		 *    as per draft-ietf-ipsec-udp-encaps-06,
		 *    section 3.1.2
		 */
		if (x->props.mode == XFRM_MODE_TRANSPORT)
			skb->ip_summed = CHECKSUM_UNNECESSARY;
	}
#endif
	skb->len = pktLen;
	skb_set_tail_pointer(skb, pktLen);
#if !defined (MTK_EIP97_DRIVER)
	__skb_pull(skb, crypto_aead_ivsize(esp->aead));
#endif	
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,14)	
	skb_set_transport_header(skb, -ihl);
#else	
	if (x->props.mode == XFRM_MODE_TUNNEL)
		skb_reset_transport_header(skb);
	else
		skb_set_transport_header(skb, -ihl);
#endif		

	/* adjust for IPSec post-routing */
	spin_lock(&x->lock);
	if (nexthdr <= 0) {
		if (nexthdr == -EBADMSG) {
			xfrm_audit_state_icvfail(x, skb, x->type->proto);
			x->stats.integrity_failed++;
		}
		XFRM_INC_STATS(net, LINUX_MIB_XFRMINSTATEPROTOERROR);
		printk("(%d)ipsec_esp_input_finish LINUX_MIB_XFRMINSTATEPROTOERROR nexthdr=%x\n",__LINE__,nexthdr);
		spin_unlock(&x->lock);
		goto drop;
	}

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,36)
	if (x->props.replay_window)
			xfrm_replay_advance(x, seq);
#else	
	if (async && x->repl->recheck(x, skb, seq)) {
			XFRM_INC_STATS(net, LINUX_MIB_XFRMINSTATESEQERROR);
			spin_unlock(&x->lock);
			goto drop;
		}
		x->repl->advance(x, seq);
#endif

	x->curlft.bytes += skb->len;
	x->curlft.packets++;
	spin_unlock(&x->lock);

	XFRM_MODE_SKB_CB(skb)->protocol = nexthdr;

	inner_mode = x->inner_mode;

	if (x->sel.family == AF_UNSPEC) {
		inner_mode = xfrm_ip2inner_mode(x, XFRM_MODE_SKB_CB(skb)->protocol);
		if (inner_mode == NULL)
		{
			printk("(%d)ipsec_esp_input_finish inner_mode NULL\n",__LINE__);	
			goto drop;
		}	
	}

	if (inner_mode->input(x, skb)) {
		printk("(%d)ipsec_esp_input_finish LINUX_MIB_XFRMINSTATEMODEERROR\n",__LINE__);
		XFRM_INC_STATS(net, LINUX_MIB_XFRMINSTATEMODEERROR);
		goto drop;
	}

	if (x->outer_mode->flags & XFRM_MODE_FLAG_TUNNEL) {
		decaps = 1;
	}
	else
	{	
		/*
		 * We need the inner address.  However, we only get here for
		 * transport mode so the outer address is identical.
		 */
		
		err = xfrm_parse_spi(skb, nexthdr, &spi, &seq);
		if (err < 0) {
			printk("(%d)ipsec_esp_input_finish LINUX_MIB_XFRMINHDRERROR\n",__LINE__);
			XFRM_INC_STATS(net, LINUX_MIB_XFRMINHDRERROR);
			goto drop;
		}
	}

	nf_reset(skb);
	mcrypto_proc.dbg_pt[9]++;
#ifdef MTK_EIP97_IPI
	if (decaps) {
		skb_dst_drop(skb);
		mcrypto_proc.dbg_pt[10]++;
		if (mcrypto_proc.ipicpu[0] < 0)
			netif_rx(skb);
		else
		{	
			skb_queue_tail(&currAdapterPtr->skbIPIQueue, skb);
			if (currAdapterPtr->skbIPIQueue.qlen > 100)
			{
				smp_call_function_single(mcrypto_proc.ipicpu[0], smp_func_call ,currAdapterPtr, 0);
			}
		}	
		return ;
	} else {
		if (mcrypto_proc.ipicpu[0] < 0)
		{	
			async = 1;
			mcrypto_proc.dbg_pt[5]++;
			x->inner_mode->afinfo->transport_finish(skb, async);
		}
		else
		{		
			skb_queue_tail(&currAdapterPtr->skbIPIQueue, skb);
			if (currAdapterPtr->skbIPIQueue.qlen > 100)
			{
				smp_call_function_single(mcrypto_proc.ipicpu[0], smp_func_call ,currAdapterPtr, 0);
			}
		}
		return;
}
#else
if (decaps) {
		skb_dst_drop(skb);
		netif_rx(skb);
		return ;
	} else {
		async = 1;
		x->inner_mode->afinfo->transport_finish(skb, async);
		return;
	}
#endif	

drop:
	printk("(%d)%s:drop\n",__LINE__,__func__);
	kfree_skb(skb);
	return;
}

static void 
ipsec_esp6_input_finish(
	eip93DescpHandler_t *resHandler, 
	struct xfrm_state *x
)
{
	struct sk_buff *skb = (struct sk_buff *) ipsec_eip93UserId_get(resHandler);
	struct ipv6hdr *iph;
	unsigned int ihl, pktLen;
	struct esp_data *esp = x->data;
	int decaps = 0;
	__be32 spi, seq;
	int err;
	struct net *net = dev_net(skb->dev);
	int nexthdr = 0;
	struct xfrm_mode *inner_mode = x->inner_mode;
	int async = 0;
	struct ip_esp_hdr *esph = (struct ip_esp_hdr *)skb->data;
	ipsecEip93Adapter_t *currAdapterPtr;
	unsigned int *addrCurrAdapter;

	addrCurrAdapter = (unsigned int *) &(skb->cb[36]);
	currAdapterPtr = (ipsecEip93Adapter_t *)(*addrCurrAdapter);

	esph->seq_no = htonl(ipsec_espSeqNum_get(resHandler));
	esph->spi = currAdapterPtr->spi;

	skb->ip_summed = CHECKSUM_NONE;	
	iph = ipv6_hdr(skb);
	ihl = 40;	
	iph->nexthdr = ipsec_espNextHeader_get(resHandler);
	nexthdr = iph->nexthdr;
		
	//adjest skb->tail & skb->len
	pktLen = ipsec_pktLength_get(resHandler);

	//*(skb->data-20+9) = 0x32;
	skb->len = pktLen;
	skb_set_tail_pointer(skb, pktLen);
	__skb_pull(skb, crypto_aead_ivsize(esp->aead));
	
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,14)
	skb_set_transport_header(skb, -ihl);
#else	
	if (x->props.mode == XFRM_MODE_TUNNEL)
		skb_reset_transport_header(skb);
	else
		skb_set_transport_header(skb, -ihl);
#endif
	
	/* adjust for IPSec post-routing */
	spin_lock(&x->lock);
	if (nexthdr <= 0) {
		if (nexthdr == -EBADMSG) {
			xfrm_audit_state_icvfail(x, skb, x->type->proto);
			x->stats.integrity_failed++;
		}
		XFRM_INC_STATS(net, LINUX_MIB_XFRMINSTATEPROTOERROR);
		printk("(%d)ipsec_esp_input_finish LINUX_MIB_XFRMINSTATEPROTOERROR\n",__LINE__);
		spin_unlock(&x->lock);
		goto drop;
	}

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,36)
	if (x->props.replay_window)
			xfrm_replay_advance(x, esph->seq_no);
#else	
	seq = esph->seq_no;
	if (async && x->repl->recheck(x, skb, seq)) {
			XFRM_INC_STATS(net, LINUX_MIB_XFRMINSTATESEQERROR);
			spin_unlock(&x->lock);
			goto drop;
	}
	x->repl->advance(x, seq);
#endif

	x->curlft.bytes += skb->len;
	x->curlft.packets++;
	spin_unlock(&x->lock);

	XFRM_MODE_SKB_CB(skb)->protocol = nexthdr;

	inner_mode = x->inner_mode;

	if (x->sel.family == AF_UNSPEC) {
		inner_mode = xfrm_ip2inner_mode(x, XFRM_MODE_SKB_CB(skb)->protocol);
		if (inner_mode == NULL)
		{
			printk("(%d)ipsec_esp_input_finish inner_mode NULL\n",__LINE__);	
			goto drop;
		}	
	}

	if (inner_mode->input(x, skb)) {
		printk("(%d)ipsec_esp_input_finish LINUX_MIB_XFRMINSTATEMODEERROR\n",__LINE__);
		XFRM_INC_STATS(net, LINUX_MIB_XFRMINSTATEMODEERROR);
		goto drop;
	}

	if (x->outer_mode->flags & XFRM_MODE_FLAG_TUNNEL) {
		decaps = 1;
	}
	else
	{	
		/*
		 * We need the inner address.  However, we only get here for
		 * transport mode so the outer address is identical.
		 */
	
		err = xfrm_parse_spi(skb, nexthdr, &spi, &seq);
		if (err < 0) {
			printk("(%d)ipsec_esp_input_finish LINUX_MIB_XFRMINHDRERROR\n",__LINE__);
			XFRM_INC_STATS(net, LINUX_MIB_XFRMINHDRERROR);
			goto drop;
		}
	}

	nf_reset(skb);
	mcrypto_proc.dbg_pt[9]++;
	if (decaps) {
		skb_dst_drop(skb);
		netif_rx(skb);
		return ;
	} else {
		async = 1;
		x->inner_mode->afinfo->transport_finish(skb, async);
		return;
	}	

drop:
	printk("(%d)%s:drop\n",__LINE__,__func__);
	kfree_skb(skb);
	return;
}
/************************************************************************
*              P U B L I C     F U N C T I O N S
*************************************************************************
*/
#if defined (CONFIG_HWCRYPTO_MEMPOOL)
void 
ipsec_eip93Adapter_clean(
	unsigned int index, ipsecEip93Adapter_t *currAdapterPtr, unsigned int dir
)
{
	unsigned char* ptr;
	addrHandler_t tmpaddr;
	addrsDigestPreCompute_t* addrsPreCompute;
#if defined (MTK_EIP97_DRIVER)	
	eip97DescpHandler_t *cmdHandler;
#else
	eip93DescpHandler_t *cmdHandler;
#endif	
	currAdapterPtr->seqno_out = 0;
	currAdapterPtr->seqno_in = 0;	
	currAdapterPtr->isHashPreCompute = 0;
	
	addrsPreCompute = currAdapterPtr->addrsPreCompute;	
	
	tmpaddr.addr = addrsPreCompute->RecPoolHandler.addr;
	tmpaddr.phyAddr = addrsPreCompute->RecPoolHandler.phyAddr;
	ptr = addrsPreCompute->LocalPool;
	cmdHandler = addrsPreCompute->cmdHandler;
	memset(addrsPreCompute, 0 , sizeof(addrsDigestPreCompute_t));
#if defined (MTK_EIP97_DRIVER)	
	memset(cmdHandler, 0 , sizeof(eip97DescpHandler_t));
#else
	memset(cmdHandler, 0 , sizeof(eip93DescpHandler_t));
#endif	
	addrsPreCompute->cmdHandler = cmdHandler;	
	addrsPreCompute->RecPoolHandler.size = RECPOOLSIZE;
	addrsPreCompute->RecPoolHandler.addr = tmpaddr.addr;
	addrsPreCompute->RecPoolHandler.phyAddr = tmpaddr.phyAddr;
	memset(addrsPreCompute->RecPoolHandler.addr, 0, RECPOOLSIZE);
	
	addrsPreCompute->LocalPool = ptr;
	addrsPreCompute->hashKeyTank = addrsPreCompute->LocalPool + HASHKEYTANK_OFFSET;
	addrsPreCompute->pIDigest = addrsPreCompute->LocalPool + PRECOMPUTE_IDIGEST_OFFSET;
	addrsPreCompute->pODigest = addrsPreCompute->LocalPool + PRECOMPUTE_ODIGEST_OFFSET;
	
	memset(addrsPreCompute->LocalPool, 0, LOCALPOOLSIZE);
	
	
	cmdHandler = currAdapterPtr->cmdHandler;
#if defined (MTK_EIP97_DRIVER)
	cmdHandler->pIDigest = cmdHandler->LocalPool + CMD_IDIGEST_OFFSET;
	cmdHandler->pODigest = cmdHandler->LocalPool + CMD_ODIGEST_OFFSET;
	memset(cmdHandler->LocalPool, 0, CMDLOCALPOOLSIZE);
#endif	
	memset(cmdHandler->saAddr.addr, 0, SAPOOLSIZE);
	
	if (dir==HASH_DIGEST_OUT)
	{	
		spi_outbound_tbl[index] = 0xFFFFFFFF;
		mcrypto_proc.dbg_pt[11] &= ~(1<<index);
	}
	else
	{	
		spi_inbound_tbl[index] = 0xFFFFFFFF;
		mcrypto_proc.dbg_pt[11] &= ~(1<<(index+IPESC_EIP93_ADAPTERS));
	}
	if (currAdapterPtr->skbQueue.qlen > 0 )
		printk("CONN_%s[%d] qlen=%d\n",(dir==HASH_DIGEST_IN)? "IN" : "OUT",index,currAdapterPtr->skbQueue.qlen);
	while (currAdapterPtr->skbQueue.qlen > 0)
	{
		struct sk_buff *pSkb = skb_dequeue(&currAdapterPtr->skbQueue);
		if (pSkb)
			kfree_skb(pSkb);
	}
	currAdapterPtr->packet_count = 0;
	currAdapterPtr->status = TBL_EMPTY;
}
#endif
void 
ipsec_eip93Adapter_mark_free(
	unsigned int spi
)
{
	unsigned int i;
	ipsecEip93Adapter_t *currAdapterPtr;
	spin_lock_bh(&ipsec_adapters_outlock);
	for (i = 0; i < IPESC_EIP93_ADAPTERS; i++)
	{
#if defined (CONFIG_HWCRYPTO_MEMPOOL)
		if (spi_outbound_tbl[i] == spi)
		{
			currAdapterPtr = ipsecEip93AdapterListOut[i];
			spin_lock(&currAdapterPtr->lock);
			if (currAdapterPtr->packet_count <= 0)
			{
				spin_unlock(&currAdapterPtr->lock);
				printk("free AdapterListOut[%d] spi=%x packet_count=%d\n",i,spi,currAdapterPtr->packet_count);					
				ipsec_eip93Adapter_clean(i, currAdapterPtr, HASH_DIGEST_OUT);
			}
			else
			{	
				spin_unlock(&currAdapterPtr->lock);
				currAdapterPtr->status = TBL_DEL;
				mcrypto_proc.dbg_pt[11] |= (1<<i);
				printk("mark free AdapterListOut[%d] currAdapterPtr=%08X \
						packet_count=%d \n",i,currAdapterPtr,currAdapterPtr->packet_count);
			}
			break;
		}
#else
		if ((currAdapterPtr = ipsecEip93AdapterListOut[i]) != NULL)
		{
			if (currAdapterPtr->spi == spi)
			{
				if (currAdapterPtr->packet_count <= 0)
				{
					printk("free AdapterListOut[%d] spi=%x \n",i,spi);
					ipsec_cmdHandler_free(currAdapterPtr->cmdHandler);
					kfree(currAdapterPtr);
					ipsecEip93AdapterListOut[i] = NULL;
					mcrypto_proc.dbg_pt[3] &= ~(1<<i);
				}
				else
				{	
					currAdapterPtr->status = TBL_DEL;
					mcrypto_proc.dbg_pt[3] |= (1<<i);
					printk("mark free AdapterListOut[%d] currAdapterPtr=%08X \
							packet_count=%d \n",i,currAdapterPtr,currAdapterPtr->packet_count);
				}
				spin_unlock_bh(&ipsec_adapters_outlock);
				return;
			}
		}
#endif
	}
	spin_unlock_bh(&ipsec_adapters_outlock);
#if defined (CONFIG_HWCRYPTO_MEMPOOL)
	if (i < IPESC_EIP93_ADAPTERS)
		return;
#endif
	spin_lock_bh(&ipsec_adapters_inlock);
	for (i = 0; i < IPESC_EIP93_ADAPTERS; i++)
	{
#if defined (CONFIG_HWCRYPTO_MEMPOOL)
		if (spi_inbound_tbl[i] == spi)
		{
			currAdapterPtr = ipsecEip93AdapterListIn[i];
			spin_lock(&currAdapterPtr->lock);
			if (currAdapterPtr->packet_count <= 0)
			{
				spin_unlock(&currAdapterPtr->lock);
				printk("free AdapterListIn[%d] spi=%x packet_count=%d\n",i,spi,currAdapterPtr->packet_count);
				ipsec_eip93Adapter_clean(i, currAdapterPtr, HASH_DIGEST_IN);
			}
			else
			{
				spin_unlock(&currAdapterPtr->lock);
				printk("mark free AdapterListIn[%d] currAdapterPtr=%08X \
						packet_count=%d \n",i,currAdapterPtr,currAdapterPtr->packet_count);
				currAdapterPtr->status = TBL_DEL;
				mcrypto_proc.dbg_pt[11] |= (1<<(i+IPESC_EIP93_ADAPTERS));
			}
			break;
		}
#else
		if ((currAdapterPtr = ipsecEip93AdapterListIn[i]) != NULL)
		{
			if (currAdapterPtr->spi == spi)
			{
				if (currAdapterPtr->packet_count <= 0)
				{
					printk("free AdapterListIn[%d] spi=%x \n",i,spi);
					ipsec_cmdHandler_free(currAdapterPtr->cmdHandler);
					kfree(currAdapterPtr);
					ipsecEip93AdapterListIn[i] = NULL;
					mcrypto_proc.dbg_pt[3] &= ~(1<<(i+IPESC_EIP93_ADAPTERS));
				}
				else
				{
					printk("mark free AdapterListIn[%d] currAdapterPtr=%08X\
						   	packet_count=%d \n",i,currAdapterPtr,currAdapterPtr->packet_count);
					currAdapterPtr->status = TBL_DEL;
					mcrypto_proc.dbg_pt[3] |= (1<<(i+IPESC_EIP93_ADAPTERS));
				}
				spin_unlock_bh(&ipsec_adapters_inlock);
				return;
			}
		}
#endif
	}
	spin_unlock_bh(&ipsec_adapters_inlock);
	if (i == IPESC_EIP93_ADAPTERS)
	printk("(%d)%s: spi=%x not found!!\n",__LINE__,__func__,spi);

}

void 
ipsec_eip93Adapter_free(
	unsigned int spi
)
{
	unsigned int i;
	ipsecEip93Adapter_t *currAdapterPtr;
	printk("(%d)%s:free spi=%x \n",__LINE__,__func__,spi);
#if defined (CONFIG_HWCRYPTO_MEMPOOL)
	spin_lock(&ipsec_adapters_outlock);
	for (i = 0; i < IPESC_EIP93_ADAPTERS; i++)
	{
		if (spi_outbound_tbl[i] == spi)
		{
			currAdapterPtr = ipsecEip93AdapterListOut[i];			
			ipsec_eip93Adapter_clean(i, currAdapterPtr, HASH_DIGEST_OUT);
			break;
		}
	}
	spin_unlock(&ipsec_adapters_outlock);
#else
	spin_lock(&ipsec_adapters_outlock);
	for (i = 0; i < IPESC_EIP93_ADAPTERS; i++)
	{
		if ((currAdapterPtr = ipsecEip93AdapterListOut[i]) != NULL)
		{
			if (currAdapterPtr->spi == spi)
			{
				ipsec_cmdHandler_free(currAdapterPtr->cmdHandler);
				kfree(currAdapterPtr);
				ipsecEip93AdapterListOut[i] = NULL;
				spin_unlock(&ipsec_adapters_outlock);
				return;
			}
		}
	}
	spin_unlock(&ipsec_adapters_outlock);
#endif
#if defined (CONFIG_HWCRYPTO_MEMPOOL)
if (i < IPESC_EIP93_ADAPTERS)
		return;
	spin_lock(&ipsec_adapters_inlock);
	for (i = 0; i < IPESC_EIP93_ADAPTERS; i++)
	{
		if (spi_inbound_tbl[i] == spi)
		{
			currAdapterPtr = ipsecEip93AdapterListIn[i];
			ipsec_eip93Adapter_clean(i, currAdapterPtr, HASH_DIGEST_IN);
			break;
		}
	}
	spin_unlock(&ipsec_adapters_inlock);
	if (i == IPESC_EIP93_ADAPTERS)
		printk("(%d)%s: spi=%x not found!!\n",__LINE__,__func__,spi);
#else
	spin_lock(&ipsec_adapters_inlock);
	for (i = 0; i < IPESC_EIP93_ADAPTERS; i++)
	{
		if ((currAdapterPtr = ipsecEip93AdapterListIn[i]) != NULL)
		{
			if (currAdapterPtr->spi == spi)
			{
				ipsec_cmdHandler_free(currAdapterPtr->cmdHandler);
				kfree(currAdapterPtr);
				ipsecEip93AdapterListIn[i] = NULL;
				spin_unlock(&ipsec_adapters_inlock);
				return;
			}
		}
	}
	spin_unlock(&ipsec_adapters_inlock);
#endif
}
/*_______________________________________________________________________
**function name: ipsec_esp_output
**
**description:
*   Replace Linux Kernel's esp_output(), in order to use EIP93
*	to do encryption for a IPSec ESP flow.
**parameters:
*   x -- point to the structure that stores IPSec SA information
*	skb -- the packet that is going to be encrypted.
**global:
*   none
**return:
*   -EPERM, -ENOMEM -- failed: the pakcet will be dropped!
*	1 -- success: the packet's command decsriptor is put into
*		EIP93's Command Descriptor Ring.
**call:
*   none
**revision:
*   1.Trey 20120209
**_______________________________________________________________________*/
int 
ipsec_esp_output(
	struct xfrm_state *x, 
	struct sk_buff *skb
)
{
	ipsecEip93Adapter_t *currAdapterPtr;
	int err;
	eip93DescpHandler_t *cmdHandler;
	struct iphdr *top_iph = ip_hdr(skb);
	unsigned int *addrCurrAdapter;
	
	err = ipsec_esp_preProcess(x, skb, HASH_DIGEST_OUT);
	if (err < 0)
	{
		printk("\n\n ipsec_esp_preProcess for HASH_DIGEST_OUT failed! \n\n");
		return -EINPROGRESS;
	}

	if (err == HWCRYPTO_PREPROCESS_DROP)
	{
		return HWCRYPTO_OK;
	}	
	addrCurrAdapter = (unsigned int *) &(skb->cb[36]);
	currAdapterPtr = (ipsecEip93Adapter_t *)(*addrCurrAdapter);
	cmdHandler = currAdapterPtr->cmdHandler;

#ifdef RALINK_HWCRYPTO_NAT_T
#else		
	/* this is non-NULL only with UDP Encapsulation for NAT-T */
	if (unlikely(x->encap)) 
	{		
		printk("\n\n NAT-T is not supported yet! \n\n");
		//return -EPERM;
		return -EINPROGRESS;
	}
#endif	
	/* in case user will change between tunnel and transport mode,
	 * we have to set "padValue" every time before every packet 
	 * goes into EIP93 for esp outbound! */
	ipsec_espNextHeader_set(cmdHandler, top_iph->protocol);
	//let skb->data point to the payload which is going to be encrypted
	if (x->encap==0)	
		__skb_pull(skb, skb_transport_offset(skb));

#if defined (FEATURE_AVOID_QUEUE_PACKET)
	err = ipsec_esp_pktPut(NULL, skb);
	return err;
#else
	return ipsec_esp_pktPut(NULL, skb);
#endif
}

int ipsec_esp6_output(
	struct xfrm_state *x, 
	struct sk_buff *skb
)
{
	ipsecEip93Adapter_t *currAdapterPtr;
	int err;
	eip93DescpHandler_t *cmdHandler;
	struct ipv6hdr *top_iph = ipv6_hdr(skb);
	unsigned int *addrCurrAdapter;

	err = ipsec_esp_preProcess(x, skb, HASH_DIGEST_OUT);
	if (err < 0)
	{
		printk("\n\n ipsec_esp_preProcess for HASH_DIGEST_OUT failed! \n\n");
		return err;
	}

	if (err == HWCRYPTO_PREPROCESS_DROP)
	{
		return HWCRYPTO_OK;
	}	
	
	addrCurrAdapter = (unsigned int *) &(skb->cb[36]);
	currAdapterPtr = (ipsecEip93Adapter_t *)(*addrCurrAdapter);
	cmdHandler = currAdapterPtr->cmdHandler;	
#ifdef RALINK_HWCRYPTO_NAT_T
#else		
	/* this is non-NULL only with UDP Encapsulation for NAT-T */
	if (unlikely(x->encap)) 
	{		
		printk("\n\n NAT-T is not supported yet! \n\n");
		return -EPERM;
	}
#endif	
	/* in case user will change between tunnel and transport mode,
	 * we have to set "padValue" every time before every packet 
	 * goes into EIP93 for esp outbound! */

	ipsec_espNextHeader_set(cmdHandler, top_iph->nexthdr);
	//let skb->data point to the payload which is going to be encrypted
	if (x->encap==0)	
		__skb_pull(skb, skb_transport_offset(skb));

	return ipsec_esp_pktPut(NULL, skb);

}
/*_______________________________________________________________________
**function name: ipsec_esp_input
**
**description:
*   Replace Linux Kernel's esp_input(), in order to use EIP93
*	to do decryption for a IPSec ESP flow.
**parameters:
*   x -- point to the structure that stores IPSec SA information
*	skb -- the packet that is going to be decrypted.
**global:
*   none
**return:
*   -EPERM, -ENOMEM -- failed: the pakcet will be dropped!
*	1 -- success: the packet's command decsriptor is put into
*		EIP93's Command Descriptor Ring.
**call:
*   none
**revision:
*   1.Trey 20120209
**_______________________________________________________________________*/
int 
ipsec_esp_input(
	struct xfrm_state *x, 
	struct sk_buff *skb
)
{	
	int err;
	struct esp_data *esp = x->data;
	int blksize = ALIGN(crypto_aead_blocksize(esp->aead), 4);
	int alen = crypto_aead_authsize(esp->aead);
	int elen = skb->len - sizeof(struct ip_esp_hdr) - crypto_aead_ivsize(esp->aead) - alen;	

	err = ipsec_esp_preProcess(x, skb, HASH_DIGEST_IN);
	if (err < 0)
	{
		printk("\n\n ipsec_esp_preProcess for HASH_DIGEST_IN failed! \n\n");
		return err;
	}

	if (err == HWCRYPTO_PREPROCESS_DROP)
	{
		return HWCRYPTO_OK;
	}	
	
	{
		ipsecEip93Adapter_t *currAdapterPtr;
		unsigned int *addrCurrAdapter;
		static struct sk_buff *skb_old = NULL;
		static unsigned char* ptr = NULL;
		struct ip_esp_hdr *esph = skb->data;
		addrCurrAdapter = (unsigned int *) &(skb->cb[36]);
		currAdapterPtr = (ipsecEip93Adapter_t *)(*addrCurrAdapter);
		unsigned int seqno_in = 0;	
		spin_lock(&currAdapterPtr->seqlock);
		seqno_in = currAdapterPtr->seqno_in; 
		if ((seqno_in+1) != ntohl(esph->seq_no))
		{	
			if (seqno_in >= ntohl(esph->seq_no))
			{	
			if (ptr)
				ra_dbg("seqno_in[%d]: %08X %08X (%08X %08X) (%08X %08X)\n",	\
						currAdapterPtr->idx,seqno_in,ntohl(esph->seq_no),skb_old,skb,ptr,skb->data);	
			else
				ra_dbg("seqno_in[%d]: %08X %08X (%08X %08X)\n",	\
						currAdapterPtr->idx, seqno_in,ntohl(esph->seq_no));	
			}
			mcrypto_proc.dbg_pt[14]++;
		}
		currAdapterPtr->seqno_in = ntohl(esph->seq_no);
		skb_old = skb;
		ptr = skb->data;		
		spin_unlock(&currAdapterPtr->seqlock);
	}
	if (!pskb_may_pull(skb, sizeof(struct ip_esp_hdr)))
	{
		printk("\n skb.len=%d esp_hdr=%d\n",skb->len,sizeof(struct ip_esp_hdr));	
		goto out;
	}

	if (elen <= 0 || (elen & (blksize-1)))
	{
		printk("\n skb->len=%d elen=%d blksize=%d esp=%d iv=%d alen=%d\n",\
				skb->len,elen,blksize,sizeof(struct ip_esp_hdr),crypto_aead_ivsize(esp->aead),alen);	
		goto out;
	}
#ifdef RALINK_HWCRYPTO_NAT_T
#else
	if (x->encap) 
	{
		printk("\n !NAT-T is not supported! \n");
		goto out;
	}
#endif

	err = ipsec_esp_pktPut(NULL, skb);
	return err;	

out:
	printk("\n Something's wrong! Go out! \n");
	kfree_skb(skb);
	//return -EINVAL;
	return -EINPROGRESS;
}

int 
ipsec_esp6_input(
	struct xfrm_state *x, 
	struct sk_buff *skb
)
{	
	int err;
	struct esp_data *esp = x->data;
	int blksize = ALIGN(crypto_aead_blocksize(esp->aead), 4);
	int alen = crypto_aead_authsize(esp->aead);
	int elen = skb->len - sizeof(struct ip_esp_hdr) - crypto_aead_ivsize(esp->aead) - alen;	

	err = ipsec_esp_preProcess(x, skb, HASH_DIGEST_IN);
	if (err < 0)
	{
		printk("\n\n ipsec_esp_preProcess for HASH_DIGEST_IN failed! \n\n");
		return err;
	}

	if (err == HWCRYPTO_PREPROCESS_DROP)
	{
		return HWCRYPTO_OK;
	}	
	
	if (!pskb_may_pull(skb, sizeof(struct ip_esp_hdr)))
	{	
		printk("[%s]pskb_may_pull failed\n",__func__);
		goto out;
	}
		
	if (elen <= 0 || (elen & (blksize-1)))
	{	
		printk("[%s]elen=%d blksize=%d\n",__func__,elen,blksize);
		goto out;
	}
#ifdef RALINK_HWCRYPTO_NAT_T
#else
	if (x->encap) 
	{
		printk("\n !NAT-T is not supported! \n");
		goto out;
	}
#endif

	err = ipsec_esp_pktPut(NULL, skb);
	return err;	

out:
	printk("\n[%s] Something's wrong! Go out! \n",__func__);
	return -EINVAL;
}
/************************************************************************
*              E X T E R N E L     F U N C T I O N S
*************************************************************************
*/
/*_______________________________________________________________________
**function name: ipsec_eip93_adapters_init
**
**description:
*   initialize ipsecEip93AdapterListOut[] and ipsecEip93AdapterListIn[]
*	durin EIP93's initialization.
**parameters:
*   none
**global:
*   none
**return:
*   none
**call:
*   none
**revision:
*   1.Trey 20120209
**_______________________________________________________________________*/
void 
ipsec_eip93_adapters_init(
	void
)
{
	unsigned int i, j;
	
	for (i = 0; i < IPESC_EIP93_ADAPTERS; i++)
	{
		ipsecEip93AdapterListOut[i] = NULL;
		ipsecEip93AdapterListIn[i] = NULL;
#if defined (CONFIG_HWCRYPTO_MEMPOOL)
		spi_outbound_tbl[i] = 0xFFFFFFFF;
		spi_inbound_tbl[i] = 0xFFFFFFFF;
#endif
	}
#if defined (CONFIG_HWCRYPTO_MEMPOOL)	
	for (j = 0; j < 2; j++)
	{
		for (i = 0; i < IPESC_EIP93_ADAPTERS; i++)
		{
			ipsecEip93Adapter_t *currAdapterPtr;
#if defined (MTK_EIP97_DRIVER)			
			eip97DescpHandler_t *cmdHandler;
#else
			eip93DescpHandler_t *cmdHandler;
#endif			
			addrsDigestPreCompute_t* addrsPreCompute;
			
			currAdapterPtr = (ipsecEip93Adapter_t *) kzalloc(sizeof(ipsecEip93Adapter_t), GFP_KERNEL);
			addrsPreCompute = (addrsDigestPreCompute_t *) kzalloc(sizeof(addrsDigestPreCompute_t), GFP_KERNEL);
#if defined (MTK_EIP97_DRIVER)			
			cmdHandler = (eip97DescpHandler_t *) kzalloc(sizeof(eip97DescpHandler_t), GFP_KERNEL);
#else
			cmdHandler = (eip93DescpHandler_t *) kzalloc(sizeof(eip93DescpHandler_t), GFP_KERNEL);
#endif			
			currAdapterPtr->addrsPreCompute = addrsPreCompute;
			addrsPreCompute->cmdHandler = cmdHandler;
			
			
			addrsPreCompute->RecPoolHandler.size = RECPOOLSIZE;
			addrsPreCompute->RecPoolHandler.addr = (unsigned int *) dma_alloc_coherent(NULL, addrsPreCompute->RecPoolHandler.size, &addrsPreCompute->RecPoolHandler.phyAddr, GFP_KERNEL);
			memset(addrsPreCompute->RecPoolHandler.addr, 0, RECPOOLSIZE);
			
			addrsPreCompute->LocalPool = kzalloc(LOCALPOOLSIZE, GFP_KERNEL);
			addrsPreCompute->hashKeyTank = addrsPreCompute->LocalPool + HASHKEYTANK_OFFSET;	
			addrsPreCompute->pIDigest = addrsPreCompute->LocalPool + PRECOMPUTE_IDIGEST_OFFSET;
			addrsPreCompute->pODigest = addrsPreCompute->LocalPool + PRECOMPUTE_ODIGEST_OFFSET;
			
#if defined (MTK_EIP97_DRIVER)			
			cmdHandler = (eip97DescpHandler_t *) kzalloc(sizeof(eip97DescpHandler_t), GFP_KERNEL);
#else
			cmdHandler = (eip93DescpHandler_t *) kzalloc(sizeof(eip93DescpHandler_t), GFP_KERNEL);
#endif
			currAdapterPtr->cmdHandler = cmdHandler;
#if defined (MTK_EIP97_DRIVER)			
			cmdHandler->LocalPool = kzalloc(CMDLOCALPOOLSIZE, GFP_KERNEL);
			cmdHandler->pIDigest = cmdHandler->LocalPool + CMD_IDIGEST_OFFSET;//kzalloc(512, GFP_KERNEL);
			cmdHandler->pODigest = cmdHandler->LocalPool + CMD_ODIGEST_OFFSET;//kzalloc(512, GFP_KERNEL);
#endif			
			cmdHandler->saAddr.addr = dma_alloc_coherent(NULL, SAPOOLSIZE, &cmdHandler->saAddr.phyAddr, GFP_KERNEL);
			if (j==0)
			{	
				ipsecEip93AdapterListOut[i] = currAdapterPtr;
				ra_dbg("ipsecEip93AdapterListOut[%d]=%08X\n",i,currAdapterPtr);	
			}
			else
			{	
				ipsecEip93AdapterListIn[i] = currAdapterPtr;
				ra_dbg("ipsecEip93AdapterListIn[%d]=%08X\n",i,currAdapterPtr);	
			}
	}
	}
#endif
}

/*_______________________________________________________________________
**function name: ipsec_cryptoLock_init
**
**description:
*   initialize cryptoLock durin EIP93's initialization. cryptoLock is
*	used to make sure only one process can access EIP93 at a time.
**parameters:
*   none
**global:
*   none
**return:
*   none
**call:
*   none
**revision:
*   1.Trey 20120209
**_______________________________________________________________________*/
void 
ipsec_cryptoLock_init(
	void
)
{
	int i;
	for ( i = 0 ; i < NUM_CMD_RING; i++ )
		spin_lock_init(&cryptoLock[i]);
	
	spin_lock_init(&ipsec_adapters_outlock);
	spin_lock_init(&ipsec_adapters_inlock);
}

EXPORT_SYMBOL(ipsec_eip93_adapters_init);
EXPORT_SYMBOL(ipsec_cryptoLock_init);

/*_______________________________________________________________________
**function name: ipsec_BH_handler_resultGet
**
**description:
*   This tasklet is raised by EIP93's interrupt after EIP93 finishs
*	a command descriptor and puts the result in Result Descriptor Ring.
*	This tasklet gets a result descriptor from EIP93 at a time and do
*	the corresponding atcion until all results from EIP93 are finished.
**parameters:
*   none
**global:
*   none
**return:
*   none
**call:
*   ipsec_esp_output_finish() when the result is for encryption.
*	ipsec_esp_input_finish() when the result is for decryption.
**revision:
*   1.Trey 20120209
**_______________________________________________________________________*/
void 
ipsec_BH_handler_resultGet(
	unsigned long data
)
{
	int retVal;
	struct sk_buff *skb = NULL;
	ipsecEip93Adapter_t *currAdapterPtr;
	unsigned int *addrCurrAdapter;

	while (1)	
	{
#ifdef MTK_EIP97_DRIVER
		if ((data >= 4) || (data < 0))
			printk("==[%s] data=%d==\n",__func__,data);
		memset(&resDescpHandler[data], 0, sizeof(eip97DescpHandler_t));
#else
		data = 0;		
		memset(&resDescpHandler[data], 0, sizeof(eip93DescpHandler_t));
#endif
		retVal = ipsec_packet_get(&resDescpHandler[data], data);
		//got the correct result from eip93
#ifdef MTK_EIP97_DRIVER
		if (likely(retVal > 0))
#else		
		if (likely(retVal == 1))
#endif
		{
			//the result is for encrypted or encrypted packet
#ifdef MTK_EIP97_DRIVER
			if (likely(retVal > 1))
#else			
			if (ipsec_eip93HashFinal_get(&resDescpHandler[data]) == 0x1)
#endif
			{		
				skb = (struct sk_buff *) ipsec_eip93UserId_get(&resDescpHandler[data]);
				if (skb==NULL)
				{	
					printk("UserId got NULL skb\n");
					break;
				}

				addrCurrAdapter = (unsigned int *) &(skb->cb[36]);
				currAdapterPtr = (ipsecEip93Adapter_t *)(*addrCurrAdapter);
				if (currAdapterPtr==NULL)
				{	
					printk("cb got NULL currAdapterPtr\n");
					break;
				}

				if (currAdapterPtr->status==TBL_DEL)
				{
#ifndef MTK_EIP97_IPI					
					mcrypto_proc.dbg_pt[5]++;
#endif					
					kfree_skb(skb);
					if (currAdapterPtr->packet_count <= 0)
					{
						ipsec_eip93Adapter_free(currAdapterPtr->spi);
						break;
					}
				}		
				if (skb->protocol == htons(ETH_P_IPV6))
				{
					if (currAdapterPtr->isEncryptOrDecrypt == CRYPTO_ENCRYPTION)
					{
						ipsec_esp6_output_finish(&resDescpHandler[data]);
					}
					else if (currAdapterPtr->isEncryptOrDecrypt == CRYPTO_DECRYPTION)
					{			
						ipsec_esp6_input_finish(&resDescpHandler[data], currAdapterPtr->x);
					}
					else
					{
						printk("\n\n !can't tell encrypt or decrypt! %08X\n\n",currAdapterPtr->isEncryptOrDecrypt);
						return;
					}
				}
				else
				{			
					if (currAdapterPtr->isEncryptOrDecrypt == CRYPTO_ENCRYPTION)
					{
						ipsec_esp_output_finish(&resDescpHandler[data]);
					}
					else if (currAdapterPtr->isEncryptOrDecrypt == CRYPTO_DECRYPTION)
					{			
						ipsec_esp_input_finish(&resDescpHandler[data], currAdapterPtr->x);
					}
					else
					{
						printk("\n\n !can't tell encrypt or decrypt! %08X\n\n",currAdapterPtr->isEncryptOrDecrypt);
						return;
					}
				}

			}
			//the result is for inner and outer hash digest pre-compute
			else
			{
				currAdapterPtr = (ipsecEip93Adapter_t *) ipsec_eip93UserId_get(&resDescpHandler[data]);
				if (currAdapterPtr)
				printk("=== Build IPSec %s Connection[%d] [P%d](SPI=%X)===\n",\
							(currAdapterPtr->isEncryptOrDecrypt==CRYPTO_ENCRYPTION) ? "outbound" : " inbound",\
								currAdapterPtr->idx,currAdapterPtr->isHashPreCompute, currAdapterPtr->spi);
				else
				{	
					printk("No connection entry in table\n");
					return;				
				}

				//for the inner digests	
				if (currAdapterPtr->isHashPreCompute == 0)
				{
					//resDescpHandler only has physical addresses, so we have to get saState's virtual address from addrsPreCompute.
					ipsec_hashDigests_set(currAdapterPtr, 1);
					//inner digest done
					currAdapterPtr->isHashPreCompute = 1; 
				}
				//for the outer digests	
				else if (currAdapterPtr->isHashPreCompute == 1)
				{
					ipsec_hashDigests_set(currAdapterPtr, 2);
					//outer digest done
					currAdapterPtr->isHashPreCompute = 2;				
#if defined (FEATURE_AVOID_QUEUE_PACKET)
					//Hash Digests are ready
					ipsec_hashDigests_get(currAdapterPtr);
					currAdapterPtr->isHashPreCompute = 3; //pre-compute done
#if !defined (CONFIG_HWCRYPTO_MEMPOOL)
					ipsec_addrsDigestPreCompute_free(currAdapterPtr);
#endif
					ipsec_esp_pktPut(currAdapterPtr, NULL);
#endif
				}
				else
				{
					printk("\n\n !can't tell inner or outer digests! \n\n");				
					return;
				}
			}
		}
		//if packet is not done, don't wait! (for speeding up)
		else if (retVal == 0)
		{
			int i;
			
			for (i = 0; i < IPESC_EIP93_ADAPTERS; i++)
			{
				currAdapterPtr = ipsecEip93AdapterListIn[i];
				if (currAdapterPtr!=NULL)
				{
					if (currAdapterPtr->status == TBL_ACTIVE)
					{
						if (currAdapterPtr->skbQueue.qlen > 0)
					ipsec_esp_pktPut(currAdapterPtr, NULL);
#ifdef MTK_EIP97_IPI
					if (currAdapterPtr->skbIPIQueue.qlen >0)
						smp_call_function_single(mcrypto_proc.ipicpu[0], smp_func_call ,currAdapterPtr, 0);
#endif
				}
			}
			}
			for (i = 0; i < IPESC_EIP93_ADAPTERS; i++)
			{
				currAdapterPtr = ipsecEip93AdapterListOut[i];
				if (currAdapterPtr!=NULL)
				{	
					if (currAdapterPtr->status == TBL_ACTIVE)
					{	
						if (currAdapterPtr->skbQueue.qlen > 0)
					ipsec_esp_pktPut(currAdapterPtr, NULL);	
#ifdef MTK_EIP97_IPI
					if (currAdapterPtr->skbIPIQueue.qlen >0)
						smp_call_function_single(mcrypto_proc.ipicpu[0], smp_func_call ,currAdapterPtr, 0);
#endif
				}
			}
			}
			break;
		}
		else if (retVal < 0)
			break;
	} //end while (1)
	
	return;
}
EXPORT_SYMBOL(ipsec_BH_handler_resultGet);

#ifdef MTK_EIP97_IPI
static void smp_func_call_BH_handler(unsigned long data)
{
	struct sk_buff *skb;
	ipsecEip93Adapter_t *currAdapterPtr = data;
	struct xfrm_state *x ;

	if (smp_processor_id()!=mcrypto_proc.ipicpu[0])
	{
		mcrypto_proc.dbg_pt[6]++;
		return;
	}
	else
		mcrypto_proc.dbg_pt[7]++;	
	x = currAdapterPtr->x;
	
	spin_lock(&currAdapterPtr->lock);
	
	while (currAdapterPtr->skbIPIQueue.qlen > 0)
	{
		skb = skb_dequeue(&currAdapterPtr->skbIPIQueue);
		if (skb)
		{
			if (currAdapterPtr->tunnel==0)	
				x->inner_mode->afinfo->transport_finish(skb, 1);
			else
				netif_rx(skb);
		}	
	}
	spin_unlock(&currAdapterPtr->lock);
	return;
}

static void smp_func_call(void *info)
{
	if (smp_processor_id()!=mcrypto_proc.ipicpu[0])
	{
		mcrypto_proc.dbg_pt[4]++;
	}	
	smp_func_call_tsk.data = info;
	tasklet_hi_schedule(&smp_func_call_tsk);
	return;
}
#endif
