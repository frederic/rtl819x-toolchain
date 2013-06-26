#ifndef _WAPI_H_
#define _WAPI_H_

#include "WapiCrypto.h"

#define	DEFAULT_TIMEOUT    						1000  		
#define	CERTIFICATE_REQ_TIMEOUT				10000
#define	MAX_ACTIVE_ACCESS_AUTHENTICATION_RETRY_COUNT  2
#define	MAX_CERTIFICATE_REQ_RETTRY_COUNT		7
#define	MAX_USK_REQ_RETRY_COUNT				7
#define	MAX_MSK_NOTIFY_RETRY_COUNT			7
#define	MAX_RCVD_BK_UPDATE_RETRY_COUNT		7
#define	MAX_RCVD_USK_UPDATE_RETRY_COUNT		7
#define   	MAX_SEND_COUNT							7
#define MAX_AE_INSTANCE_NUM   					32
#define MAX_TRUST_ASU_NUM	  					32
#define       Max_send_Count              					5
#define MAX_ASUE_INSTANCE_NUM				32


#define eqMacAddr(a,b)						( ((a)[0]==(b)[0] && (a)[1]==(b)[1] && (a)[2]==(b)[2] && (a)[3]==(b)[3] && (a)[4]==(b)[4] && (a)[5]==(b)[5]) ? 1:0 )
#define cpMacAddr(des,src)					((des)[0]=(src)[0],(des)[1]=(src)[1],(des)[2]=(src)[2],(des)[3]=(src)[3],(des)[4]=(src)[4],(des)[5]=(src)[5])
#define   SET_WAI_HEADER_ProtocolVersion(hdr, val)          memset(hdr,val,1)   
#define   SET_WAI_HEADER_TYPE(hdr, val)                            memset(hdr,val,2)
#define   SET_WAI_HEADER_SUBTYPE(hdr, val)                      memset(hdr,val,1)   
#define   SET_WAI_HEADER_Length(hdr, val)                         memcpy(hdr,val,2)   
#define   SET_WAI_HEADER_SeqNum(hdr, val)                       memcpy(hdr,val,2)   
#define   SET_WAI_HEADER_Fragment(hdr, val)                    memset(hdr,val,1)   
#define   SET_WAI_HEADRE_Flag(hdr, val)                             memset(hdr,val,1)   


/************************************************************
Base parameter data struture
by sherry 20090223
*************************************************************/

//define the common data structure to store the field in WAPI
typedef unsigned char muint8;
typedef unsigned int muint32;

typedef struct _WAPI_SEC_DATA{
	UCHAR   Length;
	UCHAR   Content[256];
} WAPI_SEC_DATA, *PWAPI_SEC_DATA;


typedef struct _WAPI_CERTIFICATE_IDENTITY_DATA{
	USHORT   Length;
	USHORT   Flag;
	UCHAR   Content[1500];
} WAPI_CERTIFICATE_IDENTITY_DATA, *PWAPI_CERTIFICATE_IDENTITY_DATA;


typedef struct _WAPI_PARAMETER_DATA{
	UCHAR    Flag;
	USHORT   Length;
	UCHAR   Content[1000];
} WAPI_PARAMETER_DATA, *PWAPI_PARAMETER_DATA;


typedef struct _WAPI_SIGNATURE_DATA{
	USHORT   Length;
	UCHAR   Content[1000];
}WAPI_SIGNATURE_DATA, *PWAPI_SIGNATURE_DATA;


typedef struct _WAPI_SUITE_SECLECTOR{
	UCHAR   OUI[3];
	UCHAR   suiteType;
} WAPI_SUITE_SECLECTOR, *PWAPI_SUITE_SECLECTOR;


//WAI HEADER definition
typedef struct _WAPI_WAI_HEADER{
	 	USHORT    	protocolVersion;
		UCHAR		type;
		UCHAR		subType;
		USHORT		reserved;
		USHORT		length;
		USHORT		sequenceNum;
		UCHAR		fragmentNum;
		UCHAR		moreFragment;
} WAPI_WAI_HEADER, *PWAPI_WAI_HEADER;

//signature algorithm
typedef  struct _WAPI_SIGNATURE_ALGORITHM{
		USHORT       Length;
              	UCHAR          zacou_Algorithm;                        // 1 :SHA-256 algorithm
		UCHAR          Signature_Algorithm;	                // 1  : ECDSA-192 algorithm
		WAPI_PARAMETER_DATA      Parameter;
} WAPI_SIGNATURE_ALGORITHM, *PWAPI_SIGNATURE_ALGORITHM;

//signature attribute
typedef  struct _WAPI_SIGNATURE_ATTRIBUTE{
              	UCHAR     type;
		USHORT     length;
		WAPI_CERTIFICATE_IDENTITY_DATA    identity;
		WAPI_SIGNATURE_ALGORITHM          Signature_Algorithm;
		WAPI_SIGNATURE_DATA                   Signature_Value;			 
}WAPI_SIGNATURE_ATTRIBUTE,*PWAPI_SIGNATURE_ATTRIBUTE;

/***********************************************************
AE related data struture definition
by sherry  20090223
************************************************************/
//AE STATE during WAI Handshake
typedef enum _WAPI_AE_STATE{ 
	ST_WAPI_AE_IDLE	, 
	ST_WAPI_AE_ACTIVE_AUTHENTICATION_SNT,
	ST_WAPI_AE_ACCESS_AUTHENTICATE_REQ_RCVD,
	ST_WAPI_AE_ACCESS_CERTIFICATE_REQ_SNT,
	ST_WAPI_AE_BKSA_ESTABLISH,
	ST_WAPI_AE_USK_AGGREMENT_REQ_SNT,
	ST_WAPI_AE_USK_AGGREMENT_RSP_RCVD,
	ST_WAPI_AE_USKA_ESTABLISH,
	ST_WAPI_AE_MSK_NOTIFICATION_SNT,
	ST_WAPI_AE_MSK_RSP_RCVD,
	ST_WAPI_AE_MSKA_ESTABLISH
}WAPI_AE_STATE;


//One Handshake process data structure
typedef struct _WAPI_AE_HANDSHK_INSTANCE_DATA{
	WAPI_AE_STATE       	wapiAECurrentState;
	UCHAR			   	wapiAEIdentificationFlag[32];
	UCHAR			 wapiAEFlagSendAccessAuthentificationPacket;
	UCHAR			 wapiAEFlagRecvAccessAuthenticateReqPacket;
	UCHAR			  wapiRecvASUENonceForBKSA[32];
	WAPI_SEC_DATA       wapiRecvASUEEDCHPublicKeyForBKSA;
	WAPI_CERTIFICATE_IDENTITY_DATA	wapiRecvASUEIdentityForBKSA;
	WAPI_CERTIFICATE_IDENTITY_DATA   wapiRecvASUETrustASU[MAX_TRUST_ASU_NUM];
	WAPI_SIGNATURE_DATA		wapiRecvASUESignatureValue;
	UCHAR			 wapiAENonceForBKSA[32];
	WAPI_SEC_DATA		wapiAEEDCHPublicKeyForBKSA;
	WAPI_SEC_DATA		wapiASUEEDSCAPublicKeyForBKSA;
	WAPI_CERTIFICATE_IDENTITY_DATA	wapiAECertificateForBKSA;
	UCHAR			wapiASUECertificateValidateResult;
	UCHAR			wapiAECertificateValidateResult;
	UCHAR			wapiAccessAuthenticateReqResult;
	UCHAR			wapiAERecvUSKAggrementReqFlag;
	UCHAR			wapiAEUSKNonce[32];
	UCHAR			wapiRecvASUEUSKNonce[32];
	WAPI_SEC_DATA		wapiRecvASUEWAPIContentForUSKA;
	UCHAR				wapiAERxBuff[1500];
	USHORT				wapiAERxLength;
	UCHAR				wapiAETxBuff[1500];
	UCHAR				wapiAERetryCount;
	unsigned long			wapiAELastSendTime;
	UCHAR				wapiAERecvCertificateRspContent[1000];
	unsigned int			wapiAERecvCertificateRspLength;
} WAPI_AE_HANDSHK_INSTANCE_DATA, *PWAPI_AE_HANDSHK_INSTANCE_DATA;


//One Handshake AE Entity  data strucure
typedef struct _WAPI_AE_ENTITY_DATA{
	WAPI_AE_HANDSHK_INSTANCE_DATA 	  wapiAEHandshkInstance;
	void*				wapiAEOpenContext;
	UCHAR				wapiASUEMacAddr[6];
	UCHAR				wapiAEBKID[16];
	UCHAR				wapiAEBK[16];
	UCHAR				wapiAEUSKID;
	UCHAR				wapiAEUSK[16];
	UCHAR				wapiAEUnicastMICKey[16];
	UCHAR				wapiAEWAIKey[16];
	UCHAR				wapiAEProtectMulticastKeyDataKey[16];
	WAPI_SUITE_SECLECTOR		wapiAEUSKSuite;
	WAPI_SEC_DATA		wapiAERecvWAPIContentFromAssocReq;
	WAPI_SEC_DATA		wapiAESendWAPIContentForAssocRsp;
	UCHAR				wapiAEMSKID;
	UCHAR				wapiAEMSK[16];
	UCHAR				wapiAEMSKNK[16];
	UCHAR				wapiAEMSKMICKey[16];
	WAPI_SUITE_SECLECTOR		wapiAEMSKSuite;
	BOOLEAN				bPreSharedKeyMethod;
	BOOLEAN						bWapiUsed;
	BOOLEAN						bSetKeyOk;
	BOOLEAN						bUpdateBKInProcess;
	BOOLEAN						bUpdateUSKInProcess;
	BOOLEAN						bUpdateMSKInProcess;
	BOOLEAN						bRecvWAIPacketForUpdateKey;
	BOOLEAN						bFirstAuthenticateInProcess;
	BOOLEAN						bWapiAEWAIFail;
	UCHAR						wapiAENextIdentificationFlag[32];
	UCHAR						wapiAENextUSKAggrementNonce[32];
	WAPI_SEC_DATA				wapiAEBMK;
	BOOLEAN						bWapiAEIbss;
	UCHAR						wapiAEBssid[6];
	BOOLEAN						bWapiAEInstanceThreadExist;
	EC_KEY						*pPublicAndPrivateKeyForEDCH;
	EC_KEY						*pPublicAndPrivateKeyForEDSCA;
} WAPI_AE_ENTITY_DATA, *PWAPI_AE_ENTITY_DATA;

//AE data struct
typedef struct _WAPI_AE_OPEN_CONTEXT_DATA{
	WAPI_AE_ENTITY_DATA		wapiAEEntity[MAX_AE_INSTANCE_NUM];
	UCHAR						wapiAECurrentMacAddr[6];
	UCHAR						wapiAENMK[16];
	UCHAR								wapiAEMSKNotifyFlag[16];
	WAPI_CERTIFICATE_IDENTITY_DATA		wapiAECertificate;
	WAPI_CERTIFICATE_IDENTITY_DATA		wapiAEIdentity;
	WAPI_CERTIFICATE_IDENTITY_DATA		wapiAETrustASUIdentity;
	WAPI_PARAMETER_DATA				wapiAEECDHParameter;
	BOOLEAN							bWapiAEIbssMode;
	WAPI_SUITE_SECLECTOR		wapiAEUSKSuiteLocal;
	UCHAR						wapiAEUpdateMSKNum;
	UCHAR						wapiAEWPIMSTDataPN[16];
	UCHAR						wapiAEPresharedBK[16];
	UCHAR				wapiAEPassPhraseKey[64];
	ULONG				wapiAEPassPhraseKeyLength;
	BOOLEAN				bWapiAEPreShared; 
	USHORT						wapiAELastSendSequenceNum;
	HANDLE					hMutexForGlobalData;
	unsigned int				wapiFragmentThreshold;
	BOOLEAN				bWapiFirstMSKUpdate;
	BOOLEAN				bWapiMSKUpdateInprocess;
	WAPI_SEC_DATA		WapiPublicKeyFromCertificate;
	void					*wapiOpenContext;
} WAPI_AE_OPEN_CONTEXT_DATA, *PWAPI_AE_OPEN_CONTEXT_DATA;

/***************************************************************
ASEU related data structrue
by zhiyuan 2009/02/24
****************************************************************/
//ASUE State
typedef enum  _WAPI_ASUE_STATE{
	ST_WAPI_ASUE_IDLE, 
	ST_WAPI_ASUE_ACTIVE_AUTHENTICATION_RCVD,
	ST_WAPI_ASUE_ACCESS_AUTHENTICATE_REQ_SNT,
	ST_WAPI_ASUE_ACCESS_AUTHENTICATE_RSP_RCVD,
	ST_WAPI_ASUE_BKSA_ESTABLISH,
	ST_WAPI_ASUE_USK_AGGREMENT_REQ_RCVD,
	ST_WAPI_ASUE_USK_AGGREMENT_RSP_SNT,
         ST_WAPI_ASUE_USK_AGGREMENT_CONFIRM_RCVD,
	ST_WAPI_ASUE_USKA_ESTABLISH,
	ST_WAPI_ASUE_MSK_NOTIFICATION_RCVD,
	ST_WAPI_ASUE_MSK_RSP_SNT,
	ST_WAPI_ASUE_MSKA_ESTABLISH
} WAPI_ASUE_STATE;


typedef  enum  _WAPI_PACKET_STATE{
             WAPI_RCEV_PACKET=1,
	    WAPI_START_BK_UPDATE=2,	 
	    WAPI_START_USK_UPDATE=3,
	    WAPI_START_STA_LEAVE=4,
}WAPI_PACKET_STATE;//shoud re-value after set oid

//defined the structure for ASUE instance
typedef  struct  _WAPI_ASUE_INSTANCE{
       BOOLEAN   bpreauthentication;    
	BOOLEAN   bASUE_USK_Update;            //will be set TRUE if ASUE start USK update;
	BOOLEAN   bAE_USK_Update;              // will be set true if AE start USK update

	void            *pWAPI_OPEN_CONTEXT;
	EC_KEY			*pTemp_Privatekey_EDCH;
	EC_KEY			*pPrivate_Public_Key_For_EDSCA;
		
	BOOLEAN   bBK_Update;              //will be set True if ASUE or AE started BK update;
       UCHAR     destAddress[6];
	UCHAR     CurrentAddress[6];
	UCHAR     ADDID[12];   


        WAPI_ASUE_STATE        ASUE_Current_State;
	 UCHAR                          identification_Flag;	
	 UCHAR                          authenticate_Flag[32];
	 UCHAR                          Last_authenticate_Flag[32];


	 WAPI_CERTIFICATE_IDENTITY_DATA     ASU_Identity;
	 WAPI_CERTIFICATE_IDENTITY_DATA     AE_Identity;
	 WAPI_CERTIFICATE_IDENTITY_DATA     ASUE_Identity;
	 WAPI_CERTIFICATE_IDENTITY_DATA     AE_Certification;
	 WAPI_CERTIFICATE_IDENTITY_DATA     ASUE_Certification;

	 WAPI_PARAMETER_DATA                      EDCH_Parameter;
	 	
	 WAPI_CERTIFICATE_IDENTITY_DATA     ASUE_Trust_ASU[MAX_TRUST_ASU_NUM];
	 
        WAPI_SIGNATURE_ATTRIBUTE        ASUE_Signature;
	 WAPI_SIGNATURE_ATTRIBUTE        AE_Signature;

	 UCHAR                             BK_AE_nonce[32];
	 UCHAR                             BK_ASUE_nonce[32];
	 UCHAR                             BK_Next_AE_nonce[32];
	 UCHAR                             USK_AE_nonce[32];
	 UCHAR                             USK_Last_AE_nonce[32];
	 UCHAR                             USK_Next_AE_nonce[32];
	 UCHAR                             USK_ASUE_nonce[32];
	 UCHAR                             WPIMSTDataPN[16];

	 UCHAR                             Access_Result; 

	 UCHAR                             ASUE_BK[16];
	 UCHAR                             ASUE_BKID[16];
	 UCHAR                             ASUE_USKID;
	 UCHAR                             ASUE_MSKID;
	 UCHAR                             ASUE_MSK_USKID;
        UCHAR                             ASUE_NMK[16];       // Notice master key
        UCHAR                             ASUE_MSK[16];
	 UCHAR                             ASUE_MSK_MIC[16];	

	 
	 UCHAR                             Seckey_notice_flag[16];
	 UCHAR                             Seckey_notice_flag_reserved[16];
	 USHORT                           SeqNum;
	 unsigned int                      FragThreshold;

        UCHAR                             ASUE_Unicast_EncryptKey[16];
	 UCHAR	                          ASUE_Unicast_Integrity_VerifyKey[16];
	 UCHAR	                          ASUE_Msg_AuthenticateKey[16];
	 UCHAR	                          ASUE_MulticastOrSTA_EncryptKey[16];

	 WAPI_SEC_DATA               WAPI_INFO_From_asso_request;
	 WAPI_SEC_DATA               WAPI_INFO_From_asso_response;
	
	 
	 WAPI_SEC_DATA              ASUE_sec_data;
	 WAPI_SEC_DATA              AE_sec_data;
	 WAPI_SEC_DATA		     AE_EDSCA_sec_data;

	 //for time out mechenism
	 UCHAR                         TimeOut;
	unsigned long			Last_Send_Time;
	 BOOLEAN                         bFirst_Authenticate;
	 BOOLEAN                         bNetworkType;                      //  1:bss     0:ibss????
        BOOLEAN                          bInProcess;
	 BOOLEAN                          bfail;   // fail of authentication
	 BOOLEAN			    bASUE_InstanceThread_Exist;
	 
} WAPI_ASUE_INSTANCE, *PWAPI_ASUE_INSTANCE;

//defined structure for supplicant
typedef struct _WAPI_ASUE_OPEN_CONTEXT
{

      UCHAR                   CurrentAddress[6];
      WAPI_ASUE_INSTANCE    ASUE_Instance[MAX_ASUE_INSTANCE_NUM];
      UCHAR                    Instance_id;	  
      UCHAR                    PassPhraseKey[100];
      unsigned int             PassPhraseKeyLength;	  
      UCHAR                    Preshared_BK[16];	  

      unsigned int              FragThreshold;

      WAPI_CERTIFICATE_IDENTITY_DATA		ASUE_Trust_ASU_Identity;
      WAPI_CERTIFICATE_IDENTITY_DATA           ASUE_Certificate;
      WAPI_CERTIFICATE_IDENTITY_DATA           ASUE_Identity;
      WAPI_PARAMETER_DATA                            EDCH_Parameter;
      WAPI_SEC_DATA                                        Public_Key;

	 HANDLE                    hMutexForGlobalData;

	 BOOLEAN                bNetworkType;
	 BOOLEAN                bPresharedkey;


}WAPI_ASUE_OPEN_CONTEXT, *PWAPI_ASUE_OPEN_CONTEXT;


typedef struct _WAPI_OPEN_CONTEXT
{
	void *  wapiASUEContext;
	void *  wapiAEContext;
	HANDLE	wapiRXMutex;
	BOOLEAN		bInitializedRXMutex;
	BOOLEAN       bInfrastructure;         //NETWORK TYPE;
	BOOLEAN       bWAPI_PSK;
	BOOLEAN       bWAPI_Certificate;
	ULONG          passphrase_len;
	UCHAR           passphrase[256];
	BOOLEAN       bAEinProcess;
	BOOLEAN       bASUEinProcess;
}WAPI_OPEN_CONTEXT,*PWAPI_OPEN_CONTEXT;


//Added by zj
int    ReadCertificate(const char * certName, BIO * out);
int wapiCertVerify(const char * user_cer,const char * as_cer,BIO * out);//return 0: success;return -1: failed
//int    ReadCertificateASUE(void *pArg);
//unsigned long __stdcall WapiSupplicantProcess(void * pArg);
//unsigned long __stdcall WapiSupplicantAuthInstanceThread(void * pArg);

//unsigned long __stdcall WapiAuthenticatorProcess(void * pArg);
//unsigned long __stdcall WapiAuthenticatorAuthInstanceThread(void * pArg);
void
HMAC_SHA256(
unsigned char *text,
UINT text_len, unsigned char *key, 
UINT key_len, unsigned char *digest, 
UINT *digest_len
);

void 
KD_HMAC_SHA256(
unsigned char *text,
unsigned int text_len,
unsigned char *key, 
unsigned int key_len,
unsigned char *output,
unsigned int length
);

void
GenerateRandomData(
UCHAR * data,
UINT len);

BOOLEAN 
WapiReceiveFragment(
	UCHAR				*WapiRXBuff,
	USHORT				WapiRXBuffLength,
	UINT				Timeout,
	UCHAR				*DestAddr,
	BOOLEAN				bAuthenticator
);

BOOLEAN 
WapiReceivePacket(
	PWAPI_OPEN_CONTEXT pOpenContext,
	UCHAR				*PeerMacAddr,
	UCHAR				*RxPacketBuff,
	USHORT				*RxPacketBuffLength,
	UINT				Timeout,
	unsigned int			RxThreshold,
	BOOLEAN				bAuthenticator
);

UINT 
WapiRecvAccessAuthenticateRequest(
	PWAPI_AE_ENTITY_DATA	pWapiAEEntry
);

void
WapiAuthenticatorHandleUSKAggrementRspRcvd(
		PWAPI_AE_ENTITY_DATA	pWapiAEEntry
);

UINT 
WapiRecvUnicastKeyAggrementResponse(
		PWAPI_AE_ENTITY_DATA	pWapiAEEntry
);

UINT 
WapiRecvMulticastKeyResponse(
		PWAPI_AE_ENTITY_DATA	pWapiAEEntry
);

BOOLEAN 
WapiSendPacket(
	UCHAR *WaiDataBuf, 
	unsigned int DataLen,
	unsigned int FragThreshold,
	UCHAR	*WlanHeaderBuff
);

UINT 
WapiSendActivateAuthenticationPacket(
		PWAPI_AE_ENTITY_DATA	pWapiAEEntry
);


UINT WapiSendAccessAuthenticateRequest(
		void *		pArg
);


UINT 
WapiSendAccessAuthenticateResponse(
			PWAPI_AE_ENTITY_DATA	pWapiAEEntry,
			UCHAR			resultReq

);

UINT 
WapiSendUnicastKeyAggrementResponse(
			void *		pArg
);

UINT 
WapiSendUnicastKeyAggrementRequest(
		PWAPI_AE_ENTITY_DATA	pWapiAEEntry
);


UINT
WapiSendUnicastKeyAggrementConfirm(
		PWAPI_AE_ENTITY_DATA	pWapiAEEntry
);

UINT 
WapiSendMulticastKeyNotification(
	PWAPI_AE_ENTITY_DATA	pWapiAEEntry
);

UINT 
WapiSendMulticastKeyResponse(
	void *				pArg
);


void 
WapiAuthenticatorCalculateMSKSA(
	PWAPI_AE_ENTITY_DATA	pWapiAEEntry
);

void 
WapiAuthenticatorCalculateUSKSA(
	PWAPI_AE_ENTITY_DATA	pWapiAEEntry
);

void 
WapiAuthenticatorCalculateBKSA(
	PWAPI_AE_ENTITY_DATA	pWapiAEEntry
);

void 
SMS4KeyExt(
muint8 *Key,
muint32 *rk, 
muint32 CryptFlag);

void 
SMS4Crypt(
muint8 *Input, 
muint8 *Output, 
muint32 *rk
);

void 
WapiSMS4ForMNKEncrypt(
UCHAR *key,
UCHAR *IV,
UCHAR *input, 
UINT inputLength,
UCHAR *output, 
UCHAR *outputLength,
UINT    CryptFlag
);


BOOLEAN 
Compare_Two(
UCHAR *pA,
UCHAR *pB,
USHORT len) ;


INT 
Compare_address(
UCHAR *Pa,
UCHAR *Pb,
UINT NUM);

void  
WAPIASUEStartUSKupdate(
void *pArg
);

UINT 
WapiRecvActivateAuthenticationPacket(
void *pArg,
UCHAR *pRxBuffer, 
UINT Length
);

UINT 
WapiRecvAccessAuthenticateResponse(
void *pArg,
UCHAR *pRxBuffer, 
UINT Length
);

UINT 
WapiRecvUnicastKeyAggrementRequest(
void *pArg,
UCHAR *pRxBuffer, 
UINT Length
);

UINT 
WapiRecvUnicastKeyAggrementConfirm(
void *pArg, 
UCHAR *pRxBuffer, 
UINT Length
);

UINT 
WapiRecvMulticastKeyNotification(
void *pArg,
UCHAR *pRxBuffer, 
UINT Length
);

BOOLEAN    
HandleIdleState(
void *pArg,
void *pASUEINSTANCE
);

void    
HandleSendAccessAuthenticateRequest(
void *pArg,
void *pASUEINSTANCE
);

void   
HandleSendUnicastKeyAggrementResponse(
void *pArg,
void *pASUEINSTANCE
);

BOOLEAN 
HandMulticastKeyResponseSent(
void *pArg,
void *pASUEINSTANCE
);




//int    ReadCertificate(void *pArg);












#endif




