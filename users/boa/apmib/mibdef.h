/*
 *      Header file of AP mib
 *      Authors: David Hsu	<davidhsu@realtek.com.tw>
 *
 *      $Id: apmib.h,v 1.55 2009/10/06 05:49:10 bradhuang Exp $
 *
 */

#define NOREP

#ifdef MIB_HW_IMPORT
/* _ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl */
MIBDEF(unsigned char,	boardVer,	,	BOARD_VER,	BYTE_T, HW_SETTING_T, 0, 0)
MIBDEF(unsigned char,	nic0Addr,	[6],	NIC0_ADDR,	BYTE6_T, HW_SETTING_T, 0, 0)
MIBDEF(unsigned char,	nic1Addr,	[6],	NIC1_ADDR,	BYTE6_T, HW_SETTING_T, 0, 0)
MIBDEF(HW_WLAN_SETTING_T,	wlan, [NUM_WLAN_INTERFACE],	WLAN_ROOT,	TABLE_LIST_T, HW_SETTING_T, 0, hwmib_wlan_table)
#endif // #ifdef MIB_HW_IMPORT

#ifdef MIB_HW_WLAN_IMPORT
/* _ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl */
MIBDEF(unsigned char, macAddr, [6],	WLAN_ADDR,	BYTE6_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, macAddr1, [6],	WLAN_ADDR1,	BYTE6_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, macAddr2, [6],	WLAN_ADDR2,	BYTE6_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, macAddr3, [6],	WLAN_ADDR3,	BYTE6_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, macAddr4, [6],	WLAN_ADDR4,	BYTE6_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, macAddr5, [6],	WLAN_ADDR5,	BYTE6_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, macAddr6,[6],	    WLAN_ADDR6,	BYTE6_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, macAddr7, [6],	WLAN_ADDR7,	BYTE6_T, HW_WLAN_SETTING_T, 0, 0)
#if defined(CONFIG_RTL_8196B)
MIBDEF(unsigned char, txPowerCCK, [MAX_CCK_CHAN_NUM],	TX_POWER_CCK,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, txPowerOFDM_HT_OFDM_1S, [MAX_OFDM_CHAN_NUM],	TX_POWER_OFDM_1S,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, txPowerOFDM_HT_OFDM_2S, [MAX_OFDM_CHAN_NUM],	TX_POWER_OFDM_2S,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, regDomain, ,	REG_DOMAIN,	BYTE_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, rfType, ,	RF_TYPE,	BYTE_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, xCap, ,	11N_XCAP,	BYTE_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, LOFDMPwDiffA, ,	11N_LOFDMPWDA,	BYTE_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, LOFDMPwDiffB, ,	11N_LOFDMPWDB,	BYTE_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, TSSI1, ,	11N_TSSI1,	BYTE_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, TSSI2, ,	11N_TSSI2,	BYTE_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, Ther, ,	11N_THER,	BYTE_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, ledType, ,	LED_TYPE,	BYTE_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, Reserved1, ,	11N_RESERVED1,	BYTE_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, Reserved2, ,	11N_RESERVED2,	BYTE_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, Reserved3, ,	11N_RESERVED3,	BYTE_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, Reserved4, ,	11N_RESERVED4,	BYTE_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, Reserved5, ,	11N_RESERVED5,	BYTE_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, Reserved6, ,	11N_RESERVED6,	BYTE_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, Reserved7, ,	11N_RESERVED7,	BYTE_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, Reserved8, ,	11N_RESERVED8,	BYTE_T, HW_WLAN_SETTING_T, 0, 0)
#else /*rtl8196c*/
MIBDEF(unsigned char, pwrlevelCCK_A, [MAX_2G_CHANNEL_NUM_MIB],	TX_POWER_CCK_A,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrlevelCCK_B, [MAX_2G_CHANNEL_NUM_MIB],	TX_POWER_CCK_B,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrlevelHT40_1S_A, [MAX_2G_CHANNEL_NUM_MIB],	TX_POWER_HT40_1S_A,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrlevelHT40_1S_B, [MAX_2G_CHANNEL_NUM_MIB],	TX_POWER_HT40_1S_B,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiffHT40_2S, [MAX_2G_CHANNEL_NUM_MIB],	TX_POWER_DIFF_HT40_2S,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiffHT20, [MAX_2G_CHANNEL_NUM_MIB],	TX_POWER_DIFF_HT20,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiffOFDM, [MAX_2G_CHANNEL_NUM_MIB],	TX_POWER_DIFF_OFDM,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, regDomain, ,	REG_DOMAIN,	BYTE_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, rfType, ,	RF_TYPE,	BYTE_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, ledType, ,	LED_TYPE,	BYTE_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, xCap, ,	11N_XCAP,	BYTE_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, TSSI1, ,	11N_TSSI1,	BYTE_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, TSSI2, ,	11N_TSSI2,	BYTE_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, Ther, ,	11N_THER,	BYTE_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, trswitch, ,      11N_TRSWITCH,   BYTE_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, trswpape_C9, ,	11N_TRSWPAPE_C9, BYTE_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, trswpape_CC, ,	11N_TRSWPAPE_CC, BYTE_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, target_pwr, ,	11N_TARGET_PWR,	BYTE_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, Reserved5, ,	11N_RESERVED5,	BYTE_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, Reserved6, ,	11N_RESERVED6,	BYTE_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, Reserved7, ,	11N_RESERVED7,	BYTE_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, Reserved8, ,	11N_RESERVED8,	BYTE_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, Reserved9, ,	11N_RESERVED9,	BYTE_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, Reserved10, ,	11N_RESERVED10,	BYTE_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrlevel5GHT40_1S_A, [MAX_5G_CHANNEL_NUM_MIB],	TX_POWER_5G_HT40_1S_A,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrlevel5GHT40_1S_B, [MAX_5G_CHANNEL_NUM_MIB],	TX_POWER_5G_HT40_1S_B,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff5GHT40_2S, [MAX_5G_CHANNEL_NUM_MIB],	TX_POWER_DIFF_5G_HT40_2S,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff5GHT20, [MAX_5G_CHANNEL_NUM_MIB],	TX_POWER_DIFF_5G_HT20,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff5GOFDM, [MAX_5G_CHANNEL_NUM_MIB],	TX_POWER_DIFF_5G_OFDM,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
#endif
#ifdef WIFI_SIMPLE_CONFIG
MIBDEF(unsigned char, wscPin, [PIN_LEN+1],	WSC_PIN,	STRING_T, HW_WLAN_SETTING_T, 0, 0)
#endif

#endif // #ifdef MIB_HW_WLAN_IMPORT

#ifdef MIB_IMPORT
/* _ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl */
// TCP/IP stuffs
MIBDEF(unsigned char,	ipAddr, [4],	IP_ADDR,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	subnetMask, [4],	SUBNET_MASK,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	defaultGateway, [4],	DEFAULT_GATEWAY,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	dhcp, ,	DHCP,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	dhcpClientStart, [4],	DHCP_CLIENT_START,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	dhcpClientEnd, [4],	DHCP_CLIENT_END,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned long,	dhcpLeaseTime, ,	DHCP_LEASE_TIME, DWORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	elanMacAddr, [6],	ELAN_MAC_ADDR,	BYTE6_T, APMIB_T, 0, 0)
//Brad add for static dhcp
MIBDEF(unsigned char,	dns1,	[4],	DNS1,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	dns2,	[4],	DNS2,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	dns3,	[4],	DNS3,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	stpEnabled,	,	STP_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	deviceName,	[MAX_NAME_LEN],	DEVICE_NAME,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	scrlogEnabled,	,	SCRLOG_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	autoDiscoveryEnabled,	,	AUTO_DISCOVERY_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	domainName,	[MAX_NAME_LEN],	DOMAIN_NAME,	STRING_T, APMIB_T, 0, 0)

// Supervisor of web server account
MIBDEF(unsigned char,	superName,	[MAX_NAME_LEN],	SUPER_NAME,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	superPassword, [MAX_NAME_LEN],	SUPER_PASSWORD,	STRING_T, APMIB_T, 0, 0)

// web server account
MIBDEF(unsigned char,	userName, [MAX_NAME_LEN],	USER_NAME,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	userPassword, [MAX_NAME_LEN],	USER_PASSWORD,	STRING_T, APMIB_T, 0, 0)

#if defined(CONFIG_RTL_8198_AP_ROOT)
MIBDEF(unsigned char,   ntpEnabled, ,   NTP_ENABLED,    BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,   daylightsaveEnabled, ,  DAYLIGHT_SAVE,  BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,   ntpServerId, ,  NTP_SERVER_ID,  BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,   ntpTimeZone, [8],       NTP_TIMEZONE,   STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,   ntpServerIp1, [4],      NTP_SERVER_IP1, IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,   ntpServerIp2, [4],      NTP_SERVER_IP2, IA_T, APMIB_T, 0, 0)
#endif

#ifdef HOME_GATEWAY
MIBDEF(unsigned char,	wanMacAddr, [6],	WAN_MAC_ADDR,	BYTE6_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wanDhcp,	,	WAN_DHCP,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wanIpAddr, [4],	WAN_IP_ADDR,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wanSubnetMask, [4],	WAN_SUBNET_MASK,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wanDefaultGateway, [4],	WAN_DEFAULT_GATEWAY,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	pppUserName, [MAX_NAME_LEN_LONG],	PPP_USER_NAME,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	pppPassword, [MAX_NAME_LEN_LONG],	PPP_PASSWORD,	STRING_T, APMIB_T, 0, 0)
//dzh begin


//MIBDEF(unsigned short,	pppConnectCount,	,PPP_CONNECT_COUNT,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	pppConnectCount,	,	PPP_CONNECT_COUNT,	BYTE_T, APMIB_T, 0, 0)

MIBDEF(unsigned char,	pppUserName2, [MAX_NAME_LEN_LONG],	PPP_USER_NAME2,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	pppPassword2, [MAX_NAME_LEN_LONG],	PPP_PASSWORD2,	STRING_T, APMIB_T, 0, 0)

MIBDEF(unsigned char,	pppUserName3, [MAX_NAME_LEN_LONG],	PPP_USER_NAME3,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	pppPassword3, [MAX_NAME_LEN_LONG],	PPP_PASSWORD3,	STRING_T, APMIB_T, 0, 0)

MIBDEF(unsigned char,	pppUserName4, [MAX_NAME_LEN_LONG],	PPP_USER_NAME4,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	pppPassword4, [MAX_NAME_LEN_LONG],	PPP_PASSWORD4,	STRING_T, APMIB_T, 0, 0)

MIBDEF(unsigned short,	SubNet1Count,	,SUBNET1_COUNT,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	SubNet1_F1_start, [4],SUBNET1_F1_START,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	SubNet1_F1_end,   [4],SUBNET1_F1_END,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	SubNet1_F2_start, [4],SUBNET1_F2_START,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	SubNet1_F2_end,   [4],SUBNET1_F2_END,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	SubNet1_F3_start, [4],SUBNET1_F3_START,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	SubNet1_F3_end,   [4],SUBNET1_F3_END,	IA_T, APMIB_T, 0, 0)	

MIBDEF(unsigned short,	SubNet2Count,	,SUBNET2_COUNT,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	SubNet2_F1_start, [4],SUBNET2_F1_START, IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	SubNet2_F1_end,   [4],SUBNET2_F1_END,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	SubNet2_F2_start, [4],SUBNET2_F2_START, IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	SubNet2_F2_end,   [4],SUBNET2_F2_END,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	SubNet2_F3_start, [4],SUBNET2_F3_START, IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	SubNet2_F3_end,   [4],SUBNET2_F3_END,	IA_T, APMIB_T, 0, 0)

MIBDEF(unsigned short,	SubNet3Count,	,SUBNET3_COUNT, WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	SubNet3_F1_start, [4],SUBNET3_F1_START, IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	SubNet3_F1_end,   [4],SUBNET3_F1_END,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	SubNet3_F2_start, [4],SUBNET3_F2_START, IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	SubNet3_F2_end,   [4],SUBNET3_F2_END,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	SubNet3_F3_start, [4],SUBNET3_F3_START, IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	SubNet3_F3_end,   [4],SUBNET3_F3_END,	IA_T, APMIB_T, 0, 0)

MIBDEF(unsigned short,	SubNet4Count,	,SUBNET4_COUNT,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	SubNet4_F1_start, [4],SUBNET4_F1_START, IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	SubNet4_F1_end,   [4],SUBNET4_F1_END,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	SubNet4_F2_start, [4],SUBNET4_F2_START, IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	SubNet4_F2_end,   [4],SUBNET4_F2_END,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	SubNet4_F3_start, [4],SUBNET4_F3_START, IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	SubNet4_F3_end,   [4],SUBNET4_F3_END,	IA_T, APMIB_T, 0, 0)

MIBDEF(unsigned short,	pppIdleTime2,	,	PPP_IDLE_TIME2,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	pppConnectType2,	,	PPP_CONNECT_TYPE2,	BYTE_T, APMIB_T, 0, 0)

MIBDEF(unsigned short,	pppIdleTime3,	,	PPP_IDLE_TIME3,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	pppConnectType3,	,	PPP_CONNECT_TYPE3,	BYTE_T, APMIB_T, 0, 0)

MIBDEF(unsigned short,	pppIdleTime4,	,	PPP_IDLE_TIME4,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	pppConnectType4,	,	PPP_CONNECT_TYPE4,	BYTE_T, APMIB_T, 0, 0)

MIBDEF(unsigned short,	pppMtuSize2, ,	PPP_MTU_SIZE2,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	pppMtuSize3, ,	PPP_MTU_SIZE3,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	pppMtuSize4, ,	PPP_MTU_SIZE4,	WORD_T, APMIB_T, 0, 0)

MIBDEF(unsigned char,	pppServiceName2, [41],	PPP_SERVICE_NAME2,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	pppServiceName3, [41],	PPP_SERVICE_NAME3,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	pppServiceName4, [41],	PPP_SERVICE_NAME4,	STRING_T, APMIB_T, 0, 0)

MIBDEF(unsigned char,	pppSubNet1, [30],	PPP_SUBNET1,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	pppSubNet2, [30],	PPP_SUBNET2,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	pppSubNet3, [30],	PPP_SUBNET3,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	pppSubNet4, [30],	PPP_SUBNET4,	STRING_T, APMIB_T, 0, 0)

MIBDEF(unsigned short,	pppSessionNum2, ,	PPP_SESSION_NUM2,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	pppServerMac2, [6],	PPP_SERVER_MAC2,	BYTE6_T, APMIB_T, 0, 0)

MIBDEF(unsigned short,	pppSessionNum3, ,	PPP_SESSION_NUM3,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	pppServerMac3, [6],	PPP_SERVER_MAC3,	BYTE6_T, APMIB_T, 0, 0)

MIBDEF(unsigned short,	pppSessionNum4, ,	PPP_SESSION_NUM4,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	pppServerMac4, [6],	PPP_SERVER_MAC4,	BYTE6_T, APMIB_T, 0, 0)

//dzh end

MIBDEF(DNS_TYPE_T,	dnsMode,	,	DNS_MODE,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	pppIdleTime,	,	PPP_IDLE_TIME,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	pppConnectType,	,	PPP_CONNECT_TYPE,	BYTE_T, APMIB_T, 0, 0)

MIBDEF(unsigned char,	dmzEnabled,	,	DMZ_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	dmzHost, [4],	DMZ_HOST,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	upnpEnabled, ,	UPNP_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	pppMtuSize, ,	PPP_MTU_SIZE,	WORD_T, APMIB_T, 0, 0)

MIBDEF(unsigned char,	pptpIpAddr, [4],	PPTP_IP_ADDR,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	pptpSubnetMask, [4],	PPTP_SUBNET_MASK,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	pptpServerIpAddr, [4],	PPTP_SERVER_IP_ADDR,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	pptpUserName, [MAX_NAME_LEN_LONG],	PPTP_USER_NAME,	STRING_T, APMIB_T, 0, 0)

MIBDEF(unsigned char,	pptpPassword, [MAX_NAME_LEN_LONG],	PPTP_PASSWORD,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	pptpMtuSize, ,	PPTP_MTU_SIZE,	WORD_T, APMIB_T, 0, 0)

/* # keith: add l2tp support. 20080515 */
MIBDEF(unsigned char,	l2tpIpAddr, [4],	L2TP_IP_ADDR,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	l2tpSubnetMask, [4],	L2TP_SUBNET_MASK,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	l2tpServerIpAddr, [MAX_PPTP_HOST_NAME_LEN],	L2TP_SERVER_IP_ADDR,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	l2tpGateway, [4],	L2TP_GATEWAY,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	l2tpUserName, [MAX_NAME_LEN_LONG],	L2TP_USER_NAME,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	l2tpPassword, [MAX_NAME_LEN_LONG],	L2TP_PASSWORD,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	l2tpMtuSize, ,	L2TP_MTU_SIZE,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	l2tpIdleTime, ,	L2TP_IDLE_TIME,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	l2tpConnectType, ,	L2TP_CONNECTION_TYPE,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	L2tpwanIPMode, ,	L2TP_WAN_IP_DYNAMIC,	BYTE_T, APMIB_T, 0, 0)
#if defined(CONFIG_DYNAMIC_WAN_IP)
MIBDEF(unsigned char,	l2tpDefGw, [4],	L2TP_DEFAULT_GW,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	pptpDefGw, [4],	PPTP_DEFAULT_GW,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	pptpWanIPMode, ,	PPTP_WAN_IP_DYNAMIC,	BYTE_T, APMIB_T, 0, 0)
#ifdef CONFIG_GET_SERVER_IP_BY_DOMAIN
MIBDEF(unsigned char,	pptpGetServByDomain, ,	PPTP_GET_SERV_BY_DOMAIN,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	pptpServerDomain, [MAX_SERVER_DOMAIN_LEN],  PPTP_SERVER_DOMAIN, STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	l2tpGetServByDomain, ,	L2TP_GET_SERV_BY_DOMAIN,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	l2tpServerDomain, [MAX_SERVER_DOMAIN_LEN],  L2TP_SERVER_DOMAIN, STRING_T, APMIB_T, 0, 0)
#endif
#endif

/* USB3G */
MIBDEF(unsigned char,   usb3g_user,     [32],    USB3G_USER,        STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,   usb3g_pass,     [32],    USB3G_PASS,        STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,   usb3g_pin,      [5],     USB3G_PIN,         STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,   usb3g_apn,      [20],    USB3G_APN,         STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,   usb3g_dialnum,  [12],    USB3G_DIALNUM,     STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,   usb3g_connType, [5],     USB3G_CONN_TYPE,   STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,   usb3g_idleTime, [5] ,    USB3G_IDLE_TIME,   STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,   usb3g_mtuSize,  [5],     USB3G_MTU_SIZE,    STRING_T, APMIB_T, 0, 0)

MIBDEF(unsigned char,	ntpEnabled, ,	NTP_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	daylightsaveEnabled, ,	DAYLIGHT_SAVE,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	ntpServerId, ,	NTP_SERVER_ID,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	ntpTimeZone, [8],	NTP_TIMEZONE,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	ntpServerIp1, [4],	NTP_SERVER_IP1,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	ntpServerIp2, [4],	NTP_SERVER_IP2,	IA_T, APMIB_T, 0, 0)

MIBDEF(unsigned char,	ddnsEnabled, ,	DDNS_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	ddnsType, ,	DDNS_TYPE,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	ddnsDomainName, [MAX_DOMAIN_LEN],	DDNS_DOMAIN_NAME,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	ddnsUser, [MAX_DOMAIN_LEN],	DDNS_USER,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	ddnsPassword, [MAX_NAME_LEN],	DDNS_PASSWORD,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	fixedIpMtuSize, ,	FIXED_IP_MTU_SIZE,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	dhcpMtuSize, ,	DHCP_MTU_SIZE,	WORD_T, APMIB_T, 0, 0)
#endif // HOME_GATEWAY

MIBDEF(unsigned char,	opMode, ,	OP_MODE,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wispWanId, ,	WISP_WAN_ID,	BYTE_T, APMIB_T, 0, 0)

#ifdef HOME_GATEWAY
MIBDEF(unsigned char,	wanAccessEnabled, ,	WEB_WAN_ACCESS_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	pingAccessEnabled, ,	PING_WAN_ACCESS_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	hostName, [MAX_NAME_LEN],	HOST_NAME,	STRING_T, APMIB_T, 0, 0)
#endif // #ifdef HOME_GATEWAY

MIBDEF(unsigned char,	rtLogEnabled, ,	REMOTELOG_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	rtLogServer, [4],	REMOTELOG_SERVER,	IA_T, APMIB_T, 0, 0)

#ifdef UNIVERSAL_REPEATER
// for wlan0 interface
MIBDEF(unsigned char,	repeaterEnabled1, ,	REPEATER_ENABLED1,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	repeaterSSID1, [MAX_SSID_LEN],	REPEATER_SSID1,	STRING_T, APMIB_T, 0, 0)

// for wlan1 interface
MIBDEF(unsigned char,	repeaterEnabled2, ,	REPEATER_ENABLED2,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	repeaterSSID2, [MAX_SSID_LEN],	REPEATER_SSID2,	STRING_T, APMIB_T, 0, 0)
#endif // #ifdef UNIVERSAL_REPEATER

MIBDEF(unsigned char,	wifiSpecific, ,	WIFI_SPECIFIC,	BYTE_T, APMIB_T, 0, 0)

#ifdef HOME_GATEWAY
MIBDEF(unsigned char,	pppServiceName, [41],	PPP_SERVICE_NAME,	STRING_T, APMIB_T, 0, 0)


#ifdef DOS_SUPPORT
MIBDEF(unsigned long,	dosEnabled, ,	DOS_ENABLED,	DWORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	syssynFlood, ,	DOS_SYSSYN_FLOOD,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	sysfinFlood, ,	DOS_SYSFIN_FLOOD,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	sysudpFlood, ,	DOS_SYSUDP_FLOOD,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	sysicmpFlood, ,	DOS_SYSICMP_FLOOD,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	pipsynFlood, ,	DOS_PIPSYN_FLOOD,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	pipfinFlood, ,	DOS_PIPFIN_FLOOD,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	pipudpFlood, ,	DOS_PIPUDP_FLOOD,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	pipicmpFlood, ,	DOS_PIPICMP_FLOOD,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	blockTime, ,	DOS_BLOCK_TIME,	WORD_T, APMIB_T, 0, 0)
#endif // #ifdef DOS_SUPPORT

MIBDEF(unsigned char,	vpnPassthruIPsecEnabled, ,	VPN_PASSTHRU_IPSEC_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	vpnPassthruPPTPEnabled, ,	VPN_PASSTHRU_PPTP_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	vpnPassthruL2TPEnabled, ,	VPN_PASSTHRU_L2TP_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	cusPassThru, ,	CUSTOM_PASSTHRU_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	pptpSecurityEnabled, ,	PPTP_SECURITY_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	igmpproxyDisabled, ,	IGMP_PROXY_DISABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	pptpMppcEnabled, ,	PPTP_MPPC_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	pptpIdleTime, ,	PPTP_IDLE_TIME,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	pptpConnectType, ,	PPTP_CONNECTION_TYPE,	BYTE_T, APMIB_T, 0, 0)
#endif // #ifdef HOME_GATEWAY

MIBDEF(unsigned char,   mibVer, , MIB_VER,    BYTE_T, APMIB_T, 0, 0)

// added by rock /////////////////////////////////////////
#ifdef VOIP_SUPPORT 
MIBDEF(voipCfgParam_t,	voipCfgParam, ,	VOIP_CFG,	VOIP_T, APMIB_T, 0, 0) 
#endif

MIBDEF(unsigned char,	startMp, ,	START_MP,	BYTE_T, APMIB_T, 0, 0)

#ifdef HOME_GATEWAY
#ifdef CONFIG_IPV6
MIBDEF(radvdCfgParam_t,	radvdCfgParam, ,	IPV6_RADVD_PARAM,	RADVDPREFIX_T, APMIB_T, 0, 0)
MIBDEF(dnsv6CfgParam_t,	        dnsCfgParam, ,	IPV6_DNSV6_PARAM,	DNSV6_T, APMIB_T, 0, 0)
MIBDEF(dhcp6sCfgParam_t,	    dhcp6sCfgParam, ,	IPV6_DHCPV6S_PARAM,	DHCPV6S_T, APMIB_T, 0, 0)
MIBDEF(addrIPv6CfgParam_t,	        addrIPv6CfgParam, ,	IPV6_ADDR_PARAM,	ADDR6_T, APMIB_T, 0, 0)
MIBDEF(tunnelCfgParam_t,	    tunnelCfgParam, ,	IPV6_TUNNEL_PARAM,	TUNNEL6_T, APMIB_T, 0, 0)
#endif /* #ifdef CONFIG_IPV6*/
#endif

#ifdef CONFIG_RTL_BT_CLIENT
MIBDEF(unsigned char,	uploadDir, [64] , BT_UPLOAD_DIR,  STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	downloadDir, [64] , BT_DOWNLOAD_DIR,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned int,	uLimit, ,	BT_TOTAL_ULIMIT,  DWORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned int,	dLimit, ,	BT_TOTAL_DLIMIT,	 DWORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	refreshTime, ,	BT_REFRESH_TIME, BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	bt_enabled, ,	BT_ENABLED,	BYTE_T, APMIB_T, 0, 0)
#endif

#if defined(CONFIG_RTL_ULINKER)
MIBDEF(unsigned char,	ulinker_auto,	, ULINKER_AUTO,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	ulinker_cur_mode, , ULINKER_CURRENT_MODE,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	ulinker_lst_mode, , ULINKER_LATEST_MODE,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	ulinker_cur_wl_mode, , ULINKER_CURRENT_WLAN_MODE,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	ulinker_lst_wl_mode, , ULINKER_LATEST_WLAN_MODE,	BYTE_T, APMIB_T, 0, 0)

MIBDEF(unsigned char,	ulinker_repeaterEnabled1, ,	ULINKER_REPEATER_ENABLED1,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	ulinker_repeaterEnabled2, , ULINKER_REPEATER_ENABLED2,	BYTE_T, APMIB_T, 0, 0)

#endif

/*+++++added by Jack for Tr-069 configuration+++++*/
#ifdef CONFIG_APP_TR069

MIBDEF(unsigned char,	cwmp_enabled, ,	CWMP_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	cwmp_ProvisioningCode, [CWMP_PROVISION_CODE_LEN],	CWMP_PROVISIONINGCODE,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	cwmp_ACSURL, [CWMP_ACS_URL_LEN],	CWMP_ACS_URL,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	cwmp_ACSUserName, [CWMP_ACS_USERNAME_LEN],	CWMP_ACS_USERNAME,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	cwmp_ACSPassword, [CWMP_ACS_PASSWD_LEN],	CWMP_ACS_PASSWORD,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	cwmp_InformEnable, ,	CWMP_INFORM_ENABLE,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned int,	cwmp_InformInterval, ,	CWMP_INFORM_INTERVAL,	DWORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned int,	cwmp_InformTime, ,	CWMP_INFORM_TIME,	DWORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	cwmp_ConnReqUserName, [CWMP_CONREQ_USERNAME_LEN],	CWMP_CONREQ_USERNAME,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	cwmp_ConnReqPassword, [CWMP_CONREQ_PASSWD_LEN],	CWMP_CONREQ_PASSWORD,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	cwmp_UpgradesManaged, ,	CWMP_ACS_UPGRADESMANAGED,	BYTE_T, APMIB_T, 0, 0)
#if 0
MIBDEF(unsigned char,	cwmp_LANConfPassword, [CWMP_LANCONF_PASSWD_LEN],	CWMP_LAN_CONFIGPASSWD,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	cwmp_SerialNumber, [CWMP_SERIALNUMBER_LEN],	CWMP_SERIALNUMBER,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	cwmp_DHCP_ServerConf, ,	CWMP_DHCP_SERVERCONF,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	cwmp_LAN_IPIFEnable, ,	CWMP_LAN_IPIFENABLE,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	cwmp_LAN_EthIFEnable, ,	CWMP_LAN_ETHIFENABLE,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	cwmp_WLAN_BasicEncry, ,	CWMP_WLAN_BASICENCRY,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	cwmp_WLAN_WPAEncry, ,	CWMP_WLAN_WPAENCRY,	BYTE_T, APMIB_T, 0, 0)
#endif
MIBDEF(unsigned char,	cwmp_DL_CommandKey, [CWMP_COMMAND_KEY_LEN+1],	CWMP_DL_COMMANDKEY,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned int,	cwmp_DL_StartTime, ,	CWMP_DL_STARTTIME,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned int,	cwmp_DL_CompleteTime, ,	CWMP_DL_COMPLETETIME,	WORD_T, APMIB_T, 0, 0)

MIBDEF(unsigned int,	cwmp_DL_FaultCode, ,	CWMP_DL_FAULTCODE,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned int,	cwmp_Inform_EventCode, ,	CWMP_INFORM_EVENTCODE,	WORD_T, APMIB_T, 0, 0)





MIBDEF(unsigned char,	cwmp_RB_CommandKey, [CWMP_COMMAND_KEY_LEN+1],	CWMP_RB_COMMANDKEY,	STRING_T, APMIB_T, 0, 0)
//MIBDEF(unsigned char,	cwmp_ACS_ParameterKey, [CWMP_COMMAND_KEY_LEN+1],	CWMP_ACS_PARAMETERKEY,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	cwmp_CERT_Password, [CWMP_CERT_PASSWD_LEN+1],	CWMP_CERT_PASSWORD,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	cwmp_Flag, ,	CWMP_FLAG,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	cwmp_SI_CommandKey, [CWMP_COMMAND_KEY_LEN+1],	CWMP_SI_COMMANDKEY,	STRING_T, APMIB_T, 0, 0)

#ifdef _PRMT_USERINTERFACE_
MIBDEF(unsigned char,	UIF_PW_Required, ,	UIF_PW_REQUIRED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	UIF_PW_User_Sel, ,	UIF_PW_USER_SEL,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	UIF_Upgrade, ,	UIF_UPGRADE,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned int,	UIF_WarrantyDate, ,	UIF_WARRANTYDATE,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	UIF_AutoUpdateServer, [256],	UIF_AUTOUPDATESERVER,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	UIF_UserUpdateServer, [256],	UIF_USERUPDATESERVER,	BYTE_T, APMIB_T, 0, 0)
#endif // #ifdef _PRMT_USERINTERFACE_

MIBDEF(unsigned char,	cwmp_ACS_KickURL, [CWMP_KICK_URL],	CWMP_ACS_KICKURL,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	cwmp_ACS_DownloadURL, [CWMP_DOWNLOAD_URL],	CWMP_ACS_DOWNLOADURL,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned int,	cwmp_ConnReqPort, ,	CWMP_CONREQ_PORT,	DWORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	cwmp_ConnReqPath, [CONN_REQ_PATH_LEN],	CWMP_CONREQ_PATH,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	cwmp_Flag2, ,	CWMP_FLAG2,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	cwmp_NotifyList, [CWMP_NOTIFY_LIST_LEN],	CWMP_NOTIFY_LIST,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	cwmp_ACSURL_old, [CWMP_ACS_URL_LEN],	CWMP_ACS_URL_OLD,	STRING_T, APMIB_T, 0, 0)

#ifdef _PRMT_TR143_
MIBDEF(unsigned char,	tr143_udpecho_enable, ,	TR143_UDPECHO_ENABLE,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	tr143_udpecho_itftype, ,	TR143_UDPECHO_ITFTYPE,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	tr143_udpecho_srcip, [4],	TR143_UDPECHO_SRCIP,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	tr143_udpecho_port, ,	TR143_UDPECHO_PORT,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	tr143_udpecho_plus, ,	TR143_UDPECHO_PLUS,	BYTE_T, APMIB_T, 0, 0)
#endif // #ifdef _PRMT_TR143_
#endif // #ifdef CONFIG_APP_TR069

#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
MIBDEF(unsigned char,	vlan_wan_enable,	 				NOREP,	VLAN_WAN_ENALE,						BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	vlan_wan_tag, 						NOREP,	VLAN_WAN_TAG,						WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	vlan_wan_bridge_enable, 			NOREP,	VLAN_WAN_BRIDGE_ENABLE,				BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	vlan_wan_bridge_tag, 				NOREP,	VLAN_WAN_BRIDGE_TAG,				WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	vlan_wan_bridge_port, 				NOREP,	VLAN_WAN_BRIDGE_PORT,				WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	vlan_wan_bridge_multicast_enable, 	NOREP,	VLAN_WAN_BRIDGE_MULTICAST_ENABLE,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	vlan_wan_bridge_multicast_tag, 		NOREP,	VLAN_WAN_BRIDGE_MULTICAST_TAG,		WORD_T, APMIB_T, 0, 0)

MIBDEF(unsigned char,	vlan_wan_host_enable,		NOREP,	VLAN_WAN_HOST_ENABLE,		BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	vlan_wan_host_tag, 			NOREP,	VLAN_WAN_HOST_TAG,			WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	vlan_wan_host_pri, 			NOREP,	VLAN_WAN_HOST_PRI,			WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	vlan_wan_wifi_root_enable,	NOREP,	VLAN_WAN_WIFI_ROOT_ENABLE, 	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	vlan_wan_wifi_root_tag, 	NOREP,	VLAN_WAN_WIFI_ROOT_TAG,		WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	vlan_wan_wifi_root_pri,		NOREP,	VLAN_WAN_WIFI_ROOT_PRI, 	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	vlan_wan_wifi_vap0_enable,	NOREP,	VLAN_WAN_WIFI_VAP0_ENABLE,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	vlan_wan_wifi_vap0_tag, 	NOREP,	VLAN_WAN_WIFI_VAP0_TAG,		WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	vlan_wan_wifi_vap0_pri,		NOREP,	VLAN_WAN_WIFI_VAP0_PRI, 	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	vlan_wan_wifi_vap1_enable,	NOREP,	VLAN_WAN_WIFI_VAP1_ENABLE,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	vlan_wan_wifi_vap1_tag, 	NOREP,	VLAN_WAN_WIFI_VAP1_TAG,		WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	vlan_wan_wifi_vap1_pri,		NOREP,	VLAN_WAN_WIFI_VAP1_PRI, 	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	vlan_wan_wifi_vap2_enable,	NOREP,	VLAN_WAN_WIFI_VAP2_ENABLE,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	vlan_wan_wifi_vap2_tag, 	NOREP,	VLAN_WAN_WIFI_VAP2_TAG,		WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	vlan_wan_wifi_vap2_pri,		NOREP,	VLAN_WAN_WIFI_VAP2_PRI, 	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	vlan_wan_wifi_vap3_enable,	NOREP,	VLAN_WAN_WIFI_VAP3_ENABLE,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	vlan_wan_wifi_vap3_tag, 	NOREP,	VLAN_WAN_WIFI_VAP3_TAG,		WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	vlan_wan_wifi_vap3_pri,		NOREP,	VLAN_WAN_WIFI_VAP3_PRI, 	WORD_T, APMIB_T, 0, 0)
#endif
// SNMP, Forrest added, 2007.10.25.
#ifdef CONFIG_SNMP
MIBDEF(unsigned char,	snmpEnabled, ,	SNMP_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	snmpName, [MAX_SNMP_NAME_LEN],	SNMP_NAME,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	snmpLocation, [MAX_SNMP_LOCATION_LEN],	SNMP_LOCATION,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	snmpContact, [MAX_SNMP_CONTACT_LEN],	SNMP_CONTACT,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	snmpRWCommunity, [MAX_SNMP_COMMUNITY_LEN],	SNMP_RWCOMMUNITY,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	snmpROCommunity, [MAX_SNMP_COMMUNITY_LEN],	SNMP_ROCOMMUNITY,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	snmpTrapReceiver1, [4],	SNMP_TRAP_RECEIVER1,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	snmpTrapReceiver2, [4],	SNMP_TRAP_RECEIVER2,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	snmpTrapReceiver3, [4],	SNMP_TRAP_RECEIVER3,	IA_T, APMIB_T, 0, 0)
#endif // #ifdef CONFIG_SNMP

MIBDEF(unsigned short,	system_time_year, ,	SYSTIME_YEAR,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	system_time_month, ,	SYSTIME_MON,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	system_time_day, ,	SYSTIME_DAY,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	system_time_hour, ,	SYSTIME_HOUR,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	system_time_min, ,	SYSTIME_MIN,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	system_time_sec, ,	SYSTIME_SEC,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlan11nOnOffTKIP, ,	WLAN_11N_ONOFF_TKIP,	BYTE_T, APMIB_T, 0, 0)

MIBDEF(unsigned char,	dhcpRsvdIpEnabled, ,	DHCPRSVDIP_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	dhcpRsvdIpNum, ,	DHCPRSVDIP_TBL_NUM,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(DHCPRSVDIP_T,	dhcpRsvdIpArray, [MAX_DHCP_RSVD_IP_NUM],	DHCPRSVDIP_TBL,	DHCPRSVDIP_ARRY_T, APMIB_T, 0, mib_dhcpRsvdIp_tbl)

#ifdef WLAN_PROFILE
MIBDEF(unsigned char,	wlan_profile_enable1, ,	PROFILE_ENABLED1,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlan_profile_num1, ,	PROFILE_NUM1,		BYTE_T, APMIB_T, 0, 0)
MIBDEF(WLAN_PROFILE_T,	wlan_profile_arrary1, [MAX_WLAN_PROFILE_NUM],PROFILE_TBL1,PROFILE_ARRAY_T, APMIB_T, 0, mib_wlan_profile_tbl1)

MIBDEF(unsigned char,	wlan_profile_enable2, ,	PROFILE_ENABLED2,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlan_profile_num2, ,	PROFILE_NUM2,		BYTE_T, APMIB_T, 0, 0)
MIBDEF(WLAN_PROFILE_T,	wlan_profile_arrary2, [MAX_WLAN_PROFILE_NUM],PROFILE_TBL2,PROFILE_ARRAY_T, APMIB_T, 0, mib_wlan_profile_tbl2)
#endif

#if defined(CONFIG_RTL_8198_AP_ROOT)
MIBDEF(unsigned char,   VlanConfigEnabled, ,    VLANCONFIG_ENABLED,     BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,   VlanConfigNum, ,        VLANCONFIG_TBL_NUM,     BYTE_T, APMIB_T, 0, 0)
#if defined(VLAN_CONFIG_SUPPORTED)
MIBDEF(VLAN_CONFIG_T,   VlanConfigArray, [MAX_IFACE_VLAN_CONFIG],       VLANCONFIG_TBL, VLANCONFIG_ARRAY_T, APMIB_T, 0, mib_vlanconfig_tbl)
#endif
#endif

#ifdef HOME_GATEWAY
MIBDEF(unsigned char,	portFwEnabled, ,	PORTFW_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	portFwNum, ,	PORTFW_TBL_NUM,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(PORTFW_T,	portFwArray, [MAX_FILTER_NUM],	PORTFW_TBL,	PORTFW_ARRAY_T, APMIB_T, 0, mib_portfw_tbl)

MIBDEF(unsigned char,	ipFilterEnabled, ,	IPFILTER_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	ipFilterNum, ,	IPFILTER_TBL_NUM,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(IPFILTER_T,	ipFilterArray, [MAX_FILTER_NUM],	IPFILTER_TBL,	IPFILTER_ARRAY_T, APMIB_T, 0, mib_ipfilter_tbl)

MIBDEF(unsigned char,	portFilterEnabled, ,	PORTFILTER_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	portFilterNum, ,	PORTFILTER_TBL_NUM,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(PORTFILTER_T,	portFilterArray, [MAX_FILTER_NUM],	PORTFILTER_TBL,	PORTFILTER_ARRAY_T, APMIB_T, 0, mib_portfilter_tbl)

MIBDEF(unsigned char,	macFilterEnabled, ,	MACFILTER_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	macFilterNum, ,	MACFILTER_TBL_NUM,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(MACFILTER_T,	macFilterArray, [MAX_FILTER_NUM],	MACFILTER_TBL,	MACFILTER_ARRAY_T, APMIB_T, 0, mib_macfilter_tbl)

MIBDEF(unsigned char,	triggerPortEnabled, ,	TRIGGERPORT_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	triggerPortNum, ,	TRIGGERPORT_TBL_NUM,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(TRIGGERPORT_T,	triggerPortArray, [MAX_FILTER_NUM],	TRIGGERPORT_TBL,	TRIGGERPORT_ARRAY_T, APMIB_T, 0, mib_triggerport_tbl)

MIBDEF(unsigned char,	urlFilterEnabled, ,	URLFILTER_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	urlFilterMode, , URLFILTER_MODE,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	urlFilterNum, ,	URLFILTER_TBL_NUM,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(URLFILTER_T,	urlFilterArray, [MAX_URLFILTER_NUM],	URLFILTER_TBL,	URLFILTER_ARRAY_T, APMIB_T, 0, mib_urlfilter_tbl)

MIBDEF(unsigned char,	VlanConfigEnabled, ,	VLANCONFIG_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	VlanConfigNum, ,	VLANCONFIG_TBL_NUM,	BYTE_T, APMIB_T, 0, 0)
#if defined(VLAN_CONFIG_SUPPORTED)
MIBDEF(VLAN_CONFIG_T,	VlanConfigArray, [MAX_IFACE_VLAN_CONFIG],	VLANCONFIG_TBL,	VLANCONFIG_ARRAY_T, APMIB_T, 0, mib_vlanconfig_tbl)
#endif
#ifdef ROUTE_SUPPORT
MIBDEF(unsigned char,	staticRouteEnabled, ,	STATICROUTE_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	staticRouteNum, ,	STATICROUTE_TBL_NUM,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(STATICROUTE_T,	staticRouteArray, [MAX_ROUTE_NUM],	STATICROUTE_TBL,	STATICROUTE_ARRAY_T, APMIB_T, 0, mib_staticroute_tbl)
MIBDEF(unsigned char,	ripEnabled, ,	RIP_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	ripLanTx, ,	RIP_LAN_TX,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	ripLanRx, ,	RIP_LAN_RX,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	ripWanTx, ,	RIP_WAN_TX,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	ripWanRx, ,	RIP_WAN_RX,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	natEnabled, ,	NAT_ENABLED,	BYTE_T, APMIB_T, 0, 0)
#endif // #ifdef ROUTE_SUPPORT
MIBDEF(unsigned char,	sambaEnabled, ,	SAMBA_ENABLED,	BYTE_T, APMIB_T, 0, 0)
#ifdef VPN_SUPPORT
MIBDEF(unsigned char,	ipsecTunnelEnabled, ,	IPSECTUNNEL_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	ipsecTunnelNum, ,	IPSECTUNNEL_TBL_NUM,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(IPSECTUNNEL_T,	ipsecTunnelArray, [MAX_TUNNEL_NUM],	IPSECTUNNEL_TBL,	IPSECTUNNEL_ARRAY_T, APMIB_T, 0, mib_ipsectunnel_tbl)
MIBDEF(unsigned char,	ipsecNattEnabled, ,	IPSEC_NATT_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	ipsecRsaKeyFile, [MAX_RSA_FILE_LEN],	IPSEC_RSA_FILE,	BYTE_ARRAY_T, APMIB_T, 0, 0)
#endif // #ifdef VPN_SUPPORT


MIBDEF(unsigned short,	pppSessionNum, ,	PPP_SESSION_NUM,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	pppServerMac, [6],	PPP_SERVER_MAC,	BYTE6_T, APMIB_T, 0, 0)
#if defined(PPPOE_DISC_FLOW_PATCH)
MIBDEF(unsigned short,	pppNormalFinish,,	PPP_NORMAL_FINISH, WORD_T, APMIB_T, 0, 0)
#endif

MIBDEF(unsigned char,	l2tpPayload, [MAX_L2TP_BUFF_LEN],	L2TP_PAYLOAD, BYTE_ARRAY_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	l2tpPayloadLength, ,	L2TP_PAYLOAD_LENGTH, WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	l2tpNs, ,	L2TP_NS, WORD_T, APMIB_T, 0, 0)


#endif // #ifdef HOME_GATEWAY

#ifdef TLS_CLIENT
MIBDEF(unsigned char,	certRootNum, ,	CERTROOT_TBL_NUM,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(CERTROOT_T,	certRootArray, [MAX_CERTROOT_NUM],	CERTROOT_TBL,	CERTROOT_ARRAY_T, APMIB_T, 0, mib_certroot_tbl)
MIBDEF(unsigned char,	certUserNum, ,	CERTUSER_TBL_NUM,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(CERTUSER_T,	certUserArray, [MAX_CERTUSER_NUM],	CERTUSER_TBL,	CERTUSER_ARRAY_T, APMIB_T, 0, mib_certuser_tbl)
MIBDEF(unsigned char,	rootIdx, ,	ROOT_IDX,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	userIdx, ,	USER_IDX,	BYTE_T, APMIB_T, 0, 0)
#endif // #ifdef TLS_CLIENT

#if defined(GW_QOS_ENGINE) || defined(QOS_BY_BANDWIDTH)
MIBDEF(unsigned char,	qosEnabled, ,	QOS_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	qosAutoUplinkSpeed, ,	QOS_AUTO_UPLINK_SPEED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned long,	qosManualUplinkSpeed, ,	QOS_MANUAL_UPLINK_SPEED,	DWORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	qosAutoDownLinkSpeed, ,	QOS_AUTO_DOWNLINK_SPEED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned long,	qosManualDownLinkSpeed, ,	QOS_MANUAL_DOWNLINK_SPEED,	DWORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	qosRuleNum, ,	QOS_RULE_TBL_NUM,	BYTE_T, APMIB_T, 0, 0)
#endif // #if defined(GW_QOS_ENGINE) || defined(QOS_BY_BANDWIDTH)

#if defined(GW_QOS_ENGINE)
MIBDEF(QOS_T,	qosRuleArray, [MAX_QOS_RULE_NUM],	QOS_RULE_TBL,	QOS_ARRAY_T, APMIB_T, 0, mib_qos_tbl)
#endif // #if defined(GW_QOS_ENGINE)

#if defined(QOS_BY_BANDWIDTH)
MIBDEF(IPQOS_T,	qosRuleArray, [MAX_QOS_RULE_NUM],	QOS_RULE_TBL,	QOS_ARRAY_T, APMIB_T, 0, mib_qos_tbl)
#endif // #if defined(GW_QOS_ENGINE)

//=========add for MESH=========
MIBDEF(unsigned char,	meshEnabled, ,	MESH_ENABLE,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	meshRootEnabled, ,	MESH_ROOT_ENABLE,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	meshID, [33],	MESH_ID,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	meshMaxNumOfNeighbors, ,	MESH_MAX_NEIGHTBOR,	WORD_T, APMIB_T, 0, 0)

// for backbone security
MIBDEF(unsigned char,	meshEncrypt, ,	MESH_ENCRYPT,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	meshWpaPSKFormat, ,	MESH_PSK_FORMAT,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	meshWpaPSK, [MAX_PSK_LEN+1],	MESH_WPA_PSK,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	meshWpaAuth, ,	MESH_WPA_AUTH,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	meshWpa2Cipher, ,	MESH_WPA2_CIPHER_SUITE,	BYTE_T, APMIB_T, 0, 0)

MIBDEF(unsigned char,	meshAclEnabled, ,	MESH_ACL_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	meshAclNum, ,	MESH_ACL_NUM,	BYTE_T, APMIB_T, 0, 0)
//#if defined(CONFIG_RTK_MESH) && defined(_MESH_ACL_ENABLE_) // below code copy above ACL code
MIBDEF(MACFILTER_T,	meshAclAddrArray, [MAX_MESH_ACL_NUM],	MESH_ACL_ADDR,	MESH_ACL_ARRAY_T, APMIB_T, 0, mib_mech_acl_tbl)
//#endif
#ifdef 	_11s_TEST_MODE_	
MIBDEF(unsigned short,	meshTestParam1, ,	MESH_TEST_PARAM1,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	meshTestParam2, ,	MESH_TEST_PARAM2,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	meshTestParam3, ,	MESH_TEST_PARAM3,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	meshTestParam4, ,	MESH_TEST_PARAM4,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	meshTestParam5, ,	MESH_TEST_PARAM5,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	meshTestParam6, ,	MESH_TEST_PARAM6,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	meshTestParam7, ,	MESH_TEST_PARAM7,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	meshTestParam8, ,	MESH_TEST_PARAM8,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	meshTestParam9, ,	MESH_TEST_PARAM9,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	meshTestParama, ,	MESH_TEST_PARAMA,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	meshTestParamb, ,	MESH_TEST_PARAMB,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	meshTestParamc, ,	MESH_TEST_PARAMC	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	meshTestParamd, ,	MESH_TEST_PARAMD,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	meshTestParame, ,	MESH_TEST_PARAME	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	meshTestParamf, ,	MESH_TEST_PARAMF,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	meshTestParamStr1, [16],	MESH_TEST_PARAMSTR1,	STRING_T, APMIB_T, 0, 0)
#endif // #ifdef 	_11s_TEST_MODE_	

MIBDEF(unsigned char,	snmpROcommunity, [MAX_SNMP_COMMUNITY_LEN],	SNMP_RO_COMMUNITY,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	snmpRWcommunity, [MAX_SNMP_COMMUNITY_LEN],	SNMP_RW_COMMUNITY,	STRING_T, APMIB_T, 0, 0)

MIBDEF(CONFIG_WLAN_SETTING_T,	wlan, [NUM_WLAN_INTERFACE][NUM_VWLAN_INTERFACE+1],	WLAN_ROOT,	TABLE_LIST_T, APMIB_T, 0, mib_wlan_table)

//#ifdef CONFIG_RTL_FLASH_DUAL_IMAGE_ENABLE
MIBDEF(unsigned char,	dualBankEnabled,	, DUALBANK_ENABLED,	BYTE_T, APMIB_T, 0, 0) //default test
MIBDEF(unsigned char,	wlanBand2G5GSelect,	, WLAN_BAND2G5G_SELECT,	BYTE_T, APMIB_T, 0, 0)

#endif // #ifdef MIB_IMPORT

#ifdef MIB_DHCPRSVDIP_IMPORT
/* _ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl */
MIBDEF(unsigned char,	ipAddr, [4],	DHCPRSVDIP_IPADDR,	IA_T, DHCPRSVDIP_T, 0, 0)
MIBDEF(unsigned char,	macAddr,	[6],	DHCPRSVDIP_MACADDR,	BYTE6_T, DHCPRSVDIP_T, 0, 0)
MIBDEF(unsigned char,	hostName, [32],	DHCPRSVDIP_HOSTNAME,	STRING_T, DHCPRSVDIP_T, 0, 0)
#endif // #ifdef MIB_DHCPRSVDIP_IMPORT

#ifdef WLAN_PROFILE
#ifdef MIB_WLAN_PROFILE_IMPORT
/* _ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl */
MIBDEF(unsigned char,	ssid, [MAX_SSID_LEN],	PROFILE_SSID,	STRING_T,	WLAN_PROFILE_T, 0, 0)
MIBDEF(unsigned char,	encryption, ,			PROFILE_ENC,	BYTE_T, 	WLAN_PROFILE_T, 0, 0)
MIBDEF(unsigned char,	auth, ,					PROFILE_AUTH,	BYTE_T, 	WLAN_PROFILE_T, 0, 0)
MIBDEF(unsigned char,	wpa_cipher, , 			PROFILE_WPA_CIPHER,	BYTE_T, WLAN_PROFILE_T, 0, 0)
MIBDEF(unsigned char,	wpaPSK, [MAX_PSK_LEN+1],PROFILE_WPA_PSK,STRING_T, 	WLAN_PROFILE_T, 0, 0)
MIBDEF(unsigned char,	wep_default_key, ,		PROFILE_WEP_DEFAULT_KEY,BYTE_T,WLAN_PROFILE_T, 0, 0)
MIBDEF(unsigned char,	wepKey1, [WEP128_KEY_LEN],PROFILE_WEP_KEY1,BYTE_ARRAY_T, WLAN_PROFILE_T, 0, 0)
MIBDEF(unsigned char,	wepKey2, [WEP128_KEY_LEN],PROFILE_WEP_KEY2,BYTE_ARRAY_T, WLAN_PROFILE_T, 0, 0)
MIBDEF(unsigned char,	wepKey3, [WEP128_KEY_LEN],PROFILE_WEP_KEY3,BYTE_ARRAY_T, WLAN_PROFILE_T, 0, 0)
MIBDEF(unsigned char,	wepKey4, [WEP128_KEY_LEN],PROFILE_WEP_KEY4,BYTE_ARRAY_T, WLAN_PROFILE_T, 0, 0)
MIBDEF(unsigned char,	wepKeyType, ,	PROFILE_WEP_KEY_TYPE,	BYTE_T, WLAN_PROFILE_T, 0, 0)
MIBDEF(unsigned char,	wpaPSKFormat, ,	PROFILE_PSK_FORMAT,	BYTE_T, WLAN_PROFILE_T, 0, 0)
#endif // #ifdef MIB_WLAN_PROFILE_IMPORT
#endif

#ifdef MIB_SCHEDULE_IMPORT
/* _ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl */
MIBDEF(unsigned char,	text, [SCHEDULE_NAME_LEN],	SCHEDULE_TEXT,	STRING_T, SCHEDULE_T, 0, 0)
MIBDEF(unsigned short,	eco,	,	SCHEDULE_ECO,	WORD_T, SCHEDULE_T, 0, 0)
MIBDEF(unsigned short,	fTime, ,	SCHEDULE_FTIME,	WORD_T, SCHEDULE_T, 0, 0)
MIBDEF(unsigned short,	tTime,	,	SCHEDULE_TTIME,	WORD_T, SCHEDULE_T, 0, 0)
MIBDEF(unsigned short,	day,	,	SCHEDULE_DAY,	WORD_T, SCHEDULE_T, 0, 0)
#endif // #ifdef MIB_SCHEDULE_IMPORT

#ifdef MIB_MACFILTER_IMPORT
/* _ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl */
MIBDEF(unsigned char,	macAddr, [6],	MACFILTER_MACADDR,	BYTE6_T, MACFILTER_T, 0, 0)
MIBDEF(unsigned char,	comment, [COMMENT_LEN],	MACFILTER_COMMENT,	STRING_T, MACFILTER_T, 0, 0)
#endif // #ifdef MIB_MACFILTER_IMPORT

#if defined(CONFIG_RTL_8198_AP_ROOT)
#ifdef MIB_VLAN_CONFIG_IMPORT
/* _ctype,      _cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl */
MIBDEF(unsigned char,   enabled, ,      VLANCONFIG_ENTRY_ENABLED,       BYTE_T, VLAN_CONFIG_T, 0, 0)
MIBDEF(unsigned char,   netIface, [IFNAMSIZE],  VLANCONFIG_NETIFACE,    STRING_T, VLAN_CONFIG_T, 0, 0)
MIBDEF(unsigned char,   tagged, ,       VLANCONFIG_TAGGED,      BYTE_T, VLAN_CONFIG_T, 0, 0)
//MIBDEF(unsigned char, untagged, ,     VLANCONFIG_UNTAGGED,    BYTE_T, VLAN_CONFIG_T, 0, 0)
MIBDEF(unsigned char,   priority, ,     VLANCONFIG_PRIORITY,    BYTE_T, VLAN_CONFIG_T, 0, 0)
MIBDEF(unsigned char,   cfi, ,  VLANCONFIG_CFI, BYTE_T, VLAN_CONFIG_T, 0, 0)
//MIBDEF(unsigned char, groupId, ,      VLANCONFIG_GROUPID,     BYTE_T, VLAN_CONFIG_T, 0, 0)
MIBDEF(unsigned short,  vlanId, ,       VLANCONFIG_VLANID,      WORD_T, VLAN_CONFIG_T, 0, 0)
#if defined(CONFIG_RTK_VLAN_NEW_FEATURE) ||defined(CONFIG_RTL_HW_VLAN_SUPPORT)
MIBDEF(unsigned char,	forwarding_rule, ,	VLANCONFIG_FORWARDING_RULE, BYTE_T, VLAN_CONFIG_T, 0, 0)
#endif
#endif // #ifdef MIB_VLAN_CONFIG_IMPORT
#endif

#ifdef HOME_GATEWAY
#ifdef MIB_PORTFW_IMPORT
/* _ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl */
MIBDEF(unsigned char,	ipAddr, [4],	PORTFW_IPADDR,	IA_T, PORTFW_T, 0, 0)
MIBDEF(unsigned short,	fromPort,	,	PORTFW_FROMPORT,	WORD_T, PORTFW_T, 0, 0)
MIBDEF(unsigned short,	toPort, ,	PORTFW_TOPORT,	WORD_T, PORTFW_T, 0, 0)
MIBDEF(unsigned char,	protoType, ,	PORTFW_PROTOTYPE,	BYTE_T, PORTFW_T, 0, 0)
MIBDEF(unsigned char,	comment,	 [COMMENT_LEN],	PORTFW_COMMENT,	STRING_T, PORTFW_T, 0, 0)
#endif // #ifdef MIB_PORTFW_IMPORT

#ifdef MIB_IPFILTER_IMPORT
/* _ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl */
MIBDEF(unsigned char,	ipAddr, [4],	IPFILTER_IPADDR,	IA_T, IPFILTER_T, 0, 0)
MIBDEF(unsigned char,	protoType,	,	IPFILTER_PROTOTYPE,	BYTE_T, IPFILTER_T, 0, 0)
MIBDEF(unsigned char,	comment, [COMMENT_LEN],	IPFILTER_COMMENT,	STRING_T, IPFILTER_T, 0, 0)
#endif // #ifdef MIB_IPFILTER_IMPORT

#ifdef MIB_PORTFILTER_IMPORT
/* _ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl */
MIBDEF(unsigned short,	fromPort, ,	PORTFILTER_FROMPORT,	WORD_T, PORTFILTER_T, 0, 0)
MIBDEF(unsigned short,	toPort, ,	PORTFILTER_TOPORT,	WORD_T, PORTFILTER_T, 0, 0)
MIBDEF(unsigned char,	protoType, ,	PORTFILTER_PROTOTYPE,	BYTE_T, PORTFILTER_T, 0, 0)
MIBDEF(unsigned char,	comment, [COMMENT_LEN],	PORTFILTER_COMMENT,	STRING_T, PORTFILTER_T, 0, 0)
#endif // #ifdef MIB_PORTFILTER_IMPORT

#ifdef MIB_TRIGGERPORT_IMPORT
/* _ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl */
MIBDEF(unsigned short,	tri_fromPort, ,	TRIGGERPORT_TRI_FROMPORT,	WORD_T, TRIGGERPORT_T, 0, 0)
MIBDEF(unsigned short,	tri_toPort, ,	TRIGGERPORT_TRI_TOPORT,	WORD_T, TRIGGERPORT_T, 0, 0)
MIBDEF(unsigned char,	tri_protoType, ,	TRIGGERPORT_TRI_PROTOTYPE,	BYTE_T, TRIGGERPORT_T, 0, 0)
MIBDEF(unsigned short,	inc_fromPort, ,	TRIGGERPORT_INC_FROMPORT,	WORD_T, TRIGGERPORT_T, 0, 0)
MIBDEF(unsigned short,	inc_toPort, ,	TRIGGERPORT_INC_TOPORT,	WORD_T, TRIGGERPORT_T, 0, 0)
MIBDEF(unsigned char,	inc_protoType, ,	TRIGGERPORT_INC_PROTOTYPE,	BYTE_T, TRIGGERPORT_T, 0, 0)
MIBDEF(unsigned char,	comment, [COMMENT_LEN],	TRIGGERPORT_COMMENT,	STRING_T, TRIGGERPORT_T, 0, 0)
#endif // #ifdef MIB_TRIGGERPORT_IMPORT

#ifdef MIB_URLFILTER_IMPORT
/* _ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl */
MIBDEF(unsigned char,	urlAddr, [31],	URLFILTER_URLADDR,	STRING_T, URLFILTER_T, 0, 0)
MIBDEF(unsigned char,	ruleMode, ,	URLFILTER_RULE_MODE,	BYTE_T, URLFILTER_T, 0, 0)
#endif // #ifdef MIB_URLFILTER_IMPORT

#ifdef MIB_VLAN_CONFIG_IMPORT
/* _ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl */
MIBDEF(unsigned char,	enabled, ,	VLANCONFIG_ENTRY_ENABLED,	BYTE_T, VLAN_CONFIG_T, 0, 0)
MIBDEF(unsigned char,	netIface, [IFNAMSIZE],	VLANCONFIG_NETIFACE,	STRING_T, VLAN_CONFIG_T, 0, 0)
MIBDEF(unsigned char,	tagged, ,	VLANCONFIG_TAGGED,	BYTE_T, VLAN_CONFIG_T, 0, 0)
//MIBDEF(unsigned char,	untagged, ,	VLANCONFIG_UNTAGGED,	BYTE_T, VLAN_CONFIG_T, 0, 0)
MIBDEF(unsigned char,	priority, ,	VLANCONFIG_PRIORITY,	BYTE_T, VLAN_CONFIG_T, 0, 0)
MIBDEF(unsigned char,	cfi, ,	VLANCONFIG_CFI,	BYTE_T, VLAN_CONFIG_T, 0, 0)
//MIBDEF(unsigned char,	groupId, ,	VLANCONFIG_GROUPID,	BYTE_T, VLAN_CONFIG_T, 0, 0)
MIBDEF(unsigned short,	vlanId, ,	VLANCONFIG_VLANID,	WORD_T, VLAN_CONFIG_T, 0, 0)
#if defined(CONFIG_RTK_VLAN_NEW_FEATURE) ||defined(CONFIG_RTL_HW_VLAN_SUPPORT)
MIBDEF(unsigned char,	forwarding_rule, ,	VLANCONFIG_FORWARDING_RULE, BYTE_T, VLAN_CONFIG_T, 0, 0)
#endif
#endif // #ifdef MIB_VLAN_CONFIG_IMPORT

#ifdef ROUTE_SUPPORT
#ifdef MIB_STATICROUTE_IMPORT
/* _ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl */
MIBDEF(unsigned char,	dstAddr, [4],	STATICROUTE_DSTADDR,	IA_T, STATICROUTE_T, 0, 0)
MIBDEF(unsigned char,	netmask, [4],	STATICROUTE_NETMASK,	IA_T, STATICROUTE_T, 0, 0)
MIBDEF(unsigned char,	gateway, [4],	STATICROUTE_GATEWAY,	IA_T, STATICROUTE_T, 0, 0)
MIBDEF(unsigned char,	interface, ,	STATICROUTE_INTERFACE,	BYTE_T, STATICROUTE_T, 0, 0)
MIBDEF(unsigned char,	metric, ,	STATICROUTE_METRIC,	BYTE_T, STATICROUTE_T, 0, 0)
#endif // #ifdef MIB_STATICROUTE_IMPORT
#endif // #ifdef ROUTE_SUPPORT

#ifdef VPN_SUPPORT
#ifdef MIB_IPSECTUNNEL_IMPORT
/* _ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl */
MIBDEF(unsigned char,	tunnelId, ,	IPSECTUNNEL_TUNNELID,	IA_T, IPSECTUNNEL_T, 0, 0)
MIBDEF(unsigned char,	authType, ,	IPSECTUNNEL_AUTHTYPE,	IA_T, IPSECTUNNEL_T, 0, 0)
//local info
MIBDEF(unsigned char,	lcType, ,	IPSECTUNNEL_LCTYPE,	IA_T, IPSECTUNNEL_T, 0, 0)
MIBDEF(unsigned char,	lc_ipAddr, [4],	IPSECTUNNEL_LC_IPADDR,	IA_T, IPSECTUNNEL_T, 0, 0)
MIBDEF(unsigned char,	lc_maskLen, ,	IPSECTUNNEL_LC_MASKLEN,	BYTE_T, IPSECTUNNEL_T, 0, 0)
//remote Info
MIBDEF(unsigned char,	rtType, ,	IPSECTUNNEL_RTTYPE,	IA_T, IPSECTUNNEL_T, 0, 0)
MIBDEF(unsigned char,	rt_ipAddr, [4],	IPSECTUNNEL_RT_IPADDR,	IA_T, IPSECTUNNEL_T, 0, 0)
MIBDEF(unsigned char,	rt_maskLen, ,	IPSECTUNNEL_RT_MASKLEN,	BYTE_T, IPSECTUNNEL_T, 0, 0)
MIBDEF(unsigned char,	rt_gwAddr, [4],	IPSECTUNNEL_RT_GWADDR,	IA_T, IPSECTUNNEL_T, 0, 0)
// Key mode common
MIBDEF(unsigned char,	keyMode, ,	IPSECTUNNEL_KEYMODE,	BYTE_T, IPSECTUNNEL_T, 0, 0)
//MIBDEF(unsigned char,	espAh, ,	IPSECTUNNEL_ESPAH,	BYTE_T, IPSECTUNNEL_T, 0, 0)
MIBDEF(unsigned char,	espEncr, ,	IPSECTUNNEL_ESPENCR,	BYTE_T, IPSECTUNNEL_T, 0, 0)
MIBDEF(unsigned char,	espAuth, ,	IPSECTUNNEL_ESPAUTH,	BYTE_T, IPSECTUNNEL_T, 0, 0)
//MIBDEF(unsigned char,	ahAuth, ,	IPSECTUNNEL_AHAUTH,	BYTE_T, IPSECTUNNEL_T, 0, 0)
//IKE mode
MIBDEF(unsigned char,	conType, ,	IPSECTUNNEL_CONTYPE,	BYTE_T, IPSECTUNNEL_T, 0, 0)
MIBDEF(unsigned char,	psKey, [MAX_NAME_LEN],	IPSECTUNNEL_PSKEY,	STRING_T, IPSECTUNNEL_T, 0, 0)
MIBDEF(unsigned char,	rsaKey, [MAX_RSA_KEY_LEN],	IPSECTUNNEL_RSAKEY,	STRING_T, IPSECTUNNEL_T, 0, 0)
//Manual Mode
MIBDEF(unsigned char,	spi, [MAX_SPI_LEN],	IPSECTUNNEL_SPI,	STRING_T, IPSECTUNNEL_T, 0, 0)
MIBDEF(unsigned char,	encrKey, [MAX_ENCRKEY_LEN],	IPSECTUNNEL_ENCRKEY,	STRING_T, IPSECTUNNEL_T, 0, 0)
MIBDEF(unsigned char,	authKey, [MAX_AUTHKEY_LEN],	IPSECTUNNEL_AUTHKEY,	STRING_T, IPSECTUNNEL_T, 0, 0)
// tunnel info
MIBDEF(unsigned char,	enable, ,	IPSECTUNNEL_ENABLE,	BYTE_T, IPSECTUNNEL_T, 0, 0)
MIBDEF(unsigned char,	connName, [MAX_NAME_LEN],	IPSECTUNNEL_CONNNAME,	STRING_T, IPSECTUNNEL_T, 0, 0)
MIBDEF(unsigned char,	lcIdType, ,	IPSECTUNNEL_LCIDTYPE,	BYTE_T, IPSECTUNNEL_T, 0, 0)
MIBDEF(unsigned char,	rtIdType, ,	IPSECTUNNEL_LCIDTYPE,	BYTE_T, IPSECTUNNEL_T, 0, 0)
MIBDEF(unsigned char,	lcId, [MAX_NAME_LEN],	IPSECTUNNEL_LCID,	STRING_T, IPSECTUNNEL_T, 0, 0)
MIBDEF(unsigned char,	rtId, [MAX_NAME_LEN],	IPSECTUNNEL_RTID,	STRING_T, IPSECTUNNEL_T, 0, 0)
// ike Advanced setup
MIBDEF(unsigned long,	ikeLifeTime, ,	IPSECTUNNEL_IKELIFETIME,	DWORD_T, IPSECTUNNEL_T, 0, 0)
MIBDEF(unsigned char,	ikeEncr, ,	IPSECTUNNEL_IKEENCR,	BYTE_T, IPSECTUNNEL_T, 0, 0)
MIBDEF(unsigned char,	ikeAuth, ,	IPSECTUNNEL_IKEAUTH,	BYTE_T, IPSECTUNNEL_T, 0, 0)
MIBDEF(unsigned char,	ikeKeyGroup, ,	IPSECTUNNEL_IKEKEYGROUP,	BYTE_T, IPSECTUNNEL_T, 0, 0)
MIBDEF(unsigned long,	ipsecLifeTime, ,	IPSECTUNNEL_IPSECLIFETIME,	DWORD_T, IPSECTUNNEL_T, 0, 0)
MIBDEF(unsigned char,	ipsecPfs, ,	IPSECTUNNEL_IPSECPFS,	BYTE_T, IPSECTUNNEL_T, 0, 0)
#endif // #ifdef MIB_IPSECTUNNEL_IMPORT
#endif //#ifdef VPN_SUPPORT
#endif // #ifdef HOME_GATEWAY

#ifdef TLS_CLIENT
#ifdef MIB_CERTROOT_IMPORT
/* _ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl */
MIBDEF(unsigned char,	comment, [COMMENT_LEN],	CERTROOT_COMMENT,	STRING_T, CERTROOT_T, 0, 0)
#endif // #ifdef MIB_CERTROOT_IMPORT

#ifdef MIB_CERTUSER_IMPORT
/* _ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl */
MIBDEF(unsigned char,	comment, [COMMENT_LEN],	CERTUSER_COMMENT,	STRING_T, CERTUSER_T, 0, 0)
MIBDEF(unsigned char,	pass, [MAX_RS_PASS_LEN],	CERTROOT_PASS,	STRING_T, CERTUSER_T, 0, 0)
#endif // #ifdef MIB_CERTUSER_IMPORT
#endif //#ifdef TLS_CLIENT

#if defined(GW_QOS_ENGINE)
#ifdef MIB_QOS_IMPORT
/* _ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl */
MIBDEF(unsigned char,	entry_name, [MAX_QOS_NAME_LEN+1],	QOS_ENTRY_NAME,	STRING_T, QOS_T, 0, 0)
MIBDEF(unsigned char,	enabled, ,	QOS_ENTRY_ENABLED,	STRING_T, QOS_T, 0, 0)
MIBDEF(unsigned char,	priority, ,	QOS_PRIORITY,	STRING_T, QOS_T, 0, 0)
MIBDEF(unsigned short,	protocol, ,	QOS_PROTOCOL,	WORD_T, QOS_T, 0, 0)
MIBDEF(unsigned char,	local_ip_start, [4],	QOS_LOCAL_IP_START,	IA_T, QOS_T, 0, 0)
MIBDEF(unsigned char,	local_ip_end, [4],	QOS_LOCAL_IP_END,	IA_T, QOS_T, 0, 0)
MIBDEF(unsigned short,	local_port_start, ,	QOS_LOCAL_PORT_START,	WORD_T, QOS_T, 0, 0)
MIBDEF(unsigned short,	local_port_end, ,	QOS_LOCAL_PORT_END,	WORD_T, QOS_T, 0, 0)
MIBDEF(unsigned char,	remote_ip_start, [4],	QOS_REMOTE_IP_START,	IA_T, QOS_T, 0, 0)
MIBDEF(unsigned char,	remote_ip_end, [4],	QOS_REMOTE_IP_END,	IA_T, QOS_T, 0, 0)
MIBDEF(unsigned short,	remote_port_start, ,	QOS_REMOTE_PORT_START,	WORD_T, QOS_T, 0, 0)
MIBDEF(unsigned short,	remote_port_send, ,	QOS_REMOTE_PORT_END,	WORD_T, QOS_T, 0, 0)

#endif // #ifdef MIB_QOS_IMPORT
#endif // #if defined(GW_QOS_ENGINE)

#if defined(QOS_BY_BANDWIDTH)
#ifdef MIB_IPQOS_IMPORT
/* _ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl */
MIBDEF(unsigned char,	entry_name, [MAX_QOS_NAME_LEN+1],	IPQOS_ENTRY_NAME,	STRING_T, IPQOS_T, 0, 0)
MIBDEF(unsigned char,	enabled, ,	IPQOS_ENABLED,	BYTE_T, IPQOS_T, 0, 0)
MIBDEF(unsigned char,	mac, [MAC_ADDR_LEN],	IPQOS_MAC,	BYTE6_T, IPQOS_T, 0, 0)
MIBDEF(unsigned char,	mode, ,	IPQOS_MODE,	BYTE_T, IPQOS_T, 0, 0)
MIBDEF(unsigned char,	local_ip_start, [4],	IPQOS_LOCAL_IP_START,	IA_T, IPQOS_T, 0, 0)
MIBDEF(unsigned char,	local_ip_end, [4],	IPQOS_LOCAL_IP_END,	IA_T, IPQOS_T, 0, 0)
MIBDEF(unsigned long,	bandwidth, ,	IPQOS_BANDWIDTH,	DWORD_T, IPQOS_T, 0, 0)
MIBDEF(unsigned long,	bandwidth_downlink, ,	IPQOS_BANDWIDTH_DOWNLINK,	DWORD_T, IPQOS_T, 0, 0)
MIBDEF(unsigned char,	l7_protocol, [64+1],	IPQOS_LAYER7_PROTOCOL,	STRING_T, IPQOS_T, 0, 0)
#endif // #ifdef MIB_IPQOS_IMPORT
#endif // #if defined(QOS_BY_BANDWIDTH)

#ifdef MIB_MESH_MACFILTER_IMPORT
/* _ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl */
MIBDEF(unsigned char,	macAddr, [6],	MECH_ACL_MACADDR,	BYTE6_T, MACFILTER_T, 0, 0)
MIBDEF(unsigned char,	comment, [COMMENT_LEN],	MECH_ACL_COMMENT,	STRING_T, MACFILTER_T, 0, 0)
#endif // #ifdef MIB_MESH_MACFILTER_IMPORT

#ifdef MIB_WLAN_MACFILTER_IMPORT
/* _ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl */
MIBDEF(unsigned char,	macAddr, [6],	WLAN_ACL_ADDR_MACADDR,	BYTE6_T, MACFILTER_T, 0, 0)
MIBDEF(unsigned char,	comment, [COMMENT_LEN],	WLAN_ACL_ADDR_COMMENT,	STRING_T, MACFILTER_T, 0, 0)
#endif // #ifdef MIB_WLAN_MACFILTER_IMPORT

#ifdef MIB_WDS_IMPORT
/* _ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl */
MIBDEF(unsigned char,	macAddr, [6],	WLAN_WDS_MACADDR,	BYTE6_T, WDS_T, 0, 0)
MIBDEF(unsigned int,	fixedTxRate, ,	WLAN_WDS_FIXEDTXRATE,	DWORD_T, WDS_T, 0, 0)
MIBDEF(unsigned char,	comment, [COMMENT_LEN],	WLAN_WDS_COMMENT,	STRING_T, WDS_T, 0, 0)
#endif // #ifdef MIB_WDS_IMPORT

#ifdef MIB_CONFIG_WLAN_SETTING_IMPORT
/* _ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl */
MIBDEF(unsigned char,	ssid, [MAX_SSID_LEN],	SSID,	STRING_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	channel, ,	CHANNEL,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wlanMacAddr, [6],	WLAN_MAC_ADDR,	BYTE6_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wep, ,	WEP,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
//MIBDEF(unsigned char,	wep64Key, [WEP64_KEY_LEN],	WEP64_KEY,	BYTE5_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wep64Key1, [WEP64_KEY_LEN],	WEP64_KEY1,	BYTE5_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wep64Key2, [WEP64_KEY_LEN],	WEP64_KEY2,	BYTE5_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wep64Key3, [WEP64_KEY_LEN],	WEP64_KEY3,	BYTE5_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wep64Key4, [WEP64_KEY_LEN],	WEP64_KEY4,	BYTE5_T, CONFIG_WLAN_SETTING_T, 0, 0)
//MIBDEF(unsigned char,	wep128Key, [WEP128_KEY_LEN],	WEP128_KEY,	BYTE13_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wep128Key1, [WEP128_KEY_LEN],	WEP128_KEY1,	BYTE13_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wep128Key2, [WEP128_KEY_LEN],	WEP128_KEY2,	BYTE13_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wep128Key3, [WEP128_KEY_LEN],	WEP128_KEY3,	BYTE13_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wep128Key4, [WEP128_KEY_LEN],	WEP128_KEY4,	BYTE13_T, CONFIG_WLAN_SETTING_T, 0, 0)

MIBDEF(unsigned char,	wepDefaultKey, ,	WEP_DEFAULT_KEY,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wepKeyType, ,	WEP_KEY_TYPE,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)

MIBDEF(unsigned short,	fragThreshold, ,	FRAG_THRESHOLD,	WORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned short,	rtsThreshold, ,	RTS_THRESHOLD,	WORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned short,	supportedRates, ,	SUPPORTED_RATES,	WORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned short,	basicRates, ,	BASIC_RATES,	WORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned short,	beaconInterval, ,	BEACON_INTERVAL,	WORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	preambleType, ,	PREAMBLE_TYPE,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	authType, ,	AUTH_TYPE,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,   ackTimeout, , ACK_TIMEOUT, BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)

MIBDEF(unsigned char,	acEnabled, ,	MACAC_ENABLED,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	acNum, ,	MACAC_NUM,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(MACFILTER_T,	acAddrArray, [MAX_WLAN_AC_NUM],	MACAC_ADDR,	WLAC_ARRAY_T, CONFIG_WLAN_SETTING_T, 0, wlan_acl_addr_tbl)

MIBDEF(unsigned char,	scheduleRuleEnabled, ,	SCHEDULE_ENABLED,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	scheduleRuleNum, ,	SCHEDULE_TBL_NUM,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(SCHEDULE_T,	scheduleRuleArray, [MAX_SCHEDULE_NUM],	SCHEDULE_TBL,	SCHEDULE_ARRAY_T, CONFIG_WLAN_SETTING_T, 0, mib_schedule_tbl)

MIBDEF(unsigned char,	hiddenSSID, ,	HIDDEN_SSID,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wlanDisabled, ,	WLAN_DISABLED,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned long,	inactivityTime, ,	INACTIVITY_TIME,	DWORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	rateAdaptiveEnabled, ,	RATE_ADAPTIVE_ENABLED,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	dtimPeriod, ,	DTIM_PERIOD,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wlanMode, ,	MODE,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	networkType, ,	NETWORK_TYPE,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	iappDisabled, ,	IAPP_DISABLED,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	protectionDisabled, ,	PROTECTION_DISABLED,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	defaultSsid, [MAX_SSID_LEN],	DEFAULT_SSID,	STRING_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	blockRelay, ,	BLOCK_RELAY,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	maccloneEnabled, ,	MACCLONE_ENABLED,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wlanBand, ,	BAND,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned int,	fixedTxRate, ,	FIX_RATE,	DWORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	turboMode, ,	TURBO_MODE,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	RFPowerScale, ,	RFPOWER_SCALE,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)

// WPA stuffs
MIBDEF(unsigned char,	encrypt, ,	ENCRYPT,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	enableSuppNonWpa, ,	ENABLE_SUPP_NONWPA,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	suppNonWpa, ,	SUPP_NONWPA,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wpaAuth, ,	WPA_AUTH,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wpaCipher, ,	WPA_CIPHER_SUITE,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wpaPSK, [MAX_PSK_LEN+1],	WPA_PSK,	STRING_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned long,	rs_reauth_to, ,	RS_REAUTH_TO,	DWORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned long,	wpaGroupRekeyTime, ,	WPA_GROUP_REKEY_TIME,	DWORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	rsIpAddr, [4],	RS_IP,	IA_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned short,	rsPort, ,	RS_PORT,	WORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	rsPassword, [MAX_RS_PASS_LEN],	RS_PASSWORD,	STRING_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,   rs2IpAddr, [4],  RS2_IP,  IA_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned short,  rs2Port, ,       RS2_PORT,        WORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,   rs2Password, [MAX_RS_PASS_LEN],  RS2_PASSWORD,    STRING_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	enable1X, ,	ENABLE_1X,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wpaPSKFormat, ,	PSK_FORMAT,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	accountRsEnabled, ,	ACCOUNT_RS_ENABLED,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	accountRsIpAddr, [4],	ACCOUNT_RS_IP,	IA_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned short,	accountRsPort, ,	ACCOUNT_RS_PORT,	WORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	accountRsPassword, [MAX_RS_PASS_LEN],	ACCOUNT_RS_PASSWORD,	STRING_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	accountRs2IpAddr, [4],	ACCOUNT_RS2_IP,	IA_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned short,	accountRs2Port, ,	ACCOUNT_RS2_PORT,	WORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	accountRs2Password, [MAX_RS_PASS_LEN],	ACCOUNT_RS2_PASSWORD,	STRING_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	accountRsUpdateEnabled, ,	ACCOUNT_RS_UPDATE_ENABLED,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned short,	accountRsUpdateDelay, ,	ACCOUNT_RS_UPDATE_DELAY,	WORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	macAuthEnabled, ,	MAC_AUTH_ENABLED,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	rsMaxRetry, ,	RS_MAXRETRY,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned short,	rsIntervalTime, ,	RS_INTERVAL_TIME,	WORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	accountRsMaxRetry, ,	ACCOUNT_RS_MAXRETRY,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned short,	accountRsIntervalTime, ,	ACCOUNT_RS_INTERVAL_TIME,	WORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wpa2PreAuth, ,	WPA2_PRE_AUTH,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wpa2Cipher, ,	WPA2_CIPHER_SUITE,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)

// WDS stuffs
MIBDEF(unsigned char,	wdsEnabled, ,	WDS_ENABLED,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wdsNum, ,	WDS_NUM,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(WDS_T,	wdsArray, [MAX_WDS_NUM],	WDS,	WDS_ARRAY_T, CONFIG_WLAN_SETTING_T, 0, wlan_wds_tbl)
MIBDEF(unsigned char,	wdsEncrypt, ,	WDS_ENCRYPT,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wdsWepKeyFormat, ,	WDS_WEP_FORMAT,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wdsWepKey, [WEP128_KEY_LEN*2+1],	WDS_WEP_KEY,	STRING_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wdsPskFormat, ,	WDS_PSK_FORMAT,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wdsPsk, [MAX_PSK_LEN+1],	WDS_PSK,	STRING_T, CONFIG_WLAN_SETTING_T, 0, 0)

// for WMM
MIBDEF(unsigned char,	wmmEnabled, ,	WMM_ENABLED,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)

#ifdef WLAN_EASY_CONFIG
MIBDEF(unsigned char,	acfEnabled, ,	EASYCFG_ENABLED,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	acfMode, ,	EASYCFG_MODE,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	acfSSID, [MAX_SSID_LEN],	EASYCFG_SSID,	STRING_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	acfKey, [MAX_ACF_KEY_LEN+1],	EASYCFG_KEY,	STRING_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	acfDigest, [MAX_ACF_DIGEST_LEN+1],	EASYCFG_DIGEST,	STRING_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	acfAlgReq, ,	EASYCFG_ALG_REQ,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	acfAlgSupp, ,	EASYCFG_ALG_SUPP,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	acfRole, ,	EASYCFG_ROLE,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	acfScanSSID, [MAX_SSID_LEN],	EASYCFG_SCAN_SSID,	STRING_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	acfWlanMode, ,	EASYCFG_WLAN_MODE,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
#endif // #ifdef WLAN_EASY_CONFIG

/*for P2P_SUPPORT*/
MIBDEF(unsigned int,	p2p_type	, ,		P2P_TYPE,	DWORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	p2p_intent	, ,		P2P_INTENT,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	p2p_listen_channel, ,	P2P_LISTEN_CHANNEL,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	p2p_op_channel, ,	P2P_OPERATION_CHANNEL,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)



#ifdef WIFI_SIMPLE_CONFIG
MIBDEF(unsigned char,	wscDisable, ,	WSC_DISABLE,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wscMethod, ,	WSC_METHOD,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wscConfigured, ,	WSC_CONFIGURED,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wscAuth, ,	WSC_AUTH,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wscEnc, ,	WSC_ENC,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wscManualEnabled, ,	WSC_MANUAL_ENABLED,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wscUpnpEnabled, ,	WSC_UPNP_ENABLED,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wscRegistrarEnabled, ,	WSC_REGISTRAR_ENABLED,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wscSsid, [MAX_SSID_LEN],	WSC_SSID,	STRING_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wscPsk, [MAX_PSK_LEN+1],	WSC_PSK,	STRING_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wscConfigByExtReg, ,	WSC_CONFIGBYEXTREG,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
#endif // #ifdef WIFI_SIMPLE_CONFIG

//for 11N
MIBDEF(unsigned char,	channelbonding, ,	CHANNEL_BONDING,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	controlsideband, ,	CONTROL_SIDEBAND,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	aggregation, ,	AGGREGATION,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	shortgiEnabled, ,	SHORT_GI,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	access, ,	ACCESS,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	priority, ,	PRIORITY,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)

// for WAPI
#if CONFIG_RTL_WAPI_SUPPORT
MIBDEF(unsigned char,	wapiPsk, [MAX_PSK_LEN+1],	WAPI_PSK,	STRING_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wapiPskLen, ,	WAPI_PSKLEN,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wapiAuth, ,	WAPI_AUTH,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wapiPskFormat, ,	WAPI_PSK_FORMAT,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wapiAsIpAddr, [4],	WAPI_ASIPADDR,	IA_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wapiMcastkey, ,	WAPI_MCASTREKEY,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned long,	wapiMcastRekeyTime, ,	WAPI_MCAST_TIME,	DWORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned long,	wapiMcastRekeyPackets, ,	WAPI_MCAST_PACKETS,	DWORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wapiUcastkey, ,	WAPI_UCASTREKEY,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned long,	wapiUcastRekeyTime, ,	WAPI_UCAST_TIME,	DWORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned long,	wapiUcastRekeyPackets, ,	WAPI_UCAST_PACKETS,	DWORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
//internal use
MIBDEF(unsigned char,	wapiSearchCertInfo, [32],	WAPI_SEARCHINFO,	STRING_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wapiSearchIndex, ,	WAPI_SEARCHINDEX,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wapiCAInit, ,	WAPI_CA_INIT,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)

MIBDEF(unsigned char,	wapiCertSel, ,	WAPI_CERT_SEL,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)

MIBDEF(unsigned char,	wapiAuthMode2or3Cert, ,	WAPI_AUTH_MODE_2or3_CERT,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)

#endif // #if CONFIG_RTL_WAPI_SUPPORT

MIBDEF(unsigned char,	STBCEnabled, ,	STBC_ENABLED,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	CoexistEnabled, ,	COEXIST_ENABLED,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	phyBandSelect,	, PHY_BAND_SELECT,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0) //bit1:2G bit2:5G
MIBDEF(unsigned char,	macPhyMode,	, MAC_PHY_MODE,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0) //bit0:SmSphy. bit1:DmSphy. bit2:DmDphy.
//### add by sen_liu 2011.3.29 add TX Beamforming in 92D
MIBDEF(unsigned char,	TxBeamforming, ,	TX_BEAMFORMING,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
//### end
MIBDEF(unsigned char,	CountryStr, [4],	COUNTRY_STRING,	STRING_T, CONFIG_WLAN_SETTING_T, 0, 0)
#ifdef CONFIG_RTL_802_1X_CLIENT_SUPPORT
MIBDEF(unsigned char,	eapType, ,	EAP_TYPE,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	eapInsideType, ,	EAP_INSIDE_TYPE,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	eapUserId, [MAX_EAP_USER_ID_LEN+1],	EAP_USER_ID,	STRING_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	rsUserName, [MAX_RS_USER_NAME_LEN+1],	RS_USER_NAME,	STRING_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	rsUserPasswd, [MAX_RS_USER_PASS_LEN+1],	RS_USER_PASSWD,	STRING_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	rsUserCertPasswd, [MAX_RS_USER_CERT_PASS_LEN+1],	RS_USER_CERT_PASSWD,	STRING_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	rsBandSel, ,	RS_BAND_SEL,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
#endif
MIBDEF(unsigned int,	tx_restrict	, ,		TX_RESTRICT,	DWORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned int,	rx_restrict	, ,		RX_RESTRICT,	DWORD_T, CONFIG_WLAN_SETTING_T, 0, 0)

#endif // #ifdef MIB_CONFIG_WLAN_SETTING_IMPORT

