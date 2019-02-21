
#ifndef EIP93_CS_DRIVER_H
#define EIP93_CS_DRIVER_H

// Define this when there is no actual device installed
//#define ADAPTER_EIP93_NO_ACTUAL_DEVICE


//Define if building for Simulator
//#define ADAPTER_USER_DOMAIN_BUILD

// enable for big-endian CPU
//#define ADAPTER_EIP93_ARMRING_ENABLE_SWAP


// activates DMA-enabled autonomous ring mode (ARM)
// or CPU-managed direct host mode (DHM)
// ARM can use overlapping command/result rings, or separate
#define VDRIVER_PE_MODE_ARM
//#define VDRIVER_PE_MODE_DHM

// when defined, two memory block of the size ADAPTER_EIP93_RINGSIZE_BYTES will be allocated
// one for commands, the other for results
#define ADAPTER_EIP93_SEPARATE_RINGS //if no seperate rings, Crypto Engine will die when SmartBits uses more that 30M of flow speed --Trey


// activates interrupts for EIP,
// when disabled, polling will be used
#define VDRIVER_INTERRUPTS


// if not activated, this will switch on bounce-buffer support for all DMA services
// if activated, then bounce buffers will not be created
#define ADAPTER_REMOVE_BOUNCEBUFFERS  //if no BounceBuffer, dma_cache_wback_inv() is needed before kicking off crypto engine --Trey

// when activated, disables all strict-args checking
// and reduces logging to a Critical-only
//#define VDRIVER_PERFORMANCE




#endif 

