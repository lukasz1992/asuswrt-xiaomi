
#ifndef EIP93_ADAPTER_INTERNAL_H
#define EIP93_ADAPTER_INTERNAL_H

#include "mtk_baseDefs.h"
#include "mtk_cLib.h"
#include "mtk_hwDmaAccess.h"          // HWPAL_DMAResource_t
#include "mtk_dmaBuf.h"             // DMABuf_Handle_t
#include "mtk_eip93.h"                // EIP93_IOArea_t
#include "mtk_cAdapter.h"


/*----------------------------------------------------------------------------
 *                           Implementation helper macros
 *----------------------------------------------------------------------------
 */

#define ZEROINIT(_x)  memset(&_x, 0, sizeof(_x))



/*----------------------------------------------------------------------------
 *                           Adapter_EIP93
 *----------------------------------------------------------------------------
 */

extern EIP93_IOArea_t Adapter_EIP93_IOArea;
extern unsigned int Adapter_EIP93_MaxDescriptorsInRing;

bool
Adapter_EIP93_Init(void);

bool
Adapter_EIP93_SetMode_Idle(void);

bool
Adapter_EIP93_SetMode_ARM(
        const bool fEnableDynamicSA);


void
Adapter_EIP93_UnInit(void);


void
Adapter_GetEIP93PhysAddr(
        DMABuf_Handle_t Handle,
        HWPAL_DMAResource_Handle_t * const DMAHandle_p,
        EIP93_DeviceAddress_t * const EIP93PhysAddr_p);

#ifdef MTK_CRYPTO_DRIVER
extern void mtk_interruptHandler_descriptorDone(void);
extern void mtk_BH_handler_resultGet(unsigned long data);
#endif

/*----------------------------------------------------------------------------
 * Adapter_EIP93_InterruptHandler_DescriptorDone
 *
 * This function is invoked when the EIP93 has activated the descriptor done
 * interrupt.
 */
extern void
Adapter_EIP93_InterruptHandler_DescriptorDone(void);


extern void
Adapter_EIP93_BH_Handler_ResultGet(
        unsigned long data);


/*----------------------------------------------------------------------------
 *                           Adapter_Interrupts
 *----------------------------------------------------------------------------
 */

bool
Adapter_Interrupts_Init(
        const int nIRQ);

void
Adapter_Interrupts_UnInit(void);

typedef void (* Adapter_InterruptHandler_t)(void);

// nIRQ is defined in mtk_interrupts.h
void
Adapter_Interrupt_SetHandler(
        const int nIRQ,
        Adapter_InterruptHandler_t HandlerFunction);

void
Adapter_Interrupt_Enable(
        const int nIRQ);

void
Adapter_Interrupt_ClearAndEnable(
        const int nIRQ);

void
Adapter_Interrupt_Disable(
        const int nIRQ);




/*----------------------------------------------------------------------------
 *                           Adapter_DMABuf
 *----------------------------------------------------------------------------
 */

#define ADAPTER_DMABUF_ALLOCATORREF_KMALLOC 'k'   /* kmalloc */

extern const DMABuf_Handle_t Adapter_DMABuf_NullHandle;

bool
Adapter_DMABuf_IsValidHandle(
        DMABuf_Handle_t Handle);

HWPAL_DMAResource_Handle_t
Adapter_DMABuf_Handle2DMAResourceHandle(
        DMABuf_Handle_t Handle);

bool
Adapter_DMABuf_IsForeignAllocated(
        DMABuf_Handle_t Handle);

bool
Adapter_DMABuf_IsSameHandle(
        const DMABuf_Handle_t * const Handle1_p,
        const DMABuf_Handle_t * const Handle2_p);

/*----------------------------------------------------------------------------
 *                           VTBAL Global device
 *----------------------------------------------------------------------------
 */
extern void *  GlobalVTBALDevice ;

#endif 

