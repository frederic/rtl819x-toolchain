/*
** Copyright (c) 2007-2009 by Silicon Laboratories
**
** $Id: si3217x_intf.c 1030 2009-09-01 22:59:44Z nizajerk@SILABS.COM $
**
** SI3217X_Intf.c
** SI3217X ProSLIC user define interface implementation file
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
#include "rtk_voip.h"
//#include "Slic_api.h"
#include "snd_define.h"
#include "si_voice_datatypes.h"
#include "si_voice_ctrl.h"
#include "si_voice_timer_intf.h"
#include "proslic.h"
#include "si3217x_intf.h"
#include "si3217x.h"
#include "si3217x_registers.h"
#include "proslic_api_config.h"

#define PRAM_ADDR (334 + 0x400)
#define PRAM_DATA (335 + 0x400)

#define WriteReg        pProslic->deviceId->ctrlInterface->WriteRegister_fptr
#define ReadReg         pProslic->deviceId->ctrlInterface->ReadRegister_fptr
#define pProHW          pProslic->deviceId->ctrlInterface->hCtrl
#define Reset           pProslic->deviceId->ctrlInterface->Reset_fptr
#define Delay           pProslic->deviceId->ctrlInterface->Delay_fptr
#define pProTimer       pProslic->deviceId->ctrlInterface->hTimer
#define WriteRAM        pProslic->deviceId->ctrlInterface->WriteRAM_fptr
#define ReadRAM         pProslic->deviceId->ctrlInterface->ReadRAM_fptr
#define TimeElapsed pProslic->deviceId->ctrlInterface->timeElapsed_fptr
#define getTime      pProslic->deviceId->ctrlInterface->getTime_fptr
#define SetSemaphore pProslic->deviceId->ctrlInterface->Semaphore_fptr

#define WriteRegX               deviceId->ctrlInterface->WriteRegister_fptr
#define ReadRegX                deviceId->ctrlInterface->ReadRegister_fptr
#define pProHWX                 deviceId->ctrlInterface->hCtrl
#define DelayX                  deviceId->ctrlInterface->Delay_fptr
#define pProTimerX              deviceId->ctrlInterface->hTimer
#define WriteRAMX               deviceId->ctrlInterface->WriteRAM_fptr
#define ReadRAMX                deviceId->ctrlInterface->ReadRAM_fptr
#define getTimeX             deviceId->ctrlInterface->getTime_fptr
#define TimeElapsedX    deviceId->ctrlInterface->timeElapsed_fptr

#define BROADCAST 0xff

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
    data = ReadReg(pProHW,pProslic->channel,USERMODE_ENABLE);
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



/* Add by Thlin */

int Si3217x_GetLinefeedStatus (proslicChanType *pProslic,uInt8 *newLinefeed)
{
	*newLinefeed = ReadReg (pProHW, pProslic->channel, LINEFEED);
	return 0;
}

void Si3217x_Set_Ring_Cadence_ON(proslicChanType *pProslic, unsigned short msec)
{
	unsigned int l_time, h_time, tmp;
	unsigned long flags;

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

void Si3217x_Set_Ring_Cadence_OFF(proslicChanType *pProslic, unsigned short msec)
{
	unsigned int l_time, h_time, tmp;
	unsigned long flags;

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

void Si3217x_Set_Impendance_Silicon(proslicChanType *pProslic, unsigned short country, unsigned short impd /*reserve*/)
{
	switch(country)
	{
		case COUNTRY_USA:
		case COUNTRY_HK:
		case COUNTRY_TW:
			Si3217x_ZsynthSetup(pProslic, 0); 	// 600 ohm
			break;

		case COUNTRY_JP:
			Si3217x_ZsynthSetup(pProslic, 2); 	// 600 ohm + 1uF
			break;
		
		case COUNTRY_AUSTRALIA:	
			Si3217x_ZsynthSetup(pProslic, 6); 	// (220 ohm + (820 ohm||120nF))	
			break;
			
		case COUNTRY_CN:
			//Si3217x_ZsynthSetup(pProslic, 3); 	// 200 ohm + (560 ohm || 100 nF)
			Si3217x_ZsynthSetup(pProslic, 4);	// 200 ohm + (680 ohm || 100 nF)
			break;
		
		case COUNTRY_GR:
			Si3217x_ZsynthSetup(pProslic, 7); 	// 220 ohm + (820 ohm || 115 nF)
			break;
			
		case COUNTRY_UK:
			Si3217x_ZsynthSetup(pProslic, 5); 	// 
			break;
			
		case COUNTRY_SE:
			Si3217x_ZsynthSetup(pProslic, 9); 	// 200 ohm + (1000 ohm || 100 nF)
			break;
			
		case COUNTRY_FR:
			Si3217x_ZsynthSetup(pProslic, 11); 	// 215 ohm + (1000 ohm || 137 nF)
			break;
			
		case COUNTRY_BE:
			Si3217x_ZsynthSetup(pProslic, 10); 	// 150 ohm + (830 ohm || 72 nF)
			break;
			
		case COUNTRY_FL:
			Si3217x_ZsynthSetup(pProslic, 5); 	//
			break;
			
		case COUNTRY_IT:
			Si3217x_ZsynthSetup(pProslic, 8); 	// 400 ohm + (700 ohm || 200 nF)
			break;
		

			
		default:
			Si3217x_ZsynthSetup(pProslic, 0); 
			PRINT_MSG(" Not support impedance of this country. Set to default SLIC impedance 600 ohm.\n");
			break;

	}

	//Si3226_ZsynthCheck(pProslic, 2);
	//Si3226_ImpdanceDump(pProslic);
}

void Si3217x_Set_Impendance(proslicChanType *pProslic, unsigned short preset)
{
	switch (preset)
	{
		case 0:
			Si3217x_ZsynthSetup(pProslic, 0);
			PRINT_MSG("Set SLIC impedance = 600 ohm\n");
			break;
		case 1:
			Si3217x_ZsynthSetup(pProslic, 1);
			PRINT_MSG("Set SLIC impedance = 900 ohm\n");
			break;
		case 2:
			Si3217x_ZsynthSetup(pProslic, 14);
			PRINT_MSG("Set SLIC impedance = 250+(750||150nf)\n");
			break;
		case 3:
			Si3217x_ZsynthSetup(pProslic, 15);
			PRINT_MSG("Set SLIC impedance = 320+(1150||230nf)\n");
			break;
		case 4:
			Si3217x_ZsynthSetup(pProslic, 16);
			PRINT_MSG("Set SLIC impedance = 350+(1000||210nf)\n");
			break;
		
		default:
			Si3217x_ZsynthSetup(pProslic, 0);
			PRINT_MSG("Set SLIC impedance = 600 ohm\n");
			break;
	}
}

#include <linux/kernel.h>
#include <linux/delay.h>

static unsigned long l_buf[4];
static unsigned char c_buf[9]; // remove 74 (VBATH)
static int b_num[]={RINGOF, RINGFR, RINGAMP, RINGPHAS, RINGCON, RINGTALO, RINGTAHI, RINGTILO, RINGTIHI};
static unsigned long l_data[]={0x0000, 0x7F46000, 0x248000, 0x0000, 0x18, 0xa0, 0x0f, 0xa0, 0x0f};


void Si3217x_SendNTTCAR(proslicChanType *pProslic)
{
	int i;
	/************** backup the register ***************/

	for ( i=0;i<4;i++)
		l_buf[i] = ReadRAM(pProHW, pProslic->channel, b_num[i]);

	for ( i=4;i<9;i++)
		c_buf[i] = ReadReg(pProHW, pProslic->channel,b_num[i]);

	/*********** To Create Short Ring *****************/
	//send CAR
	for (i=0;i<4;i++)
		WriteRAM(pProHW,pProslic->channel,b_num[i],l_data[i]);
	for (i=4;i<9;i++)
		WriteReg(pProHW,pProslic->channel,b_num[i],l_data[i]);

	Si3217x_RingStart(pProslic);
}

unsigned int Si3217x_SendNTTCAR_check(unsigned int chid, proslicChanType *pProslic, unsigned long time_out)
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
	for (i=0;i<4;i++)
		WriteRAM(pProHW,pProslic->channel,b_num[i],l_buf[i]);
	for (i=4;i<9;i++)
		WriteReg(pProHW,pProslic->channel,b_num[i],c_buf[i]);


	return 1;
}

int SI3217x_SetUserMode(proslicChanType *pProslic, int on)
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


void SI3217X_Set_SO_DTx_Loopback(proslicChanType *pProslic, unsigned int enable)
{
	unsigned char reg_val;
	
	reg_val = ReadReg(pProHW, pProslic->channel, 74);
	
	if (enable == 1)
	{
		WriteReg(pProHW, pProslic->channel, 74, reg_val|0xC);
		printk("SI3217X_Set_SO_DTx_Loopback enable\n");
	}
	else if (enable == 0)
	{
		WriteReg(pProHW, pProslic->channel, 74, reg_val&0xF3);
		printk("SI3217X_Set_SO_DTx_Loopback disable\n");
	}
}

