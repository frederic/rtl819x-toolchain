#ifndef __INC_RA_H
#define __INC_RA_H
/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	RateAdaptive.h
	
Abstract:
	Prototype of RA and related data structure.
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2011-08-12 Page            Create.	
--*/

// Rate adaptive define
#define	PERENTRY	23
#define	RETRYSIZE	5
#define	RATESIZE	28
#define	TX_RPT2_ITEM_SIZE 	8

// End rate adaptive define

VOID
ODM_RASupport_Init(
	IN	PDM_ODM_T	pDM_Odm
	);

int 
ODM_RAInfo_Init_all(
	IN    PDM_ODM_T		pDM_Odm
	);

int 
ODM_RAInfo_Init(
	IN 	PDM_ODM_T 	pDM_Odm,
	IN 	u1Byte 		MacID	
	);

u1Byte 
ODM_RA_GetShortGI_8188E(
	IN 	PDM_ODM_T 	pDM_Odm, 
	IN 	u1Byte 		MacID
	);

u1Byte 
ODM_RA_GetDecisionRate_8188E(
	IN 	PDM_ODM_T 	pDM_Odm, 
	IN 	u1Byte 		MacID
	);

u1Byte
ODM_RA_GetHwPwrStatus_8188E(
	IN 	PDM_ODM_T 	pDM_Odm, 
	IN 	u1Byte 		MacID
	);
VOID 
ODM_RA_UpdateRateInfo_8188E(
	IN PDM_ODM_T pDM_Odm,
	IN u1Byte MacID,
	IN u1Byte RateID, 
	IN u4Byte RateMask,
	IN u1Byte SGIEnable
	);

VOID 
ODM_RA_SetRSSI_8188E(
	IN 	PDM_ODM_T 		pDM_Odm, 
	IN 	u1Byte 			MacID, 
	IN 	u1Byte 			Rssi
	);

VOID
ODM_RA_TxRPT2Handle_8188E(	
	IN	PDM_ODM_T		pDM_Odm,
	IN	pu1Byte			TxRPT_Buf,
	IN	u2Byte			TxRPT_Len,
	IN	u4Byte			MacIDValidEntry0,
	IN	u4Byte			MacIDValidEntry1
	);
	

VOID 
ODM_RA_Set_TxRPT_Time(
	IN	PDM_ODM_T		pDM_Odm,
	IN	u2Byte 			minRptTime
	);


#endif
