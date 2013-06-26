#ifndef __HAL88XX_DEF_H__
#define __HAL88XX_DEF_H__

/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	Hal88XXDef.h
	
Abstract:
	Defined HAL 88XX common data structure & Define
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2012-03-23 Filen            Create.	
--*/

#ifdef  WLAN_HAL_INTERNAL_USED

MIMO_TR_STATUS
GetChipIDMIMO88XX(
    IN  HAL_PADAPTER        Adapter
);


VOID
CAMEmptyEntry88XX(
    IN  HAL_PADAPTER    Adapter,
    IN  u1Byte          index
);


u4Byte
CAMFindUsable88XX(
    IN  HAL_PADAPTER    Adapter,
    IN  u4Byte          for_begin
);


VOID
CAMReadMACConfig88XX
(
    IN  HAL_PADAPTER    Adapter,
    IN  u1Byte          index, 
    OUT pu1Byte         pMacad,
    OUT PCAM_ENTRY_CFG  pCfg
);


VOID
CAMProgramEntry88XX(
    IN	HAL_PADAPTER		Adapter,
    IN  u1Byte              index,
    IN  pu1Byte             macad,
    IN  pu1Byte             key128,
    IN  u2Byte              config
);


VOID
SetHwReg88XX(
    IN	HAL_PADAPTER		Adapter,
    IN	u1Byte				variable,
    IN	pu1Byte				val
);


VOID
GetHwReg88XX(
    IN      HAL_PADAPTER    Adapter,
    IN      u1Byte          variable,
    OUT     pu1Byte         val
);

RT_STATUS
InitMAC88XX(
    IN  HAL_PADAPTER Adapter
);

VOID
InitIMR88XX(
    IN  HAL_PADAPTER    Adapter,
    IN  RT_OP_MODE      OPMode
);

VOID
InitVAPIMR88XX(
    IN  HAL_PADAPTER    Adapter,
    IN  u4Byte          VapSeq
);


RT_STATUS      
InitHCIDMAMem88XX(
    IN      HAL_PADAPTER    Adapter
);  

RT_STATUS
InitHCIDMAReg88XX(
    IN      HAL_PADAPTER    Adapter
);  

VOID
StopHCIDMASW88XX(
    IN  HAL_PADAPTER Adapter
);

VOID
StopHCIDMAHW88XX(
    IN  HAL_PADAPTER Adapter
);

#if CFG_HAL_SUPPORT_MBSSID
VOID
InitMBSSID88XX(
    IN  HAL_PADAPTER Adapter
);

VOID
StopMBSSID88XX(
    IN  HAL_PADAPTER Adapter
);
#endif  //CFG_HAL_SUPPORT_MBSSID

RT_STATUS
StopHW88XX(
    IN  HAL_PADAPTER Adapter
);

RT_STATUS
StopSW88XX(
    IN  HAL_PADAPTER Adapter
);

VOID
DisableVXDAP88XX(
    IN  HAL_PADAPTER Adapter
);



#endif  //WLAN_HAL_INTERNAL_USED

#endif  //__HAL88XX_DEF_H__
