//============================================================
// File Name: Hal8188EDMOutSrc.h
//
// Description:
//
// This file is for 92CE/92CU outsource dynamic mechanism for partner.
//
//
//============================================================
#ifndef	__HAL8812DMOUTSRC_H__
#define __HAL8812DMOUTSRC_H__

//============================================================
// function prototype
//============================================================

void 
odm_1R_CCA_8812(
	IN	PADAPTER	pAdapter
);



VOID
odm_TXPowerTrackingCallback_ThermalMeter_8812(
            IN PADAPTER	Adapter
);

VOID
odm_PSD_Monitor_8812(
	PADAPTER	Adapter
);


VOID
odm_PSDMonitor_8812(
	PADAPTER	Adapter
    );


VOID
ODM_DMWatchdog_8812(
			IN PADAPTER	Adapter
	);


#endif
