/*
** Copyright (c) 2007 by Silicon Laboratories
**
** $Id: si_voice.c 455 2009-03-10 22:45:56Z cdp@SILABS.COM $
**
** si_voice.c
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
#include "proslic.h"

int SiVoice_createControlInterface (controlInterfaceType **pCtrlIntf){
	return ProSLIC_createControlInterface(pCtrlIntf);
}
int SiVoice_destroyControlInterface (controlInterfaceType **pCtrlIntf){
	return ProSLIC_destroyControlInterface (pCtrlIntf);
}
int SiVoice_createDevice (SiVoiceDeviceType **pDev){
	return ProSLIC_createDevice(pDev);
}
int SiVoice_destroyDevice (SiVoiceDeviceType **pDev){
	return ProSLIC_destroyDevice (pDev);
}
int SiVoice_setControlInterfaceCtrlObj (controlInterfaceType *pCtrlIntf, void *hCtrl){
	return ProSLIC_setControlInterfaceCtrlObj(pCtrlIntf,hCtrl);
}
int SiVoice_setControlInterfaceReset (controlInterfaceType *pCtrlIntf, ctrl_Reset_fptr Reset_fptr){
	return ProSLIC_setControlInterfaceReset(pCtrlIntf,Reset_fptr);
}
int SiVoice_setControlInterfaceWriteRegister (controlInterfaceType *pCtrlIntf, ctrl_WriteRegister_fptr WriteRegister_fptr){
	return ProSLIC_setControlInterfaceWriteRegister(pCtrlIntf,WriteRegister_fptr);
}
int SiVoice_setControlInterfaceReadRegister (controlInterfaceType *pCtrlIntf, ctrl_ReadRegister_fptr ReadRegister_fptr){
	return ProSLIC_setControlInterfaceReadRegister (pCtrlIntf,ReadRegister_fptr);
}
int SiVoice_setControlInterfaceWriteRAM (controlInterfaceType *pCtrlIntf, ctrl_WriteRAM_fptr WriteRAM_fptr){
	return ProSLIC_setControlInterfaceWriteRAM(pCtrlIntf,WriteRAM_fptr);
}
int SiVoice_setControlInterfaceReadRAM (controlInterfaceType *pCtrlIntf, ctrl_ReadRAM_fptr ReadRAM_fptr){
	return ProSLIC_setControlInterfaceReadRAM(pCtrlIntf,ReadRAM_fptr);
}
int SiVoice_setControlInterfaceTimerObj (controlInterfaceType *pCtrlIntf, void *hTimer){
	return ProSLIC_setControlInterfaceTimerObj(pCtrlIntf,hTimer);
}
int SiVoice_setControlInterfaceDelay (controlInterfaceType *pCtrlIntf, system_delay_fptr Delay_fptr){
	return ProSLIC_setControlInterfaceDelay(pCtrlIntf,Delay_fptr);
}
int SiVoice_setControlInterfaceSemaphore (controlInterfaceType *pCtrlIntf, ctrl_Semaphore_fptr semaphore_fptr){
	return ProSLIC_setControlInterfaceSemaphore(pCtrlIntf,semaphore_fptr);
}
int SiVoice_setControlInterfaceTimeElapsed (controlInterfaceType *pCtrlIntf, system_timeElapsed_fptr timeElapsed_fptr){
	return ProSLIC_setControlInterfaceTimeElapsed(pCtrlIntf,timeElapsed_fptr);
}
int SiVoice_setControlInterfaceGetTime (controlInterfaceType *pCtrlIntf, system_getTime_fptr getTime_fptr){
	return ProSLIC_setControlInterfaceGetTime(pCtrlIntf,getTime_fptr);
}
