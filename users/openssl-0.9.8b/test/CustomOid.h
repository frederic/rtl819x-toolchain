#ifndef	__CUSTOM_OID_H
#define __CUSTOM_OID_H


//3*****************************************************************************************************
//3 0xFF818000 - 0xFF81802F		RTL81xx Mass Production Kit
//3 0xFF818500 - 0xFF81850F		RTL8185 Setup Utility
//3 0xFF818580 - 0xFF81858F		RTL8185 Phy Status Utility
//3 0xFF818700 - 0xFF8187FF		RTL8187 related
//3 0xFF819000 - 0xFF8190FF		RTL8190 related

// by Owen for Production Kit
//3 For Production Kit with Agilent Equipments
//3 in order to make our custom oids hopefully somewhat unique
//3 we will use 0xFF (indicating implementation specific OID)
//3               		81(first byte of non zero Realtek unique identifier)
//3	        80 (second byte of non zero Realtek unique identifier)
//3               		XX (the custom OID number - providing 255 possible custom oids)

//3*****************************************************************************************************


//3--------------------------------------------------------------------
//3 			0xFF8180xx: Mass Prodution
//3--------------------------------------------------------------------
#define		OID_RT_PRO_RESET_DUT							0xFF818000
#define		OID_RT_PRO_SET_DATA_RATE						0xFF818001
#define		OID_RT_PRO_START_TEST							0xFF818002
#define		OID_RT_PRO_STOP_TEST    						0xFF818003
#define		OID_RT_PRO_SET_PREAMBLE							0xFF818004
#define		OID_RT_PRO_SET_SCRAMBLER						0xFF818005
#define		OID_RT_PRO_SET_FILTER_BB						0xFF818006
#define		OID_RT_PRO_SET_MANUAL_DIVERSITY_BB					0xFF818007
#define		OID_RT_PRO_SET_CHANNEL_DIRECT_CALL					0xFF818008
#define		OID_RT_PRO_SET_SLEEP_MODE_DIRECT_CALL					0xFF818009
#define		OID_RT_PRO_SET_WAKE_MODE_DIRECT_CALL					0xFF81800A
#define		OID_RT_PRO_SET_TX_CONTINUOUS_DIRECT_CALL				0xFF81800B
#define		OID_RT_PRO_SET_SINGLE_CARRIER_TX_CONTINUOUS				0xFF81800C
#define		OID_RT_PRO_SET_TX_ANTENNA_BB						0xFF81800D
#define		OID_RT_PRO_SET_ANTENNA_BB						0xFF81800E
#define		OID_RT_PRO_SET_CR_SCRAMBLER						0xFF81800F
#define		OID_RT_PRO_SET_CR_NEW_FILTER						0xFF818010
#define		OID_RT_PRO_SET_TX_POWER_CONTROL						0xFF818011
#define		OID_RT_PRO_SET_CR_TX_CONFIG						0xFF818012
#define		OID_RT_PRO_GET_TX_POWER_CONTROL						0xFF818013
#define		OID_RT_PRO_GET_CR_SIGNAL_QUALITY					0xFF818014
#define		OID_RT_PRO_SET_CR_SETPOINT						0xFF818015
#define		OID_RT_PRO_SET_INTEGRATOR						0xFF818016
#define		OID_RT_PRO_SET_SIGNAL_QUALITY						0xFF818017
#define		OID_RT_PRO_GET_INTEGRATOR						0xFF818018
#define		OID_RT_PRO_GET_SIGNAL_QUALITY						0xFF818019
#define		OID_RT_PRO_QUERY_EEPROM_TYPE						0xFF81801A
#define		OID_RT_PRO_WRITE_MAC_ADDRESS						0xFF81801B
#define		OID_RT_PRO_READ_MAC_ADDRESS						0xFF81801C
#define		OID_RT_PRO_WRITE_CIS_DATA						0xFF81801D
#define		OID_RT_PRO_READ_CIS_DATA						0xFF81801E
#define		OID_RT_PRO_WRITE_POWER_CONTROL						0xFF81801F
#define		OID_RT_PRO_READ_POWER_CONTROL						0xFF818020
#define		OID_RT_PRO_WRITE_EEPROM							0xFF818021
#define		OID_RT_PRO_READ_EEPROM							0xFF818022
#define		OID_RT_PRO_RESET_TX_PACKET_SENT						0xFF818023
#define		OID_RT_PRO_QUERY_TX_PACKET_SENT						0xFF818024
#define		OID_RT_PRO_RESET_RX_PACKET_RECEIVED					0xFF818025 
#define		OID_RT_PRO_QUERY_RX_PACKET_RECEIVED					0xFF818026
#define		OID_RT_PRO_QUERY_RX_PACKET_CRC32_ERROR					0xFF818027
#define		OID_RT_PRO_QUERY_CURRENT_ADDRESS					0xFF818028
#define		OID_RT_PRO_QUERY_PERMANENT_ADDRESS					0xFF818029
#define		OID_RT_PRO_SET_PHILIPS_RF_PARAMETERS					0xFF81802A
#define		OID_RT_PRO_SET_CARRIER_SUPPRESSION_TX_CONTINUOUS			0xFF81802B
#define		OID_RT_PRO_RECEIVE_PACKET						0xFF81802C
// added by Owen on 04/08/03 for Cameo's request
#define		OID_RT_PRO_WRITE_EEPROM_BYTE						0xFF81802D
#define		OID_RT_PRO_READ_EEPROM_BYTE						0xFF81802E
#define		OID_RT_PRO_SET_MODULATION						0xFF81802F
//

//Sean		
#define		OID_RT_DRIVER_OPTION							0xFF818080
#define		OID_RT_RF_OFF								0xFF818081
#define		OID_RT_AUTH_STATUS							0xFF818082
#define		OID_RT_INACTIVE_PS							0xFF818083

//3

//3--------------------------------------------------------------------
//3 		0xFF03xxxx~
//3--------------------------------------------------------------------

#define 	OID_RT_GET_CONNECT_STATE             				  	0xFF030001
#define 	OID_RT_RESCAN	                        				0xFF030002
#define 	OID_RT_SET_KEY_LENGTH                   				0xFF030003

#define 	OID_RT_SET_DEFAULT_KEY_ID               				0xFF030004
#define 	OID_RT_GET_DEFAULT_KEY_ID						0xFF030004 //Same defnition as OID_RT_SET_DEFAULT_KEY_ID but provide query


//3--------------------------------------------------------------------
//3 		0xFF01xxxx~
//3--------------------------------------------------------------------
#define 	OID_RT_SET_CHANNEL							0xFF010182
#define 	OID_RT_SET_SNIFFER_MODE                 				0xFF010183
#define 	OID_RT_GET_SIGNAL_QUALITY              					0xFF010184
#define 	OID_RT_GET_SMALL_PACKET_CRC						0xFF010185		
#define 	OID_RT_GET_MIDDLE_PACKET_CRC						0xFF010186
#define 	OID_RT_GET_LARGE_PACKET_CRC						0xFF010187
#define 	OID_RT_GET_TX_RETRY							0xFF010188
#define 	OID_RT_GET_RX_RETRY							0xFF010189
#define 	OID_RT_GET_RX_TOTAL_PACKET						0xFF010190
#define 	OID_RT_GET_TX_BEACON_OK							0xFF010191
#define 	OID_RT_GET_TX_BEACON_ERR						0xFF010192
#define 	OID_RT_GET_RX_ICV_ERR							0xFF010193
#define 	OID_RT_SET_ENCRYPTION_ALGORITHM						0xFF010194
#define 	OID_RT_SET_NO_AUTO_RESCAN						0xFF010195
#define 	OID_RT_GET_PREAMBLE_MODE						0xFF010196
#define 	OID_RT_GET_DRIVER_UP_DELTA_TIME						0xFF010197
#define 	OID_RT_GET_AP_IP							0xFF010198
#define 	OID_RT_GET_CHANNELPLAN							0xFF010199
#define 	OID_RT_SET_PREAMBLE_MODE						0xFF01019A
#define 	OID_RT_SET_BCN_INTVL							0xFF01019B
#define		OID_RT_GET_BCN_INTVL							0xFF01019B //same definition as OID_RT_SET_BCN_INTVL but provide query
#define 	OID_RT_GET_RF_VENDER							0xFF01019C
#define 	OID_RT_DEDICATE_PROBE							0xFF01019D
#define 	OID_RT_PRO_RX_FILTER_PATTERN						0xFF01019E
#define 	OID_RT_GET_DCST_CURRENT_THRESHOLD					0xFF01019F
#define 	OID_RT_GET_CCA_ERR							0xFF0101A0
#define 	OID_RT_GET_CCA_UPGRADE_THRESHOLD					0xFF0101A1
#define 	OID_RT_GET_CCA_FALLBACK_THRESHOLD					0xFF0101A2
#define 	OID_RT_GET_CCA_UPGRADE_EVALUATE_TIMES					0xFF0101A3
#define 	OID_RT_GET_CCA_FALLBACK_EVALUATE_TIMES					0xFF0101A4

// by Owen on 03/31/03 for Cameo's request
//#define 	OID_RT_SET_RATE_ADAPTIVE						0xFF0101A5
//
#define 	OID_RT_GET_DCST_EVALUATE_PERIOD						0xFF0101A5
#define 	OID_RT_GET_DCST_TIME_UNIT_INDEX						0xFF0101A6
#define 	OID_RT_GET_TOTAL_TX_BYTES						0xFF0101A7
#define 	OID_RT_GET_TOTAL_RX_BYTES						0xFF0101A8
#define 	OID_RT_CURRENT_TX_POWER_LEVEL						0xFF0101A9
#define 	OID_RT_GET_ENC_KEY_MISMATCH_COUNT					0xFF0101AA
#define 	OID_RT_GET_ENC_KEY_MATCH_COUNT						0xFF0101AB
#define 	OID_RT_GET_CHANNEL							0xFF0101AC
#define 	OID_RT_SET_CHANNELPLAN							0xFF0101AD
#define 	OID_RT_GET_HARDWARE_RADIO_OFF						0xFF0101AE
#define 	OID_RT_CHANNELPLAN_BY_COUNTRY						0xFF0101AF
#define 	OID_RT_SCAN_AVAILABLE_BSSID						0xFF0101B0
#define 	OID_RT_GET_HARDWARE_VERSION						0xFF0101B1
#define 	OID_RT_GET_IS_ROAMING							0xFF0101B2
#define 	OID_RT_GET_IS_PRIVACY							0xFF0101B3
#define 	OID_RT_GET_KEY_MISMATCH							0xFF0101B4
#define 	OID_RT_SET_RSSI_ROAM_TRAFFIC_TH						0xFF0101B5
#define 	OID_RT_SET_RSSI_ROAM_SIGNAL_TH						0xFF0101B6
#define 	OID_RT_RESET_LOG							0xFF0101B7
#define 	OID_RT_GET_LOG								0xFF0101B8
#define 	OID_RT_SET_INDICATE_HIDDEN_AP						0xFF0101B9
#define 	OID_RT_GET_HEADER_FAIL							0xFF0101BA
#define 	OID_RT_SUPPORTED_WIRELESS_MODE						0xFF0101BB
#define 	OID_RT_GET_CHANNEL_LIST							0xFF0101BC
#define 	OID_RT_GET_SCAN_IN_PROGRESS						0xFF0101BD
#define 	OID_RT_GET_TX_INFO							0xFF0101BE
#define 	OID_RT_IO_READ_WRITE_INFO						0xFF0101BF
#define 	OID_RT_IO_READ_WRITE							0xFF0101C0
// For Netgear request.
#define 	OID_RT_FORCED_DATA_RATE							0xFF0101C1
// Auto-Config OID.
#define 	OID_RT_WIRELESS_MODE_FOR_SCAN_LIST      				0xFF0101C2
#define 	OID_RT_GET_BSS_WIRELESS_MODE			       			0xFF0101C3
// For AZ project.
#define 	OID_RT_SCAN_WITH_MAGIC_PACKET						0xFF0101C4
// Auto-Config OID.
#define 	OID_RT_AUTOCFG_SCAN							0xFF0101C5
// Auto select channel of the BSS.
#define 	OID_RT_AUTO_SELECT_CHANNEL                         			0xFF0101C6
//for AutoTurbomode
#define 	OID_RT_TURBOMODE							0xFF0101C7
// Send specific 802.11 frame for developing and verification purpose, 2005.12.23, by rcnjko.
#define 	OID_RT_SEND_SPECIFIC_PACKET   						0xFF0101C8
// For debug purpose.
#define 	OID_RT_DBG_COMPONENT           						0xFF0101C9
#define 	OID_RT_DBG_LEVEL                    					0xFF0101CA
#define 	OID_RT_DEVICE_ID_INFO							0xFF0101CB 	// To identify Realtek WLAN device, 2006.01.24, by rcnjko.

#define 	OID_RT_HIDDEN_SSID							0xFF0101CC
#define 	OID_RT_LOCKED_STA_ADDRESS						0xFF0101CD
#define 	OID_RT_PER_STA_DATA_RATE						0xFF0101CE

#define 	OID_RT_SimpleConfScan							0xFF0101D0
#define 	OID_RT_CTS_TO_SELF_RATE							0xFF0101D1
// For CCX test plan v3.61 4.3, 060927, by rcnjko.
#define 	OID_RT_TX_POWER								0xFF0101D2
#define 	OID_RT_SET_MAC_FILTER_TYPE          					0xFF0101D3   //Add by Jacken 2007/04/27
#define 	OID_RT_WEP_STATUS							0xFF0101D4
#define 	OID_RT_AUTHENTICATION_MODE						0xFF0101D5
#define 	OID_RT_ADD_KEY								0xFF0101D6
#define 	OID_RT_UI_ENABLE_HIGH_PRIORITY						0xFF0101D7
#define 	OID_RT_UI_DISABLE_HIGH_PRIORITY						0xFF0101D8
//For scan limit
#define OID_RT_SET_SCAN_LIMIT								0xFF0101D9
// 8187BMP for product string in eeprom. Added by Bruce, 2007-1-19.
#define 	OID_RT_PRO_CORRECT_PRODUCT_STRING					0xFF0101DA
#define 	OID_RT_PRO_CHECK_PRODUCT_STRING						0xFF0101DB
// For ASUS request
//For Mesh MAC_FILTER ,2008.01.09 ,By rcnjko ,Add By Karl
#define 	OID_RT_FILTER_STA_ADDRESS						0xFF0101DC

#define 	OID_RT_WPS_RECIEVE_PACKET						0xFF0101DE

// For PSP XLink status. 2007.01.12, by shien chang.
#define 	OID_RT_GET_PSP_XLINK_STATUS						0xFF0101CF
#define 	OID_RT_SET_PSP_XLINK_STATUS						0xFF0101E0

// For WMM and WMM-UAPSD, 2007.01.15, by shien chang.
#define 	OID_RT_GET_WMM_ENABLE							0xFF0101E1
#define 	OID_RT_SET_WMM_ENABLE							0xFF0101E2
#define 	OID_RT_GET_WMM_UAPSD_ENABLE						0xFF0101E3
#define 	OID_RT_SET_WMM_UAPSD_ENABLE						0xFF0101E4

// 070208, rcnjko: For 802.11d.
#define 	OID_RT_DOT11D								0xFF0101E5

// 070301, rcnjko: for driver log event mechanism.
#define 	OID_RT_GET_LOGV2_TYPE_LIST						0xFF0101E6
#define 	OID_RT_GET_LOGV2_ATTR_LIST						0xFF0101E7
#define 	OID_RT_GET_LOGV2_DATA_LIST						0xFF0101E8

//For CCX yest plane v3.61 4.4 , 061020 by CCW
#define 	OID_RT_ROAM_TO_SELECT_BSSID            		 			0xFF0101E9

// For WMM Admission Control, 2007.08.16, by shine chang.
#define 	OID_RT_SEND_WMM_ADDTS							0xFF0101EA
#define 	OID_RT_SEND_WMM_DELTS							0xFF0101EB

//--- for Mesh Mode ,080103 Add By karl
#define 	OID_RT_MESH_MODE							0xFF0101EC
#define 	OID_RT_MESH_ID								0xFF0101ED
//--- 
// For Report noise power, By chiyokolin, 2008-03-20
#define 	OID_RT_GET_NOISE_POWER							0xFF0101EF

// Vincent 8185MP
#define 	OID_RT_PRO_RX_FILTER							0xFF0111C0
#define 	OID_RT_PRO_WRITE_REGISTRY						0xFF0111C1
#define 	OID_RT_PRO_READ_REGISTRY						0xFF0111C2
#define		OID_RT_PRO_SET_INITIAL_GAIN						0xFF0111C3
#define		OID_RT_PRO_SET_BB_RF_STANDBY_MODE					0xFF0111C4
#define		OID_RT_PRO_SET_BB_RF_SHUTDOWN_MODE					0xFF0111C5
#define		OID_RT_PRO_SET_TX_CHARGE_PUMP						0xFF0111C6
#define		OID_RT_PRO_SET_RX_CHARGE_PUMP						0xFF0111C7
#define 	OID_RT_PRO_RF_WRITE_REGISTRY						0xFF0111C8
#define 	OID_RT_PRO_RF_READ_REGISTRY						0xFF0111C9
#define		OID_RT_PRO_QUERY_RF_TYPE						0xFF0111CA
#define		OID_RT_PRO_SW_RF_READ_REGISTRY						0xFF0111CB  //SW Three Wire for SD3 Required.
#define 	OID_RT_PRO_SW_RF_WRITE_REGISTRY						0xFF0111CC  //SW Three Wire for SD3 Required.

//8185 use only so far
#define 	OID_RT_GetTxPower                                                      	0xFF010200

// AP OID
#define 	OID_RT_AP_GET_ASSOCIATED_STATION_LIST					0xFF010300
#define 	OID_RT_AP_GET_CURRENT_TIME_STAMP					0xFF010301
#define 	OID_RT_AP_SWITCH_INTO_AP_MODE						0xFF010302

#define 	OID_RT_AP_SET_DTIM_PERIOD						0xFF010303
#define 	OID_RT_AP_GET_DTIM_PERIOD						0xFF010303  //Same definition as OID_RT_AP_SET_DTIM_PERIOD but provide query

#define 	OID_RT_AP_SUPPORTED							0xFF010304  // Determine if driver supports AP mode. 2004.08.27, by rcnjko.
#define 	OID_RT_AP_SET_PASSPHRASE						0xFF010305  // Set WPA-PSK passphrase into authenticator. 2005.07.08, byrcnjko.

#define	 	OID_RT_AP_WDS_MODE							0xFF010306  // 0: WDS disabled, 1: WDS enabled. 2006.06.12, by rcnjko.
#define 	OID_RT_AP_WDS_AP_LIST							0xFF010307  // WDS AP address list. 
//

// 802.11 engineering page OID
// --802.11h
// --802.11d
#define 	OID_RT_SET_80211H_SWITCH_CHANNEL					0xFF010400

//3--------------------------------------------------------------------
//3 			0xFF8185xx: 
//3--------------------------------------------------------------------
// by Owen for RTL8185 Phy Status Report Utility
#define		OID_RT_UTILITY_FALSE_ALARM_COUNTERS					0xFF818580
#define		OID_RT_UTILITY_SELECT_DEBUG_MODE					0xFF818581
#define		OID_RT_UTILITY_SELECT_SUBCARRIER_NUMBER					0xFF818582
#define		OID_RT_UTILITY_GET_RSSI_STATUS						0xFF818583
#define		OID_RT_UTILITY_GET_FRAME_DETECTION_STATUS				0xFF818584
#define		OID_RT_UTILITY_GET_AGC_AND_FREQUENCY_OFFSET_ESTIMATION_STATUS		0xFF818585
#define		OID_RT_UTILITY_GET_CHANNEL_ESTIMATION_STATUS				0xFF818586
//
//by Ida
#define		OID_RT_GET_ENC_CAM_STATUS						0xFF818587
//
// by Owen on 03/09/19-03/09/22 for RTL8185
#define		OID_RT_WIRELESS_MODE							0xFF818500
#define		OID_RT_SUPPORTED_RATES							0xFF818501
#define		OID_RT_DESIRED_RATES							0xFF818502
#define		OID_RT_WIRELESS_MODE_STARTING_ADHOC					0xFF818503
#define		OID_RT_ADHOC_DEFAULT_WIRELESS_MODE					0xFF818504

//3 --------------------------------------------------------------------
//3 		0xFF8187xx: 8187 related
//3--------------------------------------------------------------------

// 8187MP. 2004.09.06, by rcnjko.
#define 	OID_RT_PRO8187_WI_POLL							0xFF818780
#define 	OID_RT_PRO_WRITE_BB_REG							0xFF818781
#define 	OID_RT_PRO_READ_BB_REG							0xFF818782
//

// 8187MP for KY's request. 2005.09.09, by rcnjko.
#define 	OID_RT_PRO_ENABLE_ACK_COUNTER						0xFF818783
#define 	OID_RT_PRO_RESET_ACK_COUNTER						0xFF818784
#define 	OID_RT_PRO_GET_ACK_COUNTER						0xFF818785
//

// 8187MP for AZ's request. 2005.09.09, by rcnjko.
#define 	OID_RT_PRO_SET_TX_POWER_BASE_OFFSET					0xFF818786
#define 	OID_RT_PRO_GET_TX_POWER_BASE_OFFSET					0xFF818787
//
//87S-USB, out command. 2007-07-23 , by Isaiah
#define 	OID_RT_PRO_87SU_OUT_CMD							0xFF818788
//
//3 --------------------------------------------------------------------
//3 		0xFF8190xx: 8190 related
//3--------------------------------------------------------------------
// add for RTL8190 MIMO
#define OID_RT_GET_11N_MIMO_RSSI							0xFF81900A
#define OID_RT_GET_11N_MIMOPO_EVM           						0xFF81900B
#define OID_RT_11N_BANDWIDTH_MODE           						0xFF81900D 

//----RxTx Rate for N WLan
//--for Debug 
#define OID_RT_11N_TX_LINK_SPEED							0xFF819021	 //Add by Jacken 2008/03/12
#define OID_RT_11N_RX_LINK_SPEED							0xFF819022	 //Add by Jacken 2008/03/12

#define OID_RT_11N_INITIAL_TX_RATE							0xFF819033   //define By vivi ,add by Karl
#define OID_RT_11N_RETRY_COUNT							    0xFF819034   //define By vivi ,add by Karl
//--
//---For Hardward PBC , Add by Jacken 2008/03/12
#define OID_RT_WPS_HWSET_PBC_PRESSED							0xFF819028
#define OID_RT_WPS_HWGET_PBC_PRESSED							0xFF819029

// 2008-03-20 Add by hpfan for WPS LED control
#define OID_RT_WPS_LED_CTL_START							0xFF81902A
#define OID_RT_WPS_CUSTOMIZED_LED							0xFF81902F

#define OID_RT_11N_UI_Show_TxRate							0xFF81902B
#define OID_RT_11N_UI_Show_RxRate           						0xFF81901D 	//define By Emily ,Add By Karl For N
//----

//-Add for RTL8190 Debug   //Add by Jacken 2007/07/03
#define	OID_RT_PRO_DBGCMD_SEND								0xFF818040
#define	OID_RT_PRO_DBGCMD_CHECK								0xFF818041
#define	OID_RT_PRO_DBGCMD_RETURN							0xFF818042

//3--------------------------------------------------------------------
//3		0xFFEDC1xx: For Meeting House
//3--------------------------------------------------------------------

// Meeting House. added by Annie, 2005-07-20.
#define 	OID_RT_MH_VENDER_ID							0xFFEDC100
//CCX Rogue AP , 2006.07.27, by CCW
#define 	MH_OID_CCX_ROGUE_AP                   					0xFFEDC10B
#define 	MH_OID_CCX_ROGUE_AP_STATUS      					0xFFEDC101

//CCX NETWORK EAP for LEAP 2006.07.31,by CCW
#define 	MH_OID_CCX_NETWORK_EAP            					0xFFEDC102
//CCX  2006.08.01,by CCW
#define  	MH_OID_CCX_MIXED_CELL              					0xFFEDC103
#define  	MH_OID_CCX_FAST_ROAM               					0xFFEDC105
#define  	MH_OID_CCX_FAST_ROAM_RESULT           					0xFFEDC106
#define  	MH_OID_CCX_ADD_KRK                   					0xFFEDC107
#define  	MH_OID_CCX_REMOVE_KRK             					0xFFEDC108 
#define 	MH_OID_CCX_VERSION                   					0xFFEDC109
#define 	MH_OID_CCX_ENABLE                     					0xFFEDC10A


#define OID_RT_CURRENT_WIRELESSMODE         0xFF0101EE
#define OID_RT_11N_CURRENT_BANDWIDTH        0xFF819035

//WAPI related OID
#define	OID_RT_WAPI_AE_START				0xFFEDC200
#define 	OID_RT_WAPI_GET_INFO				0xFFEDC201
#define	OID_RT_WAPI_SET_KEY				0xFFEDC202
#define	OID_RT_WAPI_GET_PN					0xFFEDC203
#define	OID_RT_WAPI_RECEIVE_PACKET			0xFFEDC204
#define	OID_RT_WAPI_SET_DESTNATION_ADDRESS 	0xFFEDC205
#define	OID_RT_WAPI_SEND_WAI_PACKET			0xFFEDC206
#define	OID_WAPI_ASUE_CREATE_THREAD			0xFFEDC207
#define	OID_WAPI_ASUE_SET_MUTICAST_PN			0xFFEDC208

/*----------------------------------------------------------------------------
	OID_RT_DEVICE_ID_INFO used data structure
----------------------------------------------------------------------------*/

#define 	RT_DEVICE_ID_INFO_TAG							0x10ec0211
#define 	RT_DEVICE_ID_PCI							0x00000000
#define 	RT_DEVICE_ID_USB							0x00000001

typedef struct _RT_DEVICE_ID_HEADER{
	//
	// Identify whether this is a Realtek WLAN device.
	// RT_DEVICE_ID_INFO_TAG means Realtek WLAN NIC device, other values are not valid.
	//
	ULONG	RtWlanDevTag;

	//
	// Identify which IC.
	// Examples of (ChipID, ChipVer):
	// (0x8185, 0x1)	=> 8185 
	// (0x8187, 0x1)	=> 8187 
	// (0x8185, 0x2)	=> 8185B 
	// (0x8187, 0x2)	=> 8187B 
	//
	ULONG	ChipID;
	ULONG	ChipVer;

	//
	// BusType is used to identify BUS type of the device and corresponding data type, 
	// for example:
	// RT_PCI_DEVICE => struc _RT_PCI_ID_INFO
	// RT_USB_DEVICE => struc _RT_USB_ID_INFO
	//
	ULONG	BusType; 
}RT_DEVICE_ID_HEADER, *PRT_DEVICE_ID_HEADER;

typedef struct _RT_PCI_ID_INFO{
	RT_DEVICE_ID_HEADER	DevIDHeader;

	// 
	// Vendor ID and Device ID from PCI configuration space. 
	//
	USHORT	VID;
	USHORT	DID;

	// 
	// Sub Vendor ID and Subsystem ID from PCI configuration space. 
	//
	USHORT	SVID;
	USHORT	SMID;

	//
	// Revision ID from PCI configuration space.
	//
	UCHAR	RevID;

	//
	// Customer ID.
	//
	USHORT	CustomerID;
}RT_PCI_ID_INFO, *PRT_PCI_ID_INFO;

typedef struct _RT_USB_ID_INFO{
	RT_DEVICE_ID_HEADER	DevIDHeader;

	// 
	// Vendor ID and Product ID from USB Device Descriptor.
	//
	USHORT	VID;
	USHORT	PID;

	//
	// bcdDevice from USB Device Descriptor.
	//
	USHORT	RevID;

	//
	// Interface index.
	//
	USHORT	InterfaceIdx;
}RT_USB_ID_INFO, *PRT_USB_ID_INFO;
//

//ShienChang: OID mapping to Vista DOT11 OID
//Scope: 0xFF010601 ~ 0xFF0106FF
#define 	OID_RT_POWER_MGMT_REQUEST						0xFF010601
#define 	OID_RT_OPERATIONAL_RATE_SET						0xFF010602
#define 	OID_RT_FRAGMENTATION_THRESHOLD						0xFF010603
#define 	OID_RT_RTS_THRESHOLD							0xFF010604
#define 	OID_RT_DISCONNECT_REQUEST						0xFF010605


//ShienChang: OID mapping for WinXP style 802.11 OID
//Scope: 0xFF070101 ~ 0xFF0702FF
#define 	RT_80211_START								0xFF070000
#define 	OID_RT_802_11_BSSID				( RT_80211_START | OID_802_11_BSSID )
#define 	OID_RT_802_11_SSID				( RT_80211_START | OID_802_11_SSID )
#define 	OID_RT_802_11_NETWORK_TYPES_SUPPORTED		( RT_80211_START | OID_802_11_NETWORK_TYPES_SUPPORTED )
#define 	OID_RT_802_11_NETWORK_TYPE_IN_USE		( RT_80211_START | OID_802_11_NETWORK_TYPE_IN_USE )
#define 	OID_RT_802_11_TX_POWER_LEVEL			( RT_80211_START | OID_802_11_TX_POWER_LEVEL )
#define 	OID_RT_802_11_RSSI				( RT_80211_START | OID_802_11_RSSI )
#define 	OID_RT_802_11_RSSI_TRIGGER			( RT_80211_START | OID_802_11_RSSI_TRIGGER )
#define 	OID_RT_802_11_INFRASTRUCTURE_MODE		( RT_80211_START | OID_802_11_INFRASTRUCTURE_MODE )
#define 	OID_RT_802_11_FRAGMENTATION_THRESHOLD		( RT_80211_START | OID_802_11_FRAGMENTATION_THRESHOLD )
#define 	OID_RT_802_11_RTS_THRESHOLD			( RT_80211_START | OID_802_11_RTS_THRESHOLD )
#define 	OID_RT_802_11_NUMBER_OF_ANTENNAS		( RT_80211_START | OID_802_11_NUMBER_OF_ANTENNAS )
#define 	OID_RT_802_11_RX_ANTENNA_SELECTED		( RT_80211_START | OID_802_11_RX_ANTENNA_SELECTED )
#define 	OID_RT_802_11_TX_ANTENNA_SELECTED		( RT_80211_START | OID_802_11_TX_ANTENNA_SELECTED )
#define 	OID_RT_802_11_SUPPORTED_RATES			( RT_80211_START | OID_802_11_SUPPORTED_RATES )
#define 	OID_RT_802_11_DESIRED_RATES			( RT_80211_START | OID_802_11_DESIRED_RATES )
#define 	OID_RT_802_11_CONFIGURATION			( RT_80211_START | OID_802_11_CONFIGURATION )
#define 	OID_RT_802_11_STATISTICS			( RT_80211_START | OID_802_11_STATISTICS )
#define 	OID_RT_802_11_ADD_WEP				( RT_80211_START | OID_802_11_ADD_WEP )
#define 	OID_RT_802_11_REMOVE_WEP			( RT_80211_START | OID_802_11_REMOVE_WEP )
#define 	OID_RT_802_11_DISASSOCIATE			( RT_80211_START | OID_802_11_DISASSOCIATE )
#define 	OID_RT_802_11_POWER_MODE			( RT_80211_START | OID_802_11_POWER_MODE )
#define 	OID_RT_802_11_BSSID_LIST			( RT_80211_START | OID_802_11_BSSID_LIST )
#define 	OID_RT_802_11_AUTHENTICATION_MODE		( RT_80211_START | OID_802_11_AUTHENTICATION_MODE )
#define 	OID_RT_802_11_PRIVACY_FILTER			( RT_80211_START | OID_802_11_PRIVACY_FILTER )
#define 	OID_RT_802_11_BSSID_LIST_SCAN			( RT_80211_START | OID_802_11_BSSID_LIST_SCAN )
#define 	OID_RT_802_11_WEP_STATUS			( RT_80211_START | OID_802_11_WEP_STATUS )
#define 	OID_RT_802_11_RELOAD_DEFAULTS			( RT_80211_START | OID_802_11_RELOAD_DEFAULTS )
#define 	OID_RT_802_11_TEST				( RT_80211_START | OID_802_11_TEST )
#define 	OID_RT_802_11_CAPABILITY			( RT_80211_START | OID_802_11_CAPABILITY )
#define 	OID_RT_802_11_PMKID				( RT_80211_START | OID_802_11_PMKID )
#define 	OID_RT_802_11_ASSOCIATION_INFORMATION		( RT_80211_START | OID_802_11_ASSOCIATION_INFORMATION )
#define 	OID_RT_802_11_ENCRYPTION_STATUS			( RT_80211_START | OID_802_11_ENCRYPTION_STATUS )
#define 	OID_RT_802_11_ADD_KEY				( RT_80211_START | OID_802_11_ADD_KEY )
#define 	OID_RT_802_11_REMOVE_KEY			( RT_80211_START | OID_802_11_REMOVE_KEY )


#endif //#ifndef	__CUSTOM_OID_H
