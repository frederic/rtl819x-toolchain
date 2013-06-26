/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
 *                                        
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/
#ifndef	__ODM_RTL8192D_H__
#define __ODM_RTL8192D_H__


//#if (DM_ODM_SUPPORT_TYPE == ODM_MP)
VOID
ODM_Write_DIG_DMSP(
	IN	PDM_ODM_T		pDM_Odm,
	IN	u1Byte			CurrentIGI
	);

VOID
ODM_Write_CCK_CCA_Thres_92D(
	IN	PDM_ODM_T		pDM_Odm,
	IN	u1Byte			CurCCK_CCAThres
	);

VOID
ODM_Write_CCK_CCA_DMSP(
	IN	PDM_ODM_T		pDM_Odm,
	IN	u1Byte			CurCCK_CCAThres
	);

VOID
odm_GetCCKFalseAlarm_92D(
	IN		PDM_ODM_T		pDM_Odm
	);

VOID
odm_ResetFACounter_92D(
	IN		PDM_ODM_T		pDM_Odm
	);

VOID 
odm_FalseAlarmCounterStatistics_ForSlaveOfDMSP(
	IN		PDM_ODM_T		pDM_Odm
	);

VOID 
ODM_DynamicEarlyMode(
	IN		PDM_ODM_T		pDM_Odm
	);
//#endif


#if (DM_ODM_SUPPORT_TYPE == ODM_MP)
VOID
odm_TXPowerTrackingCallback_ThermalMeter_92D(
	IN PADAPTER	Adapter
	);

#endif

#endif

