#ifndef __HAL88XX_FIRMWARE_H__
#define __HAL88XX_FIRMWARE_H__

/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	Hal88XXFirmware.h
	
Abstract:
	Defined HAL 88XX Firmware data structure & Define
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2012-04-11 Filen            Create.	
--*/

typedef struct _RTL88XX_FW_HDR_ 
{
    u2Byte      signature;
    u1Byte       category;
    u1Byte       function;

    u2Byte      version;
    u1Byte       subversion;
    u1Byte       rsvd1;

    u1Byte       month;      //human easy reading format
    u1Byte       day;        //human easy reading format
    u1Byte       hour;       //human easy reading format
    u1Byte       minute;     //human easy reading format

    u2Byte      ram_code_size;
    u1Byte       Foundry;  //0: TSMC,  1:UMC, 2:SMIC
    u1Byte       rsvd3;
    u4Byte        svnidx;
    u4Byte        rsvd5;
    u4Byte        rsvd6;
    u4Byte        rsvd7;
    
}RTL88XX_FW_HDR, *PRTL88XX_FW_HDR;

// TODO: Filen, check below
typedef enum _RTL88XX_H2C_CMD 
{
//	H2C_88XX_RSVDPAGE               = 0,
	H2C_88XX_MSRRPT             = 1,	
//	H2C_88XX_KEEP_ALIVE_CTRL    = 3,
//	H2C_88XX_WO_WLAN            = 5,	// Wake on Wlan.
//	H2C_88XX_REMOTE_WAKEUP      = 7, 
//	H2C_88XX_AP_OFFLOAD         = 8,
//	H2C_88XX_SETPWRMODE         = 0x20,		
//	H2C_88XX_P2P_PS_MODE        = 0x24,
	H2C_88XX_RA_MASK            = 0x40,
	H2C_88XX_RSSI_REPORT        = 0x42,
	H2C_88XX_AP_REQ_TXREP		= 0x43,
	MAX_88XX_H2CCMD
}RTL88XX_H2C_CMD;




RT_STATUS
InitFirmware88XX(
    IN  HAL_PADAPTER    Adapter
);

#if 0
typedef struct _H2C_CONTENT_
{
    u4Byte  content;
    u2Byte  ext_content;
}H2C_CONTENT, *PH2C_CONTENT;



BOOLEAN
IsH2CBufOccupy88XX(
    IN  HAL_PADAPTER    Adapter
);


BOOLEAN
SigninH2C88XX(
    IN  HAL_PADAPTER    Adapter,
    IN  PH2C_CONTENT    pH2CContent
);
#else
BOOLEAN
CheckFwReadLastH2C88XX(
	IN  HAL_PADAPTER    Adapter,
	IN  u1Byte          BoxNum
);

RT_STATUS
FillH2CCmd88XX(
	IN  HAL_PADAPTER    Adapter,
	IN	u1Byte 		    ElementID,
	IN	u4Byte 		    CmdLen,
	IN	pu1Byte		    pCmdBuffer
);
#endif



VOID
UpdateHalRAMask88XX(
	IN HAL_PADAPTER         Adapter,	
	HAL_PSTAINFO            pEntry,
	u1Byte				    rssi_level
);

void
UpdateHalMSRRPT88XX(
	IN HAL_PADAPTER     Adapter,
	u2Byte              aid,
	u1Byte              opmode
);



#endif  //__HAL88XX_FIRMWARE_H__

