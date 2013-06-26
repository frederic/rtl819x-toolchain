/*
** Copyright (c) 2007 by Silicon Laboratories
**
** $Id: si_voice.h 458 2009-03-10 22:47:25Z cdp@SILABS.COM $
**
** si_voice.h
**
** Author(s): 
** laj
**
** Distributed by: 
** Silicon Laboratories, Inc
**
** This file contains proprietary information.	 
** No dissemination allowed without prior written permission from
** Silicon Laboratories, Inc.
**
** File Description:
** This is the header file for the ProSLIC driver.
**
** Dependancies:
** si_voice_datatypes.h
**
*/

#ifndef SI_VOICE_H
#define SI_VOICE_H

#include "si_voice_ctrl.h"
#include "si_voice_timer_intf.h"
/*
** This is the main Silabs control interface object. do not access directly!
*/
typedef struct	{
		void *hCtrl;
		ctrl_Reset_fptr Reset_fptr;
		ctrl_WriteRegister_fptr WriteRegister_fptr;
		ctrl_ReadRegister_fptr ReadRegister_fptr;
		ctrl_WriteRAM_fptr WriteRAM_fptr; /*ProSLIC only*/
		ctrl_ReadRAM_fptr ReadRAM_fptr; /*ProSLIC only*/
		ctrl_Semaphore_fptr Semaphore_fptr; /*ProSLIC only*/
		void *hTimer;
		system_delay_fptr Delay_fptr;
		system_timeElapsed_fptr timeElapsed_fptr;
		system_getTime_fptr getTime_fptr;
		int usermodeStatus; /*ProSLIC only*/
} SiVoiceControlInterfaceType;

typedef enum {
	SI3210, 
	SI3215,
	SI3216,
	SI3211, 	
	SI3212,
 	SI3210M,
 	SI3215M,
 	SI3216M,
	SI3240,
	SI3241,
	SI3242,
	SI3243,
	SI3245,
	SI3244,
	SI3246,
	SI3247,
	SI3220,
	SI3225,
	SI3226,
	SI3227,
	SI32171,
	SI32172,
	SI32174,
	SI32175,
	SI32176,
	SI32177,
	SI32178,
	SI32179
}partNumberType;

/*
** Chip revision definition for easier readability in the source code
*/
typedef enum {
	A,
	B,
	C,
	D,
	E,
	F,
	G
}revisionType ;

/*
** This is the main ProSLIC device object
*/
typedef struct	{
		SiVoiceControlInterfaceType *ctrlInterface;
		revisionType chipRev;
		partNumberType chipType;
		uInt8 lsRev;
		uInt8 lsType;
		int usermodeStatus;
} SiVoiceDeviceType;

int SiVoice_createControlInterface (SiVoiceControlInterfaceType **pCtrlIntf);
int SiVoice_destroyControlInterface (SiVoiceControlInterfaceType **pCtrlIntf);
int SiVoice_createDevice (SiVoiceDeviceType **pDev);
int SiVoice_destroyDevice (SiVoiceDeviceType **pDev);
int SiVoice_setControlInterfaceCtrlObj (SiVoiceControlInterfaceType *pCtrlIntf, void *hCtrl);
int SiVoice_setControlInterfaceReset (SiVoiceControlInterfaceType *pCtrlIntf, ctrl_Reset_fptr Reset_fptr);
int SiVoice_setControlInterfaceWriteRegister (SiVoiceControlInterfaceType *pCtrlIntf, ctrl_WriteRegister_fptr WriteRegister_fptr);
int SiVoice_setControlInterfaceReadRegister (SiVoiceControlInterfaceType *pCtrlIntf, ctrl_ReadRegister_fptr ReadRegister_fptr);
int SiVoice_setControlInterfaceWriteRAM (SiVoiceControlInterfaceType *pCtrlIntf, ctrl_WriteRAM_fptr WriteRAM_fptr);
int SiVoice_setControlInterfaceReadRAM (SiVoiceControlInterfaceType *pCtrlIntf, ctrl_ReadRAM_fptr ReadRAM_fptr);
int SiVoice_setControlInterfaceTimerObj (SiVoiceControlInterfaceType *pCtrlIntf, void *hTimer);
int SiVoice_setControlInterfaceDelay (SiVoiceControlInterfaceType *pCtrlIntf, system_delay_fptr Delay_fptr);
int SiVoice_setControlInterfaceSemaphore (SiVoiceControlInterfaceType *pCtrlIntf, ctrl_Semaphore_fptr semaphore_fptr);
int SiVoice_setControlInterfaceTimeElapsed (SiVoiceControlInterfaceType *pCtrlIntf, system_timeElapsed_fptr timeElapsed_fptr);
int SiVoice_setControlInterfaceGetTime (SiVoiceControlInterfaceType *pCtrlIntf, system_getTime_fptr getTime_fptr);

#endif
