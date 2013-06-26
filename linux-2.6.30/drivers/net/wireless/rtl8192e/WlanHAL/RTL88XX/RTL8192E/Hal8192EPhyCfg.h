#ifndef __HAL8192EPHYCFG_H__
#define __HAL8192EPHYCFG_H__

/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	Hal8192EPhyCfg.h
	
Abstract:
	Defined HAL 92E PHY BB setting functions
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2012-11-22 Eric              Create.	
--*/

void PHYSetOFDMTxPower8192E(
        IN  HAL_PADAPTER    Adapter, 
        IN  u1Byte          channel
);

void PHYSetCCKTxPower8192E(
        IN  HAL_PADAPTER    Adapter, 
        IN  u1Byte          channel
);



#endif //__HAL8192EPHYCFG_H__

