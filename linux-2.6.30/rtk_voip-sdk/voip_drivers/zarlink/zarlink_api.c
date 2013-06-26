/** \file zarlink_api.c
 * 
 *
 * This file contains the major api for upper application
 * 
 *
 * Copyright (c) 2010, Realtek Semiconductor, Inc.
 *
 */
#include "zarlinkCommonSlic.h"
#include "zarlinkCommonDaa.h"

#undef DEBUG_API 

#ifdef DEBUG_API
static unsigned int gHookMap=0;
static void Zarlink_dump_hook_map(RTKLineObj *pLine, uint8 hook);
#endif

void ZarlinkSetFxsPcmMode(RTKLineObj *pLine, int pcm_mode)
{
	VpStatusType status;

	if (pLine->line_st != LINE_S_READY)
		return;
	
	status = RTK_CALL_DEV_FUNC(SetFxsPcmMode, (pLine, pcm_mode));
	ASSERT_zarlink(status);
}

void ZarlinkSetFxsAcProfileByBand(RTKLineObj *pLine, int pcm_mode)
{
	VpStatusType status;

	if (pLine->line_st != LINE_S_READY)
		return;

	status = RTK_CALL_DEV_FUNC(SetFxsAcProfileByBand, (pLine, pcm_mode));
    ASSERT_zarlink(status);
}

void ZarlinkSetRingCadenceProfile(RTKLineObj *pLine, uint8 ring_cad)
{
	VpStatusType status;

	if (pLine->line_st != LINE_S_READY)
		return;

	status = RTK_CALL_DEV_FUNC(SetRingCadenceProfile, (pLine, ring_cad));
    ASSERT_zarlink(status);
}

void ZarlinkSetRingCadence
(	RTKLineObj *pLine, 
	unsigned short on_msec, 
	unsigned short off_msec )
{
	VpStatusType status;

	if (pLine->line_st != LINE_S_READY)
		return;

	status = RTK_CALL_DEV_FUNC(SetRingCadence, (pLine, on_msec, off_msec));
    ASSERT_zarlink(status);
}

void ZarlinkSetRingFreqAmp(RTKLineObj *pLine, uint8 profile_id)
{
	VpStatusType status;

	if (pLine->line_st != LINE_S_READY)
		return;

	status = RTK_CALL_DEV_FUNC(SetRingFreqAmp, (pLine, profile_id));
    ASSERT_zarlink(status);
}

void ZarlinkFxsRing(RTKLineObj *pLine, uint8 enable)
{
	VpStatusType status;

	if (pLine->line_st != LINE_S_READY)
		return;

	status = RTK_CALL_DEV_FUNC(FxsRing, (pLine, enable));
    ASSERT_zarlink(status);
}

void ZarlinkSendNTTCAR(RTKLineObj *pLine)
{
	VpStatusType status;

	if (pLine->line_st != LINE_S_READY)
		return;

	status = RTK_CALL_DEV_FUNC(SendNTTCAR, (pLine));
    ASSERT_zarlink(status);
}

unsigned char ZarlinkSendNTTCAR_Check(RTKLineObj *pLine, unsigned long time_out)
{
	uint32 hook_status;
	
	if (pLine->line_st != LINE_S_READY)
		return 0;

	hook_status = RTK_CALL_DEV_FUNC(SendNTTCAR_Check, (pLine, time_out));

	return hook_status;
}

unsigned char ZarlinkGetFxsHookStatus(RTKLineObj *pLine, int from_polling_timer)
{
	unsigned char hook;

	if (pLine->line_st != LINE_S_READY)
		return 0;

	hook = RTK_CALL_DEV_FUNC(GetFxsHookStatus, (pLine, from_polling_timer));

	return hook;
}

unsigned char ZarlinkGetLineState(RTKLineObj *pLine)
{
	VpLineStateType lineState;
	
	if (pLine->line_st != LINE_S_READY)
		return 0;

	lineState = RTK_CALL_DEV_FUNC(GetLineState, (pLine));

	return (unsigned char)lineState;
}

void ZarlinkSetLineState(RTKLineObj *pLine, VpLineStateType state)
{
	VpStatusType status;

	if (pLine->line_st != LINE_S_READY)
		return;

	status = RTK_CALL_DEV_FUNC(SetLineState, (pLine, state));
    ASSERT_zarlink(status);
}

void ZarlinkSetOHT(RTKLineObj *pLine, uint8 reversal)
{
	VpStatusType status;

	if (pLine->line_st != LINE_S_READY)
		return;

	status = RTK_CALL_DEV_FUNC(SetOHT, (pLine, reversal));
    ASSERT_zarlink(status);
}

void ZarlinkSetLineOpen(RTKLineObj *pLine)
{
	VpStatusType status;

	if (pLine->line_st != LINE_S_READY)
		return;

	/* Disconnect line */
	status = RTK_CALL_DEV_FUNC(SetLineOpen, (pLine));
    ASSERT_zarlink(status);
}

void ZarlinkSetPcmTxOnly(RTKLineObj *pLine, int enable)
{
	VpStatusType status;

	if (pLine->line_st != LINE_S_READY)
		return;

	status = RTK_CALL_DEV_FUNC(SetPcmTxOnly, (pLine, enable));
    ASSERT_zarlink(status);
}

void ZarlinkSetPcmRxOnly(RTKLineObj *pLine, int enable)
{
	VpStatusType status;

	if (pLine->line_st != LINE_S_READY)
		return;

	status = RTK_CALL_DEV_FUNC(SetPcmRxOnly, (pLine, enable));
    ASSERT_zarlink(status);
}

void ZarlinkSetImpedenceCountry(RTKLineObj *pLine, uint8 country)
{
	VpStatusType status;

	if (pLine->line_st != LINE_S_READY)
		return;

	status = RTK_CALL_DEV_FUNC(SetImpedenceCountry, (pLine, country));
    ASSERT_zarlink(status);
}

void ZarlinkSetImpedence(RTKLineObj *pLine, uint16 preset)
{
	VpStatusType status;

	if (pLine->line_st != LINE_S_READY)
		return;

	status = RTK_CALL_DEV_FUNC(SetImpedence, (pLine, preset));
    ASSERT_zarlink(status);
}

unsigned char ZarlinkCheckFxsRing(RTKLineObj *pLine)		
{
	unsigned char ringer = 0; //0: ring off, 1: ring on

	if (pLine->line_st != LINE_S_READY)
		return 0;

	ringer = RTK_CALL_DEV_FUNC(CheckFxsRing, (pLine));

	return ringer;
}

void ZarlinkAdjustSlicTxGain(RTKLineObj *pLine, int gain)
{
	VpStatusType status;

	if (pLine->line_st != LINE_S_READY)
		return;

	status = RTK_CALL_DEV_FUNC(TxGainAdjust, (pLine, gain));
    ASSERT_zarlink(status);
}

void ZarlinkAdjustSlicRxGain(RTKLineObj *pLine, int gain)
{
	VpStatusType status;

	if (pLine->line_st != LINE_S_READY)
		return;

	status = RTK_CALL_DEV_FUNC(RxGainAdjust, (pLine, gain));
    ASSERT_zarlink(status);
}

void ZarlinkSetIOState(RTKLineObj *pLine, VPIO IO, int bHigh )
{
	if (pLine->line_st != LINE_S_READY)
		return;

	RTK_CALL_DEV_FUNC(SetIOState, (pLine, IO, bHigh));
}

void ZarlinkGetIOState(RTKLineObj *pLine, VPIO IO, int *bHigh )
{
	if (pLine->line_st != LINE_S_READY)
		return;

	RTK_CALL_DEV_FUNC(GetIOState, (pLine, IO, bHigh));
}

void ZarlinkSetIODir(RTKLineObj *pLine, VPIO IO, int bOut )
{
	if (pLine->line_st != LINE_S_READY)
		return;

	RTK_CALL_DEV_FUNC(SetIODir, (pLine, IO, bOut));
}

/********** Below are DAA function **************/
BOOL ZarlinkGetFxoLineStatus(RTKLineObj *pLine, uint8 category)
{

	if (pLine->line_st != LINE_S_READY)
		return FALSE;

	return RTK_CALL_DEV_FUNC(GetFxoLineStatus, (pLine, category));
}

void ZarlinkSetFxoLineState(RTKLineObj *pLine, VpLineStateType lineState)
{
	VpStatusType status;

	if (pLine->line_st != LINE_S_READY)
		return;

	status = RTK_CALL_DEV_FUNC(SetFxoLineState, (pLine, lineState));
    ASSERT_zarlink(status);

}

void ZarlinkSetFxoLineOnHook(RTKLineObj *pLine)
{
	VpStatusType status;

	if (pLine->line_st != LINE_S_READY)
		return;

	status = RTK_CALL_DEV_FUNC(SetFxoLineOnHook, (pLine));
    ASSERT_zarlink(status);
}

void ZarlinkSetFxoLineOffHook(RTKLineObj *pLine)
{
	VpStatusType status;

	if (pLine->line_st != LINE_S_READY)
		return;

	status = RTK_CALL_DEV_FUNC(SetFxoLineOffHook, (pLine));
    ASSERT_zarlink(status);
}

void ZarlinkSetFxoLineOHT(RTKLineObj *pLine)
{
	VpStatusType status;

	if (pLine->line_st != LINE_S_READY)
		return;

	status = RTK_CALL_DEV_FUNC(SetFxoLineOHT, (pLine));
    ASSERT_zarlink(status);
}

void ZarlinkSetFxoRingValidation(RTKLineObj *pLine)
{
	VpStatusType status;

	if (pLine->line_st != LINE_S_READY)
		return;

	status = RTK_CALL_DEV_FUNC(SetFxoRingValidation, (pLine));
    ASSERT_zarlink(status);

	ASSERT_zarlink(status);
}

unsigned char ZarlinkGetFxoHookStatus(RTKLineObj *pLine)
{
	if (pLine->line_st != LINE_S_READY)
		return FALSE; 

	return RTK_CALL_DEV_FUNC(GetFxoHookStatus, (pLine));
}

/* Debug command */
/* Dump Zarlink device register */
void ZarlinkDumpDevReg(RTKLineObj *pLine)		
{
	RTK_CALL_DEV_FUNC(DumpDevReg, (pLine->pDev));
}

void ZarlinkRWDevReg(
	RTKLineObj *pLine, 
	unsigned int reg, 
	unsigned char *len, 
	char * regdata)
{
	RTK_CALL_DEV_FUNC(RWDevReg, (pLine,reg,len,regdata));
}

#ifdef DEBUG_API
int Zarlink_ver(int deviceId)
{
	#if 1 // read revision
    unsigned char res[14]={0};
    int i;
    uint8 reg, len;
        
    reg = 0x73;
    len= 2;
    VpMpiCmd(deviceId, 0x3, reg, len, res);
    printk("Revision: ");
        
    for (i=0; i<len; i++)
        printk("\nbyte%d = 0x%x", i, res[i]);
    printk("\n");
	#endif

	return 0;
}
#endif

#ifdef DEBUG_API
static void Zarlink_dump_hook_map(RTKLineObj *pLine, uint8 hook)
{
	unsigned int tmp=gHookMap;

	gHookMap &= ~(1<<pLine->ch_id); 	/* reset ch_id bit */
	gHookMap |= (hook<<pLine->ch_id);

	if (gHookMap!=tmp) { 		/* dump if changing */
		PRINT_Y("HookMap %d = 0x%08X\n", pLine->ch_id, gHookMap);
	}
}
#endif




