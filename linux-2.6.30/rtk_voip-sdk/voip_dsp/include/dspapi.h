
#ifndef _DSP_API_H
#define _DSP_API_H

typedef struct
{
	uint32 chid;
	RtpPayloadType uPktFormat;
	int32 nG723Type;
	int32 nFramePerPacket;
	int32 bVAD;					// bool
}payloadtype_config_t;

//// -> add by tyhuang
typedef struct
{
	uint32 chid;
	DSPCODEC_TONE nTone;
	uint32 bFlag;
	DSPCODEC_TONEDIRECTION path;
}playtone_config_t;

typedef struct
{
	uint32 chid;
	uint32 channel;
}setchannel_config_t;
//// <- add by tyhuang

typedef enum
{
	PCMSIDE =0,
	RTPSIDE

}VMPlaySide;

typedef struct
{
	uint32 chid;
	char* pVoiceMail;
	int32 nLen;
	RtpPayloadType uPktFormat;
	VMPlaySide direction;
}voicemail_config_t;


int32 DSP_ioctl(uint32* data);
int32 DSP_SetRtpConfig(uint32* data);
//int32 DSP_SetRtpConfig(void* pRtpCfg);
int32 DSP_GetRtpConfig(uint32* data);
int32 DSP_SetRtpPayloadType(uint32* data);
int32 DSP_GetRtpPayloadType(uint32* data);
int32 DSP_SetRtpSessionState(uint32* data);
int32 DSP_GetRtpSessionState(uint32* data);
int32 DSP_SetRingFXS(uint32* data);
int32 DSP_GetRingFXS(uint32* data);

//// -> add by tyhuang
int32 DSP_SetPlayTone(uint32* data);
int32 DSP_GetPlayTone(uint32* data);
int32 DSP_slic_routine(uint32* data);
//// <- add by tyhuang

//// -> add by tyhuang
int32 DSP_SetInternalCall(uint32* data);
//// <- add by tyhuang

int32 DSP_GetRtpTestCfg(uint32* data);

#define G723_RATE63 0
#define G723_RATE53 1

#define GNET_IOCTL_DSPINIT				8000
#define GNET_IOCTL_SETRTPCONFIG			8001
#define GNET_IOCTL_GETRTPCONFIG			8002
#define GNET_IOCTL_SETRTPPAYLOADTYPE	8003
#define GNET_IOCTL_GETRTPPAYLOADTYPE	8004
#define GNET_IOCTL_PLAYRTPTEMP			8005
#define GNET_IOCTL_SETRTPSESSIONSTATE	8006
#define GNET_IOCTL_GETRTPSESSIONSTATE	8007	//modified
#define GNET_IOCTL_SETRINGFXS			8008	//modified
#define GNET_IOCTL_GETRINGFXS			8009	//modified
//// -> add by tyhuang
#define GNET_IOCTL_SETPLAYTONE			8010
#define GNET_IOCTL_GETPLAYTONE			8011
#define GNET_IOCTL_SILCROUTINE			8012
//// <- add by tyhuang

#define GNET_IOCTL_INTERNALCALL			8499
#define GNET_IOCTL_GETRTPTESTCFG		8500


#endif
