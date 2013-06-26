#include "voip_timer.h"
#include "vp_api.h"
#include "zarlinkCommonSlic.h"

#undef DEBUG_API 

#if defined( CONFIG_RTK_VOIP_SLIC_ZARLINK_880_SERIES )
#include "ve880_api.h"
#endif

#if defined(CONFIG_RTK_VOIP_SLIC_ZARLINK_890_SERIES )
#include "ve890_api.h"
#endif

#if defined(DEBUG_API)
#define DEBUG_API_PRINT() printk("%s(%d) line #%d\n",__FUNCTION__,__LINE__,pLine->ch_id);
#else
#define DEBUG_API_PRINT()
#endif


/*
** Global variable 
*/
int gCHId=0;

/*
** local variable 
*/

/*
** local function 
*/
static BOOL zarlinkFxsLineIsOffhook(VpLineCtxType *pLineCtx, VpDevCtxType *pDevCtx, int from_Polling_timer);
static int Conv2ZarlinkGain(int adj);

/*
*****************************************************************************
** FUNCTION:   zarlinkFxsLineIsOffhook
**
** PURPOSE:    Determine if a channel is on or off hook
**
** PARAMETERS: 
**
** RETURNS:    TRUE if offhook, FALSE if onhook
**
*****************************************************************************
*/
static BOOL zarlinkFxsLineIsOffhook(VpLineCtxType *pLineCtx, VpDevCtxType *pDevCtx, int from_polling_timer)
{
	bool lineStatus = FALSE;

	if ( VpGetLineStatus( pLineCtx, VP_INPUT_HOOK, &lineStatus ) == VP_STATUS_SUCCESS ) {
		if ( lineStatus == TRUE ) return TRUE;
		else 					  return FALSE;
		
	} else {

		PRINT_R("SLIC ERROR %s %d\n", __FUNCTION__, __LINE__);
	}

	return FALSE;
}

/* Move from ve890_api.c *****************************************************/

VpStatusType zarlinkSetFxsPcmMode(RTKLineObj *pLine, int pcm_mode)
{
	VpStatusType status;
	VpOptionCodecType codecType = getCodecType(pcm_mode);

	DEBUG_API_PRINT(); 

	status = VpSetOption( pLine->pLineCtx, VP_NULL, 
				  VP_OPTION_ID_CODEC, (void*)&codecType );   

	/* TODO if pcm_mode is changing from widebade to narrrowband. we should change AC profile here */
   
	if ( status == VP_STATUS_SUCCESS ) {
		pLine->codec_type = codecType;
		pLine->pcm_law_save = pcm_mode;
	}

	return status;
}

VpStatusType zarlinkSetRingCadence (   
	RTKLineObj *pLine,
    unsigned short on_msec,
    unsigned short off_msec )
{
    VpStatusType status;
	uint8 * ring_cad;
    uint8 data[4];

	DEBUG_API_PRINT();

	ring_cad = (uint8*)pLine->pDev->pRing_cad_usr_profile;

	/* write cadence timer coeff. */
	data[0] = 0x20 + ((on_msec/5)>>8);
	data[1] = (on_msec/5);
	data[2] = 0x20 + ((off_msec/5)>>8);
	data[3] = (off_msec/5);
	
	ring_cad[RING_PROFILE_CAD_ON_H_IDX]  = data[0];
	ring_cad[RING_PROFILE_CAD_ON_L_IDX]  = data[1];
	ring_cad[RING_PROFILE_CAD_OFF_H_IDX] = data[2];
	ring_cad[RING_PROFILE_CAD_OFF_L_IDX] = data[3];
	
	//TODO should handle ling base ring cadence	
	status = VpInitRing( pLine->pLineCtx,
						 ring_cad,
						 VP_PTABLE_NULL);

	if ( status == VP_STATUS_SUCCESS ) {
		pLine->pRing_cad_profile = ring_cad;
		pLine->pDev->cad_on_ms   = (unsigned int)on_msec;
		pLine->pDev->cad_off_ms  = (unsigned int)off_msec;
	}

	return status;
}

#if 0
VpStatusType zarlinkSetRingFreqAmp(RTKLineObj *pLine, uint8 profile_id)
{
	VpProfilePtrType ring_profile;
	VpStatusType status;

    DEBUG_API_PRINT();

#if defined(CONFIG_RTK_VOIP_SLIC_ZARLINK_880_SERIES)
	if (VP_DEV_880_SERIES == pLine->pDev->VpDevType)
		ring_profile = Ve880RingProfile(profile_id);
#endif

#if defined(CONFIG_RTK_VOIP_SLIC_ZARLINK_890_SERIES)
	if (VP_DEV_890_SERIES == pLine->pDev->VpDevType)
		ring_profile = Ve890RingProfile(profile_id);
#endif

	status = VpConfigLine( pLine->pLineCtx,
						   VP_PTABLE_NULL,
						   VP_PTABLE_NULL,
						   ring_profile );

	pLine->pRing_profile = ring_profile;
	return status;
}
#endif

VpStatusType zarlinkFxsRing(RTKLineObj *pLine, uint8 enable)
{
	VpStatusType status;

    DEBUG_API_PRINT();

	if (enable) status = VpSetLineState( pLine->pLineCtx, VP_LINE_RINGING );
	else 		status = VpSetLineState( pLine->pLineCtx, VP_LINE_OHT);

	return status;
}

VpStatusType zarlinkSendNTTCAR(RTKLineObj *pLine)
{
	VpStatusType status;

    DEBUG_API_PRINT();

	/*********** Change Setting To Create Short Ring *****************/
#if defined(CONFIG_RTK_VOIP_SLIC_ZARLINK_880_SERIES)
	if (VP_DEV_880_SERIES == pLine->pDev->VpDevType)
		status = Ve880SetRingCadenceProfile(pLine, 1);
#endif

#if defined(CONFIG_RTK_VOIP_SLIC_ZARLINK_890_SERIES)
	if (VP_DEV_890_SERIES == pLine->pDev->VpDevType)
		status = Ve890SetRingCadenceProfile(pLine, 1);
#endif

	/*********** Ring the FXS *************************/
	status = VpSetLineState( pLine->pLineCtx, VP_LINE_RINGING );

	return status;
}

unsigned char zarlinkSendNTTCAR_Check(RTKLineObj *pLine, unsigned long time_out)
{
	VpStatusType status;
	uint32 hook_status;
	
    DEBUG_API_PRINT();

	/*********** Check Phone Hook State ***************/
	hook_status = zarlinkFxsLineIsOffhook(pLine->pLineCtx, 
						pLine->pDev->pDevCtx, 0);

	/* if phone on-hook */
	if (hook_status == 0) {
		/* time_after(a,b) returns true if the time a is after time b. */
		if (timetick_after(timetick,time_out) ) 		{
			/* don't return 0, return 1, report time out don't wait */
			PRINT_MSG("wait off-hook timeout...\n");
		} else return 0;
	}

	/******* Set Reverse On-Hook Transmission *********/		

	/* if phone off-hook, set Reverse On-Hook Transmission */
	status = VpSetLineState( pLine->pLineCtx, VP_LINE_OHT_POLREV );

	/************** restore the register ***************/
	zarlinkSetRingCadence(pLine, pLine->pDev->cad_on_ms, pLine->pDev->cad_off_ms);

	PRINT_MSG("Set normal ring\n");

	return 1;
}

unsigned char zarlinkGetFxsHookStatus(RTKLineObj *pLine, int from_polling_timer)
{
	unsigned char hook=0;
	BOOL bHook;

	bHook = (unsigned char)zarlinkFxsLineIsOffhook(pLine->pLineCtx, 
				pLine->pDev->pDevCtx, from_polling_timer);

	if (TRUE == bHook)
		hook = 1;

	#ifdef DEBUG_API
	//Ve890_dump_hook_map(pLine->ch_id, hook);
	#endif

	return hook;
}

unsigned char zarlinkGetLineState(RTKLineObj *pLine)
{
	VpStatusType status;
	VpLineStateType lineState;
	
    DEBUG_API_PRINT();

   	status = VpGetLineState( pLine->pLineCtx, &lineState );

	return (unsigned char)lineState;
}

VpStatusType zarlinkSetLineState(RTKLineObj *pLine, VpLineStateType mode)
{
	VpStatusType status;
   	VpLineStateType lineState;

    DEBUG_API_PRINT();

	switch ( mode )
	{
		case VP_LINE_DISCONNECT:
			lineState = VP_LINE_DISCONNECT;
			break;

		/* Standby mode */
		case VP_LINE_STANDBY:
		 	lineState = VP_LINE_STANDBY;
			break;

		/* On-hook transmission */
		case VP_LINE_OHT:
		 	/* Active mode supports OHT */
		 	lineState = VP_LINE_OHT;
			break;

		/* On-hook transmission reverse */
		case VP_LINE_OHT_POLREV:
		 	/* Active mode supports OHT AND reverse polarity */
		 	lineState = VP_LINE_OHT_POLREV;
			break;

		/* Ringing */
		case VP_LINE_RINGING:
		 	lineState = VP_LINE_RINGING;
			break;

		/* Loop current feed */
		case VP_LINE_TALK:
		 	lineState = VP_LINE_TALK;
			break;

		/* Reverse loop current feed */
		case VP_LINE_TALK_POLREV:
		 	lineState = VP_LINE_TALK_POLREV;
			break;

		default:
		 	PRINT_R(" ERROR: unrecognized line state (%d)\n", mode);
		 	return VP_STATUS_INVALID_ARG;
			break;
	}

   	status = VpSetLineState( pLine->pLineCtx, lineState );

	return status;
}

VpStatusType zarlinkSetOHT(RTKLineObj *pLine, uint8 reversal)
{
	VpStatusType status;

    DEBUG_API_PRINT();

	if (reversal) status = VpSetLineState( pLine->pLineCtx, VP_LINE_OHT_POLREV );
	else 		  status = VpSetLineState( pLine->pLineCtx, VP_LINE_OHT );

	return status;
}

VpStatusType zarlinkSetLineOpen(RTKLineObj *pLine)
{
	VpStatusType status;

    DEBUG_API_PRINT();

	/* Disconnect line */
	status = VpSetLineState( pLine->pLineCtx, VP_LINE_DISCONNECT );

	return status;
}

VpStatusType zarlinkSetPcmTxOnly(RTKLineObj *pLine, int enable)
{
	VpStatusType status;
	VpOptionPcmTxRxCntrlType pcm_ctrl;

    DEBUG_API_PRINT();

	if (enable == 1) {
		pcm_ctrl = VP_OPTION_PCM_TX_ONLY;
		//PRINT_MSG("Le89xxxSetPcmTxOnly!\n");
	} else if (enable == 0) {
		pcm_ctrl = VP_OPTION_PCM_BOTH;
		//PRINT_MSG("Le89xxxSetPcmBoth!\n");
	} else {
		PRINT_R("%s :Error config!\n",__FUNCTION__);
		return VP_STATUS_INVALID_ARG;
	}
	
	status = VpSetOption( pLine->pLineCtx, 
				VP_NULL, VP_OPTION_ID_PCM_TXRX_CNTRL, &pcm_ctrl);   

	return status;
}

VpStatusType zarlinkSetPcmRxOnly(RTKLineObj *pLine, int enable)
{
	VpStatusType status;
	VpOptionPcmTxRxCntrlType pcm_ctrl;
	
    DEBUG_API_PRINT();

	if (enable == 1) {
		pcm_ctrl = VP_OPTION_PCM_RX_ONLY;
		//PRINT_MSG("Le89xxxSetPcmRxOnly!\n");
	} else if (enable == 0) {
		pcm_ctrl = VP_OPTION_PCM_BOTH;
		//PRINT_MSG("Le89xxxSetPcmBoth!\n");
	} else {
		PRINT_R("%s :Error config!\n",__FUNCTION__);
		return VP_STATUS_INVALID_ARG;
	}
	
	status = VpSetOption( pLine->pLineCtx, 
				VP_NULL, VP_OPTION_ID_PCM_TXRX_CNTRL, &pcm_ctrl);   

	return status;
}

#if 0
VpStatusType zarlinkSetImpedence(RTKLineObj *pLine, uint16 preset)
{
	VpStatusType status;
	VpProfilePtrType ac_profile;
	
    DEBUG_API_PRINT();

	PRINT_MSG(" <<<<<<<<< Le89xxxSetImpedence >>>>>>>>>\n");

#if defined(CONFIG_RTK_VOIP_SLIC_ZARLINK_880_SERIES)
	if (VP_DEV_880_SERIES == pLine->pDev->VpDevType)
		ac_profile = Ve880AcProfile(preset);
#endif

#if defined(CONFIG_RTK_VOIP_SLIC_ZARLINK_890_SERIES)
	if (VP_DEV_890_SERIES == pLine->pDev->VpDevType)
		ac_profile = Ve890AcProfile(preset);
#endif
	status = VpConfigLine( pLine->pLineCtx,
						   ac_profile,
						   VP_PTABLE_NULL,
						   VP_PTABLE_NULL );

	pLine->pAC_profile = ac_profile;
	return status;
}
#endif

unsigned char zarlinkCheckFxsRing(RTKLineObj *pLine)		
{
	unsigned char ringer = 0; //0: ring off, 1: ring on
	VpStatusType status;
	VpLineStateType lineState;

    DEBUG_API_PRINT();

	/* Read 0xDF is also fine for ring-on/ring-off status */
   	status = VpGetLineStateINT( pLine->pLineCtx, &lineState );

	if ( lineState == VP_LINE_RINGING ||
       	 lineState == VP_LINE_RINGING_POLREV ) 
		ringer = 1;

	return ringer;

}

VpStatusType zarlinkTxGainAdjust(RTKLineObj *pLine, int gain)		
{
	VpStatusType status;

    DEBUG_API_PRINT();

	//printk("%s(%d)chid %d, gain %d\n",__FUNCTION__,__LINE__,pLine->ch_id, gain);

	status = VpSetRelGain(pLine->pLineCtx,Conv2ZarlinkGain(gain),Conv2ZarlinkGain(pLine->RxGainAdj),pLine->ch_id); 

	if ( status == VP_STATUS_SUCCESS ) 
		pLine->TxGainAdj = gain;

	return status;
}

VpStatusType zarlinkRxGainAdjust(RTKLineObj *pLine, int gain)		
{
	VpStatusType status;

    DEBUG_API_PRINT();

	//printk("%s(%d)chid %d, gain %d\n",__FUNCTION__,__LINE__,pLine->ch_id, gain);

	status = VpSetRelGain(pLine->pLineCtx,Conv2ZarlinkGain(pLine->TxGainAdj), Conv2ZarlinkGain(gain),pLine->ch_id); 

	if ( status == VP_STATUS_SUCCESS ) 
		pLine->RxGainAdj = gain;

	return status;
}

int GainTbl[]= {
    0xFEC9, 0xE314, 0xCA62, 0xB460, 0xA0C2, 0x8F47,         /*  12dB to   7dB */
  //0x7FB2, 0x71CF, 0x656E, 0x5A67, 0x5092, 0x47CF,         /*   6dB to   0dB */
	0x788D, 0x71CF, 0x656E, 0x5A67, 0x5092, 0x47CF,         /* 5.5dB to   0dB */
	0x4000,                                                 /*   0dB          */
	0x390A, 0x32D6, 0x2D4E, 0x2861, 0x23FD, 0x2013, 	    /*  -1dB to - 6dB */
	0x1C96, 0x197A, 0x16B5, 0x143D, 0x1209, 0x1013          /*  -7dB to -12dB */
	};

static int Conv2ZarlinkGain(int adj)
{
	int nGainVal;

	nGainVal = adj;

	if (adj > 12)  nGainVal=12;
	if (adj < -12) nGainVal=-12;

	return (GainTbl[12-nGainVal]);
}

/* 
** API	  : getCodecType()
** Desp	  :	Convert Realtek pcm_mode to Zarlink code type
** input  : Realtek pcm_mode
** return : Zarlink codec type
*/
VpOptionCodecType getCodecType(BUS_DATA_FORMAT pcm_data_format)
{
	VpOptionCodecType codecType;

	switch ( pcm_data_format ) {
		case BUSDATFMT_PCM_LINEAR:
			codecType = VP_OPTION_LINEAR;
			break;

		case BUSDATFMT_PCM_ALAW:
			codecType = VP_OPTION_ALAW;
			break;

		case BUSDATFMT_PCM_ULAW:
			codecType = VP_OPTION_MLAW;
			break;

		case BUSDATFMT_PCM_WIDEBAND_LINEAR:
			codecType = VP_OPTION_WIDEBAND;
			break;

		case BUSDATFMT_PCM_WIDEBAND_ALAW:
			codecType = VP_OPTION_WIDEBAND;
			break;

		case BUSDATFMT_PCM_WIDEBAND_ULAW:
			codecType = VP_OPTION_WIDEBAND;
			break;

		default:
			codecType = VP_OPTION_LINEAR;
			break;
	}
	return ( codecType );
}

//RTKDevObj
VpStatusType zarlinkDumpDevReg (
	RTKDevObj *pDev )
{
	VpRegisterDump (pDev->pDevCtx);

}

/* 
** API	  : zarlinkReadRegister()
** Desp	  :	Read Zarlink register
** input  : pDev, Reg# 
** return : print Register value
*/
VpStatusType zarlinkRWDevReg (
	RTKLineObj *pLine,
	unsigned int reg, 
	unsigned char *len, 
	char * regdata) 
{
	VpRegisterReadWrite(pLine->pLineCtx, reg, len, regdata);
}

/* 
** API	  : rtkGetNewChID()
** Desp	  :	Increase global ch id
** input  : Realtek NA
** return : Zarlink global ch id
*/
int rtkGetNewChID()
{
	return gCHId++;
}


