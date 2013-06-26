/*
** Copyright (c) 2007-2009 by Silicon Laboratories
**
** $Id: si322x_intf.c 1030 2009-09-01 22:59:44Z nizajerk@SILABS.COM $
**
** SI322X_Intf.c
** SI322X ProSLIC user define interface implementation file
**
** Author(s): 
** Thlin
**
** 2009.09.24
**
** Copyright 2009 Realtek Semiconductor Corp.
** 
** File Description:
** This is the implementation file for the main ProSLIC API.
**
*/

#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/sched.h>
#include "si_voice_datatypes.h"
#include "si_voice_ctrl.h"
#include "si_voice_timer_intf.h"
#include "proslic.h"
#include "si3226_intf.h"
#include "si3226_userdef.h"
#include "si3226.h"
#include "si3226_registers.h"
#include "proslic_api_config.h"
#include "rtk_voip.h"

//#include "Slic_api.h"
#include "snd_define.h"


#define PRAM_ADDR (334 + 0x400)
#define PRAM_DATA (335 + 0x400)

#define WriteReg	pProslic->deviceId->ctrlInterface->WriteRegister_fptr
#define ReadReg		pProslic->deviceId->ctrlInterface->ReadRegister_fptr
#define pProHW		pProslic->deviceId->ctrlInterface->hCtrl
#define Reset		pProslic->deviceId->ctrlInterface->Reset_fptr
#define Delay		pProslic->deviceId->ctrlInterface->Delay_fptr
#define pProTimer	pProslic->deviceId->ctrlInterface->hTimer
#define WriteRAM	pProslic->deviceId->ctrlInterface->WriteRAM_fptr
#define ReadRAM		pProslic->deviceId->ctrlInterface->ReadRAM_fptr
#define TimeElapsed pProslic->deviceId->ctrlInterface->timeElapsed_fptr
#define SetSemaphore pProslic->deviceId->ctrlInterface->Semaphore_fptr

#define WriteRegX		deviceId->ctrlInterface->WriteRegister_fptr
#define ReadRegX		deviceId->ctrlInterface->ReadRegister_fptr
#define pProHWX			deviceId->ctrlInterface->hCtrl
#define DelayX			deviceId->ctrlInterface->Delay_fptr
#define pProTimerX		deviceId->ctrlInterface->hTimer
#define WriteRAMX		deviceId->ctrlInterface->WriteRAM_fptr
#define ReadRAMX		deviceId->ctrlInterface->ReadRAM_fptr
#define TimeElapsedX	deviceId->ctrlInterface->timeElapsed_fptr

#define BROADCAST 0xff


/*
** Local functions are defined first
*/

/*
** Function: setUserMode
**
** Description: 
** Puts ProSLIC into user mode or out of user mode
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object
** on: specifies whether user mode should be turned on (TRUE) or off (FALSE)
**
** Return:
** none
*/
static int setUserMode (proslicChanType *pProslic,BOOLEAN on){
	uInt8 data;
	if (SetSemaphore != NULL){
		while (!(SetSemaphore (pProHW,1)));
		if (on == TRUE){
			if (pProslic->deviceId->usermodeStatus<2)
				pProslic->deviceId->usermodeStatus++;
		} else {
			if (pProslic->deviceId->usermodeStatus>0)
				pProslic->deviceId->usermodeStatus--;
			if (pProslic->deviceId->usermodeStatus != 0)
				return -1;
		}
	}
	data = ReadReg(pProHW,pProslic->channel,126);
	if (((data&1) != 0) == on)
		return 0;
	WriteReg(pProHW,pProslic->channel,USERMODE_ENABLE,2);
	WriteReg(pProHW,pProslic->channel,USERMODE_ENABLE,8);
	WriteReg(pProHW,pProslic->channel,USERMODE_ENABLE,0xe);
	WriteReg(pProHW,pProslic->channel,USERMODE_ENABLE,0);
	if (SetSemaphore != NULL)
		SetSemaphore(pProHW,0);
	return 0;
}


/* Thlin Add */

void Si3226_Set_Ring_Cadence_ON(proslicChanType *pProslic, unsigned short msec)
{
	unsigned int l_time, h_time, tmp;

	tmp = msec << 3;
	if (tmp > 255)
	{
		l_time = 255; 
		h_time = (tmp - 255)>>8;
	}
	else
	{
		l_time = tmp;
		h_time = 0;
	}
		
	WriteReg(pProHW,pProslic->channel,RINGTALO, l_time);
	WriteReg(pProHW,pProslic->channel,RINGTAHI, h_time);
}

void Si3226_Set_Ring_Cadence_OFF(proslicChanType *pProslic, unsigned short msec)
{
	unsigned int l_time, h_time, tmp;

	tmp = msec << 3;
	if (tmp > 255)
	{
		l_time = 255; 
		h_time = (tmp - 255)>>8;
	}
	else
	{
		l_time = tmp;
		h_time = 0;
	}
		
	WriteReg(pProHW,pProslic->channel,RINGTILO, l_time);
	WriteReg(pProHW,pProslic->channel,RINGTIHI, h_time);
}

static unsigned char ring_cad[4] = {0};

static void Si3226_Backup_Ring_Cadence(proslicChanType *pProslic)
{
	// backup ring on
	ring_cad[0] = ReadReg(pProHW,pProslic->channel,RINGTALO);
	ring_cad[1] = ReadReg(pProHW,pProslic->channel,RINGTAHI);
	// backup ring off
	ring_cad[2] = ReadReg(pProHW,pProslic->channel,RINGTILO);
	ring_cad[3] = ReadReg(pProHW,pProslic->channel,RINGTIHI);
}

static void Si3226_Restore_Ring_Cadence(proslicChanType *pProslic)
{
	WriteReg(pProHW,pProslic->channel,RINGTALO, ring_cad[0]);
	WriteReg(pProHW,pProslic->channel,RINGTAHI, ring_cad[1]);
	WriteReg(pProHW,pProslic->channel,RINGTILO, ring_cad[2]);
	WriteReg(pProHW,pProslic->channel,RINGTIHI, ring_cad[3]);
}

#include <linux/kernel.h>
#include <linux/delay.h>

void Si3226_SendNTTCAR(proslicChanType *pProslic)
{
	int i;
	/************** backup the register ***************/
	Si3226_Backup_Ring_Cadence(pProslic);

	/*********** To Create Short Ring *****************/
	Si3226_Set_Ring_Cadence_ON(pProslic, 500);
	Si3226_Set_Ring_Cadence_OFF(pProslic, 500);

	Si3226_RingStart(pProslic);
}

unsigned int Si3226_SendNTTCAR_check(unsigned int chid, proslicChanType *pProslic, unsigned long time_out)
{
	//int protect_cnt = 0;
	int i;
	/*********** Check Phone Hook State ***************/

	if ( !(ReadReg(pProHW,pProslic->channel,LCRRTP) & 2) ) // if phone on-hook
	{

/*		while( !(ReadReg(pProHW,pProslic->channel,LCRRTP) & 2) )  //wait phone off-hook atuomatically
		{
			if (protect_cnt == 30)	// wait 6 sec, if no off-hook-> break to prevent watch dog reset. 
			{
				break;
			}
			mdelay(200);
			protect_cnt ++;
		}	
*/
		if (timetick_after(timetick,time_out) ) //time_after(a,b) returns true if the time a is after time b.
		{
			/* don't return 0, return 1, report time out don't wait */

		}
		else
			return 0;
			
	}

	/******* Set Reverse On-Hook Transmission *********/		
	ProSLIC_SetLinefeedStatus(pProslic, LF_REV_OHT);
	
	/************** restore the register ***************/
	Si3226_Restore_Ring_Cadence(pProslic);

	return 1;

}


#if 0 // for impedance settings (RTK_VOIP)

int Si3226_ImpdanceSetup (proslicChanType *pProslic, int preset)
{
	uInt8 lf;
	lf = ReadReg(pProHW,pProslic->channel,LINEFEED);
	WriteReg(pProHW,pProslic->channel,LINEFEED,0);
	
        WriteRAM(pProHW,pProslic->channel,TXACEQ_C0,RTK_Si3226_impedence_Presets[preset].txaceq_c0);
        WriteRAM(pProHW,pProslic->channel,TXACEQ_C1,RTK_Si3226_impedence_Presets[preset].txaceq_c1);
        WriteRAM(pProHW,pProslic->channel,TXACEQ_C2,RTK_Si3226_impedence_Presets[preset].txaceq_c2);
        WriteRAM(pProHW,pProslic->channel,TXACEQ_C3,RTK_Si3226_impedence_Presets[preset].txaceq_c3);

        WriteRAM(pProHW,pProslic->channel,RXACEQ_C0,RTK_Si3226_impedence_Presets[preset].rxaceq_c0);
        WriteRAM(pProHW,pProslic->channel,RXACEQ_C1,RTK_Si3226_impedence_Presets[preset].rxaceq_c1);
        WriteRAM(pProHW,pProslic->channel,RXACEQ_C2,RTK_Si3226_impedence_Presets[preset].rxaceq_c2);
        WriteRAM(pProHW,pProslic->channel,RXACEQ_C3,RTK_Si3226_impedence_Presets[preset].rxaceq_c3);

	WriteRAM(pProHW,pProslic->channel,ECFIR_C2,RTK_Si3226_impedence_Presets[preset].ecfir_c2);
	WriteRAM(pProHW,pProslic->channel,ECFIR_C3,RTK_Si3226_impedence_Presets[preset].ecfir_c3);
        WriteRAM(pProHW,pProslic->channel,ECFIR_C4,RTK_Si3226_impedence_Presets[preset].ecfir_c4);
        WriteRAM(pProHW,pProslic->channel,ECFIR_C5,RTK_Si3226_impedence_Presets[preset].ecfir_c5);
        WriteRAM(pProHW,pProslic->channel,ECFIR_C6,RTK_Si3226_impedence_Presets[preset].ecfir_c6);
        WriteRAM(pProHW,pProslic->channel,ECFIR_C7,RTK_Si3226_impedence_Presets[preset].ecfir_c7);
        WriteRAM(pProHW,pProslic->channel,ECFIR_C8,RTK_Si3226_impedence_Presets[preset].ecfir_c8);
        WriteRAM(pProHW,pProslic->channel,ECFIR_C9,RTK_Si3226_impedence_Presets[preset].ecfir_c9);

        WriteRAM(pProHW,pProslic->channel,ECIIR_B0,RTK_Si3226_impedence_Presets[preset].eciir_b0);
        WriteRAM(pProHW,pProslic->channel,ECIIR_B1,RTK_Si3226_impedence_Presets[preset].eciir_b1);

        WriteRAM(pProHW,pProslic->channel,ECIIR_A1,RTK_Si3226_impedence_Presets[preset].eciir_a1);
        WriteRAM(pProHW,pProslic->channel,ECIIR_A2,RTK_Si3226_impedence_Presets[preset].eciir_a2);

	WriteRAM(pProHW,pProslic->channel,ZSYNTH_A1,RTK_Si3226_impedence_Presets[preset].zsynth_a1);
	WriteRAM(pProHW,pProslic->channel,ZSYNTH_A2,RTK_Si3226_impedence_Presets[preset].zsynth_a2);
	WriteRAM(pProHW,pProslic->channel,ZSYNTH_B1,RTK_Si3226_impedence_Presets[preset].zsynth_b1);
	WriteRAM(pProHW,pProslic->channel,ZSYNTH_B0,RTK_Si3226_impedence_Presets[preset].zsynth_b0);
	WriteRAM(pProHW,pProslic->channel,ZSYNTH_B2,RTK_Si3226_impedence_Presets[preset].zsynth_b2);

	WriteReg(pProHW,pProslic->channel,RA,RTK_Si3226_impedence_Presets[preset].ra);

 	WriteRAM(pProHW,pProslic->channel,TXACGAIN,RTK_Si3226_impedence_Presets[preset].txacgain);

        WriteRAM(pProHW,pProslic->channel,RXACGAIN,RTK_Si3226_impedence_Presets[preset].rxacgain);
        WriteRAM(pProHW,pProslic->channel,RXACGAIN_SAVE,RTK_Si3226_impedence_Presets[preset].rxacgain_save);
	
        WriteRAM(pProHW,pProslic->channel,RXACHPF_B0_1,RTK_Si3226_impedence_Presets[preset].rxachpf_b0_1);
        WriteRAM(pProHW,pProslic->channel,RXACHPF_B1_1,RTK_Si3226_impedence_Presets[preset].rxachpf_b1_1);
        WriteRAM(pProHW,pProslic->channel,RXACHPF_A1_1,RTK_Si3226_impedence_Presets[preset].rxachpf_a1_1);

        WriteReg(pProHW,pProslic->channel,LINEFEED,lf);
	return 0;
}
#endif


void Si3226_Set_Impendance_Silicon(proslicChanType *pProslic, unsigned short country, unsigned short impd /*reserve*/)
{

	switch(country)
	{
		case COUNTRY_USA:
		case COUNTRY_HK:
		case COUNTRY_TW:
			Si3226_ZsynthSetup(pProslic, 0); 	// 600 ohm
			break;

		case COUNTRY_JP:
			Si3226_ZsynthSetup(pProslic, 2); 	// 600 ohm + 1uF
			break;

		
		case COUNTRY_AUSTRALIA:	
			Si3226_ZsynthSetup(pProslic, 6); 	// 220 onm + (600 omh || 100 nF)	
			break;
			
		case COUNTRY_CN:
			//Si3226_ZsynthSetup(pProslic, 3); 	// 200 ohm + (560 ohm || 100 nF)
			Si3226_ZsynthSetup(pProslic, 4);	// 200 ohm + (680 ohm || 100 nF)
			break;
		
		case COUNTRY_GR:
			Si3226_ZsynthSetup(pProslic, 7); 	// 220 ohm + (820 ohm || 115 nF)
			break;
			
		case COUNTRY_UK:
			Si3226_ZsynthSetup(pProslic, 5); 	// 
			break;
			
		case COUNTRY_SE:
			Si3226_ZsynthSetup(pProslic, 9); 	// 200 ohm + (1000 ohm || 100 nF)
			break;
			
		case COUNTRY_FR:
			Si3226_ZsynthSetup(pProslic, 11); 	// 215 ohm + (1000 ohm || 137 nF)
			break;
			
		case COUNTRY_BE:
			Si3226_ZsynthSetup(pProslic, 10); 	// 150 ohm + (830 ohm || 72 nF)
			break;
			
		case COUNTRY_FL:
			Si3226_ZsynthSetup(pProslic, 5); 	// 
			break;
			
		case COUNTRY_IT:
			Si3226_ZsynthSetup(pProslic, 8); 	// 400 ohm + (700 ohm || 200 nF)
			break;

#ifdef COUNTRY_TONE_RESERVED
		case COUNTRY_RESERVE:
#endif
		case COUNTRY_CUSTOME:
			Si3226_ZsynthSetup(pProslic, 0); 	// 600 ohm
			break;
			
		default:
			Si3226_ZsynthSetup(pProslic, 0); 
			PRINT_MSG(" Not support impedance of this country. Set to default SLIC impedance 600 ohm.\n");
			break;

	}

	//Si3226_ZsynthCheck(pProslic, 2);
	//Si3226_ImpdanceDump(pProslic);
}


void Si3226_Set_Impendance(proslicChanType *pProslic, unsigned short preset)
{
	switch (preset)
	{
		case 0:
			Si3226_ZsynthSetup(pProslic, 0);
			PRINT_MSG("Set SLIC impedance = 600 ohm\n");
			break;
		case 1:
			Si3226_ZsynthSetup(pProslic, 1);
			PRINT_MSG("Set SLIC impedance = 900 ohm\n");
			break;
		case 2:
			Si3226_ZsynthSetup(pProslic, 14);
			PRINT_MSG("Set SLIC impedance = 250+(750||150nf)\n");
			break;
		case 3:
			Si3226_ZsynthSetup(pProslic, 15);
			PRINT_MSG("Set SLIC impedance = 320+(1150||230nf)\n");
			break;
		case 4:
			Si3226_ZsynthSetup(pProslic, 16);
			PRINT_MSG("Set SLIC impedance = 350+(1000||210nf)\n");
			break;
		
		default:
			Si3226_ZsynthSetup(pProslic, 0);
			PRINT_MSG("Set SLIC impedance = 600 ohm\n");
			break;
	}
}


int Si3226_GetLinefeedStatus (proslicChanType *pProslic,uInt8 *newLinefeed){
	*newLinefeed = ReadReg (pProHW, pProslic->channel, LINEFEED);
	return 0;
}

int SI3226_SetUserMode(proslicChanType *pProslic, int on)
{
	setUserMode (pProslic,on);
#if 0	
	if (on == 1)
		printk("Si3226 channel %d enter user mode.\n", pProslic->channel);
	else if (on == 0)
		printk("Si3226 channel %d leave user mode.\n", pProslic->channel);
#endif
		
	return 0;
}





