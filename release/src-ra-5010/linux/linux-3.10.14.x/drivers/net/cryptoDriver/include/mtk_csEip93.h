#ifndef EIP93_CS_EIP93_H
#define EIP93_CS_EIP93_H

/*---------------------------------------------------------------------------
 * flag to control paramater check 
 */
#ifndef VDRIVER_PERFORMANCE
#define EIP93_STRICT_ARGS 1  //uncomment when paramters check needed
#endif



//  Enable these flags for swapping PD, SA, DATA or register read/writes
//  by packet engine

//#define  EIP93_ENABLE_SWAP_PD
//#define  EIP93_ENABLE_SWAP_SA
//#define  EIP93_ENABLE_SWAP_DATA
//#define  EIP93_ENABLE_SWAP_REG_DATA

// Size of buffer for Direct Host Mode
#define EIP93_RAM_BUFFERSIZE_BYTES 256 //for EIP93-IESW
// #define EIP93_RAM_BUFFERSIZE_BYTES 2048 - for EIP93-I

#endif
