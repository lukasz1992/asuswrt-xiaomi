
#ifndef MTK_EIP93_IPSEC_H
#define MTK_EIP93_IPSEC_H


typedef union
{
	struct
	{
		unsigned int opCode			: 3;
		unsigned int direction		: 1;
		unsigned int opGroup		: 2;
		unsigned int padType		: 2;
		unsigned int cipher			: 4;
		unsigned int hash			: 4;
		unsigned int reserved2		: 1;			
		unsigned int scPad			: 1;			
		unsigned int extPad			: 1;
		unsigned int hdrProc		: 1;	
		unsigned int digestLength	: 4;
		unsigned int ivSource		: 2;
		unsigned int hashSource		: 2;
		unsigned int saveIv			: 1;					
		unsigned int saveHash		: 1;					
		unsigned int reserved1		: 2;	
	} bits;
	unsigned int word;
		
} saCmd0_t;

typedef union
{
	struct
	{
		unsigned int copyDigest		: 1;
		unsigned int copyHeader		: 1;
		unsigned int copyPayload	: 1;
		unsigned int copyPad		: 1;
		unsigned int reserved4		: 4;
		unsigned int cipherMode		: 2;
		unsigned int reserved3		: 1;
		unsigned int sslMac			: 1;
		unsigned int hmac			: 1;
		unsigned int byteOffset		: 1;
		unsigned int reserved2		: 2;
		unsigned int hashCryptOffset: 8;
#if 1			
		unsigned int arc4KeyLen		: 5;
		unsigned int seqNumCheck	: 1;
		unsigned int reserved1		: 2;
#else		
		unsigned int aesKeyLen		: 3;
		unsigned int reserved1		: 1;
		unsigned int aesDecKey		: 1;
		unsigned int seqNumCheck	: 1;
		unsigned int reserved0		: 2;
#endif
		
	} bits;
	unsigned int word;
		
} saCmd1_t;

typedef struct saRecord_s
{
	saCmd0_t	 saCmd0;
	saCmd1_t	 saCmd1;
	unsigned int saKey[8];
	unsigned int saIDigest[8];
	unsigned int saODigest[8];
	unsigned int saSpi;
	unsigned int saSeqNum[2];
	unsigned int saSeqNumMask[2];
	unsigned int saNonce;

} saRecord_t;

typedef struct saState_s
{
	unsigned int stateIv[4];
	unsigned int stateByteCnt[2];
	unsigned int stateIDigest[8];

} saState_t;






#endif

