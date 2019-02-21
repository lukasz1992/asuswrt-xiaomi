
#ifndef EIP93_C_EIP93_H
#define EIP93_C_EIP93_H

#include "mtk_csEip93.h"

/*---------------------------------------------------------------------------
 * flag to control paramater check
 */

//#define EIP93_STRICT_ARGS 1  //uncomment when paramters check needed
//  Enable these flags for swapping PD, SA, DATA or register read/writes
//  by packet engine

//#define  EIP93_ENABLE_SWAP_PD
//#define  EIP93_ENABLE_SWAP_SA
//#define  EIP93_ENABLE_SWAP_DATA
//#define  EIP93_ENABLE_SWAP_REG_DATA

// Size of buffer for Direct Host Mode

#ifndef EIP93_RAM_BUFFERSIZE_BYTES
#define EIP93_RAM_BUFFERSIZE_BYTES 256 //for EIP93-IESW
// #define EIP93_RAM_BUFFERSIZE_BYTES 2048 - for EIP93-I
#endif

#endif
