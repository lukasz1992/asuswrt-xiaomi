#ifndef MTK_ESP_H
#define MTK_ESP_H

#include <linux/skbuff.h>
#include <linux/dma-mapping.h>
#include <linux/kernel.h>

#if defined (CONFIG_ARCH_MT7623)
#define MTK_EIP97_DRIVER	1
#define MTK_EIP97_IPI		1
#endif

#define CONFIG_HWCRYPTO_MEMPOOL 1

#define HASH_DIGEST_OUT			0
#define HASH_DIGEST_IN			1
#define CRYPTO_ENCRYPTION		1
#define CRYPTO_DECRYPTION		2
#define CRYPTO_PREDIGEST		3

#define TBL_EMPTY				0
#define TBL_ACTIVE				0xAAAAAAAA
#define TBL_DEL					0xDDDDDDDD

#define IPESC_EIP93_ADAPTERS	16

#ifdef MTK_EIP97_DRIVER
#define LOCALPOOLSIZE					1024
#define HASHKEYTANK_OFFSET				0
#define PRECOMPUTE_IDIGEST_OFFSET		128	//64
#define PRECOMPUTE_ODIGEST_OFFSET		256//128
#define PRECOMPUTE_TCRDATA_OFFSET		512//256

#define RECPOOLSIZE						2048
#define IPAD_OFFSET						0
#define OPAD_OFFSET						64
#define SA_OFFSET							128
#define TOKEN_OFFSET					512
#define STATE_OFFSET					640

#define SAPOOLSIZE						1024

#define CMDLOCALPOOLSIZE				2048
#define CMD_IDIGEST_OFFSET				0
#define CMD_ODIGEST_OFFSET				512
#define CMD_TCRDATA_OFFSET				1024
#else
#define LOCALPOOLSIZE					1024
#define HASHKEYTANK_OFFSET				0
#define PRECOMPUTE_IDIGEST_OFFSET		64
#define PRECOMPUTE_ODIGEST_OFFSET		128
#define PRECOMPUTE_TCRDATA_OFFSET		256

#define RECPOOLSIZE						2048
#define IPAD_OFFSET						0
#define OPAD_OFFSET						64
#define SA_OFFSET						128
#define TOKEN_OFFSET					512
#define STATE_OFFSET					640
#define STATE2_OFFSET					768

#define SAPOOLSIZE						1024
#define SAPOOL_STATE_OFFSET             256
#define SAPOOL_STATE2_OFFSET            512 
#define SAPOOL_ARC4STATE_OFFSET         768 

#endif

#ifdef MTK_EIP97_DRIVER
#define NUM_CMD_RING					4
#define NUM_RESULT_RING				4
#else
#define NUM_CMD_RING    			1
#define NUM_RESULT_RING 			1
#endif
/************************************************************************
*      E X T E R N E L    S T R U C T U R E    D E F I N I T I O N
*************************************************************************
*/
typedef union
{
	struct
	{
		unsigned int hostReady		: 1;
		unsigned int peReady		: 1;
		unsigned int reserved		: 1;
		unsigned int initArc4		: 1;
		unsigned int hashFinal		: 1;
		unsigned int haltMode		: 1;
		unsigned int prngMode		: 2;
		unsigned int padValue		: 8;
		unsigned int errStatus		: 8;
		unsigned int padCrtlStat	: 8;
	} bits;
	unsigned int word;
		
} peCrtlStat_t;

typedef union
{
	struct
	{
		unsigned int length			: 20;
		unsigned int reserved		: 2;
		unsigned int hostReady		: 1;
		unsigned int peReady		: 1;
		unsigned int byPass			: 8;	
	} bits;	
	unsigned int word;
		
} peLength_t;

#if defined (MTK_EIP97_DRIVER)
typedef union
{
    struct
    {
        unsigned int Particle_Fill_Level  : 17;
        unsigned int Reserved      	: 3;
        unsigned int Desc_Ovf  			: 1;
        unsigned int Buf_Ovf 				: 1;
        unsigned int LS       			: 1;
        unsigned int FS      				: 1;
        unsigned int Result_Size    : 8;
    } bits;
    unsigned int word;

} ResultDescW0_t;

typedef union
{
    struct
    {
        unsigned int Packet_Length  : 17;
        unsigned int Error_Code     : 15;
    } bits;
    unsigned int word;

} peRDataW0_t;

typedef union
{
    struct
    {
        unsigned int Bypass_Length  	: 4;
        unsigned int E15				: 1;
				unsigned int reserved			: 16;
				unsigned int H					: 1;
				unsigned int Hash_Length		: 6;
				unsigned int B					: 1;
				unsigned int C					: 1;
				unsigned int N					: 1;
				unsigned int L					: 1;
    } bits;
    unsigned int word;

} peRDataW1_t;

typedef union
{
    struct
    {
        unsigned int Application_ID  : 24;
        unsigned int reserved     	 : 8;
    } bits;
    unsigned int word;

} peRDataW2_t;

typedef union
{
    struct
    {
        unsigned int Next_Header  	: 8;
        unsigned int Pad_Length     : 8;	
        unsigned int reserved     	: 16;
    } bits;
    unsigned int word;

} peRDataW3_t;

typedef struct peRData_s
{
    peRDataW0_t			peRDataW0;
    peRDataW1_t			peRDataW1;
    peRDataW2_t			peRDataW2;
    peRDataW3_t			peRDataW3;
	unsigned int		Bypass_Token_Words[4];
}peRData_t;
#endif

typedef struct addrHandler_s
{
	unsigned int addr;
	dma_addr_t	 phyAddr;
	int			 size;

} addrHandler_t;

typedef struct eip93DescpHandler_s
{
	peCrtlStat_t	peCrtlStat;
	addrHandler_t	srcAddr;
	addrHandler_t	dstAddr;
	addrHandler_t	saAddr;
	addrHandler_t	stateAddr;
	addrHandler_t	arc4Addr;
	unsigned int	userId;
	peLength_t		peLength;
} eip93DescpHandler_t;

#if defined (MTK_EIP97_DRIVER)
typedef struct eip97DescpHandler_s
{
    addrHandler_t   srcAddr;
    addrHandler_t   dstAddr;
    addrHandler_t	ACDataAddr;				/* Token */
    void			*pTCRData;
    uint32_t		TokenHeaderWord;
    uint32_t		TokenWords;
    addrHandler_t   saAddr;						/* Context Record */
    unsigned int	KeySizeDW;
    unsigned int	digestWord;
    unsigned int	blkSize;							
    addrHandler_t   arc4Addr;
    unsigned int    userId;						/* filter_words */
    unsigned int	seq_no;
    ResultDescW0_t	RxDescW0;
    peRData_t			peRData;
    unsigned int		nexthdr;
    unsigned int 		*pIDigest;
    unsigned int 		*pODigest;
#if defined (CONFIG_HWCRYPTO_MEMPOOL)
    unsigned char		*LocalPool;
#endif
} eip97DescpHandler_t;
#endif

typedef struct addrsDigestPreCompute_s
{
	unsigned int 		*hashKeyTank;
	addrHandler_t 		ipadHandler;
	addrHandler_t 		opadHandler;
	unsigned int 		blkSize;
	void* 				cmdHandler;
	addrHandler_t 		saHandler;
#if defined (MTK_EIP97_DRIVER) || defined (CONFIG_HWCRYPTO_MEMPOOL)
addrHandler_t 		RecPoolHandler;
#endif
#if defined (MTK_EIP97_DRIVER)
	addrHandler_t 		saHandler2;
#endif
#if defined (CONFIG_HWCRYPTO_MEMPOOL)
	unsigned char		*LocalPool;		
#endif
	addrHandler_t 		stateHandler;
	addrHandler_t 		stateHandler2;
	unsigned int 		digestWord;
	unsigned int 		*pIDigest;
	unsigned int 		*pODigest;
#if defined (MTK_EIP97_DRIVER)
	addrHandler_t		ACDataAddr;				/* Token for in digest */
	addrHandler_t		ACDataAddr2;			/* Token for out digest */
#endif
} addrsDigestPreCompute_t;

typedef struct ipsecEip93Adapter_s
{
	unsigned int 				spi; //every ipsec flow has a unique spi
	struct xfrm_state 			*x; //the SA
	struct dst_entry 			*dst;
	unsigned int 				isHashPreCompute; //0:pre-compute init, 1:inner digest done, 2:inner digest done, 3:pre-compute done
	unsigned int 				isEncryptOrDecrypt; //1:encrypt, 2:decrypt
	struct sk_buff_head 		skbQueue;
	addrsDigestPreCompute_t		*addrsPreCompute; //for hash pre-compute
	void*						cmdHandler;
	spinlock_t 					lock;
	spinlock_t					seqlock;
	unsigned int 				addedLen; //refer to ssh_hwaccel_alloc_combined() in safenet_la.c
	unsigned int				tunnel;
	unsigned int				status;
	unsigned int				idx;
	int							packet_count;
	unsigned int                seqno_in;
	unsigned int                seqno_out;
	struct sk_buff_head 		skbIPIQueue;
} ipsecEip93Adapter_t;

/************************************************************************
*      E X T E R N E L     F U N C T I O N    D E C L A R A T I O N
*************************************************************************
*/
#if defined (MTK_EIP97_DRIVER)
extern int 
(*ipsec_packet_put)(
	void *descpHandler, struct sk_buff *skb, unsigned int rdx
);
#else
extern int 
(*ipsec_packet_put)(
	void *descpHandler, struct sk_buff *skb
);
#endif

extern int 
(*ipsec_packet_get)(
	void *descpHandler,
	unsigned int rdx
);
extern bool 
(*ipsec_eip93CmdResCnt_check)(
	unsigned int rdx
);
extern int 
(*ipsec_preComputeIn_cmdDescp_set)(
	ipsecEip93Adapter_t *currAdapterPtr,
	//unsigned int hashAlg,
	unsigned int direction
);
extern int 
(*ipsec_preComputeOut_cmdDescp_set)(
	ipsecEip93Adapter_t *currAdapterPtr,
	//unsigned int hashAlg, 
	unsigned int direction
);
extern int 
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
extern void 
(*ipsec_espNextHeader_set)(
	void *cmdHandler, 
	unsigned char protocol	
);
extern unsigned char 
(*ipsec_espNextHeader_get)(
	void *resHandler
);
extern unsigned int 
(*ipsec_pktLength_get)(
	void *resHandler
);
extern unsigned int 
(*ipsec_eip93HashFinal_get)(
	void *resHandler
);
extern unsigned int 
(*ipsec_eip93UserId_get)(
	void *resHandler
);

extern void 
(*ipsec_addrsDigestPreCompute_free)(
	ipsecEip93Adapter_t *currAdapterPtr
);

extern void 
(*ipsec_cmdHandler_free)(
	void *cmdHandler
);

extern void 
(*ipsec_hashDigests_get)(
	ipsecEip93Adapter_t *currAdapterPtr
);
extern void 
(*ipsec_hashDigests_set)(
	ipsecEip93Adapter_t *currAdapterPtr,
	unsigned int isInOrOut
);
	
extern unsigned int 
(*ipsec_espSeqNum_get)(
	void *resHandler
);

extern void 
ipsec_eip93_adapters_init(
	void
);
extern void 
ipsec_cryptoLock_init(
	void
);
extern void 
ipsec_BH_handler_resultGet(
unsigned long rdx
);

#define PROCNAME    "mcrypto"

typedef struct mcrypto_proc_t {
	int ipicpu[10];
	int copy_expand_count;
    int nolinear_count;
    int oom_in_put;
    unsigned int chstatus;
    unsigned int	dbg_pt[16];
    unsigned int qlen[32];
}mcrypto_proc_type;

extern mcrypto_proc_type mcrypto_proc;
extern ipsecEip93Adapter_t 	*ipsecEip93AdapterListOut[IPESC_EIP93_ADAPTERS];
extern ipsecEip93Adapter_t 	*ipsecEip93AdapterListIn[IPESC_EIP93_ADAPTERS];

#define HWCRYPTO_OK			1
#define HWCRYPTO_NOMEM		0x80
#define HWCRYPTO_PREPROCESS_DROP		0x40
#endif

