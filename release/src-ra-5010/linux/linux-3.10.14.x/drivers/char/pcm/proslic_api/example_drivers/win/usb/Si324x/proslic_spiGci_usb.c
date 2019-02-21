/*
** $Id: proslic_spiGci_usb.c 3411 2012-04-04 23:11:51Z nizajerk $
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
** in the ProSLIC demonstration code. It calls the library
** that initializes and talks to the voice motherboard.
**
** Dependancies:
** 
**
*/
#include "DLLWrapper.h" // This is the motherboard interface library
#include "si_voice_datatypes.h"
#include "si_voice_ctrl.h"
#include "spi.h"
#include "proslic.h"


#define FIRMWAREPATH "voicemb.out" //path to voice motherboard firmware

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
int SPI_Init (ctrl_S *hSpi){
	//establish usb link, initialize motherboard, start up clocks
	int result;
    SILABS_UNREFERENCED_PARAMETER(hSpi);
	result = openVoiceIntf (0x378,1,FIRMWAREPATH);  
	UNReset();
	return result;
}


/*
** Function: ctrl_ResetWrapper
**
** Description: 
** Sets the reset pin of the ProSLIC
*/
int ctrl_ResetWrapper (void *hctrl, int status){
    SILABS_UNREFERENCED_PARAMETER(hctrl);
	if (status)
		Reset();
	else
		UNReset();
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
** regAddr: Address of register to read
** return data: data to read from register
**
** Return:
** none
*/
uInt8 ctrl_ReadRegisterWrapper (void * hctrl, uInt8 channel, uInt8 regAddr){
    SILABS_UNREFERENCED_PARAMETER(hctrl);
	setChannel(channel);
	
	return QuadReadRegExp(regAddr);
		
}


/*
** Function: ctrl_WriteRegisterWrapper 
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
int ctrl_WriteRegisterWrapper (void * hctrl, uInt8 channel, uInt8 regAddr, uInt8 data){
    SILABS_UNREFERENCED_PARAMETER(hctrl);
	if (channel == 0xff){ //broadcast
		setBroadcastmode(1);
	}
	else
		setChannel(channel);
	
	QuadWriteRegExp(regAddr, data);
		
	if (channel == 0xff){
		setBroadcastmode(0);
	}
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
ramData ctrl_ReadRAMWrapper (void *hctrl, uInt8 channel, uInt16 ramAddr){
	SILABS_UNREFERENCED_PARAMETER(hctrl);
	setChannel(channel);
	return QuadReadRamExp(ramAddr);
}



static void RAMwait (ctrl_S * hctrl)
{
	unsigned char regVal; 
	
		
	do
	{
		//result=0;
		//for (i=0;i<3;i++){
			regVal = ctrl_ReadRegisterWrapper(hctrl,0,4);
		//	if (regVal != 0xff)
		//		result |= regVal;
		//}
	}while (regVal&0x01);//wait for indirect registers

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
int ctrl_WriteRAMWrapper (void *hctrl, uInt8 channel, uInt16 ramAddr, ramData data){
	uInt8 data1;
	if (channel == 0xff){//broadcast
		RAMwait((ctrl_S *)hctrl);
		data1 = (uInt8)((ramAddr>>3)&0xe0);
		ctrl_WriteRegisterWrapper(hctrl,0xff,5,data1);
		data1 = (uInt8)((data<<3)&0xff);
		ctrl_WriteRegisterWrapper(hctrl,0xff,6,data1);
		data1 = (uInt8)((data>>5)&0xff);
		ctrl_WriteRegisterWrapper(hctrl,0xff,7,data1);
		data1 = (uInt8)((data>>13)&0xff);
		ctrl_WriteRegisterWrapper(hctrl,0xff,8,data1);
		data1 = (uInt8)((data>>21)&0xff);
		ctrl_WriteRegisterWrapper(hctrl,0xff,9,data1);
		data1 =(uInt8)(ramAddr&0xff);
		ctrl_WriteRegisterWrapper(hctrl,0xff,10,data1);
		RAMwait((ctrl_S *)hctrl);
	}
	else {
		setChannel(channel);
		QuadWriteRamExp(ramAddr,data);
	}
	return 0;
}

