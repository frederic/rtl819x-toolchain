/** \file ve880_ext_int.c
 * 
 *
 * This file contains the external function for New Arch
 * 
 *
 * Copyright (c) 2010, Realtek Semiconductor, Inc.
 *
 */
#include <linux/timer.h>
#include "spi.h"			/* for init_spi() */
#include "zarlinkCommonInit.h"  
#include "ve880_int.h"
#include "vp880_api_int.h"  /* For VP880_DEV_PCN_88221 device type */

#undef DEBUG_INT 1

int gDevId=0;

/*
** Realtek memory space that API-2 requires 
** for the Device/Line contexts and objects
*/

static RTKDevObj	  	  		DevObj[ZARLINK_SLIC_DEV_NUM]; //
static RTKLineObj	  	  		LineObj[ZARLINK_SLIC_CH_NUM]; //FXS+FXO line number

/*
** Application memory space that API-2 requires 
** for the Device/Line contexts and objects
*/
static Vp880DeviceObjectType	VpDevObj [ZARLINK_SLIC_DEV_NUM];
static Vp880LineObjectType  	VpLineObj[ZARLINK_SLIC_CH_NUM];
static VpDevCtxType         	VpDevCtx [ZARLINK_SLIC_DEV_NUM];
static VpLineCtxType        	VpLineCtx[ZARLINK_SLIC_CH_NUM];

/* 
** API	  : Ve880Init()
** Desp	  :	Init Realtek dev and line obj 
**          Init Zarlink dev and line obj
** input  : pcm_mode
** return : SUCCESS/FAILED
*/
int Ve880Init(int pcm_mode)
{
   	int rtn;
	int i;
	int PcnType;
	int DevNum;
	int deviceId;
	int DevType;
	int max_line;
	int chID;
	RTKDevObj *pDev;

	/* ******************* */
	/* Init Realtek obj    */
	/* ******************* */

	chID = rtkGetNewChID();

	memset(DevObj,  0, sizeof(DevObj));
	memset(LineObj, 0, sizeof(LineObj));

	/* TODO should based on dev version to decided device type */
	for (DevType=DEV_FXS; DevType<DEV_LAST; DevType++) {
		/* Get dev# of each devce type */
 		DevNum = zarlinkCaculateDevObj(DevType);

		for (i=0;i<DevNum;i++) {
			/* Create Realtek/Zarlink dev obj */

			max_line = Ve880CreateDevObj(
				/* Realtek */
				DevType,
				chID,				/* First global ch_id for the device */
				&DevObj[gDevId],
				&LineObj[chID], 

				/* Zarlink */
				gDevId,
				VP_DEV_880_SERIES,	/* vincent TODO decided slic type */
				&VpDevObj[gDevId],
				&VpDevCtx[gDevId],
				&VpLineObj[chID],
				&VpLineCtx[chID]); 

			pDev = &DevObj[gDevId];

			if (DevType == DEV_FXS || DevType == DEV_FXSFXO || DevType == DEV_FXSFXS) {
				/* FXS 1st line */
				Ve880CreateLineObj( chID,  0, LINE_FXS, pDev->pLine[0], pcm_mode, chID);
			} else if (DevType == DEV_FXO) {
				/* FXO 1st line */
				Ve880CreateLineObj( chID,  0, LINE_FXO, pDev->pLine[0], pcm_mode, chID);
			}

			if (DevType == DEV_FXSFXS) {
				/* FXS 2nd line */
				chID = rtkGetNewChID();
				Ve880CreateLineObj( chID, 1, LINE_FXS, pDev->pLine[1], pcm_mode, chID);
			} else if (DevType == DEV_FXSFXO) {
				/* FXO 2nd line */
				chID = rtkGetNewChID();
				Ve880CreateLineObj( chID, 1, LINE_FXO, pDev->pLine[1], pcm_mode, chID);
			}

			gDevId++;
		}
	}
	
#ifdef DEBUG_INT
	printk("%s() gDevId=%d,chID=%d\n",__FUNCTION__,gDevId,chID);
#endif

	/* ******************* */
	/* Init Zarlink API-II */
	/* ******************* */

	PRINT_MSG("================================================\n");
	PRINT_MSG("Zarlink API-II Lite Version %d.%d.%d\n", 
				VP_API_VERSION_MAJOR_NUM, 
				VP_API_VERSION_MINOR_NUM, 
				VP_API_VERSION_MINI_NUM);

	/* based on CONFIG_RTK_VOIP_SLIC_NUM */
	for (deviceId = 0; deviceId < gDevId; deviceId++) {
		/* deviceId imply spi device id */
		init_spi(deviceId);

		pDev = &DevObj[deviceId];
		PcnType = Ve880GetRev( pDev );

		if ( PcnType == FAILED ) {
			PRINT_R("Error (%d:x) %s Read version fail\n", deviceId, __FUNCTION__);
			return FAILED;
		}

		/* Initialize API-2 device settings */
		rtn = zarlinkInitDevice( pDev );
		if ( rtn == FAILED ) {
			PRINT_R("Error (%d:x) %s\n", deviceId, __FUNCTION__);
			return FAILED;
		}
	}
	
	PRINT_MSG("%s() Total %d devices, %d lines are initinized\n",__FUNCTION__,deviceId,chID );

	return SUCCESS;
}

