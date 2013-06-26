

#define MACADDRLEN		6
#define PROBEIELEN		260

#define REQIELEN		123
#define RESPIELEN		123

#define HAPD_IOCTL_GETWPAIE		IEEE80211_IOCTL_GET_APPIEBUF
#define HAPD_IOCTL_SETCONFIG	SIOCIWLASTPRIV
#define RTL8192CD_IOCTL_DEL_STA	0x89f7
#define SIOCGIWIND      		0x89ff

#define HOSTAPD_WPA_VERSION_WPA BIT(0)
#define HOSTAPD_WPA_VERSION_WPA2 BIT(1)

#define WPAS_IOCTL_CUSTOM		SIOCIWLASTPRIV //0x8BFF

enum WPAS_EVENT{
	WPAS_EXIRED = 10,
	WPAS_REGISTERED = 11,
	WPAS_MIC_FAILURE = 12,
	WPAS_ASSOC_INFO = 13,
	WPAS_SCAN_DONE = 14
};

enum WIFI_STATUS_CODE {
	_STATS_SUCCESSFUL_				= 0,	// Success.
	_STATS_FAILURE_					= 1,	// Failure.
	_STATS_CAP_FAIL_				= 10,	// Capability too wide, can't support
	_STATS_NO_ASOC_					= 11,	// Denial reassociate
	_STATS_OTHER_					= 12,	// Denial connect, not 802.11 standard.
	_STATS_NO_SUPP_ALG_				= 13,	// Authenticate algorithm not support .
	_STATS_OUT_OF_AUTH_SEQ_			= 14,	// Out of authenticate sequence number.
	_STATS_CHALLENGE_FAIL_			= 15,	// Denial authenticate, Response message fail.
	_STATS_AUTH_TIMEOUT_			= 16,	// Denial authenticate, timeout.
	_STATS_UNABLE_HANDLE_STA_		= 17,	// Denial authenticate, BS resoruce insufficient.
	_STATS_RATE_FAIL_				= 18,	// Denial authenticate, STA not support BSS request datarate.
	_STATS_REQ_DECLINED_		= 37,
/*#if defined(CONFIG_RTL_WAPI_SUPPORT)*/
	__STATS_INVALID_IE_ = 40,
	__STATS_INVALID_AKMP_ = 43,
	__STATS_CIPER_REJECT_ = 46,
	__STATS_INVALID_USK_ = 47,
	__STATS_INVALID_MSK_ = 48,
	__STATS_INVALID_WAPI_VERSION_ = 49,
	__STATS_INVALID_WAPI_CAPABILITY_ = 50,
/*#endif*/

#ifdef CONFIG_RTK_MESH	// CATUTION: below undefine !! (Refer: Draft 1.06, Page 17, 7.3.1.9, Table 7-23, 2007/08/13 by popen)
	_STATS_MESH_LINK_ESTABLISHED_	= 55,	//The mesh peer link has been successfully
	_STATS_MESH_LINK_CLOSED_		= 56,	// The mesh peer link has been closed completely
	_STATS_MESH_UNDEFINE1_			= 57,	// No listed Key Holder Transport type is supported.
	_STATS_MESH_UNDEFINE2_			= 58,	// The Mesh Key Holder Security Handshake message was malformed.
#endif
};

typedef enum{
        DOT11_EVENT_NO_EVENT = 1,
        DOT11_EVENT_REQUEST = 2,
        DOT11_EVENT_ASSOCIATION_IND = 3,
        DOT11_EVENT_ASSOCIATION_RSP = 4,
        DOT11_EVENT_AUTHENTICATION_IND = 5,
        DOT11_EVENT_REAUTHENTICATION_IND = 6,
        DOT11_EVENT_DEAUTHENTICATION_IND = 7,
        DOT11_EVENT_DISASSOCIATION_IND = 8,
        DOT11_EVENT_DISCONNECT_REQ = 9,
        DOT11_EVENT_SET_802DOT11 = 10,
        DOT11_EVENT_SET_KEY = 11,
        DOT11_EVENT_SET_PORT = 12,
        DOT11_EVENT_DELETE_KEY = 13,
        DOT11_EVENT_SET_RSNIE = 14,
        DOT11_EVENT_GKEY_TSC = 15,
        DOT11_EVENT_MIC_FAILURE = 16,
        DOT11_EVENT_ASSOCIATION_INFO = 17,
        DOT11_EVENT_INIT_QUEUE = 18,
        DOT11_EVENT_EAPOLSTART = 19,
//2003-07-30 ------------
        DOT11_EVENT_ACC_SET_EXPIREDTIME = 31,
        DOT11_EVENT_ACC_QUERY_STATS = 32,
        DOT11_EVENT_ACC_QUERY_STATS_ALL = 33,
//-----------------------

// --- 2003-08-04 ---
        DOT11_EVENT_REASSOCIATION_IND = 34,
        DOT11_EVENT_REASSOCIATION_RSP = 35,
//-----------------------
        DOT11_EVENT_STA_QUERY_BSSID = 36,
        DOT11_EVENT_STA_QUERY_SSID = 37,

// jimmylin: pass EAP packet by event queue
        DOT11_EVENT_EAP_PACKET = 41,

#ifdef RTL_WPA2
        DOT11_EVENT_EAPOLSTART_PREAUTH = 45,
        DOT11_EVENT_EAP_PACKET_PREAUTH = 46,
#endif        

#ifdef RTL_WPA2_CLIENT
	DOT11_EVENT_WPA2_MULTICAST_CIPHER = 47,       
#endif

	DOT11_EVENT_WPA_MULTICAST_CIPHER = 48,       

#ifdef AUTO_CONFIG
	DOT11_EVENT_AUTOCONF_ASSOCIATION_IND = 50,
	DOT11_EVENT_AUTOCONF_ASSOCIATION_CONFIRM = 51,
	DOT11_EVENT_AUTOCONF_PACKET = 52,
	DOT11_EVENT_AUTOCONF_LINK_IND = 53,
#endif

#ifdef WIFI_SIMPLE_CONFIG
	DOT11_EVENT_WSC_SET_IE = 55,		
	DOT11_EVENT_WSC_PROBE_REQ_IND = 56,
	DOT11_EVENT_WSC_PIN_IND = 57,
	DOT11_EVENT_WSC_ASSOC_REQ_IE_IND = 58,
#ifdef CONFIG_IWPRIV_INTF
	DOT11_EVENT_WSC_START_IND = 70,
	//EV_MODE, EV_STATUS, EV_MEHOD, EV_STEP, EV_OOB
	DOT11_EVENT_WSC_MODE_IND = 71,
	DOT11_EVENT_WSC_STATUS_IND = 72,
	DOT11_EVENT_WSC_METHOD_IND = 73,
	DOT11_EVENT_WSC_STEP_IND = 74,
	DOT11_EVENT_WSC_OOB_IND = 75,
#endif  //ifdef CONFIG_IWPRIV_INTF
#endif

        DOT11_EVENT_MAX = 59,
} DOT11_EVENT;


typedef struct _DOT11_PROBE_REQUEST_IND{
        unsigned char   EventId;
        unsigned char   IsMoreEvent;
        char            MACAddr[MACADDRLEN];
        unsigned short  ProbeIELen;
        char            ProbeIE[PROBEIELEN];
}DOT11_PROBE_REQUEST_IND;


typedef struct _DOT11_ASSOCIATION_RSP{
        unsigned char   EventId;
        unsigned char   IsMoreEvent;
        char            MACAddr[MACADDRLEN];
        unsigned char   Status;
}DOT11_ASSOCIATION_RSP;


typedef struct _WPAS_ASSOCIATION_INFO
{
		unsigned short	ReqIELen;
		char			ReqIE[REQIELEN];
		unsigned short  RespIELen;
        char            RespIE[RESPIELEN];
} WPAS_ASSOCIATION_INFO;


//_Eric ?? Put these inot net80211 will be better??

