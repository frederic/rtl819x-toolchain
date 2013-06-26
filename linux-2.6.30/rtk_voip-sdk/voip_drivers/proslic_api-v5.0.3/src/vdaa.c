/*
** Copyright (c) 2008 by Silicon Laboratories
**
** $Id: vdaa.c 555 2009-03-25 23:12:08Z cdp@SILABS.COM $
**
** Vdaa.c
** Vdaa  VoiceDAA interface implementation file
**
** Author(s): 
** naqamar, laj
**
** Distributed by: 
** Silicon Laboratories, Inc
**
** This file contains proprietary information.	 
** No dissemination allowed without prior written permission from
** Silicon Laboratories, Inc.
**
** File Description:
** This is the implementation file for the main  VoiceDAA API and is used 
** in the  VoiceDAA demonstration code. 
**
** Dependancies:
** Customer Drivers
**
*/

#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/sched.h>
#include "si_voice_datatypes.h"
#include "si_voice_ctrl.h"
#include "si_voice_timer_intf.h"
#include "vdaa.h"
#include "vdaa_registers.h"
#include "vdaa_api_config.h"
//#include "vdaa_constants.c"


#ifdef WIN32
#include "stdlib.h"
#ifndef DISABLE_MALLOC
#include "memory.h" 
#endif 
#endif 




#define WriteReg			pVdaa->deviceId->ctrlInterface->WriteRegister_fptr 
#define ReadReg				pVdaa->deviceId->ctrlInterface->ReadRegister_fptr
#define pVdaaHW				pVdaa->deviceId->ctrlInterface->hCtrl
#define Reset				pVdaa->deviceId->ctrlInterface->Reset_fptr
#define Delay				pVdaa->deviceId->ctrlInterface->Delay_fptr
#define pVdaaTimer			pVdaa->deviceId->ctrlInterface->hTimer

#define WriteRegX			deviceId->ctrlInterface->WriteRegister_fptr
#define ReadRegX			deviceId->ctrlInterface->ReadRegister_fptr
#define pVdaaHWX			deviceId->ctrlInterface->hCtrl
#define DelayX				deviceId->ctrlInterface->Delay_fptr
#define pVdaaTimerX			deviceId->ctrlInterface->hTimer

/*CONSTANTS*/

#define BROADCAST 0xff
#define LCS_SCALE_NUM 33		/* Line Current Status Scale */
#define LCS_SCALE_DEN 10		/* Line Current Status Scale */

static int isVerifiedDAA(vdaaChanType *pVdaa){
	uInt8 data = ReadReg(pVdaaHW,pVdaa->channel,13);
	if ( (data & 0x40) == 0 ) {/*This bit is always 1 for DAA*/
		return ERR_VDAA_CHANNEL_TYPE;
	}
	else {
	}
	return 0;
}



int Vdaa_createChannel (vdaaChanType **pVdaa){

#ifndef DISABLE_MALLOC

	//*pVdaa = malloc(sizeof(vdaaChanType));
	*pVdaa = kmalloc(sizeof(vdaaChanType), 0x20); //GFP_ATOMIC
	memset(*pVdaa,0,sizeof(vdaaChanType));
	return 0;

#else
	return -1; 
#endif

}

int Vdaa_destroyChannel (vdaaChanType **pVdaa){

#ifndef DISABLE_MALLOC

	//free ((vdaaChanType_ptr)*pVdaa);
	kfree((vdaaChanType_ptr*)*pVdaa);
	return 0;

#else
	return -1;
#endif

}



int Vdaa_SWInitChan (vdaaChanType_ptr pVdaa,int32 channel,int chipType,
			SiVoiceDeviceType *pDeviceObj, SiVoiceControlInterfaceType *pCtrlIntf){
	pVdaa->channel = (uInt8)channel;
	pVdaa->deviceId = pDeviceObj;
	pVdaa->deviceId->ctrlInterface = pCtrlIntf;
	//printk("pVdaa->deviceId->ctrlInterface = 0x%p\n", pVdaa->deviceId->ctrlInterface);
	//printk("pVdaa->deviceId->ctrlInterface->hCtrl = 0x%p\n", pVdaa->deviceId->ctrlInterface->hCtrl);
	pVdaa->channelEnable=TRUE;
	pVdaa->error = ERR_VDAA_NONE;
	
	return 0;
}

int Vdaa_setSWDebugMode (vdaaChanType_ptr pVdaa, int32 debugEn){
	pVdaa->debugMode = debugEn;
	return 0;
}

int Vdaa_getErrorFlag (vdaaChanType_ptr pVdaa, int32*error){
	*error = pVdaa->error;
	return 0;
}

int Vdaa_clearErrorFlag (vdaaChanType_ptr pVdaa){
	pVdaa->error = ERR_VDAA_NONE;
	return 0;
}

int Vdaa_setChannelEnable (vdaaChanType_ptr pVdaa, int32 chanEn){
	pVdaa->channelEnable = chanEn;
	return 0;
}

int Vdaa_getChannelEnable (vdaaChanType_ptr pVdaa, int32* chanEn){
	*chanEn = pVdaa->channelEnable;
	return 0;
}

static int probeDaisyChain (vdaaChanType *pVdaa){
	int i=0;
	WriteReg(pVdaaHW,BROADCAST,PCMRX_CNT_LO,0x23);						/* Broadcast */
	while ((ReadReg(pVdaaHW,(uInt8)i++,PCMRX_CNT_LO) == 0x23)&&(i<=16));	/* Count number of channels */
	return i-1;													/* Return number of channels */
}
int Vdaa_PrintDebugData (vdaaChanType *pVdaa){
#ifdef ENABLE_DEBUG
		int i;
		for (i=0;i<60;i++)
			LOGPRINT ("vdaa: Register %d = %X\n",i,ReadReg(pVdaaHW,pVdaa->channel,i));
#endif
		return 0;
}

int Vdaa_VerifyControlInterface (vdaaChanType *pVdaa)
{
	int i;
	int numOfChan;
	vdaaChanType vdaa2;
	vdaa2.channel = 0;
	vdaa2.deviceId = pVdaa->deviceId;
	if (isVerifiedDAA(&vdaa2) == ERR_VDAA_CHANNEL_TYPE) { /*check chan 0 is daa*/
		if (isVerifiedDAA(pVdaa) == ERR_VDAA_CHANNEL_TYPE) {
			pVdaa->error = ERR_VDAA_CHANNEL_TYPE;
			return ERR_VDAA_CHANNEL_TYPE; /*this channel is not daa*/
		}
		else {
			/*we have a daa but can't broadcast*/
			WriteReg(pVdaaHW,pVdaa->channel,PCMRX_CNT_LO,0x5A);				
			if (ReadReg(pVdaaHW,pVdaa->channel,PCMRX_CNT_LO) != 0x5A)
				return ERR_VDAA_SPIFAIL;
			return 0;
		}
	}

	numOfChan = probeDaisyChain(pVdaa);
	if (numOfChan == 0)
		return ERR_VDAA_SPIFAIL;
#ifdef ENABLE_DEBUG
	if (pVdaa->debugMode)
		LOGPRINT ("vdaa: Found %d channels\n",numOfChan);
#endif
	
	/*Try to write innocuous register to test SPI is working*/
	WriteReg(pVdaaHW,BROADCAST,PCMRX_CNT_LO,0x5A);
		
	for (i=0;i<numOfChan;i++){
		
		if (ReadReg(pVdaa,i,PCMRX_CNT_LO) != 0x5A){
			return ERR_VDAA_SPIFAIL;
#ifdef ENABLE_DEBUG
			if (pVdaa->debugMode)
				LOGPRINT("vdaa: %d not communicating. Register access fail.\n",i);
#endif
			return ERR_VDAA_SPIFAIL;
		}	
		
	}
	return 0;
}

/*
** Local functions are defined first
*/


/*
**  VDAA CONFIGURATION FUNCTIONS
*/

/*
** Function:  VDAA_RingDetectSetup
**
** Description: 
** configure ring detect setup
*/
#ifdef DISABLE_VDAA_RING_DETECT_SETUP
#else
extern vdaa_Ring_Detect_Cfg Vdaa_Ring_Detect_Presets[];
int Vdaa_RingDetectSetup (vdaaChanType *pVdaa,int32 preset){
	uInt8 regTemp = 0;

	regTemp = ReadReg(pVdaaHW,pVdaa->channel,CTRL2) & 0xfB;
	regTemp |= Vdaa_Ring_Detect_Presets[preset].rdi<<2;
	WriteReg(pVdaaHW,pVdaa->channel,CTRL2,regTemp);

	regTemp = ReadReg(pVdaaHW,pVdaa->channel,INTL_CTRL1) & 0xfe;
	regTemp |= Vdaa_Ring_Detect_Presets[preset].rt&1;
	WriteReg(pVdaaHW,pVdaa->channel,INTL_CTRL1,regTemp);

	regTemp = ReadReg(pVdaaHW,pVdaa->channel,INTL_CTRL2) & 0xef;
	regTemp |= ((Vdaa_Ring_Detect_Presets[preset].rt>>1)<<4);
	WriteReg(pVdaaHW,pVdaa->channel,INTL_CTRL2,regTemp);

	WriteReg(pVdaaHW,pVdaa->channel,INTL_CTRL3,Vdaa_Ring_Detect_Presets[preset].rfwe<<1);
	
	regTemp = (Vdaa_Ring_Detect_Presets[preset].rdly&0x3) << 6;
	regTemp |= Vdaa_Ring_Detect_Presets[preset].rmx ;
	WriteReg(pVdaaHW,pVdaa->channel,RNG_VLD_CTRL1,regTemp);
	
	regTemp = (Vdaa_Ring_Detect_Presets[preset].rdly>>2) << 7;
	regTemp |= Vdaa_Ring_Detect_Presets[preset].rto << 3 ;
	regTemp |= Vdaa_Ring_Detect_Presets[preset].rcc ;
	WriteReg(pVdaaHW,pVdaa->channel,RNG_VLD_CTRL2,regTemp);
	
	regTemp = Vdaa_Ring_Detect_Presets[preset].rngv << 7;
	regTemp |= Vdaa_Ring_Detect_Presets[preset].ras ;
	WriteReg(pVdaaHW,pVdaa->channel,RNG_VLD_CTRL3,regTemp);

	regTemp = Vdaa_Ring_Detect_Presets[preset].rpol<<1;
	WriteReg(pVdaaHW,pVdaa->channel,DAA_CTRL4,regTemp);
	
	return 0;
}
#endif

/*
** Function:  Vdaa_TXAudioGainSetup
**
** Description: 
** configure audio gains
*/
#ifdef DISABLE_VDAA_AUDIOGAIN_SETUP
#else
extern vdaa_audioGain_Cfg Vdaa_audioGain_Presets[];

int Vdaa_TXAudioGainSetup (vdaaChanType *pVdaa,int32 preset){
	uInt8 regTemp = 0;
	
	regTemp = Vdaa_audioGain_Presets[preset].gainCtrl2 << 4 ;
	regTemp |= Vdaa_audioGain_Presets[preset].acgain2 ; 
	WriteReg(pVdaaHW,pVdaa->channel,TX_GN_CTRL2,regTemp);
	
	regTemp = Vdaa_audioGain_Presets[preset].gainCtrl3 << 4 ;
	regTemp |= Vdaa_audioGain_Presets[preset].acgain3 ; 
	WriteReg(pVdaaHW,pVdaa->channel,TX_GN_CTRL3,regTemp);

	WriteReg(pVdaaHW,pVdaa->channel,TXCALL_PROG_ATTEN,Vdaa_audioGain_Presets[preset].callProgress);
	return 0;
}
# endif
/*
** Function:  Vdaa_RXAudioGainSetup
**
** Description: 
** configure audio gains
*/
#ifdef DISABLE_VDAA_AUDIOGAIN_SETUP
#else
extern vdaa_audioGain_Cfg Vdaa_audioGain_Presets[];

int Vdaa_RXAudioGainSetup (vdaaChanType *pVdaa,int32 preset){
	uInt8 regTemp = 0;

	regTemp = Vdaa_audioGain_Presets[preset].gainCtrl2 << 4 ;
	regTemp |= Vdaa_audioGain_Presets[preset].acgain2 ; 
	WriteReg(pVdaaHW,pVdaa->channel,RX_GN_CTRL2,regTemp);
	
	regTemp = Vdaa_audioGain_Presets[preset].gainCtrl3 << 4 ;
	regTemp |= Vdaa_audioGain_Presets[preset].acgain3 ; 
	WriteReg(pVdaaHW,pVdaa->channel,RX_GN_CTRL3,regTemp);

	WriteReg(pVdaaHW,pVdaa->channel,RXCALL_PROG_ATTEN,Vdaa_audioGain_Presets[preset].callProgress);

	return 0;
}
#endif

/*
** Function:  Vdaa_PCMSetup
**
** Description: 
** configure pcm
*/
#ifdef DISABLE_VDAA_PCM_SETUP
#else
extern vdaa_PCM_Cfg Vdaa_PCM_Presets [];
int Vdaa_PCMSetup (vdaaChanType *pVdaa,int32 preset, int32 pcm_enable){
	uInt8 regTemp = 0;

	regTemp = ReadReg(pVdaaHW,pVdaa->channel,PCM_SPI_CTRL)&0xe0;
	regTemp |= Vdaa_PCM_Presets[preset].pcm_tri;
	regTemp |= Vdaa_PCM_Presets[preset].pcmHwy << 1;
	regTemp |= Vdaa_PCM_Presets[preset].pcmFormat << 3;
	regTemp |= pcm_enable << 5;
	WriteReg(pVdaaHW,pVdaa->channel,PCM_SPI_CTRL,regTemp);
	
	return 0;
}
#endif

/*
** Function:  Vdaa_PCMTimeSlotSetup
**
** Description: 
** configure pcm
*/
int Vdaa_PCMTimeSlotSetup (vdaaChanType *pVdaa, uInt16 rxcount, uInt16 txcount){
	uInt8 data = 0;
	uInt8 pcmStatus;
	
	pcmStatus = ReadReg(pVdaaHW,pVdaa->channel,PCM_SPI_CTRL);
	if (pcmStatus&0x20){
		WriteReg(pVdaaHW,pVdaa->channel,PCM_SPI_CTRL,pcmStatus&~(0x20));
	}

	/*Storing 10 bit value of Transmit PCM sample in REG 34 and REG 35[0:1]*/
	data = (uInt8)(txcount & 0xff);
	WriteReg(pVdaaHW,pVdaa->channel,PCMTX_CNT_LO,data);
	data = (uInt8)(txcount >> 8) ;
	WriteReg(pVdaaHW,pVdaa->channel,PCMTX_CNT_HI,data);
	
	/*Storing 10 bit value of Receive PCM sample in REG 34 and REG 35[0:1]*/
	data = (uInt8)(rxcount & 0xff);
	WriteReg(pVdaaHW,pVdaa->channel,PCMRX_CNT_LO,data);
	data = (uInt8)(rxcount >> 8);
	WriteReg(pVdaaHW,pVdaa->channel,PCMRX_CNT_HI,data);
	
	/* Enable back the PCM after storing the values*/
	WriteReg(pVdaaHW,pVdaa->channel,PCM_SPI_CTRL,pcmStatus);
	
	
	

	return 0;
}

/*
** Function:  Vdaa_Impedance_Cfg
**
** Description: 
** configure impedence synthesis
*/
#ifdef DISABLE_VDAA_IMPEDANCE_SETUP
#else
extern vdaa_Impedance_Cfg Vdaa_Impedance_Presets [];

int Vdaa_ImpedanceSetup (vdaaChanType *pVdaa,int32 preset){
	uInt8 regTemp = 0;

	regTemp = ReadReg(pVdaaHW,pVdaa->channel,CTRL2) & 0xFD;
	WriteReg(pVdaaHW,pVdaa->channel,CTRL2,regTemp); /* disable hybrid */

	regTemp = ReadReg(pVdaaHW,pVdaa->channel,INTL_CTRL1) & 0xFD;
	regTemp |= Vdaa_Impedance_Presets[preset].rz << 1 ;
	WriteReg(pVdaaHW,pVdaa->channel,INTL_CTRL1,regTemp);
	
	regTemp = Vdaa_Impedance_Presets[preset].dcr;
	regTemp |= Vdaa_Impedance_Presets[preset].ilim<<1;
	regTemp |= Vdaa_Impedance_Presets[preset].mini<<4;
	regTemp |= Vdaa_Impedance_Presets[preset].dcv<<6;
	WriteReg(pVdaaHW,pVdaa->channel,DC_TERM_CTRL,regTemp);

	regTemp = ReadReg(pVdaaHW,pVdaa->channel,AC_TERM_CTRL) & 0xF0;
	regTemp |= Vdaa_Impedance_Presets[preset].acim;
	WriteReg(pVdaaHW,pVdaa->channel,AC_TERM_CTRL,regTemp);

	WriteReg(pVdaaHW,pVdaa->channel,HYB1,Vdaa_Impedance_Presets[preset].hyb1);
	WriteReg(pVdaaHW,pVdaa->channel,HYB2,Vdaa_Impedance_Presets[preset].hyb2);
	WriteReg(pVdaaHW,pVdaa->channel,HYB3,Vdaa_Impedance_Presets[preset].hyb3);
	WriteReg(pVdaaHW,pVdaa->channel,HYB4,Vdaa_Impedance_Presets[preset].hyb4);
	WriteReg(pVdaaHW,pVdaa->channel,HYB5,Vdaa_Impedance_Presets[preset].hyb5);
	WriteReg(pVdaaHW,pVdaa->channel,HYB6,Vdaa_Impedance_Presets[preset].hyb6);
	WriteReg(pVdaaHW,pVdaa->channel,HYB7,Vdaa_Impedance_Presets[preset].hyb7);
	WriteReg(pVdaaHW,pVdaa->channel,HYB8,Vdaa_Impedance_Presets[preset].hyb8);

	regTemp = ReadReg(pVdaaHW,pVdaa->channel,SPRK_QNCH_CTRL) & 0xAF;
	regTemp |= ((Vdaa_Impedance_Presets[preset].ohs_sq >> 2)&1)<<4 ;
	regTemp |= ((Vdaa_Impedance_Presets[preset].ohs_sq >> 3)&1)<<6 ;
	WriteReg(pVdaaHW,pVdaa->channel,SPRK_QNCH_CTRL,regTemp);

	regTemp = ReadReg(pVdaaHW,pVdaa->channel,INTL_CTRL1) & 0xBF;
	regTemp |= ((Vdaa_Impedance_Presets[preset].ohs_sq >> 1)&1)<<6 ;
	WriteReg(pVdaaHW,pVdaa->channel,INTL_CTRL1,regTemp);

	regTemp = ReadReg(pVdaaHW,pVdaa->channel,DAA_CTRL5) & 0xF7;
	regTemp |= (Vdaa_Impedance_Presets[preset].ohs_sq&1)<<3 ;
	WriteReg(pVdaaHW,pVdaa->channel,DAA_CTRL5,regTemp);

	regTemp = ReadReg(pVdaaHW,pVdaa->channel,CTRL2) & 0xFD;
	regTemp |= (Vdaa_Impedance_Presets[preset].hbe)<<1 ;
	WriteReg(pVdaaHW,pVdaa->channel,CTRL2,regTemp);
	
	return 0;
}
#endif



/*
** Function:  Vdaa_LoopbackSetup
**
** Description: 
** configure loopback
*/
#ifdef DISABLE_VDAA_LOOPBACK_SETUP
#else
extern vdaa_Loopback_Cfg Vdaa_Loopback_Presets [];

int Vdaa_LoopbackSetup (vdaaChanType *pVdaa, int32 preset){
	uInt8 regTemp;
	regTemp = ReadReg(pVdaaHW,pVdaa->channel,CTRL1) & 0xfD;
	WriteReg(pVdaaHW,pVdaa->channel,CTRL1,regTemp | (Vdaa_Loopback_Presets[preset].isoDigLB<<1));
	regTemp = ReadReg(pVdaaHW,pVdaa->channel,DAA_CTRL3) & 0xfe;
	WriteReg(pVdaaHW,pVdaa->channel,DAA_CTRL3,regTemp | (Vdaa_Loopback_Presets[preset].digDataLB));
	return 0;	
}
#endif
/*
** Function:  Vdaa_PCMStart
**
** Description: 
** Starts PCM
*/
 int Vdaa_PCMStart (vdaaChanType *pVdaa){
	uInt8 data = 0;
	
	/*Enable PCM transfers by setting REG 33[5]=1 */
	data = ReadReg(pVdaaHW,pVdaa->channel,PCM_SPI_CTRL);
	data |= 0x20;
	WriteReg(pVdaaHW,pVdaa->channel,PCM_SPI_CTRL,data);
	return 0;
}

 /*
** Function:  Vdaa_PCMStop
**
** Description: 
** Disables PCM
*/
int Vdaa_PCMStop (vdaaChanType *pVdaa){
	uInt8 data = 0;
	
	/*disable PCM transfers by setting REG 33[5]=0 */
	data = ReadReg(pVdaaHW,pVdaa->channel,PCM_SPI_CTRL);
	data &= ~(0x20);
	WriteReg(pVdaaHW,pVdaa->channel,PCM_SPI_CTRL,data);
	return 0;
}

/*
** Function:  Vdaa_EnableInterrupts
**
** Description: 
** Enables interrupts
*/
int Vdaa_EnableInterrupts (vdaaChanType *pVdaa){
	uInt8 data = 0;
	
	WriteReg (pVdaaHW,pVdaa->channel,INTE_MSK,0xff);
		
	/*Current/Voltage Interrupt REG 44*/
	data = ReadReg(pVdaaHW,pVdaa->channel,LN_VI_THRESH_INTE_CTRL);
	data |= 0x02;
	WriteReg(pVdaaHW,pVdaa->channel,LN_VI_THRESH_INTE_CTRL,data);
	return 0;
}

/*
** Function:  Vdaa_ReadRingDetectStatus
**
** Description: 
** Reads ring detect/hook status
*/
int Vdaa_ReadRingDetectStatus (vdaaChanType *pVdaa,vdaaRingDetectStatusType *pStatus){
	uInt8 reg = ReadReg(pVdaaHW,pVdaa->channel,DAA_CTRL1);
	pStatus->offhook = reg&1;
	pStatus->ringDetected = (reg&0x4)>>2;
	pStatus->onhookLineMonitor = (reg&0x8)>>3;
	pStatus->ringDetectedPos = (reg&0x20)>>5;
	pStatus->ringDetectedNeg = (reg&0x40)>>6;
	return 0;
}

/*
** Function:  Vdaa_Init
**
** Description: 
** Initialize VDAA
*/
extern vdaa_General_Cfg Vdaa_General_Configuration;
int Vdaa_Init (vdaaChanType_ptr *pVdaa,int size){
	uInt8 data; 
	int16 k;

	for (k=0;k<size;k++) {
		/* Read Device ID and verify if SPI is working or not*/
		data = pVdaa[k]->ReadRegX(pVdaa[k]->pVdaaHWX,pVdaa[k]->channel,SYS_LINE_DEV_REV);
		pVdaa[k]->deviceId->chipRev = data&0xF;
		pVdaa[k]->deviceId->lsType= ((data&~(0xF))>>4);
	
		data = pVdaa[k]->ReadRegX(pVdaa[k]->pVdaaHWX,pVdaa[k]->channel,LSIDE_REV);
		pVdaa[k]->deviceId->lsRev= ((data&0x3C)>>2);
		if (isVerifiedDAA(pVdaa[k]) == ERR_VDAA_CHANNEL_TYPE) {
			pVdaa[k]->channelEnable = FALSE;
			pVdaa[k]->error = ERR_VDAA_CHANNEL_TYPE;
#ifdef ENABLE_DEBUG
			if (pVdaa[k]->debugMode)
				LOGPRINT("VDAA not supported on this device\n");
#endif
		}
		if (pVdaa[k]->channelEnable) {
			/*Try to write innocuous register to test SPI is working*/
			pVdaa[k]->WriteRegX(pVdaa[k]->pVdaaHWX,pVdaa[k]->channel,PCMRX_CNT_LO,0x5A);				
			if (pVdaa[k]->ReadRegX(pVdaa[k]->pVdaaHWX,pVdaa[k]->channel,PCMRX_CNT_LO) != 0x5A){
				pVdaa[k]->error = ERR_VDAA_SPIFAIL;
				pVdaa[k]->channelEnable = FALSE;
#ifdef ENABLE_DEBUG
				if (pVdaa[k]->debugMode)
					LOGPRINT("VDAA %d not communicating\n",pVdaa[k]->channel);
#endif
			}
		}
	}
	for (k=0;k<size;k++) {
		if (pVdaa[k]->channelEnable) {
			pVdaa[k]->WriteRegX(pVdaa[k]->pVdaaHWX,pVdaa[k]->channel,DAA_CTRL5,0xA0);	/* Enable full transmit and receive mode by 
															setting REG 31[7]=1 ; Default 
														   Off-Hook counter i.e. 128 ms ; Normal operation
														   for Line Voltage Force*/
			pVdaa[k]->WriteRegX(pVdaa[k]->pVdaaHWX,pVdaa[k]->channel,RX_GN_CTRL2,0x01);	/* 1 dB gain is applied to the receive path*/
			pVdaa[k]->WriteRegX(pVdaa[k]->pVdaaHWX,pVdaa[k]->channel,RX_GN_CTRL3,0x05);
			pVdaa[k]->WriteRegX(pVdaa[k]->pVdaaHWX,pVdaa[k]->channel,DC_TERM_CTRL,0xC0);	/* To enable full transmit mode REG 26[7:6] should
															be set to 1's and 
														   REG 26[5:4] should be set to 0's*/
			pVdaa[k]->WriteRegX(pVdaa[k]->pVdaaHWX,pVdaa[k]->channel,INTL_CTRL1,0x10);	/* IIR filter enabled for transmit and receive filters ;
															Max ringer impedance*/
			pVdaa[k]->WriteRegX(pVdaa[k]->pVdaaHWX,pVdaa[k]->channel,DAA_CTRL2,0x00);	/* Normal operration i.e. no powerdown line side or
															system side device*/
			data = pVdaa[k]->ReadRegX(pVdaa[k]->pVdaaHWX,pVdaa[k]->channel,CTRL2) & 0x3F;
			pVdaa[k]->WriteRegX(pVdaa[k]->pVdaaHWX,pVdaa[k]->channel,CTRL2,data | (Vdaa_General_Configuration.inte<<7) | (Vdaa_General_Configuration.intp<<6));
			data = pVdaa[k]->ReadRegX(pVdaa[k]->pVdaaHWX,pVdaa[k]->channel,CTRL1) & 0xC7;
			pVdaa[k]->WriteRegX(pVdaa[k]->pVdaaHWX,pVdaa[k]->channel,CTRL1,data | (Vdaa_General_Configuration.pwmEnable<<3) | (Vdaa_General_Configuration.pwmm<<4));
		
		}
	}
	return 0;
}

/*
** Function:  Vdaa_Reset
**
** Description: 
** Reset VDAA
*/
int Vdaa_Reset (vdaaChanType *pVdaa){
	/*
	** resets Vdaa, wait 250ms, release reset, wait 250ms
	*/
	Reset(pVdaaHW,1);
	Delay(pVdaaTimer,250);
	Reset(pVdaaHW,0);
	Delay(pVdaaTimer,250);
	return 0;
}

/*
** Function:  Vdaa_ReadLinefeedStatus
**
** Description: 
** Read Status of Line Feed
*/
int Vdaa_ReadLinefeedStatus (vdaaChanType *pVdaa,int8 *vloop, int16 *iloop){
	
	int16 regTemp = 0x1F;	
	uInt8 iloop_reg; /* REG 12[4:0] = Loop current*/
	regTemp &= ReadReg(pVdaaHW,pVdaa->channel,LSIDE_STAT);
	iloop_reg = (uInt8)regTemp;
	*iloop = (regTemp*LCS_SCALE_NUM) / LCS_SCALE_DEN;					/* Multiply the read result by 3.3mA/bit*/
	*vloop = (int8) ReadReg(pVdaaHW,pVdaa->channel,LINE_VOLT_STAT);
	if (iloop_reg == 0x1F)
		return ERR_VDAA_ILOOP_OVERLOAD;
	return 0;
}

/*
** Function:  Vdaa_GetInterrupts
**
** Description: 
** Get Interrupts
*/
int Vdaa_GetInterrupts (vdaaChanType *pVdaa,vdaaIntType *pIntData){
	uInt8 data = 0;
	int j;
	pIntData->number = 0;
	
	
	data = ReadReg(pVdaaHW,pVdaa->channel,INTE_SRC);			/*Snapshot Interrupts*/
	WriteReg(pVdaaHW,pVdaa->channel,INTE_SRC,~(data));			/*Clear interrupts*/
	for (j=0;j<8;j++){
			if (data &(1<<j)){
				
				pIntData->irqs[pIntData->number] = 	j;	
				pIntData->number++;
			
			}
	}
		data = ReadReg(pVdaaHW,pVdaa->channel,LN_VI_THRESH_INTE_CTRL);
		
			if (data &(0x08)){									/*to determine if CVI Interrupt is set*/
				pIntData->irqs[pIntData->number] = 	CVI;		
				pIntData->number++;
				data &= ~(0x08);
				WriteReg(pVdaaHW,pVdaa->channel,LN_VI_THRESH_INTE_CTRL,data);

			}

	return pIntData->number;
}

/*
** Function:  Vdaa_ClearInterrupts
**
** Description: 
** Clear Interrupts
*/
int Vdaa_ClearInterrupts (vdaaChanType *pVdaa){
	uInt8 data = 0;

	WriteReg(pVdaaHW,pVdaa->channel,INTE_SRC,0x00);		/* Clear interrupts in REG 4 by writing 0's*/
	
	/*Clear CVI interrupt by writing 0 at REG 44[3]*/
	data = ~(0x08);
	data &= ReadReg(pVdaaHW,pVdaa->channel,LN_VI_THRESH_INTE_CTRL);
	WriteReg(pVdaaHW,pVdaa->channel,LN_VI_THRESH_INTE_CTRL,data);

	return 0;
}

/*
** Function:  Vdaa_SetHookStatus
**
** Description: 
** Set VDAA Hook Status
*/
int Vdaa_SetHookStatus (vdaaChanType *pVdaa,uInt8 newHookStatus){
	uInt8 data= 0 ;

	switch (newHookStatus){
	case VDAA_DIG_LOOPBACK:
		/*Setting REG6[4]=1,REG5[0]=0,REG5[3]=0*/
		data = ReadReg(pVdaaHW,pVdaa->channel,DAA_CTRL2);	
		data |= 0x10;
		WriteReg(pVdaaHW,pVdaa->channel,DAA_CTRL2,data);
		data = ReadReg(pVdaaHW,pVdaa->channel,DAA_CTRL1);
		data &= ~(0x09);
		WriteReg(pVdaaHW,pVdaa->channel,DAA_CTRL1,data);
		break;
	case VDAA_ONHOOK:
		/*Setting REG6[4]=0,REG5[0]=0,REG5[3]=0*/
		data = ReadReg(pVdaaHW,pVdaa->channel,DAA_CTRL1);	
		data &= 0xF6;
		WriteReg(pVdaaHW,pVdaa->channel,DAA_CTRL1,data);
		data = ReadReg(pVdaaHW,pVdaa->channel,DAA_CTRL2);
		data &= 0xEF;
		WriteReg(pVdaaHW,pVdaa->channel,DAA_CTRL2,data);
		break;
	case VDAA_OFFHOOK:
		/*Setting REG6[4]=0,REG5[0]=1,REG5[3]=0*/
		data = ReadReg(pVdaaHW,pVdaa->channel,DAA_CTRL1);	
		data &= 0xF7;
		data |= 0x01;
		WriteReg(pVdaaHW,pVdaa->channel,DAA_CTRL1,data);
		data = ReadReg(pVdaaHW,pVdaa->channel,DAA_CTRL2);
		data &= 0xEF;
		WriteReg(pVdaaHW,pVdaa->channel,DAA_CTRL2,data);
		break;
	case VDAA_ONHOOK_MONITOR:
		/*Setting REG6[4]=0,REG5[0]=0,REG5[3]=1*/
		data = ReadReg(pVdaaHW,pVdaa->channel,DAA_CTRL1);	
		data &= 0xFE;
		data |= 0x08;
		WriteReg(pVdaaHW,pVdaa->channel,DAA_CTRL1,data);
		data = ReadReg(pVdaaHW,pVdaa->channel,DAA_CTRL2);
		data &= 0xEF;
		WriteReg(pVdaaHW,pVdaa->channel,DAA_CTRL2,data);
		break;
	default:
		return ERR_VDAA_INVALID_INPUT;

	}

	return 0;
}

/*
** Function:  Vdaa_ADCCal
**
** Description: 
** This function calibrates the VDAA ADC manually
*/
int Vdaa_ADCCal (vdaaChanType_ptr pVdaa, int32 size){
	uInt8 regTemp = 0;

	regTemp = ReadReg(pVdaaHW,pVdaa->channel,INTL_CTRL2); /* Clearing the previous ADC Calibration data by toggling CALZ*/
	regTemp |= 0x80;
	WriteReg(pVdaaHW,pVdaa->channel,INTL_CTRL2,regTemp);
	regTemp &= ~0x80;
	WriteReg(pVdaaHW,pVdaa->channel,INTL_CTRL2,regTemp);
	
	regTemp = ReadReg(pVdaaHW,pVdaa->channel,INTL_CTRL2); /*disable auto cal*/
	regTemp |= 0x20;
	WriteReg(pVdaaHW,pVdaa->channel,INTL_CTRL2,regTemp);

	regTemp |= 0x40;									/*initiate manual cal*/
	WriteReg(pVdaaHW,pVdaa->channel,INTL_CTRL2,regTemp);
	regTemp &= ~0x40;
	WriteReg(pVdaaHW,pVdaa->channel,INTL_CTRL2,regTemp);
	
	
	return 0;
}





/*
** Function:  Vdaa_InitBroadcast
** This function should match Vdaa_Init()

*/

int Vdaa_InitBroadcast (vdaaChanType_ptr pVdaa){
	uInt8 data; 
	uInt8 i;
	uInt8 numOfChan;
	vdaaChanType vdaa2;

	data = ReadReg(pVdaaHW,pVdaa->channel,SYS_LINE_DEV_REV);
	pVdaa->deviceId->chipRev = data&0xF;
	pVdaa->deviceId->lsType= ((data&(0xF0))>>4);
	
	data = ReadReg(pVdaaHW,pVdaa->channel,LSIDE_REV);
	pVdaa->deviceId->lsRev= ((data&0x3C)>>2);
	if (isVerifiedDAA(pVdaa) == ERR_VDAA_CHANNEL_TYPE){
		LOGPRINT ("vdaa: Channel %d type appears wrong. Check SPI and check this channel is not ProSLIC\n",pVdaa->channel);
		return ERR_VDAA_BROADCAST_FAIL;
	}
	if ((pVdaa->channel%2) != 0){ /*odd channel # - are we si3217x? check chan 0.*/
		vdaa2.channel = 0;
		vdaa2.deviceId = pVdaa->deviceId;
		if (isVerifiedDAA(&vdaa2) == ERR_VDAA_CHANNEL_TYPE){
			LOGPRINT ("vdaa: Channel %d type appears wrong. We can't broadcast to mixed channels.\n",vdaa2.channel);
			return ERR_VDAA_BROADCAST_FAIL;
		}
	}

	numOfChan = (uInt8)probeDaisyChain(pVdaa);
	if (numOfChan == 0)
		return ERR_VDAA_SPIFAIL;
	
	/*Try to write innocuous register to test SPI is working*/
	WriteReg(pVdaaHW,BROADCAST,PCMRX_CNT_LO,0x5A);				
	for (i=0;i<numOfChan;i++){
			if (ReadReg(pVdaaHW,i,PCMRX_CNT_LO) != 0x5A){
			return ERR_VDAA_SPIFAIL;

#ifdef ENABLE_DEBUG
			if (pVdaa->debugMode)
				LOGPRINT("VDAA %d not communicating\n",i);
#endif
			return ERR_VDAA_SPIFAIL;
			}	
	}
	WriteReg(pVdaaHW,BROADCAST,DAA_CTRL5,0xA0);	/* Enable full transmit and receive mode by 
															setting REG 31[7]=1 ; Default 
														   Off-Hook counter i.e. 128 ms ; Normal operation
														   for Line Voltage Force*/
	WriteReg(pVdaaHW,BROADCAST,RX_GN_CTRL2,0x01);	/* 1 dB gain is applied to the receive path*/
	WriteReg(pVdaaHW,BROADCAST,RX_GN_CTRL3,0x05);
	WriteReg(pVdaaHW,BROADCAST,DC_TERM_CTRL,0xC0);	/* To enable full transmit mode REG 26[7:6] should
															be set to 1's and 
														   REG 26[5:4] should be set to 0's*/
	WriteReg(pVdaaHW,BROADCAST,INTL_CTRL1,0x10);	/* IIR filter enabled for transmit and receive filters ;
															Max ringer impedance*/
	WriteReg(pVdaaHW,BROADCAST,DAA_CTRL2,0x00);	/* Normal operration i.e. no powerdown line side or
															system side device*/
	data = ReadReg(pVdaaHW,pVdaa->channel,CTRL2) & 0x3F;
	WriteReg(pVdaaHW,BROADCAST,CTRL2,data | (Vdaa_General_Configuration.inte<<7) | (Vdaa_General_Configuration.intp<<6));
	data = ReadReg(pVdaaHW,pVdaa->channel,CTRL1) & 0xC7;
	WriteReg(pVdaaHW,BROADCAST,CTRL1,data | (Vdaa_General_Configuration.pwmEnable<<3) | (Vdaa_General_Configuration.pwmm<<4));
			
	return 0;
}
