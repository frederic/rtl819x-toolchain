//============================================================
// File Name: Hal8192CDMOutSrc.h
//
// Description:
//
// This file is for 92CE/92CU outsource dynamic mechanism for partner.
//
//
//============================================================


#ifndef	__HALDMOUTSRC_H__
#define __HALDMOUTSRC_H__

//============================================================
// structure and define
//============================================================

#if 0

typedef struct DM_Format_UI_To_Driver
{
	INT32	Op;										/* Command packet type. */																			
	INT32	Length;									/* Command packet length. */
	INT32	Cmd_Idx;								/* Command index. */																				
	INT32	Argc;									/* Argument counter. */																				
	INT8	Argv[MAX_ARGC][MAX_ARGV];				/* Argument value array. */	
}DCMD_TMSG_T, *PDCMD_TMSG_T;


//3===========================================================
//3 DIG
//3===========================================================

typedef enum tag_Dynamic_Init_Gain_Operation_Type_Definition
{
	DIG_TYPE_THRESH_HIGH	= 0,
	DIG_TYPE_THRESH_LOW	= 1,
	DIG_TYPE_BACKOFF		= 2,
	DIG_TYPE_RX_GAIN_MIN	= 3,
	DIG_TYPE_RX_GAIN_MAX	= 4,
	DIG_TYPE_ENABLE 		= 5,
	DIG_TYPE_DISABLE 		= 6,
	DIG_OP_TYPE_MAX
}DM_DIG_OP_E;

typedef enum tag_CCK_Packet_Detection_Threshold_Type_Definition
{
	CCK_PD_STAGE_LowRssi = 0,
	CCK_PD_STAGE_HighRssi = 1,
	CCK_PD_STAGE_MAX = 3,
}DM_CCK_PDTH_E;

typedef enum tag_DIG_EXT_PORT_ALGO_Definition
{
	DIG_EXT_PORT_STAGE_0 = 0,
	DIG_EXT_PORT_STAGE_1 = 1,
	DIG_EXT_PORT_STAGE_2 = 2,
	DIG_EXT_PORT_STAGE_3 = 3,
	DIG_EXT_PORT_STAGE_MAX = 4,
}DM_DIG_EXT_PORT_ALG_E;

typedef enum tag_DIG_Connect_Definition
{
	DIG_STA_DISCONNECT = 0,	
	DIG_STA_CONNECT = 1,
	DIG_STA_BEFORE_CONNECT = 2,
	DIG_MultiSTA_DISCONNECT = 3,
	DIG_MultiSTA_CONNECT = 4,
	DIG_CONNECT_MAX
}DM_DIG_CONNECT_E;


#define DM_MultiSTA_InitGainChangeNotify(Event) {DM_DigTable.CurMultiSTAConnectState = Event;}

#define DM_MultiSTA_InitGainChangeNotify_CONNECT(_ADAPTER)	\
	DM_MultiSTA_InitGainChangeNotify(DIG_MultiSTA_CONNECT)

#define DM_MultiSTA_InitGainChangeNotify_DISCONNECT(_ADAPTER)	\
	DM_MultiSTA_InitGainChangeNotify(DIG_MultiSTA_DISCONNECT)

#define		DM_DIG_THRESH_HIGH			40
#define		DM_DIG_THRESH_LOW			35

#define		DM_FALSEALARM_THRESH_LOW	400
#define		DM_FALSEALARM_THRESH_HIGH	1000

#define		DM_DIG_MAX				0x3e
#define		DM_DIG_MIN					0x1e //0x22//0x1c

#define		DM_DIG_MAX_HP				0x46
#define		DM_DIG_MIN_HP				0x2e

#define		DM_DIG_FA_UPPER				0x32
#define		DM_DIG_FA_LOWER				0x20

//vivi 92c&92d has different definition, 20110504
//this is for 92c
#define		DM_DIG_FA_TH0				0x200//0x20
#define		DM_DIG_FA_TH1				0x300//0x100
#define		DM_DIG_FA_TH2				0x400//0x200
//this is for 92d
#define		DM_DIG_FA_TH0_92D			0x100
#define		DM_DIG_FA_TH1_92D			0x400
#define		DM_DIG_FA_TH2_92D			0x600

#define		DM_DIG_BACKOFF_MAX			12
#define		DM_DIG_BACKOFF_MIN			-4
#define		DM_DIG_BACKOFF_DEFAULT		10

//3===========================================================
//3 AGC RX High Power Mode
//3===========================================================
#define          LNA_Low_Gain_1                      0x64
#define          LNA_Low_Gain_2                      0x5A
#define          LNA_Low_Gain_3                      0x58

#define          FA_RXHP_TH1                           5000
#define          FA_RXHP_TH2                           1500
#define          FA_RXHP_TH3                             800
#define          FA_RXHP_TH4                             600
#define          FA_RXHP_TH5                             500

//3===========================================================
//3 EDCA
//3===========================================================

//3===========================================================
//3 Dynamic Tx Power
//3===========================================================
//Dynamic Tx Power Control Threshold
#define		TX_POWER_NEAR_FIELD_THRESH_LVL2	74
#define		TX_POWER_NEAR_FIELD_THRESH_LVL1	67

#define		TxHighPwrLevel_Normal		0	
#define		TxHighPwrLevel_Level1		1
#define		TxHighPwrLevel_Level2		2
#define		TxHighPwrLevel_BT1			3
#define		TxHighPwrLevel_BT2			4
#define		TxHighPwrLevel_15			5
#define		TxHighPwrLevel_35			6
#define		TxHighPwrLevel_50			7
#define		TxHighPwrLevel_70			8
#define		TxHighPwrLevel_100			9

//3===========================================================
//3 Tx Power Tracking
//3===========================================================
#if 0 //mask this, since these have been defined in typdef.h, vivi
#define	OFDM_TABLE_SIZE 	37
#define	CCK_TABLE_SIZE		33
#endif	


//3===========================================================
//3 Rate Adaptive
//3===========================================================
#define		DM_RATR_STA_INIT			0
#define		DM_RATR_STA_HIGH			1
#define 		DM_RATR_STA_MIDDLE		2
#define 		DM_RATR_STA_LOW			3

//3===========================================================
//3 BB Power Save
//3===========================================================


typedef enum tag_1R_CCA_Type_Definition
{
	CCA_1R =0,
	CCA_2R = 1,
	CCA_MAX = 2,
}DM_1R_CCA_E;

typedef enum tag_RF_Type_Definition
{
	RF_Save =0,
	RF_Normal = 1,
	RF_MAX = 2,
}DM_RF_E;

//3===========================================================
//3 Antenna Diversity
//3===========================================================
typedef enum tag_SW_Antenna_Switch_Definition
{
	Antenna_B = 1,
	Antenna_A = 2,
	Antenna_MAX = 3,
}DM_SWAS_E;


//
// Extern Global Variables.
//
extern	u4Byte OFDMSwingTable[OFDM_TABLE_SIZE_92D];
extern	u1Byte CCKSwingTable_Ch1_Ch13[CCK_TABLE_SIZE][8];
extern	u1Byte CCKSwingTable_Ch14 [CCK_TABLE_SIZE][8];
	

// 20100514 Joseph: Add definition for antenna switching test after link.
// This indicates two different the steps. 
// In SWAW_STEP_PEAK, driver needs to switch antenna and listen to the signal on the air.
// In SWAW_STEP_DETERMINE, driver just compares the signal captured in SWAW_STEP_PEAK
// with original RSSI to determine if it is necessary to switch antenna.
#define SWAW_STEP_PEAK		0
#define SWAW_STEP_DETERMINE	1

//============================================================
// function prototype
//============================================================
#define DM_ChangeDynamicInitGainThresh		ODM_ChangeDynamicInitGainThresh
void	ODM_ChangeDynamicInitGainThresh(IN	PADAPTER	pAdapter,
											IN	INT32		DM_Type,
											IN	INT32		DM_Value);

#define DM_Write_DIG	ODM_Write_DIG
void ODM_Write_DIG(IN	PADAPTER	pAdapter);

VOID ODM_Write_CCK_CCA_Thres(IN	PADAPTER	pAdapter);

#define DM_InitEdcaTurbo		ODM_InitEdcaTurbo
void ODM_InitEdcaTurbo(	IN PADAPTER	Adapter);

BOOLEAN
ODM_CheckPowerStatus(
	IN	PADAPTER		Adapter);

#define dm_CheckTXPowerTracking	ODM_CheckTXPowerTracking
void	ODM_CheckTXPowerTracking(IN	PADAPTER	Adapter);

#define AP_InitRateAdaptiveState	ODM_ApInitRateAdaptiveState
void ODM_ApInitRateAdaptiveState(IN	PADAPTER	Adapter,	
									IN	PRT_WLAN_STA  pEntry);

#define dm_RF_Saving	ODM_RF_Saving
void ODM_RF_Saving(	IN	PADAPTER	pAdapter,
						IN	u1Byte	bForceInNormal );

#define SwAntDivResetBeforeLink		ODM_SwAntDivResetBeforeLink
VOID ODM_SwAntDivResetBeforeLink(IN	PADAPTER	Adapter);

#define SwAntDivCheckBeforeLink8192C	ODM_SwAntDivCheckBeforeLink8192C
BOOLEAN ODM_SwAntDivCheckBeforeLink8192C(IN	PADAPTER	Adapter);

#define SwAntDivRestAfterLink	ODM_SwAntDivRestAfterLink
VOID ODM_SwAntDivRestAfterLink(	IN	PADAPTER	Adapter	);

#define dm_SW_AntennaSwitchCallback	ODM_SwAntDivChkAntSwitchCallback
VOID ODM_SwAntDivChkAntSwitchCallback(PRT_TIMER		pTimer);

#define dm_SW_AntennaSwitchWorkitemCallback	ODM_SwAntDivChkAntSwitchWorkitemCallback
VOID ODM_SwAntDivChkAntSwitchWorkitemCallback(IN PVOID            pContext    );

#define dm_SWAW_RSSI_Check	ODM_SwAntDivChkPerPktRssi
VOID ODM_SwAntDivChkPerPktRssi(PADAPTER		Adapter,
										BOOLEAN			bIsDefPort,
										BOOLEAN			bMatchBSSID,
										PRT_WLAN_STA	pEntry,
										PRT_RFD			pRfd	);

extern VOID Process_RSSIForDM(	PADAPTER					Adapter,
									PRT_RFD						pRfd,
									PRT_WLAN_STA				pEntry,
									pu1Byte						pDesc	);
#define	dm_PSDMonitorCallback	odm_PSDMonitorCallback
VOID	odm_PSDMonitorCallback(PRT_TIMER		pTimer);
VOID ODM_DMInit(IN PADAPTER	Adapter);

VOID ODM_DMWatchdog(IN PADAPTER	Adapter);
VOID
odm_PSDMonitorWorkItemCallback(
    IN PVOID            pContext
    );
VOID
ODM_PSDDbgControl(
	IN	PADAPTER	Adapter,
	IN	u4Byte		mode,
	IN	u4Byte		btRssi
	);

#endif

//extern	VOID 
//ODM_DigMP(
//	IN		PADAPTER		pAdapter
//	);


#endif
