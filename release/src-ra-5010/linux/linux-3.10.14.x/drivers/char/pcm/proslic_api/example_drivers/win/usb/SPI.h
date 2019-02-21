/*
** $Id: SPI.h 3411 2012-04-04 23:11:51Z nizajerk $
** 
** This file is system specific and should be edited for your hardware platform
**
** This file is used by proslic_timer_intf.h and proslic_spiGci.h
*/
#ifndef SPI_TYPE_H
#define SPI_TYPE_H

/*
** SPI/GCI structure
*/
typedef struct{
	unsigned short portID;
} ctrl_S;

/*
** Function: SPI_Init
**
** Description: 
** Initializes the SPI interface
*/
int SPI_Init (ctrl_S *hSpi);


/*
** Function: ctrl_ResetWrapper
**
** Description: 
** Sets the reset pin of the ProSLIC
*/
int ctrl_ResetWrapper (void *hCtrl, int status);

/*
** register read 
**
** Description: 
** Reads ProSLIC registers
*/
unsigned char ctrl_ReadRegisterWrapper (void *hCtrl, uInt8 channel, unsigned char regAddr);

/*
** Function: ctrl_WriteRegisterWrapper
**
** Description: 
** Writes ProSLIC registers
*/
int ctrl_WriteRegisterWrapper (void *hSpiGci, uInt8 channel, unsigned char regAddr, unsigned char data);

/*
** Function: ctrl_WriteRAMWrapper
**
** Description: 
** Writes ProSLIC RAM
*/
int ctrl_WriteRAMWrapper (void *hSpiGci, uInt8 channel, uInt16 ramAddr, ramData data);

/*
** Function: ctrl_ReadRAMWrapper
**
** Description: 
** Reads ProSLIC RAM
*/
ramData ctrl_ReadRAMWrapper(void *hSpiGci, uInt8 channel, uInt16 ramAddr);

#endif
