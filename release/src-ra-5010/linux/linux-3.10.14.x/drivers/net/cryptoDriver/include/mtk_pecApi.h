
#ifndef EIP93_PEC_API_H
#define EIP93_PEC_API_H

#include "mtk_baseDefs.h"
#include "mtk_dmaBuf.h"         // DMABuf_Handle_t

/*----------------------------------------------------------------------------
 *         Dependency on DMA Buffer Allocat API (mtk_dmaBuf.h)
 *
 * This API requires the use of the DMA Buffer Allocation API. All buffers
 * are passed through this API as handles.
 */


/*----------------------------------------------------------------------------
 * PEC_Status_t
 */
typedef enum
{
    PEC_STATUS_OK = 0,
    PEC_ERROR_BAD_PARAMETER,
    PEC_ERROR_BAD_HANDLE,
    PEC_ERROR_BAD_USE_ORDER,
    PEC_ERROR_INTERNAL,
    PEC_ERROR_NOT_IMPLEMENTED

} PEC_Status_t;


/*----------------------------------------------------------------------------
 * PEC_InitBlock_t
 *
 * This structure contains service initialization parameters that are passed
 * to the PEC_Init call.
 *
 * For forward-compatibility with future extensions, please zero-init the
 * data structure before setting the fields and providing it to PEC_Init.
 * Example: PEC_InitBlock_t InitBlock = {0};
 */
typedef struct
{
    bool fUseDynamicSA;             // true = use variable-size SA format

    // the following fields are related to Scatter/Gather
    // not all engines support this
    unsigned int FixedScatterFragSizeInBytes;

} PEC_InitBlock_t;


/*----------------------------------------------------------------------------
 * PEC_CommandDescriptor_t
 *
 * This data structure describes a transform request that will be queued up
 * for processing by the transform engine. It refers to the input and output
 * data buffers, the SA buffer(s) and contains parameters that describe the
 * transform, like the length of the data.
 */
typedef struct
{
    // the pointer that will be returned in the related result descriptor
    void * User_p;

    // data buffers
    DMABuf_Handle_t SrcPkt_Handle;
    DMABuf_Handle_t DstPkt_Handle;
    unsigned int SrcPkt_ByteCount;
    unsigned int Bypass_WordCount;

    // SA reference
    uint32_t SA_WordCount;
    DMABuf_Handle_t SA_Handle1;
    DMABuf_Handle_t SA_Handle2;

    // Engine specific control fields
    // with Transform specific values
    uint32_t Control1;
    uint32_t Control2;

} PEC_CommandDescriptor_t;


/*----------------------------------------------------------------------------
 * PEC_ResultDescriptor_t
 *
 * This data structure contains the result of the transform request. The user
 * can match the result to a command using the User_p field.
 *
 * A number of parameters from the command descriptor are also returned, as a
 * courtesy service.
 */
typedef struct
{
    // the pointer that from the related command descriptor
    void * User_p;

    // data buffers
    DMABuf_Handle_t SrcPkt_Handle;
    DMABuf_Handle_t DstPkt_Handle;
    uint32_t DstPkt_ByteCount;
    void * DstPkt_p;

    unsigned int Bypass_WordCount;

    // Engine specific status fields
    // with Transform specific values
    uint32_t Status1;
    uint32_t Status2;

} PEC_ResultDescriptor_t;


/*----------------------------------------------------------------------------
 * PEC_NotifyFunction_t
 *
 * This type specifies the callback function prototype for the function
 * PEC_CommandNotify_Request and PEC_ResultNotify_Request.
 * The notification will occur only once.
 *
 * NOTE: The exact context in which the callback function is invoked and the
 *       allowed actions in that callback are implementation specific. The
 *       intention is that all API functions can be used, except PEC_UnInit.
 */
typedef void (* PEC_NotifyFunction_t)(void);



/*----------------------------------------------------------------------------
 * PEC_Init
 *
 * This function must be used to initialize the service. No API function may
 * be used before this function has returned.
 */
PEC_Status_t
PEC_Init(
        const PEC_InitBlock_t * const InitBlock_p);


/*----------------------------------------------------------------------------
 * PEC_UnInit
 *
 * This call un-initializes the service. Use only when there are no pending
 * transforms. The caller must make sure that no API function is used while or
 * after this function executes.
 */
PEC_Status_t
PEC_UnInit(void);



#endif 


