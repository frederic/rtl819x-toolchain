//============================================================
// File Name: Hal8192CDMOutSrc.h
//
// Description:
//
// This file is for 92CE/92CU outsource dynamic mechanism for partner.
//
//
//============================================================
#ifndef	__HAL8192CDMOUTSRC_H__
#define __HAL8192CDMOUTSRC_H__


//
// 20112/01/31 MH HW team will use ODM module to do all dynamic scheme.
//
#if 0

//============================================================
//3 EDCA
//============================================================

VOID
odm_DynamicEDCCA(
	IN	PADAPTER	Adapter
);

//===========================================//
// Neil Chen----2011--06--15--

//3 Path Diversity
//===========================================================


VOID	odm_PathDiversityInit_92C(	IN	PADAPTER	Adapter);


#define dm_PathDivCallback	ODM_PathDivChkAntSwitchCallback
VOID ODM_PathDivChkAntSwitchCallback(PRT_TIMER		pTimer);
VOID odm_PathDivChkAntSwitch(PADAPTER	Adapter,u1Byte	Step);
VOID ODM_PathDivRestAfterLink(
	IN	PADAPTER	Adapter
	);

#define dm_PathDiv_RSSI_Check	ODM_PathDivChkPerPktRssi
VOID ODM_PathDivChkPerPktRssi(PADAPTER		Adapter,
										BOOLEAN			bIsDefPort,
										BOOLEAN			bMatchBSSID,
										PRT_WLAN_STA	pEntry,
										PRT_RFD			pRfd	);

VOID
ODM_PathDivChkAntSwitchWorkitemCallback(
    IN PVOID            pContext
    );

//============================================================
// function prototype
//============================================================


void 
odm_1R_CCA_8192C(
	IN	PADAPTER	pAdapter
);

VOID
odm_TXPowerTrackingCallback_ThermalMeter_92C(
            IN PADAPTER	Adapter
);

VOID
odm_TXPowerTrackingCallback_ThermalMeter_92D(
            IN PADAPTER	Adapter
);

#define	DM_Write_RXHP		ODM_Write_RXHP
void ODM_Write_RXHP(IN	PADAPTER	Adapter);

VOID
ODM_Write_CCK_CCA_Thres(
	IN	PADAPTER	pAdapter
);


void
ODM_RF_Saving_8192C(
			IN	PADAPTER	pAdapter,
			IN	u1Byte	bForceInNormal
);

VOID
odm_PSD_Monitor_8192C(
	PADAPTER	Adapter
);

VOID
odm_PSDMonitor_8192C(
	PADAPTER	Adapter
);

VOID
ODM_DMWatchdog_8192C(
			IN PADAPTER	Adapter
);


#define PathDivCheckBeforeLink8192C	ODM_PathDiversityBeforeLink92C

BOOLEAN
ODM_PathDiversityBeforeLink92C(
	IN	PADAPTER	Adapter
);

#define dm_CCKTXPathDiversityCallback	ODM_CCKTXPathDiversityCallback
VOID	
ODM_CCKTXPathDiversityCallback(
	PRT_TIMER		pTimer
);

VOID
ODM_CCKPathDiversityChkPerPktRssi(
	PADAPTER		Adapter,
	BOOLEAN			bIsDefPort,
	BOOLEAN			bMatchBSSID,
	PRT_WLAN_STA	pEntry,
	PRT_RFD			pRfd,
	pu1Byte			pDesc
);

VOID
ODM_FillTXPathInTXDESC(
		IN	PADAPTER	Adapter,
		IN	PRT_TCB		pTcb,
		IN	pu1Byte		pDesc
);

BOOLEAN
ODM_SingleDualAntennaDetection(
	IN	PADAPTER	Adapter
	);

VOID
_PHY_SaveAFERegisters(
	IN	PADAPTER	pAdapter,
	IN	pu4Byte		AFEReg,
	IN	pu4Byte		AFEBackup,
	IN	u4Byte		RegisterNum
	);

VOID
_PHY_ReloadAFERegisters(
	IN	PADAPTER	pAdapter,
	IN	pu4Byte		AFEReg,
	IN	pu4Byte		AFEBackup,
	IN	u4Byte		RegisterNum
	);

#if(DEV_BUS_TYPE == RT_PCI_INTERFACE)||(DEV_BUS_TYPE == RT_USB_INTERFACE)

void	odm_RXHPInit(
	IN	PADAPTER	pAdapter
);
#endif

#endif

#endif
