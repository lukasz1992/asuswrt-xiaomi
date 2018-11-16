/*
** $Id: dummy_spi.c 3411 2012-04-04 23:11:51Z nizajerk $
**
** spi.h
** SPI driver implementation file
**
** Author(s): 
** laj
**
** Distributed by: 
** Silicon Laboratories, Inc
**
** File Description:
** This is the implementation file for the SPI driver used 
** in the ProSLIC demonstration code. 
**
** Dependancies:
** 
**
*/
#include "spi.h"
#include "si_voice_datatypes.h"


/*
** Function: SPI_Init
**
** Description: 
** Initializes the SPI interface
**
** Input Parameters: 
** none
**
** Return:
** none
*/
int SPI_Init (ctrl_S * hSpi){
	return 0;
}


/*
** Function: spiGci_ResetWrapper
**
** Description: 
** Sets the reset pin of the ProSLIC
*/
int ctrl_ResetWrapper (void * hSpiGci, int status){
	/*if (status)
		RSTNLow();
	else
		RSTNHigh();*/
	return 0;
}

/*
** SPI/GCI register read 
**
** Description: 
** Reads a single ProSLIC register
**
** Input Parameters: 
** channel: ProSLIC channel to read from
** num: number of reads to perform
** regAddr: Address of register to read
** addr_inc: whether to increment address after each read
** data: data to read from register
**
** Return:
** none
*/
uInt8 ctrl_ReadRegisterWrapper (void * hSpiGci, uInt8 channel,uInt8 regAddr){
	return 0xAA;
		
}


/*
** Function: spiGci_WriteRegisterWrapper 
**
** Description: 
** Writes a single ProSLIC register
**
** Input Parameters: 
** channel: ProSLIC channel to write to
** address: Address of register to write
** data: data to write to register
**
** Return:
** none
*/
int ctrl_WriteRegisterWrapper (void * hSpiGci, uInt8 channel,  uInt8 regAddr, uInt8 data){
	
		
	return 0;
}


/*
** Function: SPI_ReadRAMWrapper
**
** Description: 
** Reads a single ProSLIC RAM location
**
** Input Parameters: 
** channel: ProSLIC channel to read from
** address: Address of RAM location to read
** pData: data to read from RAM location
**
** Return:
** none
*/
uInt32 ctrl_ReadRAMWrapper (void * hSpiGci, uInt8 channel, uInt16 ramAddr){
	return 0xAAAAAAAAL;
}


/*
** Function: SPI_WriteRAMWrapper
**
** Description: 
** Writes a single ProSLIC RAM location
**
** Input Parameters: 
** channel: ProSLIC channel to write to
** address: Address of RAM location to write
** data: data to write to RAM location
**
** Return:
** none
*/
int ctrl_WriteRAMWrapper (void * hSpiGci, uInt8 channel, uInt16 ramAddr, ramData data){

	return 0;
}



/*
** $Log: dummy_spi.c,v $
** Revision 1.3  2008/07/24 21:06:16  lajordan
** no message
**
** Revision 1.2  2007/10/22 21:38:10  lajordan
** fixed some warnings
**
** Revision 1.1  2007/10/22 20:52:08  lajordan
** no message
**
*/

