#include <linux/config.h>
#include <linux/sched.h>	/* jiffies is defined */
#include <linux/kernel.h>
#include <linux/interrupt.h>


#include "si_voice_datatypes.h"
#include "si_voice_ctrl.h"
#include "sys_driv_type.h"
#include "spi/spi.h"
#include "rtk_voip.h"

#define CNUM_TO_CID_QUAD(channelNumber)   (((channelNumber<<4)&0x10)|((channelNumber<<2)&0x8)|((channelNumber>>2)&0x2)|((channelNumber>>4)&0x1)|(channelNumber&0x4))
#ifdef CONFIG_RTK_VOIP_8672_SHARED_SPI
extern void rtl8672_share_spi_write(unsigned int ch, unsigned int uses_daisy, unsigned int contorl, unsigned int address,unsigned int data);
extern unsigned int  rtl8672_share_spi_read(unsigned int ch, unsigned int uses_daisy, unsigned int control, unsigned int address,unsigned char *data);
#elif defined CONFIG_RTK_VOIP_8676_SHARED_SPI
extern void rtl8676_share_spi_write(unsigned int ch, unsigned int uses_daisy, unsigned int contorl, unsigned int address,unsigned int data);
extern unsigned int  rtl8676_share_spi_read(unsigned int ch, unsigned int uses_daisy, unsigned int control, unsigned int address,unsigned char *data);
#else
extern int32 _rtl_spi_rawWrite( rtl_spi_dev_t* pDev, void* pData, int32 bits );
extern int32 _rtl_spi_rawRead( rtl_spi_dev_t* pDev, void* pData, int32 bits );
#endif
static uInt8 ReadReg_dev (rtl_spi_dev_t* pDev, uInt8 channel, uInt8 regAddr){

	uInt8 regCtrl = CNUM_TO_CID_QUAD(channel)|0x60;
	uInt8 reg_val;
	unsigned long flags;
	
	save_flags(flags); cli();

#ifdef CONFIG_RTK_VOIP_8672_SHARED_SPI
	uInt16 portID = 0; // for daisy chain
	rtl8672_share_spi_read(portID, 1, regCtrl, regAddr,  &reg_val);	
#elif defined CONFIG_RTK_VOIP_8676_SHARED_SPI
	uInt16 portID = 0; // for daisy chain
	rtl8676_share_spi_read(portID, 1, regCtrl, regAddr,  &reg_val); 
#else
	//printk("(%d, %d)\n", portID, channel);
 	// .8138 (.2474)
 	// .6031 (.2130)	inline + dmem (gpio)
 	// .6300 (.2555)	inline
 	// .5799 (.2039)	inline + dmem (gpio/desc)
 	// .4697 (.2323)	inline + no debug 
	_rtl_spi_rawWrite(pDev, &regCtrl, 8);
	_rtl_spi_rawWrite(pDev, &regAddr, 8);
	// .5400 (.1401)
	// .5018 (.1468)	inline + dmem 
	// .4963 (.1508)	inline
	// .4920 (.1774)	inline + dmem (gpio/desc)
	// .3847 (.1772)	inline + no debug 
	_rtl_spi_rawRead(pDev, &reg_val, 8);
#endif //#ifdef CONFIG_RTK_VOIP_8672_SHARED_SPI

	restore_flags(flags);

	return reg_val;
}

static void WriteReg_dev (rtl_spi_dev_t* pDev, uInt8 channel, uInt8 regAddr, uInt8 data)
{
	uInt8 regCtrl = CNUM_TO_CID_QUAD(channel)|0x20;
	unsigned long flags;

	save_flags(flags); cli();
	if (channel == 0xff)
		regCtrl = 0x20 | 0x80;

#ifdef CONFIG_RTK_VOIP_8672_SHARED_SPI
	uInt16 portID = 0; // for daisy chain
	rtl8672_share_spi_write(portID, 1,regCtrl , regAddr,  data);
#elif defined CONFIG_RTK_VOIP_8676_SHARED_SPI
	uInt16 portID = 0; // for daisy chain
	rtl8676_share_spi_write(portID, 1,regCtrl , regAddr,  data);
#else
	_rtl_spi_rawWrite(pDev, &regCtrl, 8);
	_rtl_spi_rawWrite(pDev, &regAddr, 8);
	_rtl_spi_rawWrite(pDev, &data, 8);
#endif //CONFIG_RTK_VOIP_8672_SHARED_SPI

	restore_flags(flags);
}

unsigned char R_reg_dev(rtl_spi_dev_t *pdev, unsigned char chid, unsigned char regaddr)
{
	return ReadReg_dev( pdev, chid, regaddr );
}

unsigned char R_reg_dev2(unsigned char chid, unsigned char regaddr)
{
	return R_reg_dev( 0, chid, regaddr );
}

void W_reg_dev(rtl_spi_dev_t *pdev, unsigned char chid, unsigned char regaddr, unsigned char data)
{

	WriteReg_dev( pdev, chid, regaddr, data );
}

void W_reg_dev2(unsigned char chid, unsigned char regaddr, unsigned char data)
{

	W_reg_dev( 0, chid, regaddr, data );
}

static void RAMwait_dev (rtl_spi_dev_t* pDev,unsigned short channel)
{
	unsigned char regVal; 
	regVal = ReadReg_dev (pDev,channel,4);
	while (regVal&0x01)
	{
		regVal = ReadReg_dev (pDev,channel,4);
	}//wait for indirect registers

}

static void WriteRam_dev (rtl_spi_dev_t* pDev, uInt8 channel, uInt16 regAddr, uInt32 data)
{
	unsigned long flags;
	
	save_flags(flags); cli();
	
	if (channel == 0xff)
		RAMwait_dev(pDev,0);   
	else
		RAMwait_dev(pDev,channel);   
	WriteReg_dev(pDev,channel,5,(regAddr>>3)&0xe0); //write upper address bits
	WriteReg_dev(pDev,channel,6,(data<<3)&0xff);
	WriteReg_dev(pDev,channel,7,(data>>5)&0xff);
	WriteReg_dev(pDev,channel,8,(data>>13)&0xff);
	WriteReg_dev(pDev,channel,9,(data>>21)&0xff);
	WriteReg_dev(pDev,channel,10,regAddr&0xff); //write lower address bits  
		
	restore_flags(flags);
}

static uInt32 ReadRam_dev (rtl_spi_dev_t* pDev, uInt8 channel, uInt16 regAddr)
{
	unsigned char reg; unsigned long RegVal;
	unsigned long flags;
	
	save_flags(flags); cli();
	
	RAMwait_dev(pDev,channel);
	WriteReg_dev(pDev,channel,5,(regAddr>>3)&0xe0); //write upper address bits
	WriteReg_dev(pDev,channel,10,regAddr&0xff); //write lower address bits
	
	RAMwait_dev(pDev,channel);
	
	reg=ReadReg_dev(pDev,channel,6);
	RegVal = reg>>3;
	reg=ReadReg_dev(pDev,channel,7);
	RegVal |= ((unsigned long)reg)<<5;
	reg=ReadReg_dev(pDev,channel,8);
	RegVal |= ((unsigned long)reg)<<13;
	reg=ReadReg_dev(pDev,channel,9);
	RegVal |= ((unsigned long)reg)<<21;
	
	restore_flags(flags);
	
	return RegVal;
}

void W_ram_dev(rtl_spi_dev_t* pDev, unsigned char chid, unsigned short reg, unsigned int data)
{
	return WriteRam_dev(pDev, chid, reg, data);
}

unsigned int R_ram_dev(rtl_spi_dev_t* pdev, unsigned char chid, unsigned short reg)
{
	return ReadRam_dev(pdev, chid, reg);
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
	// chmap don't need this 
	//init_spi(hSpi->portID);

	return 0;
}

/*
** Function: spiGci_ResetWrapper
**
** Description: 
** Sets the reset pin of the ProSLIC
*/
int ctrl_ResetWrapper (ctrl_S *hSpiGci, int status){

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM865xC
	_rtl865xC_setGpioDataBit(PIN_RESET1, ~status);
#else
	//"Not implemented yet!";
#endif
	
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
	// 1.6549 (.5240)
	// 1.4985 (.4939)	inline + dmem
	// 1.3695 (.5622)	inline
	// 1.4759 (.5439)	inline + dmem (gpio/desc)
	return ReadReg_dev(&hSpiGci->spi_dev,channel,regAddr);
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

	WriteReg_dev(&hSpiGci->spi_dev,channel,regAddr, data);
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
	data = ReadRam_dev(&hSpiGci->spi_dev,channel,ramAddr);
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
	WriteRam_dev(&hSpiGci->spi_dev,channel,ramAddr,data);
#endif
	return 0;
}


