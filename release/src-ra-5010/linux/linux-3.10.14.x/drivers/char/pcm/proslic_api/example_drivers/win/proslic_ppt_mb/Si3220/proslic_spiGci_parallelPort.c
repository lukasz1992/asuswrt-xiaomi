/*ddd
** $Id: proslic_spiGci_parallelPort.c 3411 2012-04-04 23:11:51Z nizajerk $
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
#include "proslic_datatypes.h"
#include "proslic_spiGci.h"



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
		ppbuffer = single_bit | Output_clocking [i] ;
		PortOut_C (portID,ppbuffer);
		
	// When the clock is low change the data
i++;
} // while
}// byteToSpi

//*************************************************************
static uInt8 spiToByte(uInt16 portID)

{	unsigned char  ppbuffer, inputBuffer;
	int i=0,  data=0;
	unsigned short lpt1 = portID + 1;
	i=0;
	while (Input_clocking[i] !=0xff) { 
	ppbuffer= Input_clocking[i] ;
	PortOut_C(portID,ppbuffer);
	

if (Input_clocking[i]==0) {
	data <<=1; // This includes one shift which is ignored
	
	inputBuffer = PortIn_C(portID+1);
	data |= (0x80 & inputBuffer)? 0:1;
 }

i++;
} // while
	
return data;
}// spiToByte

#define CNUM_TO_CID(channelNumber)   (((channelNumber<<3)&0x8)|((channelNumber<<1)&0x4)|((channelNumber>>1)&0x2)|((channelNumber>>3)&0x1))

static uInt8 ReadReg (uInt16 portID, uInt8 channel, uInt8 regAddr){
	uInt8 regCtrl = CNUM_TO_CID(channel)|0x60;
	byteToSpi(portID,regCtrl);
	byteToSpi(portID,regAddr);
	return spiToByte(portID);
}

static void WriteReg (uInt16 portID, uInt8 channel, uInt8 regAddr, uInt8 data){
	uInt8 regCtrl = CNUM_TO_CID(channel)|0x20;
	byteToSpi(portID,regCtrl);
	byteToSpi(portID,regAddr);
	byteToSpi(portID,data);
}

static void RAMwait (uInt16 portID,unsigned short channel)
{
	unsigned char regVal; 
	regVal = ReadReg (portID,channel,4);
	while (regVal&0x01)
	{
		regVal = ReadReg (portID,channel,4);
	}//wait for indirect registers

}

static void WriteRam (uInt16 portID, uInt8 channel, uInt16 regAddr, uInt16 data){
	uInt8 regCtrl = CNUM_TO_CID(channel);
	RAMwait(portID,channel);

	byteToSpi(portID,regCtrl);
	byteToSpi(portID,regAddr);
	byteToSpi(portID,(data & 0xFF00)>>8);
	byteToSpi(portID,(data & 0xFF));
}

static uInt16 ReadRam (uInt16 portID, uInt8 channel, uInt16 regAddr){
	unsigned char reg; unsigned short RegVal;
	RAMwait(portID,channel);
	WriteReg(portID,channel,103,regAddr);
	RAMwait(portID,channel);
	RegVal = ReadReg(portID,channel,101) | (ReadReg(portID,channel,102) <<8);
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
int spiGci_Init (spiGci_Sptr hSpi){
	//establish link to driver
	int i;
	hSpi->portID = PPTADDR;
	i = LoadIODLL_C();
	PortOut_C(hSpi->portID,0x1D);
	return (!i);
}


/*
** Function: spiGci_ResetWrapper
**
** Description: 
** Sets the reset pin of the ProSLIC
*/
int spiGci_ResetWrapper (spiGci_Sptr hSpiGci, int status){
	uInt8 c;
	c = PortIn_C (hSpiGci->portID+2);
	if (status){ 
		PortOut_C(hSpiGci->portID+2,c | 0x1);
	}
	else{
		PortOut_C(hSpiGci->portID+2,c&0xfe);
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
** num: number of reads to perform
** regAddr: Address of register to read
** addr_inc: whether to increment address after each read
** data: data to read from register
**
** Return:
** none
*/
int spiGci_ReadRegisterWrapper (spiGci_Sptr hSpiGci, uInt8 channel, uInt8 num, uInt8 regAddr, int addr_inc, uInt8 *data){
	
	while (num){
		*data = ReadReg(hSpiGci->portID,channel,regAddr);
		if (addr_inc)
			regAddr++;
		data++; num--;
	}	
	return 0;
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
int spiGci_WriteRegisterWrapper (spiGci_Sptr hSpiGci, uInt8 channel, uInt8 num, uInt8 regAddr, int addr_inc, uInt8 *data){
	if (data == 0)
		return 1; //bad pointer
	while (num){
		WriteReg(hSpiGci->portID,channel,regAddr, *data);
		num--;data++;
		if (addr_inc)
			regAddr++;
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
int spiGci_ReadRAMWrapper (spiGci_Sptr hSpiGci, uInt8 channel, uInt16 ramAddr, ramData *data){
	
	*data = ReadRam(hSpiGci->portID,channel,ramAddr);
	return 0;
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
int spiGci_WriteRAMWrapper (spiGci_Sptr hSpiGci, uInt8 channel, uInt16 ramAddr, ramData data){
	
	WriteRam(hSpiGci->portID,channel,ramAddr,data);
	return 0;
}



/*
** $Log: proslic_spiGci_parallelPort.c,v $
** Revision 1.5  2006/07/21 20:30:20  lajordan
** fixed cant connect message
**
** Revision 1.4  2006/07/20 22:34:54  sasinha
** updated Read & Write RAM
**
** Revision 1.3  2006/07/19 18:16:14  lajordan
** fixed spi init
**
** Revision 1.2  2006/07/13 21:50:42  lajordan
** no message
**
*/