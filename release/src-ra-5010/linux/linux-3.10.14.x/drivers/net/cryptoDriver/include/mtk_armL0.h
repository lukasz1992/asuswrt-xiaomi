
#ifndef EIP93_ARM_LEVEL0_H
#define EIP93_ARM_LEVEL0_H

#include "mtk_baseDefs.h"             // BIT definitions, bool, uint32_t
#include "mtk_hwAccess.h"              // Read32, Write32, HWPAL_Device_t
#include "mtk_hwDmaAccess.h"          // Read32, Write32, HWPAL_DMAResource_t
#include "mtk_L0.h"         // the generic level0 functions
#include "mtk_hwInterface.h"   // the HW interface (register map)
//TreyZ
extern void (*_dma_cache_wback_inv)(unsigned long start, unsigned long size);
#define dma_cache_wback_inv(start, size)	_dma_cache_wback_inv(start,size)
#define VPint *(volatile unsigned int *)
/*-----------------------------------------------------------------------------
 * EIP93 register routines
 *
 * These routines write/read register values in EIP93 registers
 * in HW specific format.
 *
 * Note: if a function argument implies a flag ('f' is a prefix),
 *       then only the values 0 or 1 are allowed for this argument.
 */

static inline void
EIP93_Write32_PE_CDR_BASE(
        HWPAL_Device_t Device,
        const uint32_t CDRBaseAddress)
{
    EIP93_Write32(Device, EIP93_REG_PE_CDR_BASE, CDRBaseAddress);
}

static inline void
EIP93_Read32_PE_CDR_BASE(
        HWPAL_Device_t Device,
        uint32_t * const CDRBaseAddress)
{
    if(CDRBaseAddress)
        *CDRBaseAddress = EIP93_Read32(Device, EIP93_REG_PE_CDR_BASE);
}

static inline void
EIP93_Write32_PE_RDR_BASE(
        HWPAL_Device_t Device,
        const uint32_t RDRBaseAddress)
{
    EIP93_Write32(Device, EIP93_REG_PE_RDR_BASE, RDRBaseAddress);
}

static inline void
EIP93_Read32_PE_RDR_BASE(
        HWPAL_Device_t Device,
        uint32_t * const RDRBaseAddress)
{
    if(RDRBaseAddress)
        *RDRBaseAddress = EIP93_Read32(Device, EIP93_REG_PE_RDR_BASE);
}

static inline void
EIP93_Write32_PE_RING_SIZE(
        HWPAL_Device_t Device,
        const uint16_t RingOffset,
        const uint16_t RingSize)
{
    EIP93_Write32(
            Device,
            EIP93_REG_PE_RING_CONFIG,
            ((RingOffset & (BIT_8-1)) << 16) | ( RingSize & (BIT_10-1)) );
}

static inline void
EIP93_Read32_PE_RING_SIZE(
        HWPAL_Device_t Device,
        uint16_t * const RingOffset,
        uint16_t * const RingSize)
{
    uint32_t word = EIP93_Read32(Device, EIP93_REG_PE_RING_CONFIG);
    if(RingOffset)
        *RingOffset = (word >> 16) & (BIT_8-1);
    if(RingSize)
        *RingSize =  word & (BIT_10-1);
}



static inline void
EIP93_Read32_PE_DMA_STAT(
        HWPAL_Device_t Device,
        uint8_t * const fPeInputDone,
        uint8_t * const fPeOutputDone,
        uint8_t * const fEncryptionDone,
        uint8_t * const fInnerHashDone,
        uint8_t * const fOuterHashDone,
        uint8_t * const fCryptoPadFault,
        uint8_t * const fSPIMismatch,
        uint8_t * const fEXTError,
    uint8_t * const fPeOperationDone,
        uint8_t * const fInputRequestActive,
        uint8_t * const fOutputRequestActive,
        uint16_t * const PeInputSize,
        uint16_t * const PeOutputSize)
{
    uint32_t word = EIP93_Read32(Device, EIP93_REG_PE_STATUS);
    if(fPeInputDone)
        *fPeInputDone = word & 1;
    if(fPeOutputDone)
        *fPeOutputDone = (word >> 1) & 1;
    if(fEncryptionDone)
        *fEncryptionDone = (word >> 2) & 1;
    if(fInnerHashDone)
        *fInnerHashDone = (word >> 3) & 1;
    if(fOuterHashDone)
        *fOuterHashDone = (word >> 4) & 1;
    if(fCryptoPadFault)
        *fCryptoPadFault = (word >> 5) & 1;
    if(fSPIMismatch)
        *fSPIMismatch = (word >> 7) & 1;
    if(fEXTError)
        *fEXTError = (word >> 8) & 1;
    if(fPeOperationDone)
        *fPeOperationDone = (word >> 9) & 1;
    if(fInputRequestActive)
        *fInputRequestActive = (word >> 10) & 1;
    if(fOutputRequestActive)
        *fOutputRequestActive = (word >> 11) & 1;
    if(PeInputSize)
        *PeInputSize = (word >> 12) & (BIT_10-1);
    if(PeOutputSize)
        *PeOutputSize = (word >> 22) & (BIT_10-1);
}



static inline void
EIP93_Read32_PE_RING_PNTR(
        HWPAL_Device_t Device,
        uint16_t * const NextCDROffset,
    uint16_t * const NextRDROffset)
{
    uint32_t word = EIP93_Read32(Device, EIP93_REG_PE_RING_RW_PNTR);
    if(NextCDROffset)
        *NextCDROffset = word  & (BIT_10-1);
    if(NextRDROffset)
        *NextRDROffset = (word >> 16) & (BIT_10-1);
}


static inline void
EIP93_Read32_INT_UNMASK_STAT(
        HWPAL_Device_t Device,
        uint8_t * const fPeDescDoneIRQ,
        uint8_t * const fPeDescInputIRQ,
        uint8_t * const fPeOpDoneIRQ,
        uint8_t * const fPeInputBufferIRQ,
        uint8_t * const fPeOutputBufferIRQ,
        uint8_t * const fPeErrIRQ
       // uint8_t * const fPrngDoneIRQ
        )
{
    uint32_t word = EIP93_Read32(Device, EIP93_REG_INT_UNMASK_STAT);
    if(fPeDescDoneIRQ)
        *fPeDescDoneIRQ = word & 1;
    if(fPeDescInputIRQ)
        *fPeDescInputIRQ = (word >> 1) & 1;
    if(fPeOpDoneIRQ)
        *fPeOpDoneIRQ = (word >> 9) & 1;
    if(fPeInputBufferIRQ)
        *fPeInputBufferIRQ = (word >> 10) & 1;
    if(fPeOutputBufferIRQ)
        *fPeOutputBufferIRQ = (word >> 11) & 1;
    if(fPeErrIRQ)
        *fPeErrIRQ = (word >> 13) & 1;

}

static inline void
EIP93_Read32_INT_MASK_STAT(
        HWPAL_Device_t Device,
        uint8_t * const fPeDescDoneIRQ,
        uint8_t * const fPeDescInputIRQ,
        uint8_t * const fPeOpDoneIRQ,
        uint8_t * const fPeInputBufferIRQ,
        uint8_t * const fPeOutputBufferIRQ,
        uint8_t * const fPeErrIRQ,
        uint8_t * const fPrngDoneIRQ)
{
    uint32_t word = EIP93_Read32(Device, EIP93_REG_INT_MASK_STAT);
    if(fPeDescDoneIRQ)
        *fPeDescDoneIRQ = word & 1;
    if(fPeDescInputIRQ)
        *fPeDescInputIRQ = (word >> 1) & 1;
    if(fPeOpDoneIRQ)
        *fPeOpDoneIRQ = (word >> 9) & 1;
    if(fPeInputBufferIRQ)
        *fPeInputBufferIRQ = (word >> 10) & 1;
    if(fPeOutputBufferIRQ)
        *fPeOutputBufferIRQ = (word >> 11) & 1;
    if(fPeErrIRQ)
        *fPeErrIRQ = (word >> 13) & 1;
    if(fPrngDoneIRQ)
        *fPrngDoneIRQ = (word >> 12) & 1;
}


static inline void
EIP93_Write32_INT_CLR(
        HWPAL_Device_t Device,
        const uint8_t fPeDescDoneIRQ,
        const uint8_t fPeDescInputIRQ,
        const uint8_t fPeOpDoneIRQ,
        const uint8_t fPeInputBufferIRQ,
        const uint8_t fPeOutputBufferIRQ,
        const uint8_t fPeErrIRQ,
        const uint8_t fPrngDoneIRQ)
{
    EIP93_Write32(Device, EIP93_REG_INT_CLR,
                  (fPeDescDoneIRQ & 1) |
                  ((fPeDescInputIRQ & 1) << 1) |
                  ((fPeOpDoneIRQ & 1) << 9) |
                  ((fPeInputBufferIRQ & 1) << 10) |
                  ((fPeOutputBufferIRQ & 1) << 11) |
                  ((fPeErrIRQ & 1) << 13) |
                  ((fPrngDoneIRQ & 1) << 12));
}

static inline void
EIP93_Read32_PE_CD_COUNT(
        HWPAL_Device_t Device,
        uint32_t * const CmdDescrCount)
{
    uint32_t word = EIP93_Read32(Device, EIP93_REG_PE_CD_COUNT);
    if(CmdDescrCount)
        *CmdDescrCount = word & (BIT_10-1);
}

static inline void
EIP93_Write32_PE_CD_COUNT(
        HWPAL_Device_t Device,
        const uint32_t CmdDescrCount)
{
    EIP93_Write32(Device, EIP93_REG_PE_CD_COUNT,
          CmdDescrCount & (BIT_8-1));
}

static inline void
EIP93_Read32_PE_RD_COUNT(
        HWPAL_Device_t Device,
        uint32_t * const ResDescrCount)
{
    uint32_t word = EIP93_Read32(Device, EIP93_REG_PE_RD_COUNT);
    if(ResDescrCount)
        *ResDescrCount = word & (BIT_10-1);
}

static inline void
EIP93_Write32_PE_RD_COUNT(
        HWPAL_Device_t Device,
        const uint32_t ResDescrCount)
{
    EIP93_Write32(Device, EIP93_REG_PE_RD_COUNT,
          ResDescrCount & (BIT_8-1));
}

static inline void
EIP93_Write32_PE_RING_THRESH(
        HWPAL_Device_t Device,
        const uint16_t CmdDescrThreshCnt,
        const uint16_t ResDescrThreshCnt,
    const uint16_t ResRingTimeOut)
{
    EIP93_Write32(
            Device,
            EIP93_REG_PE_RING_THRESH,
            (CmdDescrThreshCnt & (BIT_10-1)) |
        ((ResDescrThreshCnt & (BIT_10-1)) << 16) |
        ((ResRingTimeOut  & (BIT_6-1)) << 26));
}

static inline void
EIP93_Read32_PE_RING_THRESH(
        HWPAL_Device_t Device,
        uint16_t * const CmdDescrThreshCnt,
        uint16_t * const ResDescrThreshCnt,
    uint16_t * const ResRingTimeOut)
{
    uint32_t word = EIP93_Read32(Device, EIP93_REG_PE_RING_THRESH);
    if(CmdDescrThreshCnt)
        *CmdDescrThreshCnt = word  & (BIT_10-1);
    if(ResDescrThreshCnt)
        *ResDescrThreshCnt =  (word >> 16) & (BIT_10-1);
    if(ResRingTimeOut)
        *ResDescrThreshCnt =  (word >> 26) & (BIT_6-1);
}

/*-----------------------------------------------------------------------------
 * ARM routines
 *
 * These routines write/read descriptors in a descriptor ring for ARM
 */
// #define RT_EIP93_DRIVER_DEBUG_H //trey here is in .h file
static inline void
EIP93_ARM_Level0_WriteDescriptor(
        HWPAL_DMAResource_Handle_t Handle,
        const unsigned int WordOffset,
        const EIP93_ARM_CommandDescriptor_t * const Descr_p)

{
    uint32_t word;
#ifdef RT_EIP93_DRIVER_DEBUG_H
    uint32_t *p, k, bytes;
#endif
    // Ctrl/stat word
    // mask out reserved and status fields.
    word = Descr_p->ControlWord & 0xFF00FFF8; //& 0xFF00FF10;
    word |= 1; // Host Ready is set
    HWPAL_DMAResource_Write32(Handle, WordOffset, word);

    // Source address
    HWPAL_DMAResource_Write32(
            Handle,
            WordOffset+1,
            Descr_p->SrcPacketAddr.Addr);



    // Destination address
    HWPAL_DMAResource_Write32(
            Handle,
            WordOffset+2,
            Descr_p->DstPacketAddr.Addr);


    // SA data address
    HWPAL_DMAResource_Write32(
            Handle,
            WordOffset+3,
            Descr_p->SADataAddr.Addr);


   // SA State data address
    HWPAL_DMAResource_Write32(
            Handle,
            WordOffset+4,
            Descr_p->SAStateDataAddr.Addr);


   HWPAL_DMAResource_Write32(
            Handle,
            WordOffset+5,
            Descr_p->SAStateDataAddr.Addr);


    // User ID address
    HWPAL_DMAResource_Write32(
            Handle,
            WordOffset+6,
            Descr_p->UserId);


    // Length word
    word = 0;
    word |= Descr_p->BypassWordLength << 24;
    word |= 1 << 22; // Host Ready is set
    word |= ((BIT_20 - 1) & Descr_p->SrcPacketByteCount);//can not exceed 1MB

    HWPAL_DMAResource_Write32(Handle, WordOffset + 7, word);

	//TreyZ
	if(((BIT_20 - 1) & Descr_p->SrcPacketByteCount) > 0)
		dma_cache_wback_inv((unsigned long)(Descr_p->SrcPacketAddr.Addr | 0x80000000), ((BIT_20 - 1) & Descr_p->SrcPacketByteCount));
 
	//Trey
	#if 0
	{
		uint32_t *Src_p;
		int bytes = ((BIT_20 - 1) & Descr_p->SrcPacketByteCount);
		int k;
		
		Src_p = (uint32_t*)(HWPAL_DMAResource_Read32(Handle, WordOffset+1) | 0xa0000000);
    	printk("\n%d-byte SRC Content from 0x%p:\n", bytes, Src_p);
    	for(k=0; k<bytes/4; k++){
        	printk("0x%08x\t", *(Src_p+k));
        	if(k%5==4) printk("\n");
    	}
    	printk("\n");		
	}
	#endif
	
		      
#ifdef RT_EIP93_DRIVER_DEBUG_H
    printk("\n[EIP93_ARM_Level0_WriteDescriptor]\n");
    printk("ControlWord:0x%08x\n", HWPAL_DMAResource_Read32(Handle, WordOffset)); 

	bytes = HWPAL_DMAResource_Read32(Handle, WordOffset+7) & (BIT_20 - 1);
	p = (uint32_t*)(HWPAL_DMAResource_Read32(Handle, WordOffset+1) | 0xa0000000);
    printk("%d-byte Src packet from 0x%p:\n", bytes, HWPAL_DMAResource_Read32(Handle, WordOffset+1));
    for(k=0; k<bytes/4; k++){
        printk("0x%08x\t", *(p+k));
        if(k%5==4) printk("\n");
    }
    printk("\n");
    
    printk("Dst packet Addr:0x%08x\n", HWPAL_DMAResource_Read32(Handle, WordOffset+2));
    
    p = (uint32_t*)(HWPAL_DMAResource_Read32(Handle, WordOffset+3) | 0xa0000000);
    printk("\n128-byte SA Content from 0x%p:\n", p);
    for(k=0; k<32; k++){
        printk("0x%08x\t", *(p+k));
        if(k%5==4) printk("\n");
    }
    printk("\n");
    
    p = (uint32_t*)(HWPAL_DMAResource_Read32(Handle, WordOffset+4) | 0xa0000000);    
    printk("56-byte SAState Content from 0x%p:\n", p);
    for(k=0; k<14; k++){
        printk("0x%08x\t", *(p+k));
        if(k%5==4) printk("\n");
    }
    printk("\n");

    p = (uint32_t*)(HWPAL_DMAResource_Read32(Handle, WordOffset+5) | 0xa0000000);    
    printk("SAARC4 Content from 0x%p:\n", p);
	printk("\n");

	printk("\nPE_userID:0x%08x\n", HWPAL_DMAResource_Read32(Handle, WordOffset+6));
		    
    printk("\nPE_length:0x%08x\n", HWPAL_DMAResource_Read32(Handle, WordOffset+7));
#endif
//#undef RT_EIP93_DRIVER_DEBUG_H //trey

    // padding words
#ifdef EIP93_ARM_NUM_OF_DESC_PADDING_WORDS
	{
        unsigned i;
        for(i = 0; i < EIP93_ARM_NUM_OF_DESC_PADDING_WORDS; i++)
        {
            HWPAL_DMAResource_Write32(
                    Handle,
                    WordOffset + i + 8 ,
                    Descr_p->PaddingWords[i]);


        }
    }
#endif

#if 0
//Treyb4
if(((BIT_20 - 1) & Descr_p->SrcPacketByteCount)==1266){
	VPint(0x8001ffb4) = VPint(0xbfbf0118);
}
#endif

}


static inline bool
EIP93_ARM_Level0_WriteDescriptor_IfFree(
        HWPAL_DMAResource_Handle_t Handle,
        const unsigned int WordOffset,
        const EIP93_ARM_CommandDescriptor_t * const Descr_p)

{
    uint32_t word0 = 0;
    uint32_t lastword = 0;
    uint32_t pe_done1 = 0;
    uint32_t pe_done2 = 0;

    word0 = HWPAL_DMAResource_Read32(Handle, WordOffset);
    lastword = HWPAL_DMAResource_Read32(
                    Handle,
                    WordOffset+7);

    pe_done1 = word0 & (BIT_2-1);
    pe_done2 = (lastword >> 22) & (BIT_2-1);
    if (pe_done1 == pe_done2 && pe_done1 == 0)
    {
        EIP93_ARM_Level0_WriteDescriptor(
                                         Handle,
                                         WordOffset,
                                         Descr_p);

        return true;
    }
    return false;
}


static inline void
EIP93_ARM_Level0_ClearDescriptor(
        HWPAL_DMAResource_Handle_t Handle,
        const unsigned int WordOffset)
{
    unsigned i = 0;

    for (i=0; i<EIP93_ARM_DESCRIPTOR_SIZE(); i++)
    {
        HWPAL_DMAResource_Write32(Handle, WordOffset + i, 0);
    }
}


static inline void
EIP93_ARM_Level0_ClearAllDescriptors(
        HWPAL_DMAResource_Handle_t Handle,
        const unsigned int DescriptorSpacing,
        const unsigned int NumberOfDescriptors)
{
    const uint32_t Words[8] = { 0 };
    const int nDescrSize = EIP93_ARM_DESCRIPTOR_SIZE();
    unsigned int i;

    for(i = 0; i < NumberOfDescriptors; i++)
    {
        HWPAL_DMAResource_Write32Array(
                Handle,
                i * DescriptorSpacing,
                nDescrSize,
                Words);
    }
}


static inline bool
EIP93_ARM_Level0_ReadDescriptor_IfReady(
        EIP93_ARM_ResultDescriptor_t * const Descr_p,
        const HWPAL_DMAResource_Handle_t Handle,
        const unsigned int WordOffset)
{
    uint32_t word0 = 0;
    uint32_t lastword = 0;
    uint32_t pe_done1 = 0;
    uint32_t pe_done2 = 0;

    word0 = HWPAL_DMAResource_Read32(Handle, WordOffset);
    lastword = HWPAL_DMAResource_Read32(Handle, WordOffset+7);
    
    pe_done1 = word0 & (BIT_2-1);
    pe_done2 = (lastword >> 22) & (BIT_2-1);
    if (pe_done1 == pe_done2 && pe_done1 == 2)
    {
        // Stat word
        Descr_p->StatusWord = word0;

        // Destination data length and bypass length
        Descr_p->BypassWordLength = (lastword >> 24) & (BIT_8-1);
        Descr_p->DstPacketByteCount = lastword & (BIT_20-1);
		
	//Trey
	#if 0
	{
		uint32_t *Dst_p;
		int k;
		
		Dst_p = (uint32_t*)(HWPAL_DMAResource_Read32(Handle, WordOffset+2) | 0xa0000000);
    	printk("\n%d-byte DST Content from 0x%p:\n", Descr_p->DstPacketByteCount, Dst_p);
    	for(k=0; k<Descr_p->DstPacketByteCount/4; k++){
        	printk("0x%08x\t", *(Dst_p+k));
        	if(k%5==4) printk("\n");
    	}
    	printk("\n");
	}
	#endif

        // padding words
#ifdef EIP93_ARM_NUM_OF_DESC_PADDING_WORDS
        {
            unsigned i;
            for (i = 0; i < EIP93_ARM_NUM_OF_DESC_PADDING_WORDS; i++)
            {
                Descr_p->PaddingWords[i] =
                    HWPAL_DMAResource_Read32(
                        Handle,
                        WordOffset + i + 8);
            }
        }
#endif

        return true;
    }
    return false;
}



#endif


