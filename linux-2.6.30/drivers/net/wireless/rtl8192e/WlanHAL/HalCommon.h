#ifndef __HALCOMMON_H__
#define __HALCOMMON_H__

/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	HalCommon.h
	
Abstract:
	Defined HAL Common
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2012-05-18  Lun-Wu            Create.	
--*/


RT_STATUS 
HAL_ReadTypeID(
	INPUT	HAL_PADAPTER	Adapter
);

RT_STATUS
HalAssociateNic(
    HAL_PADAPTER        Adapter,
    BOOLEAN			IsDefaultAdapter    
);

#if 0   //Filen: It is un-useful
RT_STATUS
HalDisAssociateNic(
    HAL_PADAPTER        Adapter,
    BOOLEAN			    IsDefaultAdapter    
);
#endif

PVOID 
SoftwareCRC32 (
    IN  pu1Byte     pBuf,
    IN  u2Byte      byteNum,
    OUT pu4Byte     pCRC32
);

PVOID 
SoftwareCRC32_RXBuffGather (
    IN  pu1Byte     pPktBufAddr,
    IN  pu2Byte     pPktBufLen,  
    IN  u2Byte      pktNum,
    OUT pu4Byte     pCRC32
);


#endif // __HALCOMMON_H__

