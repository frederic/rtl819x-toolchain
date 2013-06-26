/** \file ve890_int.c
 * 
 *
 * This file contains the major process of zarlink slic
 * 
 *
 * Copyright (c) 2010, Realtek Semiconductor, Inc.
 *
 */
#include "ve890_int.h"
#include "ve890_api.h"
#include "Ve_profile.h"
#include "zarlinkCommonInit.h"
#include "zarlinkCommonSlic.h"
#include "zarlinkCommonDaa.h"
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_ZARLINK_ON_NEW_ARCH
#include "snd_define.h"
#else
#include "Slic_api.h"	/* for PCM_MODE */
#endif

#undef DEBUG_INT 

/*
** local function 
*/
static VpProfilePtrType Ve890GetDevProvile(unsigned int devicePcn);

/*
** external variable
*/

/*
** external function
*/

/* 
** API	  : Ve890CreateDevObj()
** Desp	  :	Create Realtek dev obj
** input  : dev_id, RTKDevType 
** return : max line number
*/
int Ve890CreateDevObj(
			/* Realtek */
			RTKDevType 				dev_type, 
			int						ch_id,
			RTKDevObj 				*pDevObj, 
			RTKLineObj 				LineObj[],

			/* Zarlink */
			VpDeviceIdType 			devId, 
			VpDeviceType  			vpDevType, 
			Vp890DeviceObjectType	*pVpDevObj, 
			VpDevCtxType  			*pVpDevCtx, 
			Vp890LineObjectType  	VpLineObj[],
			VpLineCtxType 			VpLineCtx[]
            )
{
	int devicePcn;	
	VpProfilePtrType pDevProfile;
	
	pDevObj->dev_id				= (unsigned int)devId;
	pDevObj->dev_type			= dev_type;
	pDevObj->pDevObj 			= pVpDevObj;
	pDevObj->pDevCtx 			= pVpDevCtx;

	PRINT_MSG("Creating dev 0x%08x, type=%d\n", (unsigned int)devId, dev_type);

	switch(dev_type) {
		case DEV_FXS:
		case DEV_FXO:
			pDevObj->max_line			= 1;
			pDevObj->pLine[0]			= &LineObj[0];

			pDevObj->pLine[0]->pDev		= pDevObj;
			pDevObj->pLine[0]->pLineObj	= &VpLineObj[0];
			pDevObj->pLine[0]->pLineCtx	= &VpLineCtx[0];

			break;

		case DEV_FXSFXO:
		case DEV_FXSFXS:
			pDevObj->max_line			= 2;
			pDevObj->pLine[0]			= &LineObj[0];
			pDevObj->pLine[1]			= &LineObj[1];

			pDevObj->pLine[0]->pDev		= pDevObj;
			pDevObj->pLine[0]->pLineObj	= &VpLineObj[0];
			pDevObj->pLine[0]->pLineCtx	= &VpLineCtx[0];

			pDevObj->pLine[1]->pDev		= pDevObj;
			pDevObj->pLine[1]->pLineObj	= &VpLineObj[1];
			pDevObj->pLine[1]->pLineCtx	= &VpLineCtx[1];

			break;

		default:
			pDevObj->max_line			= 0;
			printk("Error: unknown line type.\n");
			return pDevObj->max_line;
			break;
	}

	if (VP_DEV_890_SERIES == vpDevType) {

		/* Decide DEV profile */
		devicePcn   = Ve890GetRev(pDevObj);
		pDevProfile = Ve890GetDevProvile(devicePcn);
		
		pDevObj->VpDevType			= vpDevType;
		pDevObj->pDev_profile		= pDevProfile;
		pDevObj->pAC_profile		= DEF_LE890_AC_PROFILE;
		pDevObj->pDC_profile		= DEF_LE890_DC_PROFILE;
		pDevObj->pRing_profile		= DEF_LE890_RING_PROFILE;
		pDevObj->pACFxoLC_profile 	= DEF_LE890_AC_FXO_LC_PROFILE;
		pDevObj->pFxoDial_profile 	= DEF_LE890_FXO_DIALING_PROFILE;

		pDevObj->pRing_cad_usr_profile 	= LE890_RING_CAD_USER_DEF;
	}

	/* FXS */
	pDevObj->SetRingCadence 		= zarlinkSetRingCadence;
	pDevObj->SetImpedenceCountry 	= Ve890SetImpedenceCountry;
	pDevObj->GetFxsHookStatus 		= zarlinkGetFxsHookStatus;
	pDevObj->SetPcmTxOnly 			= zarlinkSetPcmTxOnly;
	pDevObj->FxsRing 				= zarlinkFxsRing;
	pDevObj->SetFxsPcmMode 			= zarlinkSetFxsPcmMode;
	pDevObj->SetFxsAcProfileByBand  = Ve890SetFxsAcProfileByBand;
	pDevObj->SetRingCadenceProfile 	= Ve890SetRingCadenceProfile;
	pDevObj->SendNTTCAR 			= zarlinkSendNTTCAR;
	pDevObj->SendNTTCAR_Check 		= zarlinkSendNTTCAR_Check;
	pDevObj->GetFxsHookStatus 		= zarlinkGetFxsHookStatus;
	pDevObj->GetLineState 			= zarlinkGetLineState;
	pDevObj->SetLineState 			= zarlinkSetLineState;
	pDevObj->SetOHT 				= zarlinkSetOHT;
	pDevObj->SetLineOpen 			= zarlinkSetLineOpen;
	pDevObj->SetPcmRxOnly 			= zarlinkSetPcmRxOnly;
	pDevObj->CheckFxsRing 		    = zarlinkCheckFxsRing;
	pDevObj->TxGainAdjust 	    	= zarlinkTxGainAdjust;
	pDevObj->RxGainAdjust 	   	 	= zarlinkRxGainAdjust;

	/* FXO */
	pDevObj->GetFxoLineStatus 		= zarlinkGetFxoLineStatus;
	pDevObj->GetFxoHookStatus 		= zarlinkGetFxoHookStatus;
	pDevObj->SetFxoLineState 		= zarlinkSetFxoLineState;
	pDevObj->SetFxoLineOnHook 		= zarlinkSetFxoLineOnHook;
	pDevObj->SetFxoLineOffHook 		= zarlinkSetFxoLineOffHook;
	pDevObj->SetFxoLineOHT 			= zarlinkSetFxoLineOHT;
	pDevObj->SetFxoRingValidation 	= zarlinkSetFxoRingValidation;

	/* GPIO */
	pDevObj->SetIODir	 		    = Ve890SetIODir;
	pDevObj->SetIOState 		    = Ve890SetIOState;
	pDevObj->GetIOState 		    = Ve890GetIOState;
	pDevObj->UpdIOState 		    = Ve890UpdIOState;

	/* Debug */
	pDevObj->DumpDevReg 		    = zarlinkDumpDevReg;
	pDevObj->RWDevReg 		        = zarlinkRWDevReg;

	#if 0
	pDevObj->SetRingFreqAmp 		= zarlinkSetRingFreqAmp;
	#endif

	/* Register for event handler */
	zarlinkRegDevForEvHandle(pDevObj);

	return pDevObj->max_line;
}

/* 
** API	  : Ve890CreateLineObj()
** Desp	  :	Create Realtek line obj
** input  : ch_id 	    : unique ch_id 
**          channelId   : channel id in each device
**	        RTKLineType	: Realtek line type
**	        RTKDevObj   : point to its RTK dev obj
**	        RTKLineObj  : point to line obj
**	        slot        : tx/rx time slot
** return : SUCCESS/FAILED
*/
BOOL Ve890CreateLineObj(
	int ch_id, 
	int channelId, 	/* line# within a slic. usually 0 or 1 */
	RTKLineType	line_type,
	RTKLineObj *pLine,
	int law,
	unsigned int slot )
{
	/* channel 0 is FXS */
	pLine->ch_id 			= ch_id;
	pLine->channelId		= channelId;
	pLine->hook_st 			= 0;
	pLine->line_st 			= DEV_S_NOT_INIT;
	pLine->slot_tx 			= slot;
	pLine->slot_rx 			= slot;
	pLine->line_type		= line_type;
	pLine->pcm_law_save 	= law;
	pLine->codec_type 		= getCodecType(law);

	if (line_type == LINE_FXS) 		pLine->pAC_profile    = DEF_LE890_AC_PROFILE;
	else if (line_type == LINE_FXO) pLine->pAC_profile	  = DEF_LE890_AC_FXO_LC_PROFILE;
	else return FAILED;

	if (line_type == LINE_FXS) 		pLine->pDCfxo_profile = DEF_LE890_DC_PROFILE;
	else if (line_type == LINE_FXO) pLine->pDCfxo_profile = DEF_LE890_FXO_DIALING_PROFILE;
	else return FAILED;

	pLine->pRing_profile	= DEF_LE890_RING_PROFILE;
	pLine->pRing_cad_profile= DEF_LE890_RING_CAD_PROFILE;
	
	pLine->TxGainAdj = 0; 	/* Adjust 0dB */
	pLine->RxGainAdj = 0; 	/* Adjust 0dB */

	PRINT_MSG("DEV%x:CH%d is %s\n", pLine->pDev->dev_id, 
		ch_id, line_type == LINE_FXS?"FXS":"FXO");


	return SUCCESS;
}	

/* 
** API	  : Ve890GetRev()
** Desp	  :	Get 890 series dev revision
** input  : Realtek device obj pointer
** return : pcn
*/
int Ve890GetRev(RTKDevObj *pDev)
{
	unsigned char res[VP890_RCN_PCN_LEN]={0};
	unsigned int devicePcn;
    bool runHvCheck = TRUE;

   	
    VpMpiCmdWrapper(pDev->dev_id, VP890_EC_CH1, VP890_RCN_PCN_RD, 
						VP890_RCN_PCN_LEN, res);

	/* Simple check to see if we're even talking to the device */
	if (((res[0] == 0x00) && (res[1] == 0x00)) || 
	    ((res[0] == 0xFF) && (res[1] == 0xFF))) {
	  	return FAILED;
	}

   	PRINT_MSG("deviceId: %x, Revision: %x:%x\n",pDev->dev_id ,res[0],res[1]);
	devicePcn = (unsigned int)res[VP890_PCN_LOCATION];
    /*
     * Is the PCN one that we recognize as 890 (note: 89010 has same PCN as the
     * 89316, but the enumeration is different and invalid from the device)
     */
    if ((devicePcn >= VP890_LAST_PCN) || (devicePcn == VP890_DEV_PCN_89010)) {
		PRINT_R("Error: pcn 0x%02x\n",devicePcn);
        return FAILED;
    }

    /* Check for FXO line Types */
    if (devicePcn == VP890_DEV_PCN_89316) {
        uint8 check89010[VP890_FUSE5_REG_LEN];

        VpMpiCmdWrapper(pDev->dev_id, VP890_EC_CH1, 
			VP890_FUSE5_REG_RD, VP890_FUSE5_REG_LEN, check89010);

       /*
        * Normally, the fuse register is a trim value of vref and never 1. It's
        * only 1 if the device is being "marked" as FXO only.
        */
        if (check89010[0] == 0x01) {
            res[VP890_PCN_LOCATION] =  VP890_DEV_PCN_89010;
            /* 
 	 	     * pDevObj->stateInt |= (VP890_LINE1_IS_FXO | VP890_IS_FXO_ONLY);
             * pDevObj->staticInfo.maxChannels = 2;
  	         */
            runHvCheck = FALSE;
			/* PRINT_R("dual channel 1 0x%02x\n",devicePcn); */
        } else {
			/*	
             * pDevObj->stateInt |= VP890_LINE1_IS_FXO | VP890_WIDEBAND;
             * pDevObj->staticInfo.maxChannels = 2;
   			 */
			/* PRINT_R("dual channel 2 0x%02x\n",devicePcn); */
        }
    } else {
		/*
         * pDevObj->stateInt |= VP890_WIDEBAND | VP890_IS_SINGLE_CHANNEL;
         * pDevObj->staticInfo.maxChannels = 1;
   		 * PRINT_R("single channel 0x%02x\n",devicePcn);
 		 */
    }

    if (runHvCheck == TRUE) {
        uint8 checkHv[VP890_FUSE1_REG_LEN];
        uint8 ecTestValue = (VP890_EC_TEST_MODE_EN | VP890_EC_CH1);
        uint8 testReg1Data[VP890_TEST_REG1_LEN] = {VP890_TEST_REG1_FUSE_TEST};

        VpMpiCmdWrapper(pDev->dev_id, ecTestValue, 
			VP890_TEST_REG1_WRT, VP890_TEST_REG1_LEN, testReg1Data);

        VpMpiCmdWrapper(pDev->dev_id, ecTestValue, 
			VP890_FUSE1_REG_RD, VP890_FUSE1_REG_LEN,
            checkHv);

        /*
         * A non-blown fuse reads as '1'. So if it is blown (reads = '0') it is
         * marked as a HV device.
         */
        if (!(checkHv[0] & VP890_FUSE1_REG_ILAF_TRIM)) {
            /* pDevObj->stateInt |= VP890_IS_HIGH_VOLTAGE; */
            if (res[VP890_PCN_LOCATION] == VP890_DEV_PCN_89116) {
                res[VP890_PCN_LOCATION] = VP890_DEV_PCN_89136;
            } else if (res[VP890_PCN_LOCATION] == VP890_DEV_PCN_89316) {
                res[VP890_PCN_LOCATION] = VP890_DEV_PCN_89336;
            }
        }
        checkHv[0] = 0x00;
        VpMpiCmdWrapper(pDev->dev_id, VP890_EC_CH1, 
			VP890_TEST_REG1_WRT, VP890_TEST_REG1_LEN, checkHv);
    }

	devicePcn = (unsigned int)res[VP890_PCN_LOCATION];

	switch(devicePcn) {
		case VP890_DEV_PCN_89116: 	/**< FXS - Wideband */
			PRINT_MSG("VP890_DEV_PCN_89116\n");
			break;
		case VP890_DEV_PCN_89316: 	/**< FXO/FXS-Tracker - Wideband */
			PRINT_MSG("VP890_DEV_PCN_89316\n");
			break;
		#if 1 /* We need to do more test to recognize 89010 */
		case VP890_DEV_PCN_89010: 	/**< Single Channel FXO */
			PRINT_MSG("VP890_DEV_PCN_89010\n");
			break;
		/*
  		 * HV devices have the same silicon PCN has their LV equivalent. The SW has
  		 * to determine if HV/LV and set the PCN value returned by GetDeviceInfo().
  		 */
		#endif
		case VP890_DEV_PCN_89136: 	/**< HV FXS - Wideband */
			PRINT_MSG("VP890_DEV_PCN_89136\n");
			break;
		case VP890_DEV_PCN_89336:	/**< FXO/FXS-Tracker - Wideband */
			PRINT_MSG("VP890_DEV_PCN_89336\n");
			break;
		default:
			PRINT_MSG("unknown!\n");
			return FAILED;
	}
	return devicePcn;
}


/* 
** API	  : Ve890GetDevProfile()
** Desp	  :	Get 890 series dev profile. 
            HV and DV should use different DEV profile.
** input  : devicePcn
** return : dev profile
*/
VpProfilePtrType Ve890GetDevProvile(unsigned int devicePcn)
{
	VpProfilePtrType pDevProfile;

	switch (devicePcn) {
		case VP890_DEV_PCN_89136:
		case VP890_DEV_PCN_89336:
			#ifdef DEBUG_INT
			printk("%s(%d)DEF_LE890_DEV_PROFILE_HV\n",__FUNCTION__,__LINE__);
			#endif
			pDevProfile = DEF_LE890_DEV_PROFILE_HV;
			break;	
		
		case VP890_DEV_PCN_89116:
		case VP890_DEV_PCN_89316:
		case VP890_DEV_PCN_89010:
		default:
			#ifdef DEBUG_INT
			printk("%s(%d)DEF_LE890_DEV_PROFILE_LV\n",__FUNCTION__,__LINE__);
			#endif
			pDevProfile = DEF_LE890_DEV_PROFILE_LV;
			break;
	}

	return pDevProfile;
}


/*********** End of File ***************/

