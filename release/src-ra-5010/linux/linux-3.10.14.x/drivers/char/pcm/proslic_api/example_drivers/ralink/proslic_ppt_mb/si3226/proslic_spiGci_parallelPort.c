/*
** $Id: proslic_spiGci_parallelPort.c 109 2008-10-22 19:45:09Z lajordan@SILABS.COM $
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
** that initializes and talks to the proslic motherboard.
**
** Dependancies:
** 
**
*/
#include "../iowrapper.h" // This is the motherboard interface library
#include "si_voice_datatypes.h"
#include "si_voice_ctrl.h"
#include "../spi.h"
#include "../io.h"

#ifdef PROFILE

typedef struct {
	int regAccess;
	int ramAccess;
} profileType;

profileType profile = {0,0};

#endif

#define PPTADDR 0x378 //parallel port address 

static unsigned char init[]=
{
0x8,
0x4,
0x8,
0x4,
0x19,
0x1D,
0x19,
0x1D,
0x1D,
0
};

static unsigned char Output_clocking[]=
{
0x1C,  /* Padding CS enable time */
0x8,0xC,  
0x8,0xC,  
0x8,0xC,    
0x8,0xC,  

0x8,0xC,
0x8,0xC,
0x8,0xC,
0x8,0xC,
0x1D,
0
};
  
static unsigned char Input_clocking[]=
{
0x1C,  /* Padding CS enable time */	
0x0,0x4, 
0x0,0x4, 
0x0,0x4, 
0x0,0x4, 

0x0,0x4,
0x0,0x4,
0x0,0x4,
0x0,0x4,

0x1D,
0xFF
};

int lastChan=0;

////////////////////////////////////////////////////////////////////////////////
static void byteToSpi (uInt16 portID,uInt8 byte )

{	int i=0, single_bit_mask,single_bit;
    unsigned char ppbuffer;
	single_bit_mask= 0x080;
	single_bit = 0;
    while (Output_clocking[i]!=0) {
	if (Output_clocking [i] ==8) 
	{
		
		single_bit = (byte & single_bit_mask) ? 2 :0;  // This is bit 1 of //port
			single_bit_mask >>=1;
	}
		ppbuffer = (unsigned char )(single_bit | Output_clocking [i]);
		PortOut (portID,ppbuffer);
		
	// When the clock is low change the data
i++;
} // while
}// byteToSpi

//*************************************************************
static uInt8 spiToByte(uInt16 portID)

{	unsigned char  ppbuffer, inputBuffer;
	int i=0;
	uInt8 data=0;
	i=0;
	while (Input_clocking[i] !=0xff) 
	{ 
		ppbuffer= Input_clocking[i] ;
		PortOut(portID,ppbuffer);
	

		if (Input_clocking[i]==0) 
		{
			data <<=1; // This includes one shift which is ignored
	
			inputBuffer = PortIn(portID+1);
			data |= (0x80 & inputBuffer)? 0:1;
		}

		i++;
	} // while
	
return data;
}// spiToByte

#define CNUM_TO_CID_QUAD(channelNumber)   (((channelNumber<<4)&0x10)|((channelNumber<<2)&0x8)|((channelNumber>>2)&0x2)|((channelNumber>>4)&0x1)|(channelNumber&0x4))

static uInt8 ReadReg (uInt16 portID, uInt8 channel, uInt8 regAddr){
	uInt8 regCtrl = CNUM_TO_CID_QUAD(channel)|0x60;
#ifdef PROFILE
	profile.regAccess++;
#endif
	byteToSpi(portID,regCtrl);
	byteToSpi(portID,regAddr);
	return spiToByte(portID);
}

static void WriteReg (uInt16 portID, uInt8 channel, uInt8 regAddr, uInt8 data){
	uInt8 regCtrl = CNUM_TO_CID_QUAD(channel)|0x20;
	/*if (lastChan != channel)
		printf ("CHANNEL %d\n",channel);
	lastChan = channel;
	if (regAddr < 5 || regAddr > 10)
		printf ("REGISTER %d = %X\n",regAddr,data);*/
#ifdef PROFILE
	profile.regAccess++;
#endif
	if (channel == 0xff)
		regCtrl = 0x20 | 0x80;
	byteToSpi(portID,regCtrl);
	byteToSpi(portID,regAddr);
	byteToSpi(portID,data);
}


static void WriteRam16Bits (uInt16 portID, uInt8 channel, uInt16 regAddr, uInt16 data){
	
	if ((regAddr>>3)&0xe0)
		WriteReg(portID,channel,5,(regAddr>>3)&0xe0); //write upper address bits
	if (channel == 0xff)
		byteToSpi(portID,0x80);  // Write the control byte
	else
		byteToSpi(portID,CNUM_TO_CID_QUAD(channel));  // Write the control byte
	byteToSpi(portID,(uInt8)regAddr);                                 // Write the RAM address
	byteToSpi(portID,data >> 8);                               // Write the MSB of data
	byteToSpi(portID,data & 0xFF);                             // Write the LSB of data

	if ((regAddr>>3)&0xe0)
		WriteReg(portID,channel,5,0); //clear upper address bits
}

static uInt16 ReadRam16Bits (uInt16 portID, uInt8 channel, uInt16 regAddr){
	uInt16 data;
	if ((regAddr>>3)&0xe0)
		WriteReg(portID,channel,5,(regAddr>>3)&0xe0); //write upper address bits

	byteToSpi(portID,0x40 | CNUM_TO_CID_QUAD(channel));  // Write the control byte
	byteToSpi(portID,(uInt8)(uInt8)regAddr);                                 // Write the RAM address
	data = spiToByte(portID) << 8;                             // High byte RAM data
	data |= spiToByte(portID);  

	if ((regAddr>>3)&0xe0)
		WriteReg(portID,channel,5,0); //clear upper address bits
	return data;
}



static void RAMwait (uInt16 portID,uInt8 channel)
{
	uInt8 regVal; 
	regVal = ReadReg (portID,channel,4);
	while (regVal&0x01)
	{
		regVal = ReadReg (portID,channel,4);
	}//wait for indirect registers

}

static void WriteRam (uInt16 portID, uInt8 channel, uInt16 regAddr, uInt32 data){
#ifdef PROFILE
	profile.ramAccess++;
#endif
	/*if (lastChan != channel)
		printf ("CHANNEL %d\n",channel);
	lastChan = channel;
	if (regAddr != 1359 && regAddr != 1358)
		printf ("RAM %d = %X\n",regAddr,data);*/
	if (channel == 0xff)
		RAMwait(portID,0);   
	else
		RAMwait(portID,channel);   
	WriteReg(portID,channel,5,(regAddr>>3)&0xe0); //write upper address bits
	
		WriteReg (portID,channel,6,(data<<3)&0xff);
		WriteReg (portID,channel,7,(data>>5)&0xff);
		WriteReg (portID,channel,8,(data>>13)&0xff);
		WriteReg (portID,channel,9,(data>>21)&0xff);
	
		WriteReg (portID,channel,10,regAddr&0xff); //write lower address bits  
	
}

static uInt32 ReadRam (uInt16 portID, uInt8 channel, uInt16 regAddr){
	unsigned char reg; unsigned long RegVal;
#ifdef PROFILE
	profile.ramAccess++;
#endif
    RAMwait(portID,channel);
	WriteReg (portID,channel,5,(regAddr>>3)&0xe0); //write upper address bits
	
		WriteReg (portID,channel,10,regAddr&0xff); //write lower address bits
	
	RAMwait(portID,channel);
	
		reg=ReadReg (portID,channel,6);
		RegVal = reg>>3;
		reg=ReadReg(portID,channel,7);
		RegVal |= ((unsigned long)reg)<<5;
		reg=ReadReg(portID,channel,8);
		RegVal |= ((unsigned long)reg)<<13;
		reg=ReadReg(portID,channel,9);
		RegVal |= ((unsigned long)reg)<<21;
	
	return RegVal;
}

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
	//establish link to driver
	int i;
	hSpi->portID = PPTADDR;
	i = LoadIODLL();
	if( i == 0)
	{
		PortOut(hSpi->portID,0x1D);
	}
	return (!i);
}


/*
** Function: spiGci_ResetWrapper
**
** Description: 
** Sets the reset pin of the ProSLIC
*/
int ctrl_ResetWrapper (ctrl_S *hSpiGci, int status){
	uInt8 c;
	c = PortIn (hSpiGci->portID+2);
	if (status){ 
		PortOut(hSpiGci->portID+2,c | 0x1);
	}
	else{
		PortOut(hSpiGci->portID+2,c&0xfe);
	}
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
uInt8 ctrl_ReadRegisterWrapper (ctrl_S *hSpiGci, uInt8 channel, uInt8 regAddr){
	

	return ReadReg(hSpiGci->portID,channel,regAddr);
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
int ctrl_WriteRegisterWrapper (ctrl_S *hSpiGci, uInt8 channel, uInt8 regAddr, uInt8 data){

	WriteReg(hSpiGci->portID,channel,regAddr, data);
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
ramData ctrl_ReadRAMWrapper (ctrl_S *hSpiGci, uInt8 channel, uInt16 ramAddr){
	ramData data;
#ifdef ALLBITS
	data = ReadRamAllBits(hSpiGci->portID,channel,ramAddr);
#else
	data = ReadRam(hSpiGci->portID,channel,ramAddr);
#endif
	return data;
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
int ctrl_WriteRAMWrapper (ctrl_S *hSpiGci, uInt8 channel, uInt16 ramAddr, ramData data){
#ifdef ALLBITS
	WriteRamAllBits(hSpiGci->portID,channel,ramAddr,data);
#else
	WriteRam(hSpiGci->portID,channel,ramAddr,data);
#endif
	return 0;
}
