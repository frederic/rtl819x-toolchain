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

#ifndef __HAL_PHY_RF_8188E_H__
#define __HAL_PHY_RF_8188E_H__

/*--------------------------Define Parameters-------------------------------*/
#define	IQK_DELAY_TIME_88E		10		//ms
#define	index_mapping_NUM_88E	15
#define AVG_THERMAL_NUM_88E	4

typedef enum _PWRTRACK_CONTROL_METHOD {
	BBSWING,
	TXAGC
} PWRTRACK_METHOD;

typedef VOID (*FuncSetPwr)(PDM_ODM_T, PWRTRACK_METHOD, u1Byte, u1Byte);


VOID
ODM_TxPwrTrackAdjust88E(
	PDM_ODM_T	pDM_Odm,
	u1Byte		Type,				// 0 = OFDM, 1 = CCK
	pu1Byte		pDirection,			// 1 = +(increase) 2 = -(decrease)
	pu4Byte		pOutWriteVal		// Tx tracking CCK/OFDM BB swing index adjust
	);

VOID
odm_TXPowerTrackingCallback_ThermalMeter_8188E(
	IN PDM_ODM_T		pDM_Odm
	);

//1 7.	IQK

void	
PHY_IQCalibrate_8188E(	IN PDM_ODM_T	pDM_Odm,
							IN	BOOLEAN 	bReCovery);


//
// LC calibrate
//
void	
PHY_LCCalibrate_8188E(		IN PDM_ODM_T	pDM_Odm);

//
// AP calibrate
//
void	
PHY_APCalibrate_8188E(			IN PDM_ODM_T	pDM_Odm,
							IN 	s1Byte		delta);
void	
PHY_DigitalPredistortion_8188E(		IN	PADAPTER	pAdapter);


VOID
_PHY_SaveADDARegisters(
	IN PDM_ODM_T	pDM_Odm,
	IN	pu4Byte		ADDAReg,
	IN	pu4Byte		ADDABackup,
	IN	u4Byte		RegisterNum
	);

VOID
_PHY_PathADDAOn(
	IN PDM_ODM_T	pDM_Odm,
	IN	pu4Byte		ADDAReg,
	IN	BOOLEAN		isPathAOn,
	IN	BOOLEAN		is2T
	);

VOID
_PHY_MACSettingCalibration(
	IN PDM_ODM_T	pDM_Odm,
	IN	pu4Byte		MACReg,
	IN	pu4Byte		MACBackup	
	);


VOID
_PHY_PathAStandBy(
	IN PDM_ODM_T	pDM_Odm
	);


								
#endif	// #ifndef __HAL_PHY_RF_8188E_H__								
