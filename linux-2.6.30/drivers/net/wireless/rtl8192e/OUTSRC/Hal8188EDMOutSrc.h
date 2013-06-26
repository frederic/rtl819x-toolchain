//============================================================
// File Name: Hal8188EDMOutSrc.h
//
// Description:
//
// This file is for 92CE/92CU outsource dynamic mechanism for partner.
//
//
//============================================================
#ifndef	__HAL8188EDMOUTSRC_H__
#define __HAL8188EDMOUTSRC_H__

//
// 20112/01/31 MH HW team will use ODM module to do all dynamic scheme.
//
#if 0

//============================================================
// function prototype
//============================================================

void 
odm_1R_CCA_8188E(
	IN	PADAPTER	pAdapter
);


void
ODM_RF_Saving_8188E(	
			IN 	PDM_ODM_T	pDM_Odm,
			IN	u1Byte	bForceInNormal 
);

VOID
odm_TXPowerTrackingCallback_ThermalMeter_8188E(
            IN PADAPTER	Adapter
);

VOID
odm_PSD_Monitor_8188E(
	PADAPTER	Adapter
);


VOID
odm_PSDMonitor_8188E(
	PADAPTER	Adapter
    );


VOID
ODM_DMWatchdog_8188E(
			IN PADAPTER	Adapter
	);


#endif

#endif
