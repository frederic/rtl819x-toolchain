#ifndef __HAL8192EE_DEF_H__
#define __HAL8192EE_DEF_H__

/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	Hal8192EEDef.h
	
Abstract:
	Defined HAL 8192EE data structure & Define
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2012-03-23 Filen            Create.	
--*/


RT_STATUS
InitPON8192EE(
    IN  HAL_PADAPTER Adapter
);

RT_STATUS
StopHW8192EE(
    IN  HAL_PADAPTER Adapter
);


RT_STATUS	
hal_Associate_8192EE(
    HAL_PADAPTER            Adapter,
    BOOLEAN			    IsDefaultAdapter
);
















#endif  //__HAL8192EE_DEF_H__

