#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
// fsk date & time sync
#include <time.h>
#include <sys/time.h>
// fsk date & time sync
#include "voip_manager.h"
#ifdef CONFIG_RTK_VOIP_DECT_SITEL_SUPPORT
#include "si_dect_api.h"
#endif
#ifdef CONFIG_RTK_VOIP_DECT_DSPG_SUPPORT
#include "cmbs_api.h"
#include "appcall.h"
#include "dect_test_scenario.h"
#endif

#ifdef CONFIG_RTK_VOIP_SRTP
#include "crypto_types.h"
#endif

#define SUPPORT_RTCP
		     /* define SUPPORT_RTCP to support RTCP.
                      * It also need to define it in rtk_voip.h for kernel space.
		      * Thlin add 2006-07-04
		      */

#include "voip_ioctl.h"

#ifndef MIN
#define MIN(x,y) (x>y?y:x)
#endif

static int32 rtk_OpenRtpSession(uint32 chid, uint32 sid);
int32 rtk_enable_pcm(uint32 chid, int32 bEnable);

#ifndef __mips__
// use v400 + VE890 (2S1O) as default feature for x86
#define VOIP_X86_FEATURE ((uint64) 0xd00002efc0008021ULL)
VoipFeature_t g_VoIP_Feature = VOIP_X86_FEATURE;
uint32 g_VoIP_Ports = RTK_VOIP_CH_NUM( VOIP_X86_FEATURE );
#else
// voip feature from kernel config
VoipFeature_t g_VoIP_Feature = 0;
uint32 g_VoIP_Ports = 0;
#endif
//static int no_resource_flag[VOIP_CH_NUM]={0};

int g_VoIP_Mgr_FD = -1;

int gCad_on_msec[CON_CH_NUM] = {[0 ... CON_CH_NUM-1] = 2000};		// 2s on
int gCad_off_msec[CON_CH_NUM] = {[0 ... CON_CH_NUM-1] = 4000};	// 4s on

/* If DAA is used by CHn, set g_DAA_used[CHn]=1, otherwise, set g_DAA_used[CHn]=0 */
//int g_DAA_used[MAX_SLIC_NUM] = {0};

#ifdef CONFIG_RTK_VOIP_DRIVERS_ATA_DECT
int g_sip_register[CON_CH_NUM] = {[0 ... CON_CH_NUM-1] = 0};

#ifdef CONFIG_RTK_VOIP_DECT_DSPG_SUPPORT

#define DECT_LED 1

pthread_mutex_t         semDectEventFifoSem;

#define MUTEXOBTAIN_semDectEventFifoSem pthread_mutex_lock( &semDectEventFifoSem );
#define MUTEXRELEASE_semDectEventFifoSem pthread_mutex_unlock( &semDectEventFifoSem );

void dect_event_mutex_init(void)
{
	pthread_mutex_init(&semDectEventFifoSem,0);
}

typedef enum
{
	HS_OCCUPY_FAIL = 0,
	HS_OCCUPY_SUCC = 1,
	HS_OCCUPY_IDLE = 2
}
HS_ENT_ST;

#define DECT_EVENT_FIFO_SIZE 20
#define MAX_DECT_CH_SIZE 8
#define MAX_LINE_SUPPORT 4
#define MAX_HS_SUPPORT 5
#define NONE -1
static char dect_event_fifo[MAX_DECT_CH_SIZE][DECT_EVENT_FIFO_SIZE];
static int dect_event_wp[MAX_DECT_CH_SIZE]={0}, dect_event_rp[MAX_DECT_CH_SIZE]={0};
static int LineOccupyByHS[MAX_LINE_SUPPORT] = {[0 ... MAX_LINE_SUPPORT-1] = NONE};
static HS_ENT_ST HsEntSt[MAX_HS_SUPPORT] = {[0 ... MAX_HS_SUPPORT-1] = HS_OCCUPY_IDLE};

static int dect_event_init(void)
{
	int i;

	for (i=0; i<MAX_DECT_CH_SIZE; i++)
	{
		dect_event_wp[i] = 0;
		dect_event_rp[i] = 0;
	}

	for (i=0; i<MAX_LINE_SUPPORT; i++)
		LineOccupyByHS[i] = NONE;

	for (i=0; i<MAX_HS_SUPPORT; i++)
		HsEntSt[i] = HS_OCCUPY_IDLE;

	return 0;
}

int dect_event_in(uint32 line_id, uint32 hs_id, char input)
{
	//printf("dect_event_in, ch%d\n", line_id);
	uint32 ch_id;
	int key;

	MUTEXOBTAIN_semDectEventFifoSem;

	if ( LineOccupyByHS[line_id] == NONE ) // Line is IDLE
	{
		if (HsEntSt[hs_id] == HS_OCCUPY_IDLE)
		{
			if (input == 1) //off-hook
			{
				LineOccupyByHS[line_id] = hs_id;
				HsEntSt[hs_id] = HS_OCCUPY_SUCC; //IDLE-->SUCC
				printf("[DECT]: line%d occupy by HS%d, HS%d =HS_OCCUPY_SUCC\n.", line_id, LineOccupyByHS[line_id], hs_id);
#if DECT_LED
#ifdef CONFIG_RTK_VOIP_GPIO_8954C_V400
				rtk_SetDectLED(line_id, 2); // blinking
#endif
#endif
			}
		}
		else if (HsEntSt[hs_id] == HS_OCCUPY_FAIL)
		{
			if (input ==0) //on-hook
			{
				HsEntSt[hs_id] = HS_OCCUPY_IDLE;
				input += 100;
			}
			else if (input == 1) //off-hook
				printf("Error! Shout not go to here: %s, line%d\n", __FUNCTION__, __LINE__);

			printf("[DECT]: line%d occupy by HS%d, HS%d =HS_OCCUPY_IDLE\n.", line_id, LineOccupyByHS[line_id], hs_id);
		}
		else if (HsEntSt[hs_id] == HS_OCCUPY_SUCC)
		{
			printf("Error! Shout not go to here: %s, line%d\n", __FUNCTION__, __LINE__);
		}
	}
	else if ( LineOccupyByHS[line_id] != NONE ) // Line is Occupy
	{
		// Line has been used by other HS in the same line group.
		if ( LineOccupyByHS[line_id] != hs_id )
		{
			if (input == 1) //off-hook
			{
				HsEntSt[hs_id] = HS_OCCUPY_FAIL; //IDLE-->FAIL
				printf("[DECT]: line%d occupy by HS%d, HS%d =HS_OCCUPY_FAIL\n.", line_id, LineOccupyByHS[line_id], hs_id);
			}
			else if (input ==0) //on-hook
			{
				HsEntSt[hs_id] = HS_OCCUPY_IDLE; //FAIL-->IDLE
				printf("[DECT]: line%d occupy by HS%d, HS%d =HS_OCCUPY_IDLE\n.", line_id, LineOccupyByHS[line_id], hs_id);
			}

			input += 100;

		}
		else
		{
			if (input == 0) //on-hook
			{
				LineOccupyByHS[line_id] = NONE;
				HsEntSt[hs_id] = HS_OCCUPY_IDLE;//SUCC-->IDLE
				printf("[DECT]: line%d is IDLE, HS%d =HS_OCCUPY_IDLE\n.", line_id, hs_id);
#if DECT_LED
#ifdef CONFIG_RTK_VOIP_GPIO_8954C_V400
				if (g_sip_register[line_id] == 1)
					rtk_SetDectLED(line_id, 1); // VoIP LED0 On
				else
					rtk_SetDectLED(line_id, 0); // VoIP LED0 Off
#endif
#endif
			}
		}
	}

	ch_id = line_id;

        if ((dect_event_wp[ch_id]+1)%DECT_EVENT_FIFO_SIZE != dect_event_rp[ch_id])
	{
	 	dect_event_fifo[ch_id][dect_event_wp[ch_id]] = input;
                dect_event_wp[ch_id] = (dect_event_wp[ch_id]+1) % DECT_EVENT_FIFO_SIZE;
	  	//printf("dect_event_wp=%d\n", dect_event_wp[ch_id]);

#if 1 // play DTMF tone by VoIP DSP
		if (input >= '0' && input <= '9')
		{
			key = input - '0';
			rtk_SetPlayTone(ch_id, 0, key, 1, 0);
		}
		else if (input == '*')
		{
			key = 10;//DSPCODEC_TONE_STARSIGN;
			rtk_SetPlayTone(ch_id, 0, key, 1, 0);
		}
		else if (input == '#')
		{
			key = 11;//DSPCODEC_TONE_HASHSIGN;
			rtk_SetPlayTone(ch_id, 0, key, 1, 0);
		}
#endif
	}
	else
	{
		printf("dect_event FIFO overflow,(%d)\n", ch_id);
	}

	MUTEXRELEASE_semDectEventFifoSem;

	return 0;
}


char dect_event_out(uint32 line_id)
{
	char output;
	uint32 ch_id;

	MUTEXOBTAIN_semDectEventFifoSem;

	ch_id = line_id;

	if ( dect_event_wp[ch_id] == dect_event_rp[ch_id]) // FIFO empty
	{
		output = 'Z';
		//printf("output = %d\n", output);
	}
	else
	{
		output = dect_event_fifo[ch_id][dect_event_rp[ch_id]];
                dect_event_rp[ch_id] = (dect_event_rp[ch_id]+1) % DECT_EVENT_FIFO_SIZE;
		//printf("dect_event_rp=%d\n", dect_event_rp[ch_id]);
		//printf("output = %d\n", output);
	}

	MUTEXRELEASE_semDectEventFifoSem;

	return output;
}

int32 rtk_GetLineOccupyHS(uint32 line_id)
{
	return LineOccupyByHS[line_id];
}

#endif //CONFIG_RTK_VOIP_DECT_DSPG_SUPPORT

/*
 * @ingroup VOIP_DECT
 * @brief Set DECT power
 * @param power DECT power. 0: active, 1: inactive.
 * @retval 0 Success
 */
int32 rtk_SetDectPower( uint32 power )
{
	TstVoipSingleValue stVoipSingleValue;

	stVoipSingleValue.value = power;

	SETSOCKOPT(VOIP_MGR_DECT_SET_POWER, &stVoipSingleValue, TstVoipSingleValue, 1);

	dect_event_init();

	return 0;
}

/*
 * @ingroup VOIP_DECT
 * @brief Get DECT power
 * @retval DECT power. 0: active, 1: inactive.
 */
int32 rtk_GetDectPower( void )
{
	TstVoipSingleValue stVoipSingleValue;

	stVoipSingleValue.value = 0;

	SETSOCKOPT(VOIP_MGR_DECT_GET_POWER, &stVoipSingleValue, TstVoipSingleValue, 1);

	return stVoipSingleValue.value;
}

/*
 * @ingroup VOIP_DECT
 * @brief Get DECT page button
 * @retval DECT page. 0: active, 1: inactive.
 */
int32 rtk_GetDectPage( void )
{
	TstVoipSingleValue stVoipSingleValue;

	stVoipSingleValue.value = 0;

	SETSOCKOPT(VOIP_MGR_DECT_GET_PAGE, &stVoipSingleValue, TstVoipSingleValue, 1);

	return stVoipSingleValue.value;
}

/*
 * @ingroup VOIP_DECT
 * @brief Get DECT button event
 * @retval DECT button event.
 * @sa VEID_DECT_BUTTON_PAGE VEID_DECT_BUTTON_REGISTRATION_MODE
 * @sa VEID_DECT_BUTTON_DEL_HS VEID_DECT_BUTTON_NOT_DEFINED
 */
int32 rtk_GetDectButtonEvent( void )
{
	int ret;
	TstVoipEvent stVoipEvent;
	
	stVoipEvent.ch_id = 0;
	stVoipEvent.type = VET_DECT;
	stVoipEvent.mask = 0;
	
	if( ( ret = rtk_GetVoIPEvent( &stVoipEvent ) ) < 0 )
		return ret;
	
	return stVoipEvent.id;
}

#if 0	//thlin disable DECT LED temp
/*
 * @ingroup VOIP_DECT
 * @brief Set DECT LED
 * @param state 0: LED off, 1: LED on, 2: LED blinking
 * @retval 0 Success
 */
int32 rtk_SetDectLED( int chid, char state )
{
	TstVoipValue stVoipValue;

	stVoipValue.ch_id = chid;
	stVoipValue.value = state;

	SETSOCKOPT(VOIP_MGR_DECT_SET_LED, &stVoipValue, stVoipValue, 1);

	return stVoipValue.value;
}
#endif

/*
 * @ingroup VOIP_DECT
 * @brief Set DECT LED
 * @param state 0: LED off, 1: LED on, 2: LED blinking
 * @retval 0 Success
 */
int32 rtk_SetDectLED( int chid, char state )
{
	TstVoipValue stVoipValue;

	stVoipValue.ch_id = chid;
	stVoipValue.value = state;

	SETSOCKOPT(VOIP_MGR_DECT_SET_LED, &stVoipValue, stVoipValue, 1);

	return stVoipValue.value;
}

#endif //CONFIG_RTK_VOIP_DRIVERS_ATA_DECT

int32 rtk_InitDSP(int ch)
{
	// init rtp session
	rtk_OpenRtpSession(ch, 0);
	rtk_OpenRtpSession(ch, 1);

	// reset rtp and codec
	rtk_SetRtpSessionState(ch, 0, rtp_session_inactive);
	rtk_SetRtpSessionState(ch, 1, rtp_session_inactive);

	// reset resource
	rtk_SetTranSessionID(ch, 255);

	// Always enable FXO pcm for Caller ID detection.
	//if (ch >= RTK_VOIP_SLIC_NUM(g_VoIP_Feature))
	if( RTK_VOIP_IS_DAA_CH( ch, g_VoIP_Feature ) )
		rtk_enable_pcm(ch, 1);

    return 0;
}

int32 rtk_SetRtpConfig(rtp_config_t *cfg)
{
    TstVoipMgrSession stVoipMgrSession;

    stVoipMgrSession.ch_id = cfg->chid;
    stVoipMgrSession.m_id = cfg->sid;
    stVoipMgrSession.ip_src_addr = cfg->remIp;
    stVoipMgrSession.udp_src_port = cfg->remPort;
    stVoipMgrSession.ip_dst_addr = cfg->extIp;
    stVoipMgrSession.udp_dst_port = cfg->extPort;
#ifdef SUPPORT_VOICE_QOS
    stVoipMgrSession.tos = cfg->tos;
#endif
    stVoipMgrSession.rfc2833_dtmf_pt_local = cfg->rfc2833_payload_type_local;
    stVoipMgrSession.rfc2833_dtmf_pt_remote = cfg->rfc2833_payload_type_remote;
    stVoipMgrSession.rfc2833_fax_modem_pt_local = cfg->rfc2833_fax_modem_pt_local;
    stVoipMgrSession.rfc2833_fax_modem_pt_remote = cfg->rfc2833_fax_modem_pt_remote;
#ifdef CONFIG_RTK_VOIP_SRTP
	if(cfg->remoteCryptAlg == HMAC_SHA1){
		memcpy(stVoipMgrSession.remoteSrtpKey, cfg->remoteSrtpKey, SRTP_KEY_LEN);
		memcpy(stVoipMgrSession.localSrtpKey, cfg->localSrtpKey, SRTP_KEY_LEN);
		stVoipMgrSession.remoteCryptAlg = cfg->remoteCryptAlg;
	}
#endif /* CONFIG_RTK_VOIP_SRTP */
#ifdef SUPPORT_RTP_REDUNDANT
	stVoipMgrSession.rtp_redundant_payload_type_local = cfg ->rtp_redundant_payload_type_local;
	stVoipMgrSession.rtp_redundant_payload_type_remote = cfg ->rtp_redundant_payload_type_remote;
	stVoipMgrSession.rtp_redundant_max_Audio = cfg ->rtp_redundant_max_Audio;
	stVoipMgrSession.rtp_redundant_max_RFC2833 = cfg ->rtp_redundant_max_RFC2833;
#endif

#if 1
	stVoipMgrSession.SID_payload_type_local = cfg ->SID_payload_type_local;
	stVoipMgrSession.SID_payload_type_remote = cfg ->SID_payload_type_remote;
#else
	stVoipMgrSession.SID_payload_type_local = 0;
	stVoipMgrSession.SID_payload_type_remote = 0;
#endif

	stVoipMgrSession.init_randomly = cfg ->init_randomly;
	stVoipMgrSession.init_seqno = cfg ->init_seqno;
	stVoipMgrSession.init_SSRC = cfg ->init_SSRC;
	stVoipMgrSession.init_timestamp = cfg ->init_timestamp;
	
    if (cfg->isTcp)
    {
        printf("==> SUPPORT UDP ONLY\n");
        return -1;
    }
    else
    {
        stVoipMgrSession.protocol = 0x11;
    }

    SETSOCKOPT(VOIP_MGR_SET_SESSION, &stVoipMgrSession, TstVoipMgrSession, 1);

	return 0;//stVoipMgrSession.ret_val;
}

int32 rtk_SetT38UdpConfig(t38udp_config_t* cfg)
{
    TstVoipMgrSession stVoipMgrSession;

    stVoipMgrSession.ch_id = cfg->chid;
    stVoipMgrSession.m_id = cfg->sid;
    stVoipMgrSession.ip_src_addr = cfg->remIp;
    stVoipMgrSession.udp_src_port = cfg->remPort;
    stVoipMgrSession.ip_dst_addr = cfg->extIp;
    stVoipMgrSession.udp_dst_port = cfg->extPort;
#ifdef SUPPORT_VOICE_QOS
    stVoipMgrSession.tos = cfg->tos;
#endif
 
	stVoipMgrSession.rfc2833_dtmf_pt_local = 0;
    stVoipMgrSession.rfc2833_dtmf_pt_remote = 0;
    stVoipMgrSession.rfc2833_fax_modem_pt_local = 0;
    stVoipMgrSession.rfc2833_fax_modem_pt_remote = 0;
#ifdef CONFIG_RTK_VOIP_SRTP
	if(cfg->remoteCryptAlg == HMAC_SHA1){
		memcpy(stVoipMgrSession.remoteSrtpKey, cfg->remoteSrtpKey, SRTP_KEY_LEN);
		memcpy(stVoipMgrSession.localSrtpKey, cfg->localSrtpKey, SRTP_KEY_LEN);
		stVoipMgrSession.remoteCryptAlg = 0;
		???
	}
#endif /* CONFIG_RTK_VOIP_SRTP */
#ifdef SUPPORT_RTP_REDUNDANT
	stVoipMgrSession.rtp_redundant_payload_type_local = 0;
	stVoipMgrSession.rtp_redundant_payload_type_remote = 0;
	stVoipMgrSession.rtp_redundant_max_Audio = 0;
	stVoipMgrSession.rtp_redundant_max_RFC2833 = 0;
#endif

	stVoipMgrSession.SID_payload_type_local = 0;
	stVoipMgrSession.SID_payload_type_remote = 0;
	
	stVoipMgrSession.init_randomly = 1;
	stVoipMgrSession.init_seqno = 0;
	stVoipMgrSession.init_SSRC = 0;
	stVoipMgrSession.init_timestamp = 0;
   
    if (cfg->isTcp)
    {
        printf("==> SUPPORT UDP ONLY\n");
        return -1;
    }
    else
    {
        stVoipMgrSession.protocol = 0x11;
    }

    SETSOCKOPT(VOIP_MGR_SET_SESSION, &stVoipMgrSession, TstVoipMgrSession, 1);

	return 0;//stVoipMgrSession.ret_val;
}

#if 0
/*
 * @ingroup VOIP_RTP_SESSION
 * @brief Get RTP configuration
 * @param chid The channel number.
 * @param sid The session number.
 * @param cfg The RTP configuration.
 * @retval 0 Success
 * @note Not implement.
 */
int32 rtk_GetRtpConfig(uint32 chid, uint32 sid, rtp_config_t *cfg)
{
    return -1;
}
#endif

int32 rtk_SetRtcpConfig(rtcp_config_t *cfg)	//thlin+ for Rtcp
{
#ifdef SUPPORT_RTCP

    TstVoipRtcpSession stVoipRtcpSession;

    stVoipRtcpSession.ch_id = cfg->chid;
    stVoipRtcpSession.m_id = cfg->sid;
    stVoipRtcpSession.ip_src_addr = cfg->remIp;
    stVoipRtcpSession.rtcp_src_port = cfg->remPort;
    stVoipRtcpSession.ip_dst_addr = cfg->extIp;
    stVoipRtcpSession.rtcp_dst_port = cfg->extPort;
    
	stVoipRtcpSession.tx_interval = cfg ->txInterval;
#ifdef CONFIG_RTK_VOIP_RTCP_XR
    stVoipRtcpSession.enableXR = cfg->enableXR;
#else
    stVoipRtcpSession.enableXR = 0;
#endif
    //if (cfg->isTcp)
    //{
    //    printf("==> SUPPORT UDP ONLY\n");
    //    return -1;
    //}
    //else
    {
        stVoipRtcpSession.protocol = 0x11;
    }

    SETSOCKOPT(VOIP_MGR_SET_RTCP_SESSION, &stVoipRtcpSession, TstVoipRtcpSession, 1);

	//if (stVoipRtcpSession.ret_val != 0)
	//{
    //	return stVoipRtcpSession.ret_val;
    //}

#if 0	// merge into VOIP_MGR_SET_RTCP_SESSION
    TstVoipValue stVoipValue;
    stVoipValue.ch_id = cfg->chid;
    stVoipValue.value5 = rtcp_cfg->rtcp_tx_interval; /*unit: ms*/
    /* If rtcp_tx_interval is equal to 0, then RTCP Tx is disable.*/
    SETSOCKOPT(VOIP_MGR_SET_RTCP_TX_INTERVAL, &stVoipValue, TstVoipValue, 1);
#endif
	
	return 0;//stVoipValue.ret_val;

#else
    return 0;
#endif
}

int32 rtk_GetRtcpLogger( uint32 chid, uint32 sid, TstVoipRtcpLogger *logger )
{
	logger ->ch_id = chid;
	logger ->m_id = sid;

    SETSOCKOPT(VOIP_MGR_GET_RTCP_LOGGER, logger, TstVoipRtcpLogger, 1);

	return 0;
}

int32 rtk_SetRtpPayloadType(payloadtype_config_t *cfg)
{
    TstVoipPayLoadTypeConfig stVoipPayLoadTypeConfig;

    stVoipPayLoadTypeConfig.ch_id = cfg->chid;
    stVoipPayLoadTypeConfig.m_id = cfg->sid;
    stVoipPayLoadTypeConfig.local_pt = cfg->local_pt;
    stVoipPayLoadTypeConfig.remote_pt = cfg->remote_pt;
    stVoipPayLoadTypeConfig.uPktFormat = cfg->uPktFormat;
    stVoipPayLoadTypeConfig.nG723Type = cfg->nG723Type;
    stVoipPayLoadTypeConfig.nFramePerPacket = cfg->nFramePerPacket;
    stVoipPayLoadTypeConfig.bVAD = cfg->bVAD;
    stVoipPayLoadTypeConfig.bPLC = cfg->bPLC;
    stVoipPayLoadTypeConfig.nJitterDelay = cfg->nJitterDelay;
    stVoipPayLoadTypeConfig.nMaxDelay = cfg->nMaxDelay;
    stVoipPayLoadTypeConfig.nJitterFactor = cfg->nJitterFactor;
    stVoipPayLoadTypeConfig.nG726Packing = cfg->nG726Packing;
    stVoipPayLoadTypeConfig.bT38ParamEnable = 0;
    stVoipPayLoadTypeConfig.nT38MaxBuffer = 0;
    stVoipPayLoadTypeConfig.nT38RateMgt = 0;
	stVoipPayLoadTypeConfig.nT38MaxRate = 0;
	stVoipPayLoadTypeConfig.bT38EnableECM = 0;
	stVoipPayLoadTypeConfig.nT38ECCSignal = 0;
	stVoipPayLoadTypeConfig.nT38ECCData = 0;
	stVoipPayLoadTypeConfig.bT38EnableSpoof = 0;
	stVoipPayLoadTypeConfig.nT38DuplicateNum = 0;

#if 1	// V.152 
    stVoipPayLoadTypeConfig.local_pt_vbd = cfg->local_pt_vbd;
    stVoipPayLoadTypeConfig.remote_pt_vbd = cfg->remote_pt_vbd;
    stVoipPayLoadTypeConfig.uPktFormat_vbd = cfg->uPktFormat_vbd;	
    stVoipPayLoadTypeConfig.nFramePerPacket_vbd = cfg->nFramePerPacket_vbd;
#elif 0	
    stVoipPayLoadTypeConfig.local_pt_vbd = rtpPayloadUndefined;
    stVoipPayLoadTypeConfig.remote_pt_vbd = rtpPayloadUndefined;
    stVoipPayLoadTypeConfig.uPktFormat_vbd = rtpPayloadUndefined;
    stVoipPayLoadTypeConfig.nFramePerPacket_vbd = 1;
#else
    stVoipPayLoadTypeConfig.local_pt_vbd = 96;
    stVoipPayLoadTypeConfig.remote_pt_vbd = 96;
    stVoipPayLoadTypeConfig.uPktFormat_vbd = rtpPayloadPCMU;
    stVoipPayLoadTypeConfig.nFramePerPacket_vbd = 2;
#endif
    stVoipPayLoadTypeConfig.nG7111Mode = cfg->nG7111Mode;
    stVoipPayLoadTypeConfig.nPcmMode = cfg->nPcmMode;

    SETSOCKOPT(VOIP_MGR_SETRTPPAYLOADTYPE, &stVoipPayLoadTypeConfig, TstVoipPayLoadTypeConfig, 1);

	return 0;//stVoipPayLoadTypeConfig.ret_val;
}

int32 rtk_SetT38PayloadType(t38_payloadtype_config_t *cfg)
{
    TstVoipPayLoadTypeConfig stVoipPayLoadTypeConfig;
	
    stVoipPayLoadTypeConfig.ch_id = cfg->chid;
    stVoipPayLoadTypeConfig.m_id = cfg->sid;
    stVoipPayLoadTypeConfig.local_pt = 0;
    stVoipPayLoadTypeConfig.remote_pt = 0;
    stVoipPayLoadTypeConfig.uPktFormat = rtpPayloadT38_Virtual;
    stVoipPayLoadTypeConfig.nG723Type = 0;
    stVoipPayLoadTypeConfig.nFramePerPacket = 1;
    stVoipPayLoadTypeConfig.bVAD = 0;
    stVoipPayLoadTypeConfig.bPLC = 0;
    stVoipPayLoadTypeConfig.nJitterDelay = 4;
    stVoipPayLoadTypeConfig.nMaxDelay = 20;
    stVoipPayLoadTypeConfig.nJitterFactor = 1;
    stVoipPayLoadTypeConfig.nG726Packing = 0;
    stVoipPayLoadTypeConfig.bT38ParamEnable = cfg->bT38ParamEnable;
    stVoipPayLoadTypeConfig.nT38MaxBuffer = cfg->nT38MaxBuffer;
    stVoipPayLoadTypeConfig.nT38RateMgt = cfg->nT38RateMgt;
	stVoipPayLoadTypeConfig.nT38MaxRate = cfg->nT38MaxRate;
	stVoipPayLoadTypeConfig.bT38EnableECM = cfg->bT38EnableECM;
	stVoipPayLoadTypeConfig.nT38ECCSignal = cfg->nT38ECCSignal;
	stVoipPayLoadTypeConfig.nT38ECCData = cfg->nT38ECCData;
	stVoipPayLoadTypeConfig.bT38EnableSpoof = cfg->bT38EnableSpoof;
	stVoipPayLoadTypeConfig.nT38DuplicateNum = cfg->nT38DuplicateNum;

    stVoipPayLoadTypeConfig.local_pt_vbd = rtpPayloadUndefined;
    stVoipPayLoadTypeConfig.remote_pt_vbd = rtpPayloadUndefined;
    stVoipPayLoadTypeConfig.uPktFormat_vbd = rtpPayloadUndefined;
    stVoipPayLoadTypeConfig.nFramePerPacket_vbd = 1;
    
    stVoipPayLoadTypeConfig.nPcmMode = cfg->nPcmMode;

    SETSOCKOPT(VOIP_MGR_SETRTPPAYLOADTYPE, &stVoipPayLoadTypeConfig, TstVoipPayLoadTypeConfig, 1);

	return 0;//stVoipPayLoadTypeConfig.ret_val;
}

#if 0
/*
 * @ingroup VOIP_RTP_SESSION
 * @brief Get RTP payload type
 * @param chid The channel number.
 * @param sid The session number.
 * @param cfg The RTP payload configuration.
 * @retval 0 Success
 * @note Not implement.
 */
int32 rtk_GetRtpPayloadType(uint32 chid, uint32 sid, payloadtype_config_t *cfg)
{
    return -1;
}
#endif

int32 rtk_SetRtpSessionState(uint32 chid, uint32 sid, RtpSessionState state)
{
	TstVoipRtpSessionState stVoipRtpSessionState;
	stVoipRtpSessionState.ch_id = chid;
	stVoipRtpSessionState.m_id = sid;
	stVoipRtpSessionState.state = state;
	SETSOCKOPT(VOIP_MGR_SETRTPSESSIONSTATE, &stVoipRtpSessionState, TstVoipRtpSessionState, 1);

	//if (stVoipRtpSessionState.ret_val != 0)
	//{
	//	return stVoipRtpSessionState.ret_val;
	//}

    if ( rtp_session_inactive == state )   //disable both rx and tx
    {
		rtk_SetSessionInactive( chid, sid );
    }
    else
    	return 0;//stVoipRtpSessionState.ret_val;

    return 0;
}

int32 rtk_SetSessionInactive( uint32 chid, uint32 sid )
{
	TstVoipValue stVoipValue;
	TstVoipCfg stVoipCfg;
	
	stVoipCfg.ch_id = chid;
	stVoipCfg.m_id = sid;
	stVoipCfg.enable = 0;
	// close RTP
	SETSOCKOPT(VOIP_MGR_UNSET_SESSION, &stVoipCfg, TstVoipCfg, 1);
	//if (stVoipCfg.ret_val != 0)
	//{
	//	return stVoipCfg.ret_val;
	//}
	
#ifdef SUPPORT_RTCP
	// close RTCP
	SETSOCKOPT(VOIP_MGR_UNSET_RTCP_SESSION, &stVoipCfg, TstVoipCfg, 1); //thlin+ for Rtcp
	//if (stVoipCfg.ret_val != 0)
	//{
	//	return stVoipCfg.ret_val;
	//}
#endif

	// close Codec
	stVoipValue.ch_id = chid;
	stVoipValue.m_id = sid;
	SETSOCKOPT(VOIP_MGR_DSPCODECSTOP, &stVoipValue, TstVoipValue, 1);
	//if (stVoipValue.ret_val != 0)
	//{
	//	return stVoipValue.ret_val;
	//}
	
	return 0;
}

int32 rtk_SetThresholdVadCng( int32 chid, int32 mid, int32 nThresVAD, int32 nThresCNG, int32 nModeSID, int32 nLevelSID, int32 nGainSID )
{
	TstVoipThresVadCngConfig stVoipThresVadCngConfig;

	stVoipThresVadCngConfig.ch_id = chid;
	stVoipThresVadCngConfig.m_id = mid;
	stVoipThresVadCngConfig.nThresVAD = nThresVAD;
	stVoipThresVadCngConfig.nThresCNG = nThresCNG;
	stVoipThresVadCngConfig.nSIDMode = nModeSID;
	stVoipThresVadCngConfig.nSIDLevel = nLevelSID;
	stVoipThresVadCngConfig.nSIDGain = nGainSID;	

	SETSOCKOPT(VOIP_MGR_SET_VAD_CNG_THRESHOLD, &stVoipThresVadCngConfig, TstVoipThresVadCngConfig, 1);

	return 0;
}

#if 0
/*
 * @ingroup VOIP_RTP_SESSION
 * @brief Get RTP Session State
 * @param chid The channel number.
 * @param sid The session number.
 * @param pstate The RTP Session State.
 * @retval 0 Success
 * @note Not implement.
 */
int32 rtk_GetRtpSessionState(uint32 chid, uint32 sid, RtpSessionState *pstate)
{
    return -1;
}
#endif

int g_bDisableRingFXS = 0;

int32 rtk_DisableRingFXS(int bDisable)
{
	g_bDisableRingFXS = bDisable;
	return 0;
}

int32 rtk_SetRingFXS(uint32 chid, uint32 bRinging)
{
    TstVoipSlicRing stVoipSlicRing;

	if (g_bDisableRingFXS)
	{
		// quiet mode
		return 0;
	}

#ifdef CONFIG_RTK_VOIP_DRIVERS_ATA_DECT
	//if (chid < RTK_VOIP_DECT_NUM(g_VoIP_Feature))	//DECT Channel
	if( RTK_VOIP_IS_DECT_CH( chid, g_VoIP_Feature ) )
	{

#ifdef CONFIG_RTK_VOIP_DECT_SITEL_SUPPORT
		if( bRinging == 1 ) {
			dect_api_S2R_call_setup( 0, chid, "12345", "noname" );
		} else {
			if( dect_api_S2R_check_handset_ringing( chid ) )
				dect_api_S2R_call_release( chid );
		}
#endif

#ifdef CONFIG_RTK_VOIP_DECT_DSPG_SUPPORT
		if (bRinging == 1)
		{
			CMBS_Api_InBound_Ring_CallerID(chid, NULL, NULL);
		}
		else
		{
			if(CMBS_Api_LineStateGet(chid) == 2)//ringing
			{
				CMBS_Api_CallRelease(chid, 0);
			}
		}
#endif
		//stVoipSlicRing.ret_val = 0;
	}
	else
#endif
	//SLIC Channel
	{
		stVoipSlicRing.ch_id = chid;
		stVoipSlicRing.ring_set = bRinging;
		SETSOCKOPT(VOIP_MGR_SLIC_RING, &stVoipSlicRing, TstVoipSlicRing, 1);
	}

	return 0;//stVoipSlicRing.ret_val;
}


#if 0
/*
 * @ingroup VOIP_PHONE_RING
 * @brief Check which channel is ringing
 * @param chid The channel number.
 * @param pRinging The ringing state.
 * @retval 0 Success
 * @note Not implement.
 */
int32 rtk_GetRingFXS(uint32 chid, uint32 *pRinging)
{
    return -1;
}
#endif

#ifdef SUPPORT_IVR_HOLD_TONE

#define G729_HOLD_TONE		/* borrow g723 code to play */

#ifdef G729_HOLD_TONE
#define IVRCODEC		"729"
#define IVRFRAMESIZE	10
#define IVRFILLTWICE
#else
#define IVRCODEC		"723"
#define IVRFRAMESIZE	24
#undef  IVRFILLTWICE
#endif

FILE *g_fpG723[ 2 ];
int gStartG723[ 2 ];

void InitG723IvrFile( void )
{
	int i;

	for( i = 0; i < 2; i ++ ) {

		if( g_fpG723[ i ] )
			fclose( g_fpG723[ i ] );

		/* place G.723 sample file in /bin/723_raw */
		if( ( g_fpG723[ i ] = fopen( "/bin/" IVRCODEC "_raw", "rb" ) ) == NULL )
			printf( "Open /bin/" IVRCODEC "_raw fail(%d)\n", i );
	}
}

void StartG723IvrFile( int chid )
{
	gStartG723[ chid ] = 1;
}

void StopG723IvrFile( int chid )
{
	gStartG723[ chid ] = 0;
	rtk_IvrStopPlaying( chid );
}

void RefillG723IvrFile( int chid )
{
	/* Try to refill G723 IVR buffer periodically. */
	unsigned char buff723[ IVRFRAMESIZE * 10 ];
	unsigned int count723;
	unsigned int copied;
	FILE * const fp723 = g_fpG723[ chid ];
#ifdef IVRFILLTWICE
	unsigned int time = 2;
#endif

	if( !gStartG723[ chid ] )
		return;

#ifdef IVRFILLTWICE
	while( time -- )
#endif
	{
		count723 = fread( buff723, IVRFRAMESIZE, 10, fp723 );

		if( count723 ) {
	  #ifdef G729_HOLD_TONE
	  		copied = rtk_IvrStartPlayG729( chid, count723, buff723 );
	  #else
			copied = rtk_IvrStartPlayG72363( chid, count723, buff723 );
	  #endif

			if( count723 != copied )
				fseek( fp723, -IVRFRAMESIZE * ( int )( count723 - copied ), SEEK_CUR );
		}

		if( feof( fp723 ) )
			fseek( fp723, 0, SEEK_SET );
	}
}

#if 0
void main_program_pseudo_code( void )
{
	int chid;

	/* initialize once */
	InitG723IvrFile();

	while( 1 ) {
		/* receive message */
		/* Then, process this message or receive timeout. */

		for( chid = 0; chid < 2; chid ++ ) {

			/* refill IVR buffer in a certain period. */
			RefillG723IvrFile( chid );

			/* You may play hold tone somewhere, but it will play IVR. */
			{
				rtk_SetPlayTone( chid, sid, DSPCODEC_TONE_HOLD, 1, path );
			}
		}
	}

}
#endif
#endif /* SUPPORT_IVR_HOLD_TONE */

int32 rtk_SetPlayTone(uint32 chid, uint32 sid, DSPCODEC_TONE nTone, uint32 bFlag,
	DSPCODEC_TONEDIRECTION Path)
{
    TstVoipPlayToneConfig cfg;

#ifdef SUPPORT_IVR_HOLD_TONE
    static int bPrevHoldTone[ 2 ] = { 0, 0 };

    /* hold tone use ivr to play */
    if( nTone == DSPCODEC_TONE_HOLD && bFlag ) {
    	StartG723IvrFile( chid );
    	bPrevHoldTone[ chid ] = 1;
    	printf( "StartG723IvrFile(%d)\n", chid );
    	return;	/* don't play tone */
    } else if( bPrevHoldTone[ chid ] ) {
    	StopG723IvrFile( chid );
    	bPrevHoldTone[ chid ] = 0;
    	printf( "StopG723IvrFile(%d)\n", chid );
    }
#endif /* SUPPORT_IVR_HOLD_TONE */

	/* RING is incoming ring tone, and RINGING is ring back tone. */
    //if (nTone == DSPCODEC_TONE_RING)
    //    nTone = DSPCODEC_TONE_RINGING;

    cfg.ch_id = chid;
    cfg.m_id = sid;
    cfg.nTone = nTone;
    cfg.bFlag = bFlag;
    cfg.path = Path;

    SETSOCKOPT(VOIP_MGR_SETPLAYTONE, &cfg, TstVoipPlayToneConfig, 1);

	return 0;//cfg.ret_val;

}

#if 0
/*
 * @ingroup VOIP_PHONE_TONE
 * @brief Check tone of session
 * @param chid The channel number.
 * @param sid The session number.
 * @param pTone The tone type.
 * @param pFlag The tone state.
 * @param pPath The tone direction.
 * @retval 0 Success
 * @note Not implement.
 */
int32 rtk_GetPlayTone(uint32 chid, uint32 sid, DSPCODEC_TONE *pTone, uint32 *pFlag,
	DSPCODEC_TONEDIRECTION *pPath)
{
    return -1;
}
#endif

unsigned short rtk_VoIP_resource_check(uint32 chid, int payload_type)
{
	TstVoipCfg stVoipCfg;

	stVoipCfg.ch_id = chid;
	stVoipCfg.cfg = payload_type;
	SETSOCKOPT(VOIP_MGR_VOIP_RESOURCE_CHECK, &stVoipCfg, TstVoipCfg, 1);
	return stVoipCfg.enable;
}

int32 rtk_Onhook_Reinit(uint32 chid)
{
	TstVoipCfg stVoipCfg;

	stVoipCfg.ch_id = chid;

	/* when phone onhook, re-init CED detection */
	SETSOCKOPT(VOIP_MGR_ON_HOOK_RE_INIT, &stVoipCfg, TstVoipCfg, 1);

	return 0;//stVoipCfg.ret_val;

}

int32 rtk_GetDectEvent(uint32 chid, SIGNSTATE *pval)
{
#ifndef CONFIG_RTK_VOIP_DRIVERS_ATA_DECT
	*pval = SIGN_NONE;
	return 0;
#else
	TstVoipCfg stVoipCfg;
	TstVoipValue stVoipValue;
#ifdef CONFIG_RTK_VOIP_DECT_DSPG_SUPPORT
	int32 event;
#endif

	*pval = SIGN_NONE;

#ifdef CONFIG_RTK_VOIP_DECT_DSPG_SUPPORT
	event = rtk_GetDectButtonEvent();

	if (event == VEID_DECT_BUTTON_PAGE)
	{
		CMBS_Api_HandsetPage("all");
		//printf("page all HS\n");
	}
	else if (event == VEID_DECT_BUTTON_REGISTRATION_MODE)
	{
		CMBS_Api_RegistrationOpen();
		//printf("registration mode open\n");
	}
	else if (event == VEID_DECT_BUTTON_DEL_HS)
	{
		CMBS_Api_HandsetDelet("all");
		//printf("delete all registed HS\n");
	}
#endif

	stVoipValue.value = dect_event_out(chid);

	if ('Z' != stVoipValue.value)
	{

		// HS hook event
		if (stVoipValue.value == 0)
		{
			*pval = SIGN_ONHOOK;
			/* when phone onhook, stop play tone, disable pcm and DTMF detection */
			rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_DIAL, 0, DSPCODEC_TONEDIRECTION_LOCAL);
			usleep(100000); // [Important] sleep >= 100ms. MUST add delay for ACMW to stop tone!

			stVoipCfg.ch_id = chid;
			stVoipCfg.enable = 0;
			SETSOCKOPT(VOIP_MGR_PCM_CFG, &stVoipCfg, TstVoipCfg, 1);
			/* when phone onhook, re-init DSP */
			SETSOCKOPT(VOIP_MGR_ON_HOOK_RE_INIT, &stVoipCfg, TstVoipCfg, 1);
		}
		else if (stVoipValue.value == 1 || stVoipValue.value == 2)
		{
#ifdef CONFIG_RTK_VOIP_DECT_DSPG_SUPPORT
			CMBS_Api_OutboundAnswerCall(chid);
#endif

			*pval = SIGN_OFFHOOK;
			/* when phone offhook, enable pcm */
			stVoipCfg.ch_id = chid;
			//stVoipCfg.enable = 1;
			stVoipCfg.enable = stVoipValue.value;

			SETSOCKOPT(VOIP_MGR_PCM_CFG, &stVoipCfg, TstVoipCfg, 1);
		}
		else if (stVoipValue.value == 3)	// pkshih: change 2 to 3
		{
			*pval = SIGN_FLASHHOOK;
		}
		// HS key event
		else if (stVoipValue.value == '*')
		{
			*pval = SIGN_STAR;
		}
		else if (stVoipValue.value == '#')
		{
			*pval = SIGN_HASH;
		}
		else if (stVoipValue.value == '0')
		{
			*pval = SIGN_KEY0;
		}
		else if (stVoipValue.value >= '1' && stVoipValue.value <= '9')
		{
			*pval = SIGN_KEY1 + stVoipValue.value - '1';
		}
		else if (stVoipValue.value == 100)
		{
			//DECT on-hook of the HS which not occupy line
			*pval = SIGN_NONE;
			printf("DECT on-hook of the HS which not occupy line\n");
		}
		else if (stVoipValue.value == 101)
		{
			//DECT off-hook of the HS which not occupy line
#ifdef CONFIG_RTK_VOIP_DECT_DSPG_SUPPORT
			CMBS_Api_OutboundAnswerCall(chid);
#endif
			*pval = SIGN_NONE;
			printf("DECT off-hook of the HS which not occupy line\n");
		}
		else
		{
			*pval = SIGN_NONE;
		}


		return 0;

	}
	else
	{
		//printf("==>UNKOWN SLIC STATUS (%d)\n", chid);   /* HOOK FIFO empty also belong to this case */
		return -1;
	}
#endif
}


int32 rtk_GetDtmfEvent(uint32 chid, uint32 dir, int8 *pEvent, int8 *pEnergy, int16 *pDuration)
{
	int32 ret;
	TstVoipEvent stVoipEvent;

	*pEvent = 'X';
	
	stVoipEvent.ch_id = chid;
	stVoipEvent.type = VET_DTMF;
	stVoipEvent.mask = ( dir ? VEM_DIRIP : VEM_DIRTDM );
	
	if( ( ret = rtk_GetVoIPEvent( &stVoipEvent ) ) < 0 )
		return ret;
	
	if( stVoipEvent.id == VEID_NONE )
		*pEvent = 'Z';
	else
		*pEvent = ( stVoipEvent.id & VEID_DTMF_DIGIT_MASK );
	
	*pEnergy = stVoipEvent.p0;
	*pDuration = stVoipEvent.p1;
	
	return 0;
}

int32 rtk_GetSlicEvent(uint32 chid, SLICEVENT *pval)
{
	int32 ret;
	TstVoipEvent stVoipEvent;
	
	*pval = SLICEVENT_NONE;
	
	stVoipEvent.ch_id = chid;
	stVoipEvent.type = VET_HOOK;
	stVoipEvent.mask = 0;
	
	if( ( ret = rtk_GetVoIPEvent( &stVoipEvent ) ) < 0 )
		return ret;

	if( stVoipEvent.id == VEID_HOOK_PHONE_STILL_OFF_HOOK )
	{
		*pval = SLICEVENT_OFFHOOK_2;
	}
	else if( stVoipEvent.id == VEID_HOOK_PHONE_OFF_HOOK )
	{
		*pval = SLICEVENT_OFFHOOK; // off-hook
	}
	else if( stVoipEvent.id == VEID_HOOK_PHONE_ON_HOOK )
	{
		*pval = SLICEVENT_ONHOOK;	// on-hook
	}
	else if( stVoipEvent.id == VEID_HOOK_PHONE_FLASH_HOOK )
	{
		*pval = SLICEVENT_FLASHHOOK;
	}
	else if( stVoipEvent.id == VEID_HOOK_PHONE_STILL_ON_HOOK )
	{
	}
	else if( stVoipEvent.id == VEID_HOOK_FXO_RING_ON )
	{
		*pval = SLICEVENT_RING_ON;
	}
	else if( stVoipEvent.id == VEID_HOOK_FXO_RING_OFF )
	{
		*pval = SLICEVENT_RING_OFF;
	}
	else
	{
		printf("==>UNKOWN SLIC STATUS (%d)=%X\n", chid, stVoipEvent.id);   /* HOOK FIFO empty also belong to this case */
		return -1;
	}
	
	return 0;
}

int32 rtk_GetFxsEvent(uint32 chid, SIGNSTATE *pval)
{

	TstVoipCfg stVoipCfg;
	int8 Event;
	int8 Energy;
	int16 Duration;
	int32 ret;
	SLICEVENT SlicEvent = SLICEVENT_NONE;
	
	*pval = SIGN_NONE;

#ifdef NO_SLIC
return 0;
#endif

	if( ( ret = rtk_GetSlicEvent( chid, &SlicEvent ) ) < 0 )
		return ret;
	
	if ( SlicEvent == SLICEVENT_OFFHOOK_2 ) /* PHONE_STILL_OFF_HOOK */
	{
		// detect DTMF
		rtk_GetDtmfEvent(chid, 0, &Event, &Energy, &Duration);
		
		if (Event == 'E' )
			printf("dir=0, energey=%ddBFS, duration=%dms\n", Energy, Duration);
		
		if (Event >= '1' && Event <= '9')
			*pval = SIGN_KEY1 + Event - '1';
		else if (Event == '0')
			*pval = SIGN_KEY0;
		else if (Event == '*')
			*pval = SIGN_STAR;
		else if (Event == '#')
		    *pval = SIGN_HASH;
		else
			*pval = SIGN_OFFHOOK_2;
		
		/* Just Polling IP side DTMF FIFO to prevent from FIFO overflow */
		rtk_GetDtmfEvent(chid, 1, &Event, &Energy, &Duration);
		
		if (Event == 'E' )
			printf("dir=1, energey=%ddBFS, duration=%d0ms\n", Energy, Duration);

		return 0;
	}
	else if ( SlicEvent == SLICEVENT_OFFHOOK ) /* PHONE_OFF_HOOK */
	{
		*pval = SIGN_OFFHOOK; // off-hook
#if 1
		/* when phone offhook, enable pcm and DTMF detection */
		stVoipCfg.ch_id = chid;
		stVoipCfg.enable = 1;//stVoipSlicHook.hook_status;

		SETSOCKOPT(VOIP_MGR_PCM_CFG, &stVoipCfg, TstVoipCfg, 1);

		//if (stVoipCfg.ret_val != 0)
		//{
		//	return stVoipCfg.ret_val;
		//}

#if 0
		if (VOIP_RESOURCE_UNAVAILABLE == rtk_VoIP_resource_check(chid, NULL))
		{
			no_resource_flag[chid] = 1;
			*pval = 0;
			usleep(500000);//after phone off-hook, wait for a second,and then play IVR.
#if 1
			char text[]={IVR_TEXT_ID_NO_RESOURCE, '\0'};
			printf("play ivr (%d)...\n", chid);
			rtk_IvrStartPlaying( chid, IVR_DIR_LOCAL, text );
#else
			rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_BUSY, 1, DSPCODEC_TONEDIRECTION_LOCAL);
#endif
			return 0;
		}
#endif

		stVoipCfg.cfg = 0; /*dir0*/
		SETSOCKOPT(VOIP_MGR_DTMF_CFG, &stVoipCfg, TstVoipCfg, 1);
		//if (stVoipCfg.ret_val != 0)
		//{
		//	return stVoipCfg.ret_val;
		//}

		stVoipCfg.enable = 1;
		/* when phone offhook, enable fax detection */
		SETSOCKOPT(VOIP_MGR_FAX_OFFHOOK, &stVoipCfg, TstVoipCfg, 1);

		return 0;//stVoipCfg.ret_val;

#endif
	}
	else if ( SlicEvent == SLICEVENT_ONHOOK ) /* PHONE_ON_HOOK */
	{
		*pval = SIGN_ONHOOK;	// on-hook
#if 1
		/* when phone onhook, stop play tone, disable pcm and DTMF detection */
		rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_DIAL, 0, DSPCODEC_TONEDIRECTION_LOCAL);
		usleep(100000); // [Important] sleep >= 100ms. MUST add delay for ACMW to stop tone!

		stVoipCfg.ch_id = chid;
		stVoipCfg.enable = 0;//stVoipSlicHook.hook_status;
		SETSOCKOPT(VOIP_MGR_PCM_CFG, &stVoipCfg, TstVoipCfg, 1);
		//if (stVoipCfg.ret_val != 0)
		//{
		//	return stVoipCfg.ret_val;
		//}

#if 0
		if (no_resource_flag[chid] == 1)
		{
			no_resource_flag[chid] = 0;
			*pval = 0;
#if 1
			printf("stop play ivr(%d)...\n", chid);
			rtk_IvrStopPlaying(chid);
#else
			rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_BUSY, 0, DSPCODEC_TONEDIRECTION_LOCAL);
#endif
			return 0;
		}
#endif

		stVoipCfg.cfg = 0; /*dir0*/
		SETSOCKOPT(VOIP_MGR_DTMF_CFG, &stVoipCfg, TstVoipCfg, 1);
		//if (stVoipCfg.ret_val != 0)
		//{
		//	return stVoipCfg.ret_val;
		//}

		/* when phone onhook, re-init CED detection */
		SETSOCKOPT(VOIP_MGR_ON_HOOK_RE_INIT, &stVoipCfg, TstVoipCfg, 1);
		return 0;//stVoipCfg.ret_val;

#endif
	}
	else if (SlicEvent == SLICEVENT_FLASHHOOK) /* PHONE_FLASH_HOOK */
	{
		*pval = SIGN_FLASHHOOK;
		return 0;
	}
#if 0
	else if (stVoipSlicHook.hook_status == 3) /* PHONE_STILL_ON_HOOK */
	{
		return 0;
	}
#endif
	else if (SlicEvent == SLICEVENT_RING_ON) /* FXO_RING_ON */
	{
		*pval = SIGN_RING_ON;
		return 0;
	}
	else if (SlicEvent == SLICEVENT_RING_OFF)  /* FXO_RING_OFF */
	{
		*pval = SIGN_RING_OFF;
		return 0;
	}

	return 0;
}

int32 rtk_GetDaaEvent(uint32 chid, DAAEVENT *pval)
{
	int32 ret;
	TstVoipEvent stVoipEvent;
	
	*pval = DAAEVENT_NONE;
	
	stVoipEvent.ch_id = chid;
	stVoipEvent.type = VET_HOOK;
	stVoipEvent.mask = 0;
	
	if( ( ret = rtk_GetVoIPEvent( &stVoipEvent ) ) < 0 )
		return ret;
	
	if( stVoipEvent.id == VEID_HOOK_FXO_STILL_OFF_HOOK )
	{
		*pval = DAAEVENT_OFFHOOK_2;
	}
	else if( stVoipEvent.id == VEID_HOOK_FXO_RING_ON )
	{
		*pval = DAAEVENT_RING_ON; // off-hook
	}
	else if( stVoipEvent.id == VEID_HOOK_FXO_RING_OFF )
	{
		*pval = DAAEVENT_RING_STOP;
	}
	else if( stVoipEvent.id == VEID_HOOK_FXO_BUSY_TONE )
	{
		*pval = DAAEVENT_BUSY_TONE;
	}
	else if( stVoipEvent.id == VEID_HOOK_FXO_DIS_TONE )
	{
		*pval = DAAEVENT_DIST_TONE;
	}
	else if( stVoipEvent.id == VEID_HOOK_FXO_OFF_HOOK )
	{
	}
	else if( stVoipEvent.id == VEID_HOOK_FXO_ON_HOOK )
	{
	}
	else if( stVoipEvent.id == VEID_HOOK_FXO_FLASH_HOOK )
	{
	}
	else if( stVoipEvent.id == VEID_HOOK_FXO_STILL_ON_HOOK )
	{
	}
	else if( stVoipEvent.id == VEID_HOOK_FXO_CALLER_ID )
	{
	}
	else if( stVoipEvent.id == VEID_HOOK_FXO_POLARITY_REVERSAL )
	{
		//printf("Get FXOEVENT_POLARITY_REVERSAL, ch=%d\n", chid);
	}
	else if( stVoipEvent.id == VEID_HOOK_FXO_CURRENT_DROP )
	{
		//printf("Get FXOEVENT_BAT_DROP_OUT, ch=%d\n", chid);
	}
	else
	{
		printf("==>UNKOWN FXO STATUS (%d) = %d\n", chid, stVoipEvent.id);
		return -1;
	}
	
	return 0;
}

int32 rtk_GetFxoEvent(uint32 chid, SIGNSTATE *pval)
{

	TstVoipCfg stVoipCfg;
	int8 Event;
	int8 Energy;
	int16 Duration;
	DAAEVENT DaaEvent = DAAEVENT_NONE;
	int ret;

	*pval = SIGN_NONE;

	if( ( ret = rtk_GetDaaEvent( chid, &DaaEvent ) ) < 0 )
		return ret;

	if ( DaaEvent == DAAEVENT_OFFHOOK_2 ) /* FXO_STILL_OFF_HOOK */
	{
		// detect DTMF
		rtk_GetDtmfEvent(chid, 0, &Event, &Energy, &Duration);
		
		if (Event == 'E' )
			printf("dir=0, energey=%ddBFS, duration=%d0ms\n", Energy, Duration);
		
		if (Event >= '1' && Event <= '9')
			*pval = SIGN_KEY1 + Event - '1';
		else if (Event == '0')
			*pval = SIGN_KEY0;
		else if (Event == '*')
			*pval = SIGN_STAR;
		else if (Event == '#')
		    *pval = SIGN_HASH;
		else
			*pval = SIGN_OFFHOOK_2;
		
		/* Just Polling IP side DTMF FIFO to prevent from FIFO overflow */
		rtk_GetDtmfEvent(chid, 1, &Event, &Energy, &Duration);
		
		if (Event == 'E' )
			printf("dir=1, energey=%ddBFS, duration=%d0ms\n", Energy, Duration);
		
		return 0;
	}
	else if ( DaaEvent == DAAEVENT_RING_ON ) /* FXO_RING_ON */
	{
		*pval = SIGN_OFFHOOK; // off-hook

#if 0
		if (VOIP_RESOURCE_UNAVAILABLE == rtk_VoIP_resource_check(chid, NULL))
		{
			no_resource_flag[chid] = 1;
			*pval = 0;
#ifdef DAA_IVR
			usleep(500000);//after phone off-hook, wait for a second,and then play IVR.
			char text[]={IVR_TEXT_ID_NO_RESOURCE, '\0'};
			printf("play ivr...\n");
			rtk_FXO_offhook(chid);
			rtk_IvrStartPlaying( chid, IVR_DIR_LOCAL, text );
			//rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_BUSY, 1, DSPCODEC_TONEDIRECTION_LOCAL);
#else
#endif
			return 0;
		}
#endif

		stVoipCfg.ch_id = chid;
		stVoipCfg.enable = 1;
		 /* when phone offhook, enable fax detection */
		SETSOCKOPT(VOIP_MGR_FAX_OFFHOOK, &stVoipCfg, TstVoipCfg, 1);
		return 0;//stVoipCfg.ret_val;
	}
	else if ( 
			DaaEvent == DAAEVENT_RING_STOP ||	 /* FXO_RING_OFF */
			DaaEvent == DAAEVENT_BUSY_TONE ||	/* FXO_BUSY_TONE */
			DaaEvent == DAAEVENT_DIST_TONE )   /* FXO_DIS_TONE */
	{
       	*pval = SIGN_ONHOOK;	// on-hook
		rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_DIAL, 0, DSPCODEC_TONEDIRECTION_LOCAL);
		usleep(100000); // [Important] sleep >= 100ms. MUST add delay for ACMW to stop tone!

#if 0
		if (no_resource_flag[chid] == 1)
		{
			no_resource_flag[chid] = 0;
			*pval = 0;
#ifdef DAA_IVR
			printf("stop play ivr...\n");
			rtk_IvrStopPlaying(chid);
			rtk_FXO_onhook(chid);
#endif
			//rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_BUSY, 0, DSPCODEC_TONEDIRECTION_LOCAL);
			return 0;
		}
#endif

		/* when phone onhook, re-init CED detection */
		stVoipCfg.ch_id = chid;
		SETSOCKOPT(VOIP_MGR_ON_HOOK_RE_INIT, &stVoipCfg, TstVoipCfg, 1);
		return 0;//stVoipCfg.ret_val;
	}
#if 0
	else if (stVoipSlicHook.hook_status == 7) /* FXO_OFF_HOOK */
	{
		return 0;
	}
	else if (stVoipSlicHook.hook_status == 6) /* FXO_ON_HOOK */
	{
		return 0;
	}
	else if (stVoipSlicHook.hook_status == 8) /* FXO_FLASH_HOOK */
	{
		*pval = SIGN_FLASHHOOK;	// flash-hook
		return 0;
	}
	else if (stVoipSlicHook.hook_status == 9) /* FXO_STILL_ON_HOOK */
	{
		return 0;
	}
	else if (stVoipSlicHook.hook_status == 14) /* FXO_CALLER_ID */
	{
		return 0;
	}
	else if (stVoipSlicHook.hook_status == 17)   /* FXO_POLARITY_REVERSAL*/
	{
		//printf("Get FXOEVENT_POLARITY_REVERSAL, ch=%d\n", chid);
		return 0;
	}
	else if (stVoipSlicHook.hook_status == 18)   /* FXO_CURRENT_DROP*/
	{
		//printf("Get FXOEVENT_BAT_DROP_OUT, ch=%d\n", chid);
		return 0;
	}
#endif

	return 0;
}


int32 rtk_GetRealFxoEvent(uint32 chid, FXOEVENT *pval)
{
	int32 ret;
	TstVoipEvent stVoipEvent;
	int8 Event;
	int8 Energy;
	int16 Duration;
	
	*pval = FXOEVENT_NONE;
	
	stVoipEvent.ch_id = chid;
	stVoipEvent.type = VET_HOOK;
	stVoipEvent.mask = 0;
	
	if( ( ret = rtk_GetVoIPEvent( &stVoipEvent ) ) < 0 )
		return ret;

	if( stVoipEvent.id == VEID_HOOK_FXO_STILL_OFF_HOOK )
	{
		// detect DTMF
		rtk_GetDtmfEvent(chid, 0, &Event, &Energy, &Duration);
		
		if (Event == 'E' )
			printf("dir=0, energey=%ddBFS, duration=%d0ms\n", Energy, Duration);
		
		if (Event >= '1' && Event <= '9')
			*pval = SIGN_KEY1 + Event - '1';
		else if (Event == '0')
			*pval = SIGN_KEY0;
		else if (Event == '*')
			*pval = SIGN_STAR;
		else if (Event == '#')
		    *pval = SIGN_HASH;
		else
			*pval = SIGN_OFFHOOK_2;
		
		/* Just Polling IP side DTMF FIFO to prevent from FIFO overflow */
		rtk_GetDtmfEvent(chid, 1, &Event, &Energy, &Duration);

		if (Event == 'E' )
			printf("dir=1, energey=%ddBFS, duration=%d0ms\n", Energy, Duration);
		
		return 0;
	}
	else if( stVoipEvent.id == VEID_HOOK_FXO_RING_ON )
	{
		*pval = FXOEVENT_RING_START;
		printf("Get FXOEVENT_RING_START, ch=%d\n", chid);
		return 0;
	}
	else if( stVoipEvent.id == VEID_HOOK_FXO_RING_OFF )
	{
		*pval = FXOEVENT_RING_STOP;
		printf("Get FXOEVENT_RING_STOP, ch=%d\n", chid);
		return 0;
	}
	else if( stVoipEvent.id == VEID_HOOK_FXO_BUSY_TONE )
	{
		*pval = FXOEVENT_BUSY_TONE;
		printf("Get FXOEVENT_BUSY_TONE, ch=%d\n", chid);
		return 0;
	}
	else if( stVoipEvent.id == VEID_HOOK_FXO_POLARITY_REVERSAL )
	{
		*pval = FXOEVENT_POLARITY_REVERSAL;
		printf("Get FXOEVENT_POLARITY_REVERSAL, ch=%d\n", chid);
		return 0;
	}
	else if( stVoipEvent.id == VEID_HOOK_FXO_CURRENT_DROP )
	{
		*pval = FXOEVENT_BAT_DROP_OUT;
		printf("Get FXOEVENT_BAT_DROP_OUT, ch=%d\n", chid);
		return 0;
	}
	else if( stVoipEvent.id == VEID_HOOK_FXO_DIS_TONE )
	{
		*pval = FXOEVENT_DIST_TONE;
		printf("Get FXOEVENT_DIST_TONE, ch=%d\n", chid);
		return 0;
	}
	else if( stVoipEvent.id == VEID_HOOK_FXO_OFF_HOOK )
	{
		return 0;
	}
	else if( stVoipEvent.id == VEID_HOOK_FXO_ON_HOOK )
	{
		return 0;
	}
	else if( stVoipEvent.id == VEID_HOOK_FXO_FLASH_HOOK )
	{
		return 0;
	}
	else if( stVoipEvent.id == VEID_HOOK_FXO_STILL_ON_HOOK )
	{
		return 0;
	}
	else if( stVoipEvent.id == VEID_HOOK_FXO_CALLER_ID )
	{
		return 0;
	}
	else
	{
		printf("==>UNKOWN FXO STATUS (%d) = %d\n", chid, stVoipEvent.id);
		return -1;
	}
}

int32 rtk_GetVoIPEvent( TstVoipEvent *stVoipEvent )
{
	SETSOCKOPT( VOIP_MGR_GET_VOIP_EVENT, stVoipEvent, TstVoipEvent, 1 );
	
	return 0;
}

int32 rtk_FlushVoIPEvent( TstFlushVoipEvent *stFlushVoipEvent )
{
	SETSOCKOPT( VOIP_MGR_FLUSH_VOIP_EVENT, stFlushVoipEvent, TstFlushVoipEvent, 1 );
	
	return 0;
}

/*
 * @ingroup VOIP_RTP_SESSION
 * @brief Init the RTP session
 * @param chid The channel number.
 * @param sid The session number.
 * @retval 0 Success
 * @note internal use
 */
int32 rtk_OpenRtpSession(uint32 chid, uint32 sid)
{
	TstVoipCfg stVoipCfg;
	
	stVoipCfg.ch_id = chid;
	stVoipCfg.m_id = sid;
	stVoipCfg.enable = 1;
	SETSOCKOPT(VOIP_MGR_CTRL_RTPSESSION, &stVoipCfg, TstVoipCfg, 1);
	return 0;//stVoipCfg.ret_val;
}

int32 rtk_SetTranSessionID(uint32 chid, uint32 sid)
{
	TstVoipCfg stVoipCfg;
	
	stVoipCfg.ch_id = chid;
	stVoipCfg.m_id = sid;
	stVoipCfg.enable = 1;
	SETSOCKOPT(VOIP_MGR_CTRL_TRANSESSION_ID, &stVoipCfg, TstVoipCfg, 1);
	return 0;//stVoipCfg.ret_val;
}

int32 rtk_GetTranSessionID(uint32 chid, uint32* psid)
{
	TstVoipCfg stVoipCfg;
	
	stVoipCfg.ch_id = chid;
//        stVoipCfg.m_id = sid;
	stVoipCfg.enable = 0;
	SETSOCKOPT(VOIP_MGR_CTRL_TRANSESSION_ID, &stVoipCfg, TstVoipCfg, 1);
	*psid = stVoipCfg.t_id;
	return 0;//stVoipCfg.ret_val;
}

int32 rtk_SetConference(TstVoipMgr3WayCfg *stVoipMgr3WayCfg)
{
	SETSOCKOPT(VOIP_MGR_SETCONFERENCE, stVoipMgr3WayCfg, TstVoipMgr3WayCfg, 1);
	return 0;//stVoipMgr3WayCfg->ret_val;
}

// 0:rfc2833  1: sip info  2: inband  3: DTMF delete
int32 rtk_SetDTMFMODE(uint32 chid, uint32 mode)
{
	TstVoipCfg stVoipCfg;

	stVoipCfg.ch_id = chid;
	stVoipCfg.enable = mode;

	SETSOCKOPT(VOIP_MGR_SET_DTMF_MODE, &stVoipCfg, TstVoipCfg, 1);
	return 0;//stVoipCfg.ret_val;

}

#if 0
/*
 * @ingroup VOIP_RTP_DTMF
 * @brief Get the DTMF mode
 * @param chid The channel number.
 * @param sid The session number.
 * @param pmode The DTMF mode.
 * @retval 0 Success
 * @note Not implement.
 */
int32 rtk_GetDTMFMODE(uint32 chid, uint32 sid, uint32 *pmode)
{
	return -1;
}
#endif

// thlin 2006-05-04
int32 rtk_Set_echoTail(uint32 chid, uint32 echo_tail, uint32 nlp)
{
	TstVoipValue stVoipValue;

	stVoipValue.ch_id = chid;
	stVoipValue.value = echo_tail;
	stVoipValue.value1 = nlp;

	SETSOCKOPT(VOIP_MGR_SET_ECHO_TAIL_LENGTH, &stVoipValue, TstVoipValue, 1);

	return 0;//stVoipValue.ret_val;

}

int32 rtk_Set_G168_LEC(uint32 chid, uint32 support_lec)	/* This function can turn on/off G168 LEC. */
{
        TstVoipCfg stVoipCfg;
        stVoipCfg.ch_id = chid;
        stVoipCfg.enable = support_lec;

        SETSOCKOPT(VOIP_MGR_SET_G168_LEC_CFG, &stVoipCfg, TstVoipCfg, 1);

	return 0;//stVoipCfg.ret_val;
}

int32 rtk_Set_VBD_EC(uint32 chid, uint32 vbd_high_ec_mode, uint32 vbd_low_ec_mode, uint32 ec_restore_val)
{
        TstVoipCfg stVoipCfg;
        stVoipCfg.ch_id = chid;
        stVoipCfg.cfg = vbd_high_ec_mode;
        stVoipCfg.cfg2 = vbd_low_ec_mode;
        stVoipCfg.cfg3 = ec_restore_val;

        SETSOCKOPT(VOIP_MGR_SET_VBD_EC, &stVoipCfg, TstVoipCfg, 1);
        
	return 0;//stVoipCfg.ret_val;
}

int32 rtk_Set_GET_EC_DEBUG(TstVoipEcDebug* pstVoipEcDebug)
{
	SETSOCKOPT(VOIP_MGR_GET_EC_DEBUG, pstVoipEcDebug, TstVoipEcDebug, 1);

	return 0;//pstVoipEcDebug->ret_val;
}

#if 0
// thlin 2006-05-04
/**
 * @ingroup VOIP_PHONE_FXS
 * @brief Set the Tx Gain of FXS
 * @param chid The channel number.
 * @param gain The gain value 0..9 means -6~+3db. Default value is 6 (0db).
 * @retval 0 Success
 */
int32 rtk_Set_SLIC_Tx_Gain(uint32 chid, uint32 gain)
{
	TstVoipValue stVoipValue;

	stVoipValue.ch_id = chid;
	stVoipValue.value = gain;

	SETSOCKOPT(VOIP_MGR_SET_SLIC_TX_GAIN, &stVoipValue, TstVoipValue, 1);

	return 0;
}

// thlin 2006-05-04
/**
 * @ingroup VOIP_PHONE_FXS
 * @brief Set the Rx Gain of FXS
 * @param chid The channel number.
 * @param gain The gain value 0..9 means -6~+3db. Default value is 6 (0db).
 * @retval 0 Success
 */
int32 rtk_Set_SLIC_Rx_Gain(uint32 chid, uint32 gain)
{
	TstVoipValue stVoipValue;

	stVoipValue.ch_id = chid;
	stVoipValue.value = gain;

	SETSOCKOPT(VOIP_MGR_SET_SLIC_RX_GAIN, &stVoipValue, TstVoipValue, 1);

	return 0;
}
#endif

int32 rtk_Set_SLIC_Ring_Cadence(uint32 chid, uint16 cad_on_msec, uint16 cad_off_msec)
{
	TstVoipValue stVoipValue;

	stVoipValue.ch_id = chid;
	stVoipValue.value5 = cad_on_msec;
	stVoipValue.value6 = cad_off_msec;

	gCad_on_msec[chid] = cad_on_msec;
	gCad_off_msec[chid] = cad_off_msec;

	SETSOCKOPT(VOIP_MGR_SET_SLIC_RING_CADENCE, &stVoipValue, TstVoipValue, 1);

	return 0;//stVoipValue.ret_val;
}

int32 rtk_Set_SLIC_Ring_Freq_Amp(uint32 chid, uint8 preset)
{
	TstVoipValue stVoipValue;

	stVoipValue.ch_id = chid;
	stVoipValue.value = preset;

	SETSOCKOPT(VOIP_MGR_SET_SLIC_RING_FRQ_AMP, &stVoipValue, TstVoipValue, 1);

	rtk_Set_SLIC_Ring_Cadence(chid, gCad_on_msec[chid], gCad_off_msec[chid]);

	return 0;//stVoipValue.ret_val;
}

// flag: line voltage flag. 0: zero voltage, 1: normal voltage, 2: reverse voltage
int32 rtk_Set_SLIC_Line_Voltage(uint32 chid, uint8 flag)
{
	TstVoipValue stVoipValue;

        stVoipValue.ch_id = chid;
        stVoipValue.value = flag;

	SETSOCKOPT(VOIP_MGR_SET_SLIC_LINE_VOLTAGE, &stVoipValue, TstVoipValue, 1);

	return 0;//stVoipValue.ret_val;
}

int32 rtk_Gen_SLIC_CPC(uint32 chid, uint32 cpc_ms)
{
        TstVoipCfg stVoipCfg;
        stVoipCfg.ch_id = chid;
        stVoipCfg.cfg = cpc_ms;
        SETSOCKOPT(VOIP_MGR_GEN_SLIC_CPC, &stVoipCfg, TstVoipCfg, 1);

	return 0;//stVoipCfg.ret_val;
}

int32 rtk_Set_FXO_Ring_Detection(uint16 ring_on_msec, uint16 first_ringoff_msec, uint16 ring_off_msec)
{
        TstVoipValue stVoipValue;

        stVoipValue.value5 = ring_on_msec;
        stVoipValue.value6 = first_ringoff_msec;
        stVoipValue.value7 = ring_off_msec;

        SETSOCKOPT(VOIP_MGR_SET_RING_DETECTION, &stVoipValue, TstVoipValue, 1);

	return 0;//stVoipValue.ret_val;
}

// thlin 2006-05-10
int32 rtk_Set_DAA_Tx_Gain(uint32 gain)
{

	if ((g_VoIP_Feature & DAA_TYPE_MASK) == REAL_DAA || (g_VoIP_Feature & DAA_TYPE_MASK) == REAL_DAA_NEGO)
	{
		TstVoipValue stVoipValue;

		stVoipValue.value = gain;

		SETSOCKOPT(VOIP_MGR_SET_DAA_TX_GAIN, &stVoipValue, TstVoipValue, 1);
		return 0;//stVoipValue.ret_val;
	}
	return 0;

}

// thlin 2006-05-10
int32 rtk_Set_DAA_Rx_Gain(uint32 gain)
{

	if ((g_VoIP_Feature & DAA_TYPE_MASK) == REAL_DAA || (g_VoIP_Feature & DAA_TYPE_MASK) == REAL_DAA_NEGO)
	{
		TstVoipValue stVoipValue;

		stVoipValue.value = gain;

		SETSOCKOPT(VOIP_MGR_SET_DAA_RX_GAIN, &stVoipValue, TstVoipValue, 1);

		return 0;//stVoipValue.ret_val;
	}
	return 0;

}

// thlin 2006-06-07
int32 rtk_Set_Flash_Hook_Time(uint32 chid, uint32 min_time, uint32 time)
{
	TstVoipHook stHookTime;

	stHookTime.ch_id = chid;
	stHookTime.flash_time_min = min_time;
	stHookTime.flash_time = time;

	SETSOCKOPT(VOIP_MGR_SET_FLASH_HOOK_TIME, &stHookTime, TstVoipHook, 1);

	return 0;//stHookTime.ret_val;
}

int32 rtk_SetRFC2833SendByAP(uint32 chid, uint32 config)
{
	TstVoipCfg stVoipCfg;

	stVoipCfg.ch_id = chid;
	stVoipCfg.enable = config;
	SETSOCKOPT(VOIP_MGR_SEND_RFC2833_BY_AP, &stVoipCfg, TstVoipCfg, 1);

	return 0;//stVoipCfg.ret_val;
}

int32 rtk_SetRFC2833TxConfig(uint32 chid, uint32 tx_mode, RFC2833_VOLUME_MODE volume_mode, RFC2833_VOLUME volume) //tx_mode: 0: DSP mode, 1: AP mode
{
	TstVoip2833 stRFC2833;
	TstVoipCfg stVoipCfg;

	if (volume_mode == RFC2833_VOLUME_DSP_ATUO)
	{
		if (tx_mode == 1) //send by AP
			printf("rtk_SetRFC2833TxConfig: invalid setting. TX mode is AP, but volume is DSP auto mode. ch=%d\n", chid);
		
		return -3;
	}
	
	// TX mode must be set prior to TX volume.
	stVoipCfg.ch_id = chid;
	stVoipCfg.enable = tx_mode;
	SETSOCKOPT(VOIP_MGR_SEND_RFC2833_BY_AP, &stVoipCfg, TstVoipCfg, 1);
	
	stRFC2833.ch_id = chid;
	stRFC2833.bEnable = volume_mode;
	stRFC2833.volume = volume;
	SETSOCKOPT(VOIP_MGR_SET_RFC2833_TX_VOLUME, &stRFC2833, TstVoip2833, 1);

	return 0; //(stRFC2833.ret_val | stVoipCfg.ret_val);
}

int32 rtk_SetRTPRFC2833(uint32 chid, uint32 sid, uint32 event, unsigned int duration)
{
	TstVoip2833 stRFC2833;

	stRFC2833.ch_id = chid;
	stRFC2833.m_id = sid;
	stRFC2833.digit = event;
	stRFC2833.duration = duration; //unit: msec
	SETSOCKOPT(VOIP_MGR_SEND_RFC2833_PKT_CFG, &stRFC2833, TstVoip2833, 1);

	return 0;//stRFC2833.ret_val;
}

int32 rtk_LimitMaxRfc2833DtmfDuration(uint32 chid, uint32 duration_in_ms, uint8 bEnable)
{
	TstVoip2833 stRFC2833;
	stRFC2833.ch_id = chid;
	stRFC2833.duration = duration_in_ms;
	stRFC2833.bEnable = bEnable;

	SETSOCKOPT(VOIP_MGR_LIMIT_MAX_RFC2833_DTMF_DURATION, &stRFC2833, TstVoip2833, 1);

	return 0;//stRFC2833.ret_val;
}

int32 rtk_GetRfc2833RxEvent(uint32 chid, uint32 mid, VoipEventID *pent)
{
	int ret;
	TstVoipEvent stVoipEvent;
	
	stVoipEvent.ch_id = chid;
	stVoipEvent.type = VET_RFC2833;
	stVoipEvent.mask = ( mid ? VEM_MID1 : VEM_MID0 );
	
	if( ( ret = rtk_GetVoIPEvent( &stVoipEvent ) ) < 0 )
		return ret;
	
	*pent = stVoipEvent.id;
	
	return 0;
}

int32 rtk_SetFaxModemRfc2833(uint32 chid, uint32 relay_flag, uint32 removal_flag, uint32 tone_gen_flag)
{
	TstVoipCfg stVoipCfg;

	stVoipCfg.ch_id = chid;
	stVoipCfg.cfg = relay_flag;
	stVoipCfg.cfg2 = removal_flag;
	stVoipCfg.cfg3 = tone_gen_flag;
	
	SETSOCKOPT(VOIP_MGR_FAX_MODEM_RFC2833_CFG, &stVoipCfg, TstVoipCfg, 1);
	
	return 0;
}

int32 rtk_SetRfc2833PacketInterval(uint32 chid, uint32 mid, uint32 type, uint32 interval)
{
	TstVoipCfg stVoipCfg;

	stVoipCfg.ch_id = chid;
	stVoipCfg.m_id = mid;
	stVoipCfg.cfg = type;		//0: DTMF, 1: Fax/Modem
	stVoipCfg.cfg2 = interval;	//msec: must be multiple of 10msec

	SETSOCKOPT(VOIP_MGR_RFC2833_PKT_INTERVAL_CFG, &stVoipCfg, TstVoipCfg, 1);
	
	return 0;
}

/*
 * @ingroup VOIP_PHONE_CALLERID
 * @brief Generate CallerID via DTMF
 * @param chid The channel number.
 * @param pval The Caller ID.
 * @retval 0 Success
 * @note internal use
 */
int32 rtk_Set_Dtmf_CID_String(uint32 chid, const char *str_cid)
{
	TstVoipCID stCIDstr;

	stCIDstr.ch_id = chid;
	strcpy(stCIDstr.string, str_cid);
	SETSOCKOPT(VOIP_MGR_DTMF_CID_GEN_CFG, &stCIDstr, TstVoipCID, 1);

	return 0;//stCIDstr.ret_val;
}

/*
 * @ingroup VOIP_PHONE_CALLERID
 * @brief Check the CallerID generation via DTMF is done
 * @param chid The channel number.
 * @param pstate 0 if done
 * @retval 0 Success
 * @note internal use
 */
static int32 rtk_GetDtmfCIDState(uint32 chid, uint32 *cid_state)
{
	TstVoipCID stCIDstr;

	stCIDstr.ch_id = chid;
	SETSOCKOPT(VOIP_MGR_GET_CID_STATE_CFG, &stCIDstr, TstVoipCID, 1);
	*cid_state = stCIDstr.cid_state;

	return 0;//stCIDstr.ret_val;
}

int32 rtk_GetFaxModemEvent(uint32 chid, uint32 *pval, VoipEventID *pevent, uint32 flush)
{
	int ret;
	
	if( flush ) {
		
		TstFlushVoipEvent stFlushVoipEvent;
		
		stFlushVoipEvent.ch_id = chid;
		stFlushVoipEvent.type = VET_FAXMDM;
		stFlushVoipEvent.mask = 0;
		
		if( ( ret = rtk_FlushVoIPEvent( &stFlushVoipEvent ) ) < 0 )
			return ret;
			
	} else {
	
		TstVoipEvent stVoipEvent;
		
		stVoipEvent.ch_id = chid;
		stVoipEvent.type = VET_FAXMDM;
		stVoipEvent.mask = 0;
		
		if( ( ret = rtk_GetVoIPEvent( &stVoipEvent ) ) < 0 )
			return ret;	
		
		*pval = stVoipEvent.p1;		// p1 is translated ID in FAX/Modem event 
		
		if( pevent )
			*pevent = stVoipEvent.id;
	}
	
	return 0;
}

int32 rtk_GetFaxEndDetect(uint32 chid, uint32 *pval)
{
	TstVoipCfg stVoipCfg;
	stVoipCfg.ch_id = chid;
	SETSOCKOPT(VOIP_MGR_FAX_END_DETECT, &stVoipCfg, TstVoipCfg, 1);
	*pval = stVoipCfg.enable;
	//t.38 fax end detect, 1:fax end.

	return 0;//stVoipCfg.ret_val;
}

int32 rtk_GetFaxDisDetect(uint32 chid, uint32 *pval)
{
	TstVoipCfg stVoipCfg;
	stVoipCfg.ch_id = chid;
	SETSOCKOPT(VOIP_MGR_FAX_DIS_DETECT, &stVoipCfg, TstVoipCfg, 1);
	*pval = stVoipCfg.enable;
	//t.38 v21 dis detect, 1:dis(Digital Identification Signal) detected.

	return 0;//stVoipCfg.ret_val;
}

int32 rtk_GetFaxDisTxDetect(uint32 chid, uint32 *pval)
{
	TstVoipCfg stVoipCfg;
	stVoipCfg.ch_id = chid;
	SETSOCKOPT(VOIP_MGR_FAX_DIS_TX_DETECT, &stVoipCfg, TstVoipCfg, 1);
	*pval = stVoipCfg.enable;
	//t.38 v21 dis tx direction detect, 1:dis(Digital Identification Signal) detected.

	return 0;//stVoipCfg.ret_val;
}

int32 rtk_GetFaxDisRxDetect(uint32 chid, uint32 *pval)
{
	TstVoipCfg stVoipCfg;
	stVoipCfg.ch_id = chid;
	SETSOCKOPT(VOIP_MGR_FAX_DIS_RX_DETECT, &stVoipCfg, TstVoipCfg, 1);
	*pval = stVoipCfg.enable;
	//t.38 v21 dis rx direction detect, 1:dis(Digital Identification Signal) detected.

	return 0;//stVoipCfg.ret_val;
}

int32 rtk_GetFxoLineVoltage(uint32 chid, uint32 *pval)
{
	TstVoipValue stVoipValue;
	
	stVoipValue.ch_id = chid;
	SETSOCKOPT(VOIP_MGR_GET_DAA_LINE_VOLTAGE, &stVoipValue, TstVoipValue, 1);
	*pval = stVoipValue.value5;

	return 0;//stVoipValue.ret_val;
}

int32 rtk_enablePCM(uint32 chid, uint32 val)
{
	TstVoipCfg stVoipCfg;

	stVoipCfg.ch_id = chid;
	stVoipCfg.enable = val;
	SETSOCKOPT(VOIP_MGR_PCM_CFG, &stVoipCfg, TstVoipCfg, 1);

	return 0;//stVoipCfg.ret_val;
}

int32 rtk_SetBusFormat(uint32 chid, AP_BUS_DATA_FORMAT format)
{
	TstVoipBusDataFormat stVoipBusDataFormat;

	stVoipBusDataFormat.ch_id = chid;
	stVoipBusDataFormat.format = format;
	SETSOCKOPT(VOIP_MGR_SET_BUS_DATA_FORMAT, 
				&stVoipBusDataFormat, TstVoipBusDataFormat, 1);
	
	return 0;//stVoipCfg.ret_val;
}

int32 rtk_SetPcmTimeslot(uint32 chid, uint32 timeslot1, uint32 timeslot2)
{
	TstVoipPcmTimeslot stVoipPcmTimeslot;

	stVoipPcmTimeslot.ch_id = chid;
	stVoipPcmTimeslot.timeslot1 = timeslot1;
	stVoipPcmTimeslot.timeslot2 = timeslot2;
	SETSOCKOPT(VOIP_MGR_SET_PCM_TIMESLOT, 
				&stVoipPcmTimeslot, TstVoipPcmTimeslot, 1);
	
	return 0;//stVoipCfg.ret_val;
}

int32 rtk_Stop_CID(uint32 chid, char cid_mode)
{
	TstVoipCfg stVoipCfg;

	stVoipCfg.ch_id = chid;
	stVoipCfg.cfg = cid_mode;
	SETSOCKOPT(VOIP_MGR_STOP_CID, &stVoipCfg, TstVoipCfg, 1);
	
	return 0;//stVoipCfg.ret_val;
}

int32 rtk_Set_CID_DTMF_MODE(uint32 chid, char cid_dtmf_mode, uint32 dtmf_on_ms, uint32 dtmf_pause_ms, uint32 dtmf_pre_silence, uint32 dtmf_end_silence)
{
	TstVoipCID stCIDstr;

	stCIDstr.ch_id = chid;
	stCIDstr.cid_dtmf_mode = cid_dtmf_mode;
	stCIDstr.cid_dtmf_on_ms = dtmf_on_ms;
	stCIDstr.cid_dtmf_pause_ms = dtmf_pause_ms;
	stCIDstr.cid_dtmf_pre_silence_ms = dtmf_pre_silence;
	stCIDstr.cid_dtmf_end_silence_ms = dtmf_end_silence;

	SETSOCKOPT(VOIP_MGR_SET_CID_DTMF_MODE, &stCIDstr, TstVoipCID, 1);

	return 0;//stCIDstr.ret_val;
}

int32 rtk_Gen_Dtmf_CID(uint32 chid, const char *str_cid)
{
	int i = 0;
	rtk_enablePCM(chid, 1); // enable PCM before generating dtmf caller id
	// set cid_state
	if (str_cid[0] == 0)               // not null str
		rtk_Set_Dtmf_CID_String(chid, "0123456789");   // replace "0123456789" with From CID
	else
		rtk_Set_Dtmf_CID_String(chid, str_cid);

	// polling cid_state until be clear
	uint32 tmp=-1;
	do
	{
		if(rtk_GetDtmfCIDState(chid, &tmp) != 0)
			break;
		usleep(50000);  // 50ms
		if ((i++) > 120 )// wait 6 sec, if not get cid state =0, then break.
			break;
	}
	while (tmp);

        //printf("Get DTMF CID state = 0\n");
	rtk_enablePCM(chid, 0); // disable PCM after generating dtmf caller id
	return 0;
}

int32 rtk_Gen_FSK_CID(uint32 chid, char *str_cid, char *str_date, char *str_cid_name, char mode)
{
	time_t timer = time(0);		// fsk date & time sync
	TstVoipCID stCIDstr;

	uint32 tmp=-1;

	rtk_GetFskCIDState(chid, &tmp);

	if (tmp)
	{
		//printf("not end");
		return -2; /* don't double send caller id when sending */
	}

	if(!mode)			// on-hook
		rtk_enablePCM(chid, 1); // enable PCM before generating fsk caller id

	stCIDstr.ch_id = chid;
	stCIDstr.cid_mode = mode;       //0:on-hook
	// set cid string
	if (str_cid[0] == 0)               // not null str
		strcpy(stCIDstr.string, "0123456789");   // replace "0123456789" with From CID
	else
		strcpy(stCIDstr.string, str_cid);

	if (str_date && str_date[0])
	{
		strcpy(stCIDstr.string2, str_date);
	}
	else
	{
	//strftime(stCIDstr.string2, 9, "%m%d%H%M", gmtime(&timer));
	strftime(stCIDstr.string2, 9, "%m%d%H%M", localtime(&timer));
	}
	if(str_cid_name)
		strcpy(stCIDstr.cid_name, str_cid_name);
	SETSOCKOPT(VOIP_MGR_FSK_CID_GEN_CFG, &stCIDstr, TstVoipCID, 1);

	return 0;//stCIDstr.ret_val;
/*
	int32 tmp=-1;
	do
	{
		if(rtk_GetFskCIDState(chid, &tmp) != 0)
			break;
		usleep(50000);  // 50ms
	}
	while (tmp);
	if(!mode)			//on-hook
		rtk_eanblePCM(chid, 0); // disable PCM after generating fsk caller id
*/
}

int32 rtk_Gen_MDMF_FSK_CID(uint32 chid, TstFskClid* pClid, uint32 num_clid_element)
{
	uint32 tmp=-1;

	rtk_GetFskCIDState(chid, &tmp);

	if (tmp)
	{
		return -2; /* don't double send caller id when sending */
	}

	if( pClid->service_type == 0 )	// on-hook
		rtk_enablePCM(chid, 1); // enable PCM before generating fsk caller id

	SETSOCKOPT(VOIP_MGR_FSK_CID_MDMF_GEN, pClid, TstFskClid, 1);

	return 0;//pClid->ret_val;

}

int32 rtk_Gen_CID_And_FXS_Ring(uint32 chid, char cid_mode, char *str_cid, char *str_date, char *str_cid_name, char fsk_type, uint32 bRinging)
{

	TstVoipSlicRing stVoipSlicRing;
    int32 ret;

#ifdef CONFIG_RTK_VOIP_DRIVERS_ATA_DECT
	//if (chid < RTK_VOIP_DECT_NUM(g_VoIP_Feature))	//DECT Channel
	if( RTK_VOIP_IS_DECT_CH( chid, g_VoIP_Feature ) )
	{

#ifdef CONFIG_RTK_VOIP_DECT_SITEL_SUPPORT
		if (str_cid[0] != 0)//Send CID and Ring FXS by DSP
		{
			dect_api_S2R_call_setup( 0, chid, str_cid, str_cid_name );
		}
		else
		{
			if( dect_api_S2R_check_handset_ringing( chid ) )
				dect_api_S2R_call_release( chid );
		}
#endif

#ifdef CONFIG_RTK_VOIP_DECT_DSPG_SUPPORT

		if (str_cid[0] != 0)//Send CID and Ring FXS by DSP
		{
			if (0 == (strcmp(str_cid, "-")))
			{
				//strcpy(str_cid, "P");
				CMBS_Api_InBound_Ring_CallerID(chid, NULL, str_cid_name);
			}
			else
				CMBS_Api_InBound_Ring_CallerID(chid, str_cid, str_cid_name);

		}
		else	// Just Ring FXS
		{
			if (g_bDisableRingFXS)
			{
				// quiet mode
				return 0;
			}

			if (bRinging == 1)
			{
				CMBS_Api_InBound_Ring_CallerID(chid, NULL, NULL);
			}
			else
			{
				if(CMBS_Api_LineStateGet(chid) == 2)//ringing
				{
					CMBS_Api_CallRelease(chid, 0);
				}
			}
		}
#endif

		return 0;
	}
	else
#endif
	//SLIC Channel
	{
	if (str_cid[0] != 0)//Send CID and Ring FXS by DSP
	{
		if (cid_mode == 0)	// DTMF CID
		{
			ret = rtk_Gen_Dtmf_CID(chid, str_cid);
		}
		else	// FSK CID
		{
			ret = rtk_Gen_FSK_CID(chid, str_cid, str_date, str_cid_name, fsk_type);
		}

		return ret;
	}
	else	// Just Ring FXS
	{
		if (g_bDisableRingFXS)
		{
			// quiet mode
			return 0;
		}

		stVoipSlicRing.ch_id = chid;
		stVoipSlicRing.ring_set = bRinging;
		SETSOCKOPT(VOIP_MGR_SLIC_RING, &stVoipSlicRing, TstVoipSlicRing, 1);

		return 0;//stVoipSlicRing.ret_val;
	}
	}

	return 0;
}


#if 0
// no action reacts to this function
int32 rtk_Gen_FSK_ALERT(uint32 chid, char *str_cid)
{
	TstVoipCID stCIDstr;

	stCIDstr.ch_id = chid;
	strcpy(stCIDstr.string, str_cid);
	SETSOCKOPT(VOIP_MGR_FSK_ALERT_GEN_CFG, &stCIDstr, TstVoipCID, 1);

	return 0;
}
#endif

int32 rtk_Gen_FSK_VMWI(uint32 chid, char *state, char mode)  /* state:	point to the address of value to set VMWI state. 0 : off; 1 : on*/
{
	TstVoipCID stCIDstr;

	uint32 tmp=-1;

	rtk_GetFskCIDState(chid, &tmp);

	if (tmp)
	{
		//printf("not end");
		return -2; /* don't double send caller id/vmwi when sending */
	}

	if(!mode)			// on-hook
		rtk_enablePCM(chid, 1); // enable PCM before generating VMWI

	stCIDstr.ch_id = chid;
	stCIDstr.cid_mode = mode;       //0:on-hook
	strcpy(stCIDstr.string, state);
	SETSOCKOPT(VOIP_MGR_SET_FSK_VMWI_STATE, &stCIDstr, TstVoipCID, 1);

	return 0;//stCIDstr.ret_val;

}

int32 rtk_Set_FSK_Area(uint32 chid, uint32 area)   /* area -> 0:Bellcore 1:ETSI 2:BT 3:NTT */
{
	TstVoipCfg stVoipCfg;

	stVoipCfg.ch_id = chid;
	stVoipCfg.cfg = area;

	SETSOCKOPT(VOIP_MGR_SET_FSK_AREA, &stVoipCfg, TstVoipCfg, 1);

	return 0;//stCIDstr.ret_val;

}

int32 rtk_Set_FSK_CLID_Para(TstVoipFskPara* para)
{
	TstVoipFskPara stVoipFskPara;

	memcpy(&stVoipFskPara, para, sizeof(TstVoipFskPara));

	SETSOCKOPT(VOIP_MGR_SET_FSK_CLID_PARA, &stVoipFskPara, TstVoipFskPara, 1);

	return 0;//stVoipFskPara.ret_val;

}

int32 rtk_Get_FSK_CLID_Para(TstVoipFskPara* para)
{
	TstVoipFskPara stVoipFskPara;

	stVoipFskPara.ch_id = para->ch_id;
	stVoipFskPara.area = para->area;

	SETSOCKOPT(VOIP_MGR_GET_FSK_CLID_PARA, &stVoipFskPara, TstVoipFskPara, 1);
	memcpy(para, &stVoipFskPara, sizeof(TstVoipFskPara));

	return 0;//stVoipFskPara.ret_val;

}

#if 0
/*
 * unused
 */
int32 rtk_SetRunRing(char* number)
{
	return -1;
}
#endif

int32 rtk_Hold_Rtp(uint32 chid, uint32 sid, uint32 enable)
{
    TstVoipCfg stVoipCfg;

#if 0
    stVoipCfg.ch_id = chid;
    stVoipCfg.m_id = sid;
    stVoipCfg.enable = enable;
    SETSOCKOPT(VOIP_MGR_HOLD, &stVoipCfg, TstVoipCfg, 1);
#else
    stVoipCfg.ch_id = chid;
    stVoipCfg.m_id = sid;
    stVoipCfg.enable = !enable;
    SETSOCKOPT(VOIP_MGR_RTP_CFG, &stVoipCfg, TstVoipCfg, 1);

	return 0;//stVoipCfg.ret_val;

#endif

}

int32 rtk_enable_pcm(uint32 chid, int32 bEnable)
{
    TstVoipCfg stVoipCfg;

    stVoipCfg.ch_id = chid;
    stVoipCfg.enable = bEnable;
    SETSOCKOPT(VOIP_MGR_PCM_CFG, &stVoipCfg, TstVoipCfg, 1);
	//if (stVoipCfg.ret_val != 0)
	//{
	//	return stVoipCfg.ret_val;
	//}

    stVoipCfg.cfg = 0; /*dir0*/
    SETSOCKOPT(VOIP_MGR_DTMF_CFG, &stVoipCfg, TstVoipCfg, 1);
	return 0;//stVoipCfg.ret_val;
}

int32 rtk_Set_DTMF_CFG(uint32 chid, int32 bEnable, uint32 dir)
{
    TstDtmfDetPara stDtmfDetPara;

    stDtmfDetPara.ch_id = chid;
    stDtmfDetPara.enable = bEnable;
    stDtmfDetPara.dir = dir;
    SETSOCKOPT(VOIP_MGR_DTMF_CFG, &stDtmfDetPara, TstDtmfDetPara, 1);

    return 0;//stDtmfDetPara.ret_val;
}


int32 rtk_set_dtmf_det_param(uint32 chid, uint32 dir, int32 threshold, uint32 on_time_10ms, uint32 fore_twist, uint32 rev_twist) /* Threshold: 0 ~ 40, it means 0 ~ -40 dBm */
{
	TstDtmfDetPara stDtmfDetPara;

	stDtmfDetPara.ch_id = chid;
	stDtmfDetPara.dir = dir;
	stDtmfDetPara.thres = threshold;
	stDtmfDetPara.on_time = on_time_10ms;
	stDtmfDetPara.fore_twist = fore_twist;
	stDtmfDetPara.rev_twist = rev_twist;

	SETSOCKOPT(VOIP_MGR_DTMF_DET_PARAM, &stDtmfDetPara, TstDtmfDetPara, 1);

	return 0;//stDtmfDetPara.ret_val;
}

uint32 rtk_Get_SLIC_Reg_Val(uint32 chid, uint32 reg, uint8 *regdata)
{
	TstVoipSlicReg stSlicReg;

	stSlicReg.ch_id = chid;
	stSlicReg.reg_num = reg;
	stSlicReg.reg_len = sizeof(stSlicReg.reg_ary);

	SETSOCKOPT(VOIP_MGR_GET_SLIC_REG_VAL, &stSlicReg, TstVoipSlicReg, 1);

	memcpy(regdata,stSlicReg.reg_ary,MIN(16,stSlicReg.reg_len));

	//if (stSlicReg.ret_val != 0)
	//{
	//	return stSlicReg.ret_val;
	//}
	return stSlicReg.reg_len;
}

int rtk_Get_DAA_Used_By_Which_SLIC(uint32 chid)
{

	TstVoipValue stVoipValue;

	stVoipValue.ch_id = chid;

	SETSOCKOPT(VOIP_MGR_GET_DAA_USED_BY_WHICH_SLIC, &stVoipValue, TstVoipValue, 1);

	//if (stVoipValue.ret_val != 0)
	//{
	//	return stVoipValue.ret_val;
	//}

	return 0;//stVoipValue.value;

}

int rtk_DAA_on_hook(uint32 chid)// for virtual DAA on-hook(channel ID is FXS channel)
{
	if ((g_VoIP_Feature & DAA_TYPE_MASK) != NO_DAA )
	{
		TstVoipCfg stVoipCfg;
	    	stVoipCfg.ch_id = chid;

		SETSOCKOPT(VOIP_MGR_DAA_ON_HOOK, &stVoipCfg, TstVoipCfg, 1);

		//if (stVoipCfg.ret_val != 0)
		//{
		//	return stVoipCfg.ret_val;
		//}
        }
	return 0;
}

int rtk_DAA_off_hook(uint32 chid)// for virtual DAA off-hook(channel ID is FXS channel)
{
	if ((g_VoIP_Feature & DAA_TYPE_MASK) != NO_DAA )
	{
		TstVoipCfg stVoipCfg;
	    	stVoipCfg.ch_id = chid;

		SETSOCKOPT(VOIP_MGR_DAA_OFF_HOOK, &stVoipCfg, TstVoipCfg, 1);

		//if (stVoipCfg.ret_val != 0)
		//{
		//	return stVoipCfg.ret_val;
		//}

		return stVoipCfg.enable; /* 1: success, 0xff: line not connect or busy or not support */
	}
	else
	{
		printf("API rtk_DAA_off_hook usage error.\n");
		return 0xff;
	}
}

int rtk_DAA_ring(uint32 chid)
{

	if ((g_VoIP_Feature & DAA_TYPE_MASK) != NO_DAA )
	{
		TstVoipCfg stVoipCfg;
	    	stVoipCfg.ch_id = chid;

		SETSOCKOPT(VOIP_MGR_DAA_RING, &stVoipCfg, TstVoipCfg, 1);

		//if (stVoipCfg.ret_val != 0)
		//{
		//	return stVoipCfg.ret_val;
		//}

		return stVoipCfg.enable;
	}
	else
	{
		return 0;
	}
}

int32 rtk_Set_Country(voipCfgParam_t *voip_ptr)
{
	TstVoipValue stVoipValue;
	char country;

  	country = voip_ptr->tone_of_country;

	stVoipValue.value = country;
	SETSOCKOPT(VOIP_MGR_SET_COUNTRY, &stVoipValue, TstVoipValue, 1);

	//if (stVoipValue.ret_val != 0)
	//{
	//	return stVoipValue.ret_val;
	//}

	if (country == TONE_CUSTOMER)
	{

		stVoipValue.value1 = voip_ptr->tone_of_custdial;
		stVoipValue.value2 = voip_ptr->tone_of_custring;
		stVoipValue.value3 = voip_ptr->tone_of_custbusy;
		stVoipValue.value4 = voip_ptr->tone_of_custwaiting;
		SETSOCKOPT(VOIP_MGR_USE_CUST_TONE, &stVoipValue, TstVoipValue, 1);

		//if (stVoipValue.ret_val != 0)
		//{
		//	return stVoipValue.ret_val;
		//}
	}

	return 0;
}

int32 rtk_Set_Country_Impedance(_COUNTRY_ country)
{
	TstVoipValue stVoipValue;

	stVoipValue.value = country;
	SETSOCKOPT(VOIP_MGR_SET_COUNTRY_IMPEDANCE, &stVoipValue, TstVoipValue, 1);

	return 0;//stVoipValue.ret_val;
}

int32 rtk_Set_Country_Tone(_COUNTRY_ country)
{
	TstVoipValue stVoipValue;

	stVoipValue.value = country;
	SETSOCKOPT(VOIP_MGR_SET_COUNTRY_TONE, &stVoipValue, TstVoipValue, 1);

	return 0;//stVoipValue.ret_val;
}

int32 rtk_Set_Impedance(uint16 preset)
{
	TstVoipValue stVoipValue;

	stVoipValue.value = preset;
	SETSOCKOPT(VOIP_MGR_SET_IMPEDANCE, &stVoipValue, TstVoipValue, 1);

	return 0;//stVoipValue.ret_val;
}

int32 rtk_Set_Dis_Tone_Para(voipCfgParam_t *voip_ptr)
{
	TstVoipdistonedet_parm stVoipdistonedet_parm;

	stVoipdistonedet_parm.distone_num = voip_ptr->distone_num;

	stVoipdistonedet_parm.tone1_frequency1 = voip_ptr->d1Freq1;
	stVoipdistonedet_parm.tone1_frequency2 = voip_ptr->d1Freq2;
	if ( voip_ptr->d1freqnum < 2)
		stVoipdistonedet_parm.tone1_frequency2 = 0;
	stVoipdistonedet_parm.tone1_accuracy = voip_ptr->d1Accur;
	stVoipdistonedet_parm.tone1_level = voip_ptr->d1Level;
	stVoipdistonedet_parm.tone1_distone_on_up_limit = voip_ptr->d1ONup;
	stVoipdistonedet_parm.tone1_distone_on_low_limit = voip_ptr->d1ONlow;
	stVoipdistonedet_parm.tone1_distone_off_up_limit = voip_ptr->d1OFFup;
	stVoipdistonedet_parm.tone1_distone_off_low_limit = voip_ptr->d1OFFlow;

	stVoipdistonedet_parm.tone2_frequency1 = voip_ptr->d2Freq1;
	stVoipdistonedet_parm.tone2_frequency2 = voip_ptr->d2Freq2;
	if ( voip_ptr->d2freqnum < 2)
		stVoipdistonedet_parm.tone2_frequency2 = 0;
	stVoipdistonedet_parm.tone2_accuracy = voip_ptr->d2Accur;
	stVoipdistonedet_parm.tone2_level = voip_ptr->d2Level;
	stVoipdistonedet_parm.tone2_distone_on_up_limit = voip_ptr->d2ONup;
	stVoipdistonedet_parm.tone2_distone_on_low_limit = voip_ptr->d2ONlow;
	stVoipdistonedet_parm.tone2_distone_off_up_limit = voip_ptr->d2OFFup;
	stVoipdistonedet_parm.tone2_distone_off_low_limit = voip_ptr->d2OFFlow;

	SETSOCKOPT(VOIP_MGR_SET_DIS_TONE_DET, &stVoipdistonedet_parm, TstVoipdistonedet_parm, 1);

	return 0;//stVoipdistonedet_parm.ret_val;

}


int32 rtk_Set_Custom_Tone(uint8 custom, st_ToneCfgParam *pstToneCfgParam)
{
	TstVoipValue stVoipValue;
	TstVoipToneCfg stVoipToneCfg;

	stVoipValue.value = custom;
	memcpy(&stVoipToneCfg, pstToneCfgParam, sizeof(TstVoipToneCfg));
	SETSOCKOPT(VOIP_MGR_SET_TONE_OF_CUSTOMIZE, &stVoipValue, TstVoipValue, 1);

	//if (stVoipValue.ret_val != 0)
	//{
	//	return stVoipValue.ret_val;
	//}

	SETSOCKOPT(VOIP_MGR_SET_CUST_TONE_PARAM, &stVoipToneCfg, TstVoipToneCfg, 1);

	return 0;//stVoipToneCfg.ret_val;

}

 /*
 TstVoipValue.value1 Customer dial tone
 TstVoipValue.value2 Customer ringing tone
 TstVoipValue.value3 Customer busy tone
 TstVoipValue.value4 Customer call waiting tone
 */

int32 rtk_Use_Custom_Tone(uint8 dial, uint8 ringback, uint8 busy, uint8 waiting)
{
	TstVoipValue stVoipValue;

	stVoipValue.value1 = dial;
	stVoipValue.value2 = ringback;
	stVoipValue.value3 = busy;
	stVoipValue.value4 = waiting;

	SETSOCKOPT(VOIP_MGR_USE_CUST_TONE, &stVoipValue, TstVoipValue, 1);

	return 0;//stVoipValue.ret_val;
}

int rtk_Set_SLIC_Reg_Val(int chid, int reg, int len, char *regdata)
{
	//int i;
	TstVoipSlicReg stSlicReg;

	stSlicReg.ch_id = chid;
	stSlicReg.reg_num = reg;
	//stSlicReg.reg_val = value;
	stSlicReg.reg_len = len;
	memcpy(stSlicReg.reg_ary,regdata,MIN(len,sizeof(stSlicReg.reg_ary)));

	//for (i=0; i<MIN(argc,sizeof(stSlicReg.reg_ary));i++)
	//	stSlicReg.reg_ary[i] = atoi(argv[i]);

	SETSOCKOPT(VOIP_MGR_SET_SLIC_REG_VAL, &stSlicReg, TstVoipSlicReg, 1);

	return 0;//stSlicReg.ret_val;
}

int rtk_reset_slic(int chid, unsigned int law)
{
	TstVoipSlicRestart stVoipSlicRestart;

	stVoipSlicRestart.ch_id = chid;
	stVoipSlicRestart.codec_law = law; // 0: linear, 1: A-law, 2:u-law
	SETSOCKOPT(VOIP_MGR_SLIC_RESTART, &stVoipSlicRestart, TstVoipSlicRestart, 1);

	return 0;//stVoipSlicRestart.ret_val;
}

int rtk_Set_PCM_Loop_Mode(char group, char mode, char main_ch, char mate_ch) //mode: 0- Not loop mode, 1- loop mode, 2- loop mode with VoIP
{
	TstVoipValue stVoipValue;

	stVoipValue.value = group;
	stVoipValue.value1 = mode;
	stVoipValue.value2 = main_ch;
	stVoipValue.value3 = mate_ch;

	SETSOCKOPT(VOIP_MGR_SET_PCM_LOOP_MODE, &stVoipValue, TstVoipValue, 1);

	return 0;//stVoipValue.ret_val;
}

int rtk_Set_FXS_FXO_Loopback(unsigned int chid, unsigned int enable)
{
	TstVoipCfg stVoipCfg;

	stVoipCfg.ch_id = chid;
	stVoipCfg.enable = enable;

	SETSOCKOPT(VOIP_MGR_SET_FXS_FXO_LOOPBACK, &stVoipCfg, TstVoipCfg, 1);

	return 0;//stVoipCfg.ret_val;
}

int rtk_Set_FXS_OnHook_Trans_PCM_ON(unsigned int chid)
{
	TstVoipCfg stVoipCfg;

	stVoipCfg.ch_id = chid;

	SETSOCKOPT(VOIP_MGR_SET_SLIC_ONHOOK_TRANS_PCM_START, &stVoipCfg, TstVoipCfg, 1);

	return 0;//stVoipCfg.ret_val;
}

int rtk_debug_with_watchdog(int dbg_flag, int watchdog)
{
	TstVoipValue stVoipValue;

	stVoipValue.value = dbg_flag;
	stVoipValue.value1 = watchdog;	// 0 --> off, 1 --> on, others --> don't care
	SETSOCKOPT(VOIP_MGR_DEBUG, &stVoipValue, TstVoipValue, 1);

	return 0;//stVoipValue.ret_val;
}

int rtk_debug(int dbg_flag)
{
	return rtk_debug_with_watchdog( dbg_flag, 2 );
}

int rtk_8305_switch(unsigned short phy,unsigned short reg,unsigned short value,unsigned short r_w)
{
	TstVoipSwitch switch_value;

	switch_value.phy = phy;
	switch_value.reg = reg;
	switch_value.value = value;
	switch_value.read_write = r_w;
	SETSOCKOPT(VOIP_MGR_8305_SWITCH_VAL, &switch_value, TstVoipSwitch,1);
	return 0;
}

//add by Tim, 1/8/2008
//Wan Clone MAC
int rtk_WAN_Clone_MAC(unsigned char* MAC){
	TstVoipCloneMAC MAC_address;
	memcpy(MAC_address.CloneMACAddress, MAC, 6);
	SETSOCKOPT(VOIP_MGR_SET_WAN_CLONE_MAC, &MAC_address, TstVoipCloneMAC,1);
	return 0;
}

int rtk_Bandwidth_Mgr(	unsigned int port, unsigned int dir, unsigned int ban)
{
	TstVoipBandwidthMgr BandwidthMgr;
	BandwidthMgr.port = port;
	BandwidthMgr.dir = dir;
	BandwidthMgr.ban = ban;
	SETSOCKOPT(VOIP_MGR_BANDWIDTH_MGR, &BandwidthMgr, TstVoipBandwidthMgr,1);
	return 0;
}

int rtk_Disable_Port(unsigned int port, unsigned int disable)
{
	TstVoipPortConfig DisablePort;
	DisablePort.port = port;
	DisablePort.config = disable;
	SETSOCKOPT(VOIP_MGR_PORT_DISABLE, &DisablePort, TstVoipPortConfig,1);
	return 0;
}
int rtk_qos_set_port_priority(unsigned int port, unsigned int priority)
{
	TstVoipPortConfig port_priority;
	port_priority.port = port;
	port_priority.config = priority;
	SETSOCKOPT(VOIP_MGR_PORT_PRIORITY, &port_priority, TstVoipPortConfig,1);
	return 0;
}

int rtk_Disable_FlowControl( unsigned int port,unsigned int disable)
{
	TstVoipPortConfig Disable_FlowControl;
	Disable_FlowControl.port = port;
	Disable_FlowControl.config = disable;
	SETSOCKOPT(VOIP_MGR_PORT_DISABLE_FLOWCONTROL, &Disable_FlowControl, TstVoipPortConfig,1);
	return 0;
}
#ifdef CONFIG_RTK_VOIP_WAN_VLAN

int rtk_switch_wan_3_vlan(voipCfgParam_t *voip_ptr)
{
	TstVoipSwitch_3_VLAN_tag wan_3_vlan_tag;

	if (voip_ptr->wanVlanEnable) {
		wan_3_vlan_tag.enable = voip_ptr->wanVlanEnable;
		wan_3_vlan_tag.vlanIdVoice =  voip_ptr->wanVlanIdVoice;
		wan_3_vlan_tag.priorityVoice =  voip_ptr->wanVlanPriorityVoice;
		wan_3_vlan_tag.cfiVoice =  voip_ptr->wanVlanCfiVoice;
                wan_3_vlan_tag.vlanIdData =  voip_ptr->wanVlanIdData;
                wan_3_vlan_tag.priorityData =  voip_ptr->wanVlanPriorityData;
                wan_3_vlan_tag.cfiData =  voip_ptr->wanVlanCfiData;
                wan_3_vlan_tag.vlanIdVideo =  voip_ptr->wanVlanIdVideo;
                wan_3_vlan_tag.priorityVideo =  voip_ptr->wanVlanPriorityVideo;
                wan_3_vlan_tag.cfiVideo =  voip_ptr->wanVlanCfiVideo;
	} else {
		memset(&wan_3_vlan_tag, 0, sizeof(wan_3_vlan_tag));
	}
	SETSOCKOPT(VOIP_MGR_WAN_3_VLAN_TAG, &wan_3_vlan_tag, TstVoipSwitch_VLAN_tag,1);
	return 0;
}


int rtk_switch_wan_2_vlan(voipCfgParam_t *voip_ptr)
{
	TstVoipSwitch_2_VLAN_tag wan_2_vlan_tag;

	if (voip_ptr->wanVlanEnable) {
		wan_2_vlan_tag.enable = voip_ptr->wanVlanEnable;
		wan_2_vlan_tag.vlanIdVoice =  voip_ptr->wanVlanIdVoice;
		wan_2_vlan_tag.priorityVoice =  voip_ptr->wanVlanPriorityVoice;
		wan_2_vlan_tag.cfiVoice =  voip_ptr->wanVlanCfiVoice;
                wan_2_vlan_tag.vlanIdData =  voip_ptr->wanVlanIdData;
                wan_2_vlan_tag.priorityData =  voip_ptr->wanVlanPriorityData;
                wan_2_vlan_tag.cfiData =  voip_ptr->wanVlanCfiData;
	} else {
		memset(&wan_2_vlan_tag, 0, sizeof(wan_2_vlan_tag));
	}
	SETSOCKOPT(VOIP_MGR_WAN_2_VLAN_TAG, &wan_2_vlan_tag, TstVoipSwitch_VLAN_tag,1);
	return 0;
}


int rtk_switch_wan_vlan(voipCfgParam_t *voip_ptr)
{
	TstVoipSwitch_VLAN_tag wan_vlan_tag;

	if (voip_ptr->wanVlanEnable) {
		wan_vlan_tag.enable = voip_ptr->wanVlanEnable;
		wan_vlan_tag.vlanId =  voip_ptr->wanVlanIdVoice;
		wan_vlan_tag.priority =  voip_ptr->wanVlanPriorityVoice;
		wan_vlan_tag.cfi =  voip_ptr->wanVlanCfiVoice;
	} else {
		memset(&wan_vlan_tag, 0, sizeof(wan_vlan_tag));
	}
	SETSOCKOPT(VOIP_MGR_WAN_VLAN_TAG, &wan_vlan_tag, TstVoipSwitch_VLAN_tag,1);
	return 0;
}

#endif // CONFIG_RTK_VOIP_WAN_VLAN

#if 0
int rtk_8305_switch_wan_vlan_tag(unsigned char enable,unsigned short wan_vlan_id)
{
	TstVoipSwitch_VLAN_tag wan_vlan_tag;

	wan_vlan_tag.WAN_VLAN_TAG_ENABLE = enable;
	wan_vlan_tag.WAN_VLAN_ID = wan_vlan_id;
	SETSOCKOPT(VOIP_MGR_WAN_VLAN_TAG, &wan_vlan_tag, TstVoipSwitch_VLAN_tag,1);
	return 0;
}

int rtk_8305_switch_bridge_mode(unsigned char bridge_mode_enable)
{
	SETSOCKOPT(VOIP_MGR_BRIDGE_MODE, &bridge_mode_enable, unsigned char,1);
	return 0;
}
#endif

#ifdef CONFIG_RTK_VOIP_IVR
int rtk_IvrStartPlaying( unsigned int chid, IvrPlayDir_t dir, char *pszText2Speech )
{
	TstVoipPlayIVR_Text stVoipPlayIVR_Text;

	stVoipPlayIVR_Text.ch_id = chid;
	stVoipPlayIVR_Text.type = IVR_PLAY_TYPE_TEXT;
	stVoipPlayIVR_Text.direction = dir;
	memcpy( stVoipPlayIVR_Text.szText2speech, pszText2Speech, MAX_LEN_OF_IVR_TEXT );
	stVoipPlayIVR_Text.szText2speech[ MAX_LEN_OF_IVR_TEXT ] = '\x0';

	SETSOCKOPT( VOIP_MGR_PLAY_IVR, &stVoipPlayIVR_Text, TstVoipPlayIVR_Text, 1 );

	return ( int )stVoipPlayIVR_Text.playing_period_10ms;
}

int rtk_IvrStartPlayG72363( unsigned int chid, unsigned int nFrameCount, const unsigned char *pData )
{
	TstVoipPlayIVR_G72363 stVoipPlayIVR;

	if( nFrameCount > MAX_FRAMES_OF_G72363 )
		nFrameCount = MAX_FRAMES_OF_G72363;

	stVoipPlayIVR.ch_id = chid;
	stVoipPlayIVR.type = IVR_PLAY_TYPE_G723_63;
	stVoipPlayIVR.nFramesCount = nFrameCount;
	memcpy( stVoipPlayIVR.data, pData, nFrameCount * 24 );

    SETSOCKOPT(VOIP_MGR_PLAY_IVR, &stVoipPlayIVR, TstVoipPlayIVR_G72363, 1);

    return stVoipPlayIVR.nRetCopiedFrames;
}

int rtk_IvrStartPlayG729( unsigned int chid, unsigned int nFrameCount, const unsigned char *pData )
{
	TstVoipPlayIVR_G729 stVoipPlayIVR;

	if( nFrameCount > MAX_FRAMES_OF_G729 )
		nFrameCount = MAX_FRAMES_OF_G729;

	stVoipPlayIVR.ch_id = chid;
	stVoipPlayIVR.type = IVR_PLAY_TYPE_G729;
	stVoipPlayIVR.nFramesCount = nFrameCount;
	memcpy( stVoipPlayIVR.data, pData, nFrameCount * 10 );

    SETSOCKOPT(VOIP_MGR_PLAY_IVR, &stVoipPlayIVR, TstVoipPlayIVR_G729, 1);

    return stVoipPlayIVR.nRetCopiedFrames;
}

int rtk_IvrPollPlaying( unsigned int chid )
{
	TstVoipPollIVR stVoipPollIVR;

	stVoipPollIVR.ch_id = chid;

	SETSOCKOPT( VOIP_MGR_POLL_IVR, &stVoipPollIVR, TstVoipPollIVR, 1 );

	return ( int )stVoipPollIVR.bPlaying;
}

int rtk_IvrStopPlaying( unsigned int chid )
{
	TstVoipStopIVR stVoipStopIVR;

	stVoipStopIVR.ch_id = chid;

	SETSOCKOPT( VOIP_MGR_STOP_IVR, &stVoipStopIVR, TstVoipStopIVR, 1 );

	return 0;
}
#endif /* CONFIG_RTK_VOIP_IVR */

int rtk_sip_register(unsigned int chid, unsigned int isOK)
{
   	TstVoipCfg stVoipCfg;

	stVoipCfg.ch_id = chid;
	stVoipCfg.enable = isOK;
#ifdef CONFIG_RTK_VOIP_DRIVERS_ATA_DECT
	g_sip_register[chid] = isOK;
#endif
	SETSOCKOPT(VOIP_MGR_SIP_REGISTER, &stVoipCfg, TstVoipCfg, 1);

	return 0;//stVoipCfg.ret_val;
}

int32 rtk_SIP_INFO_play_tone(unsigned int chid, unsigned int ssid, DSPCODEC_TONE tone, unsigned int duration)
{
	TstVoipValue stVoipValue;

	stVoipValue.ch_id = chid;
	stVoipValue.m_id = ssid;
	stVoipValue.value = tone;
	stVoipValue.value5 = duration;

	SETSOCKOPT(VOIP_MGR_PLAY_SIP_INFO, &stVoipValue, TstVoipValue, 1);

	return 0;//stVoipValue.ret_val;
}


int32 rtk_Set_SPK_AGC(uint32 chid, uint32 support_gain, uint32 adaptive_threshold)
{
	TstVoipValue stVoipValue;

	stVoipValue.ch_id = chid;
	stVoipValue.value = support_gain;
	stVoipValue.value1 = adaptive_threshold;

	SETSOCKOPT(VOIP_MGR_SET_SPK_AGC, &stVoipValue, TstVoipValue, 1);

	return 0;//stVoipValue.ret_val;
}

int32 rtk_Set_SPK_AGC_LVL(uint32 chid, uint32 level)
{
	TstVoipValue stVoipValue;

	stVoipValue.ch_id = chid;
	stVoipValue.value = level;

	SETSOCKOPT(VOIP_MGR_SET_SPK_AGC_LVL, &stVoipValue, TstVoipValue, 1);


	return 0;//stVoipValue.ret_val;
}


int32 rtk_Set_SPK_AGC_GUP(uint32 chid, uint32 gain)
{
	TstVoipValue stVoipValue;

	stVoipValue.ch_id = chid;
	stVoipValue.value = gain;

	SETSOCKOPT(VOIP_MGR_SET_SPK_AGC_GUP, &stVoipValue, TstVoipValue, 1);


	return 0;//stVoipValue.ret_val;
}


int32 rtk_Set_SPK_AGC_GDOWN(uint32 chid, uint32 gain)
{
	TstVoipValue stVoipValue;

	stVoipValue.ch_id = chid;
	stVoipValue.value = gain;

	SETSOCKOPT(VOIP_MGR_SET_SPK_AGC_GDOWN, &stVoipValue, TstVoipValue, 1);


	return 0;//stVoipValue.ret_val;
}


int32 rtk_Set_MIC_AGC(uint32 chid, uint32 support_gain, uint32 adaptive_threshold)
{
	TstVoipValue stVoipValue;

	stVoipValue.ch_id = chid;
	stVoipValue.value = support_gain;
	stVoipValue.value1 = adaptive_threshold;

	SETSOCKOPT(VOIP_MGR_SET_MIC_AGC, &stVoipValue, TstVoipValue, 1);


	return 0;//stVoipValue.ret_val;
}

int32 rtk_Set_MIC_AGC_LVL(uint32 chid, uint32 level)
{
	TstVoipValue stVoipValue;

	stVoipValue.ch_id = chid;
	stVoipValue.value = level;

	SETSOCKOPT(VOIP_MGR_SET_MIC_AGC_LVL, &stVoipValue, TstVoipValue, 1);


	return 0;//stVoipValue.ret_val;
}

int32 rtk_Set_MIC_AGC_GUP(uint32 chid, uint32 gain)
{
	TstVoipValue stVoipValue;

	stVoipValue.ch_id = chid;
	stVoipValue.value = gain;

	SETSOCKOPT(VOIP_MGR_SET_MIC_AGC_GUP, &stVoipValue, TstVoipValue, 1);


	return 0;//stVoipValue.ret_val;
}


int32 rtk_Set_MIC_AGC_GDOWN(uint32 chid, uint32 gain)
{
	TstVoipValue stVoipValue;

	stVoipValue.ch_id = chid;
	stVoipValue.value = gain;

	SETSOCKOPT(VOIP_MGR_SET_MIC_AGC_GDOWN, &stVoipValue, TstVoipValue, 1);


	return 0;//stVoipValue.ret_val;
}

#if 0

int32 rtk_Get_DAA_ISR_FLOW(unsigned int chid ,unsigned int mid)
{
	unsigned int flow;
	TstVoipValue stVoipValue;

	stVoipValue.ch_id = chid;
	stVoipValue.m_id = mid;

	SETSOCKOPT(VOIP_MGR_GET_DAA_ISR_FLOW, &stVoipValue, TstVoipValue, 1);
	flow = stVoipValue.value;

	return flow;
}

/* Usage:
	  rtk_Set_DAA_ISR_FLOW(chid, DAA_FLOW_NORMAL)
	  rtk_Set_DAA_ISR_FLOW(chid, DAA_FLOW_3WAY_CONFERENCE)
	  rtk_Set_DAA_ISR_FLOW(chid, DAA_FLOW_CALL_FORWARD)
*/
int32 rtk_Set_DAA_ISR_FLOW(unsigned int chid, unsigned int mid, unsigned int flow)
{
	TstVoipValue stVoipValue;
	int res;

	stVoipValue.ch_id = chid;
	stVoipValue.m_id = mid;
	stVoipValue.value = flow;


	if( flow == 0 ) /* Normal */
	{
		if (rtk_Get_DAA_ISR_FLOW(chid, mid) == DAA_FLOW_CALL_FORWARD)
			rtk_enablePCM(chid, 0);
		rtk_DAA_on_hook(chid);
		res = 1;
	}
	else if ( flow == 1 ) /* PSTN 3-way conference */
	{
		res = rtk_DAA_off_hook(chid);

		if (res == 0xff)
			stVoipValue.value = DAA_FLOW_NORMAL;
	}
	else if ( flow == 2 ) /* PSTN call forward */
	{
		rtk_enablePCM(chid, 1);
		res = rtk_DAA_off_hook(chid);

		if (res == 0xff)
		{
			rtk_enablePCM(chid, 0);
			stVoipValue.value = DAA_FLOW_NORMAL;
		}
	}

	SETSOCKOPT(VOIP_MGR_SET_DAA_ISR_FLOW, &stVoipValue, TstVoipValue, 1);


	return res; /* 1: success, 0xff: line not connect or busy or not support */
}

#endif

int32 rtk_Dial_PSTN_Call_Forward(uint32 chid, uint32 sid, char *cf_no_str)
{
   	char cf_no[21];
   	int len = strlen(cf_no_str), i;

   	strcpy(cf_no, cf_no_str);

   	//usleep(200000);  // 200ms
   	usleep(250000);	// ok
        printf("PSTN Call Forward Dial: ");
   	for(i = 0; i < len; i++)
   	{
   		rtk_SetPlayTone(chid, sid, cf_no[i]-'0', 1, DSPCODEC_TONEDIRECTION_LOCAL);
                printf("%d ", cf_no[i]-'0');
   		usleep(100000);  // 100ms
   		rtk_SetPlayTone(chid, sid, cf_no[i]-'0', 0, DSPCODEC_TONEDIRECTION_LOCAL);
   		usleep(50000);  // 50ms
   	}
        printf("\n");

	return 0;
}

#if 0

int32 rtk_Set_PSTN_HOLD_CFG(unsigned int slic_chid, unsigned int daa_chid, unsigned int config)
{
	TstVoipValue stVoipValue;

	stVoipValue.ch_id = slic_chid;
	stVoipValue.value2 = daa_chid;
	stVoipValue.value = config; /* 1: Hold, 0: Un-Hold*/


	SETSOCKOPT(VOIP_MGR_SET_DAA_PCM_HOLD_CFG, &stVoipValue, TstVoipValue, 1);


	return 0;
}

int32 rtk_Get_DAA_BusyTone_Status(unsigned int daa_chid)
{
	TstVoipValue stVoipValue;
	int busy_flag;	/* Busy tone is  1: Detected, 0: NOT detected. */

	stVoipValue.ch_id = daa_chid;

	SETSOCKOPT(VOIP_MGR_GET_DAA_BUSY_TONE_STATUS, &stVoipValue, TstVoipValue, 1);
	busy_flag = stVoipValue.value;

	return busy_flag;
}

#endif

int32 rtk_Get_DAA_CallerID(uint32 chid, char *str_cid, char *str_date, char *str_name)
{
	TstVoipCID stCIDstr;

	stCIDstr.daa_id = chid;

	SETSOCKOPT(VOIP_MGR_GET_DAA_CALLER_ID, &stCIDstr, TstVoipCID, 1);

	//if (stCIDstr.ret_val != 0)
	//{
	//	return stCIDstr.ret_val;
	//}

	strcpy(str_cid, stCIDstr.string);
	strcpy(str_date, stCIDstr.string2);
	strcpy(str_name, stCIDstr.cid_name);

	return 0;
}

int32 rtk_Get_VoIP_Feature(void)
{
   	TstVoipFeature stVoipFeature;

	if( g_VoIP_Feature )
		return 0;	// If not zero, ignore it.

	//printf( "VOIP_MGR_GET_FEATURE=%d\n", VOIP_MGR_GET_FEATURE );

	SETSOCKOPT(VOIP_MGR_GET_FEATURE, &stVoipFeature, TstVoipFeature, 1);

	memcpy( &g_VoIP_Feature, &stVoipFeature, sizeof( TstVoipFeature ) );

	g_VoIP_Ports = RTK_VOIP_CH_NUM( g_VoIP_Feature );

	//printf("rtk_Get_VoIP_Feature: 0x%llx \n", *( ( uint64 * )( void * )&g_VoIP_Feature ) );

#if 0
	printf( "\tRTK_VOIP_SLIC_NUM=%d\n", RTK_VOIP_SLIC_NUM( g_VoIP_Feature ) );
	printf( "\tRTK_VOIP_DAA_NUM=%d\n", RTK_VOIP_DAA_NUM( g_VoIP_Feature ) );
	printf( "\tRTK_VOIP_DECT_NUM=%d\n", RTK_VOIP_DECT_NUM( g_VoIP_Feature ) );
	printf( "\tRTK_VOIP_CH_NUM=%d\n", RTK_VOIP_CH_NUM( g_VoIP_Feature ) );
	if( RTK_VOIP_PLATFORM_CHK_IS8672( g_VoIP_Feature ) )
		printf( "\tPLATFORM 8672\n" );
	else if( RTK_VOIP_PLATFORM_CHK_IS865xC( g_VoIP_Feature ) )
		printf( "\tPLATFORM 865xC\n" );
	else if( RTK_VOIP_PLATFORM_CHK_IS8972B( g_VoIP_Feature ) )
		printf( "\tPLATFORM 8972B\n" );
	else if( RTK_VOIP_PLATFORM_CHK_IS89xxC( g_VoIP_Feature ) )
		printf( "\tPLATFORM 89xxC\n" );
	else
		printf( "\tPLATFORM unknown: %llX\n", ((g_VoIP_Feature) & RTK_VOIP_PLATFORM_MASK) );
#endif

	return 0;
}

int32 rtk_Set_CID_Det_Mode(uint32 chid, int auto_det, int cid_det_mode)
{
   	TstVoipCfg stVoipCfg;

   	stVoipCfg.ch_id = chid;
   	stVoipCfg.enable = auto_det; /* 0: disable 1: enable(NTT) 2: enable (NOT NTT) */
   	stVoipCfg.cfg = cid_det_mode;

	SETSOCKOPT(VOIP_MGR_SET_CID_DET_MODE, &stVoipCfg, TstVoipCfg, 1);

	return 0;//stVoipCfg.ret_val;
}

int32 rtk_GetFskCIDState(uint32 chid, uint32 *cid_state)
{
	TstVoipCID stCIDstr;

	stCIDstr.ch_id = chid;
	SETSOCKOPT(VOIP_MGR_GET_FSK_CID_STATE_CFG, &stCIDstr, TstVoipCID, 1);
	*cid_state = stCIDstr.cid_state;

	return 0;//stCIDstr.ret_val;
}

int32 rtk_Set_CID_FSK_GEN_MODE(unsigned int chid, unsigned int isOK)
{
   	TstVoipCfg stVoipCfg;

	stVoipCfg.ch_id = chid;
	stVoipCfg.enable = isOK;
	SETSOCKOPT(VOIP_MGR_SET_CID_FSK_GEN_MODE, &stVoipCfg, TstVoipCfg, 1);

	return 0;//stVoipCfg.ret_val;
}

int32 rtk_Set_Voice_Gain(uint32 chid, int spk_gain, int mic_gain)
{
	TstVoipValue stVoipValue;

	stVoipValue.ch_id = chid;
	stVoipValue.value = spk_gain;
	stVoipValue.value1 = mic_gain;

	SETSOCKOPT(VOIP_MGR_SET_VOICE_GAIN, &stVoipValue, TstVoipValue, 1);

	return 0;//stVoipValue.ret_val;
}

int32 rtk_Set_AnswerTone_Det(uint32 chid, uint32 config, int32 threshold)
{
	TstVoipCfg stVoipCfg;

	stVoipCfg.ch_id = chid;
	stVoipCfg.cfg = config;
	stVoipCfg.cfg2 = threshold;
	
	SETSOCKOPT(VOIP_MGR_SET_ANSWERTONE_DET, &stVoipCfg, TstVoipCfg, 1);

	return 0;//stVoipValue.ret_val;
}

int32 rtk_Set_Silence_Det_Threshold(uint32 chid, uint32 energy, uint32 period)
{
	TstVoipCfg stVoipCfg;
	
	// mid = 0
	stVoipCfg.ch_id = chid;   
	stVoipCfg.m_id = 0;
	stVoipCfg.cfg = energy;
	stVoipCfg.cfg2 = period;
	
	SETSOCKOPT(VOIP_MGR_SET_FAX_SILENCE_DET, &stVoipCfg, TstVoipCfg, 1);

	// mid = 1
	stVoipCfg.ch_id = chid;   
	stVoipCfg.m_id = 1;
	stVoipCfg.cfg = energy;
	stVoipCfg.cfg2 = period;
	
	SETSOCKOPT(VOIP_MGR_SET_FAX_SILENCE_DET, &stVoipCfg, TstVoipCfg, 1);

	return 0;
}

int32 rtk_Gen_FSK_CID_VMWI(uint32 chid, char *str_cid, char *str_date, char *str_cid_name, char mode, char msg_type)
{
#if 1
	printf("Error, API %s is not workable.\n", __FUNCTION__);
	return -1;
#else
	TstVoipCID stCIDstr;
	uint32 tmp=-1;

	rtk_GetFskCIDState(chid, &tmp);

	if (tmp)
	{
		//printf("not end");
		return -2; /* don't double send caller id/vmwi when sending */
	}

	if(!mode)			// on-hook
		rtk_enablePCM(chid, 1); // enable PCM before generating VMWI

	stCIDstr.ch_id = chid;
	stCIDstr.cid_mode = mode;       //0:on-hook
	stCIDstr.cid_msg_type = msg_type;	// FSK_MSG_CALLSETUP:cid or FSK_MSG_MWSETUP:vmwi
	if (str_cid[0] == 0)               // not null str
		strcpy(stCIDstr.string, "0123456789");   // replace "0123456789" with From CID
	else
		strcpy(stCIDstr.string, str_cid);
	strcpy(stCIDstr.string2, str_date);
	strcpy(stCIDstr.cid_name, str_cid_name);

	SETSOCKOPT(VOIP_MGR_FSK_CID_VMWI_GEN_CFG, &stCIDstr, TstVoipCID, 1);
	// remember set slic in transmit mode, enable DSP pcm.
#endif
	return 0;//stCIDstr.ret_val;
}

int32 rtk_GetDspEvent(uint32 chid, uint32 mid, VoipEventID *pEvent)
{
	int ret;
	TstVoipEvent stVoipEvent;
	
	stVoipEvent.ch_id = chid;
	stVoipEvent.type = VET_DSP;
	stVoipEvent.mask = ( mid ? VEM_MID1 : VEM_MID0 );
	
	if( ( ret = rtk_GetVoIPEvent( &stVoipEvent ) ) < 0 )
		return ret;
	
	*pEvent = stVoipEvent.id;
	
	return 0;
}

int32 rtk_Set_GetPhoneStat(TstVoipCfg* pstVoipCfg)
{
	SETSOCKOPT(VOIP_MGR_GET_SLIC_STAT, pstVoipCfg, TstVoipCfg, 1);

	return 0;//pstVoipCfg->ret_val;
}

int32 rtk_Set_GETDATA_Mode(TstVoipdataget_o* pstVoipdataget_o)
{
	SETSOCKOPT(VOIP_MGR_SET_GETDATA_MODE, pstVoipdataget_o, TstVoipdataget_o, 1);

	return 0;//pstVoipdataget_o->ret_val;
}

int32 rtk_Set_Voice_Play(TstVoipdataput_o* pstVoipdataput_o)
{
	SETSOCKOPT(VOIP_MGR_VOICE_PLAY, pstVoipdataput_o, TstVoipdataput_o, 1);

	return 0;//pstVoipdataput_o->ret_val;
}

int32 rtk_Get_Rtp_Statistics( uint32 chid, uint32 bReset, TstVoipRtpStatistics *pstVoipRtpStatistics )
{
	pstVoipRtpStatistics ->ch_id = chid;
	pstVoipRtpStatistics ->bResetStatistics = bReset;

	SETSOCKOPT( VOIP_MGR_GET_RTP_STATISTICS, pstVoipRtpStatistics, TstVoipRtpStatistics, 1 );

	return 0;//pstVoipRtpStatistics->ret_val;
}

int32 rtk_Get_Session_Statistics( uint32 chid, uint32 sid, uint32 bReset, TstVoipSessionStatistics *pstVoipSessionStatistics )
{
	pstVoipSessionStatistics ->ch_id = chid;
	pstVoipSessionStatistics ->m_id = sid;
	pstVoipSessionStatistics ->bResetStatistics = bReset;

	SETSOCKOPT( VOIP_MGR_GET_SESSION_STATISTICS, pstVoipSessionStatistics, TstVoipSessionStatistics, 1 );

	return 0;//pstVoipSessionStatistics->ret_val;
}

int32 rtk_qos_set_dscp_priority(int32 dscp, int32 priority)
{
	int _dscp;
	_dscp = (dscp & 0x00FF)<<8 | (priority & 0xFF);
	SETSOCKOPT(VOIP_MGR_SET_DSCP_PRIORITY, &_dscp, int32, 1);
	return 0;
}

int32 rtk_qos_reset_dscp_priority(void)
{
	int dscp = 0xFF;

	SETSOCKOPT(VOIP_MGR_SET_DSCP_PRIORITY, &dscp, int32, 1);
	return 0;
}

int32 rtk_Set_Rtp_Tos(int32 rtp_tos)
{
	SETSOCKOPT(VOIP_MGR_SET_RTP_TOS, &rtp_tos, int32, 1);
	return 0;
}

int32 rtk_Set_Rtp_Dscp(int32 rtp_dscp)
{
	SETSOCKOPT(VOIP_MGR_SET_RTP_DSCP, &rtp_dscp, int32, 1);
	return 0;
}


int32 rtk_Set_Sip_Tos(int32 sip_tos)
{
	SETSOCKOPT(VOIP_MGR_SET_SIP_TOS, &sip_tos, int32, 1);
	return 0;
}

int32 rtk_Set_Sip_Dscp(int32 sip_dscp)
{
	SETSOCKOPT(VOIP_MGR_SET_SIP_DSCP, &sip_dscp, int32, 1);
	return 0;
}

#ifdef CONFIG_RTK_VOIP_DRIVERS_IP_PHONE
int rtk_Set_IPhone(unsigned int function_type, unsigned int reg, unsigned int value)
{
	IPhone_test iphone;

	iphone.function_type = function_type;
	iphone.reg = reg;
	iphone.value = value;
	SETSOCKOPT(VOIP_MGR_IPHONE_TEST, &iphone, IPhone_test, 1);
	return 0;
}
#endif

/******************** New Add for AudioCodes Solution ****************/
int32 rtk_Onhook_Action(uint32 chid)
{
    	TstVoipCfg stVoipCfg;

        stVoipCfg.ch_id = chid;
        SETSOCKOPT(VOIP_MGR_SLIC_ONHOOK_ACTION, &stVoipCfg, TstVoipCfg, 1);

	return 0;//stVoipCfg.ret_val;
}

int32 rtk_Offhook_Action(uint32 chid)
{
    	TstVoipCfg stVoipCfg;

        stVoipCfg.ch_id = chid;
        SETSOCKOPT(VOIP_MGR_SLIC_OFFHOOK_ACTION, &stVoipCfg, TstVoipCfg, 1);

	return 0;//stVoipCfg.ret_val;
}

/**********************************************************************/

#ifdef CONFIG_RTK_VOIP_IP_PHONE
int rtk_GetIPPhoneHookStatus( uint32 *pHookStatus )
{
	TstKeypadHookStatus stKeypadHookStatus;

	stKeypadHookStatus.cmd = KEYPAD_CMD_HOOK_STATUS;
	SETSOCKOPT( VOIP_MGR_CTL_KEYPAD, &stKeypadHookStatus, TstKeypadHookStatus, 1);

	if( stKeypadHookStatus.status )
		printf( "-------------------------------------------\nOff-hook\n" );
	else
		printf( "-------------------------------------------\nOn-hook\n" );

	return 0;
}
#endif /* CONFIG_RTK_VOIP_IP_PHONE */

int rtk_Set_flush_fifo(uint32 chid)
{
	int ret;
	TstFlushVoipEvent stFlushVoipEvent;
	
	stFlushVoipEvent.ch_id = chid;
	stFlushVoipEvent.type = VET_ALL;
	stFlushVoipEvent.mask = VEM_ALL;
	
	if( ( ret = rtk_FlushVoIPEvent( &stFlushVoipEvent ) ) < 0 )
		return ret;
	
	return 0;//stVoipValue.ret_val;
}

/*	for FXS:
	0: Phone dis-connect,
	1: Phone connect,
	2: Phone off-hook,
	3: Check time out ( may connect too many phone set => view as connect),
	4: Can not check, Linefeed should be set to active state first.

	for FXO:
	0: PSTN Line connect,
	1: PSTN Line not connect,
	2: PSTN Line busy
*/
int rtk_line_check(uint32 chid)
{
	TstVoipValue stVoipValue;

	stVoipValue.ch_id = chid;
	SETSOCKOPT(VOIP_MGR_LINE_CHECK, &stVoipValue, TstVoipValue, 1);

	//if (stVoipValue.ret_val != 0)
	//{
	//	return stVoipValue.ret_val;
	//}

	return stVoipValue.value;
}

int rtk_FXO_offhook(uint32 chid)// for real DAA off-hook(channel is FXO channel)
{
	if ((g_VoIP_Feature & DAA_TYPE_MASK) != NO_DAA )
	{
		TstVoipCfg stVoipCfg;
	    	stVoipCfg.ch_id = chid;

		SETSOCKOPT(VOIP_MGR_FXO_OFF_HOOK, &stVoipCfg, TstVoipCfg, 1);

		//if (stVoipCfg.ret_val != 0)
		//{
		//	return stVoipCfg.ret_val;
		//}

		return 1;
	}
	else
	{
		printf("API rtk_FXO_offhook usage error.\n");
		return 0;
	}
}

int rtk_FXO_onhook(uint32 chid)// for real DAA on-hook(channel is FXO channel)
{
	if ((g_VoIP_Feature & DAA_TYPE_MASK) != NO_DAA )
	{
		TstVoipCfg stVoipCfg;
	    	stVoipCfg.ch_id = chid;

		SETSOCKOPT(VOIP_MGR_FXO_ON_HOOK, &stVoipCfg, TstVoipCfg, 1);

		//if (stVoipCfg.ret_val != 0)
		//{
		//	return stVoipCfg.ret_val;
		//}

		// Re-init DSP
		SETSOCKOPT(VOIP_MGR_ON_HOOK_RE_INIT, &stVoipCfg, TstVoipCfg, 1);

		return 0;//stVoipCfg.ret_val;
	}
        else
		return 0;
}

int rtk_FXO_RingOn(uint32 chid)
{
	TstVoipCfg stVoipCfg;
	stVoipCfg.ch_id = chid;
	stVoipCfg.cfg = 11; //FXO_RING_ON
	stVoipCfg.cfg2 = VEID_HOOK_FXO_RING_ON;
	
	SETSOCKOPT(VOIP_MGR_HOOK_FIFO_IN, &stVoipCfg, TstVoipCfg, 1);
	
	return 0;//stVoipCfg.ret_val;
}

int rtk_FXO_Busy(uint32 chid)
{
	TstVoipCfg stVoipCfg;
	stVoipCfg.ch_id = chid;
	stVoipCfg.cfg = 13; //FXO_BUSY_TONE
	stVoipCfg.cfg2 = VEID_HOOK_FXO_BUSY_TONE;
	
	SETSOCKOPT(VOIP_MGR_HOOK_FIFO_IN, &stVoipCfg, TstVoipCfg, 1);
	
	return 0;//stVoipCfg.ret_val;
}

int rtk_gpio(unsigned long action, unsigned long pid, unsigned long value, unsigned long *ret_value)
{
	TstVoipGPIO stVoipGPIO;

	stVoipGPIO.action = action;
	stVoipGPIO.pid = pid;
	stVoipGPIO.value = value;
   	SETSOCKOPT(VOIP_MGR_GPIO, &stVoipGPIO, TstVoipGPIO, 1);

   	//if (stVoipGPIO.ret_val != 0)
   	//{
	//	return stVoipGPIO.ret_val;
	//}

	if (ret_value)
		*ret_value = stVoipGPIO.value;

	return stVoipGPIO.result;
}

int rtk_Set_LED_Display( uint32 chid, uint32 LED_ID, LedDisplayMode mode )
{
	TstVoipLedDisplay stVoipLedDisplay;

	if( LED_ID >= 2 )	// LED_ID has to be 0 or 1
		return -1;

	stVoipLedDisplay.ch_id = chid;
	stVoipLedDisplay.led_id = LED_ID;
	stVoipLedDisplay.mode = mode;

	SETSOCKOPT( VOIP_MGR_SET_LED_DISPLAY, &stVoipLedDisplay, TstVoipLedDisplay, 1 );

	return 0;
}

int rtk_Set_SLIC_Relay( uint32 chid, uint32 close1 )
{
	TstVoipSlicRelay stVoipSlicRelay;

	stVoipSlicRelay.ch_id = chid;
	stVoipSlicRelay.close = close1;

	SETSOCKOPT( VOIP_MGR_SET_SLIC_RELAY, &stVoipSlicRelay, TstVoipSlicRelay, 1 );

	return 0;
}

int rtk_Set_Pulse_Digit_Det(uint32 chid, uint32 enable, uint32 pause_time, uint32 min_break_ths, uint32 max_break_ths) /* 0: disable 1: enable Pulse Digit Detection */
{
	TstVoipCfg stVoipCfg;
    	stVoipCfg.ch_id = chid;
    	stVoipCfg.enable = enable;
    	stVoipCfg.cfg = pause_time;
    	stVoipCfg.cfg2 = min_break_ths;
    	stVoipCfg.cfg3 = max_break_ths;

    	SETSOCKOPT(VOIP_MGR_SET_PULSE_DIGIT_DET, &stVoipCfg, TstVoipCfg, 1);

	return 0;//stVoipCfg.ret_val;
}

int rtk_Set_Dail_Mode(uint32 chid, uint32 mode) /* 0: disable 1: enable Pulse dial */
{
	TstVoipCfg stVoipCfg;
    	stVoipCfg.ch_id = chid;
    	stVoipCfg.cfg = mode;

    	SETSOCKOPT(VOIP_MGR_SET_DIAL_MODE, &stVoipCfg, TstVoipCfg, 1);

	return 0;//stVoipCfg.ret_val;
}

int rtk_Get_Dail_Mode(uint32 chid) /* 0: disable 1: enable Pulse dial */
{
	TstVoipCfg stVoipCfg;
    	stVoipCfg.ch_id = chid;

    	SETSOCKOPT(VOIP_MGR_GET_DIAL_MODE, &stVoipCfg, TstVoipCfg, 1);

    // 	if (stVoipCfg.ret_val != 0)
    // 	{
	//	return stVoipCfg.ret_val;
	//}

    	return 	stVoipCfg.cfg;
}

int rtk_PulseDial_Gen_Cfg(char pps, short make_duration, short interdigit_duration)
{
	TstVoipValue stVoipValue;

	stVoipValue.value = pps;
	stVoipValue.value5 = make_duration;
	stVoipValue.value6 = interdigit_duration;

	SETSOCKOPT(VOIP_MGR_PULSE_DIAL_GEN_CFG, &stVoipValue, TstVoipValue, 1);

	return 0;//stVoipValue.ret_val;
}

int rtk_Gen_Pulse_Dial(uint32 chid, char digit) /* digit: 0 ~ 9 */
{
	TstVoipValue stVoipValue;

	stVoipValue.ch_id = chid;
	stVoipValue.value = digit;

	SETSOCKOPT(VOIP_MGR_GEN_PULSE_DIAL, &stVoipValue, TstVoipValue, 1);

	return 0;//stVoipValue.ret_val;
}

uint32 rtk_Get_SLIC_Ram_Val(uint8 chid, uint16 reg)
{
	TstVoipSlicRam stSlicReg;

	stSlicReg.ch_id = chid;
	stSlicReg.reg_num = reg;
	SETSOCKOPT(VOIP_MGR_GET_SLIC_RAM_VAL, &stSlicReg, TstVoipSlicRam, 1);

	//if (stSlicReg.ret_val != 0)
	//{
	//	return stSlicReg.ret_val;
	//}

	return stSlicReg.reg_val;
}

int rtk_Set_SLIC_Ram_Val(uint8 chid, uint16 reg, uint32 value)
{
	TstVoipSlicRam stSlicReg;

	stSlicReg.ch_id = chid;
	stSlicReg.reg_num = reg;
	stSlicReg.reg_val = value;
	SETSOCKOPT(VOIP_MGR_SET_SLIC_RAM_VAL, &stSlicReg, TstVoipSlicRam, 1);

	return 0;//stSlicReg.ret_val;
}

int32 rtk_SetFaxModemDet(uint32 chid, uint32 mode)
{
	TstVoipCfg stVoipCfg;

	stVoipCfg.ch_id = chid;
	stVoipCfg.cfg = mode;

	SETSOCKOPT(VOIP_MGR_SET_FAX_MODEM_DET, &stVoipCfg, TstVoipCfg, 1);

	return 0;//stVoipCfg.ret_val;
}

int32 rtk_GetPortLinkStatus( uint32 *pstatus )
{
	TstVoipPortLinkStatus stVoipPortLinkStatus;

	SETSOCKOPT(VOIP_MGR_GET_PORT_LINK_STATUS, &stVoipPortLinkStatus, TstVoipPortLinkStatus, 1);

	*pstatus = stVoipPortLinkStatus.status;

	return 0;
}

int32 rtk_WTD_Reboot(int reboot)
{
	SETSOCKOPT(VOIP_MGR_SET_FW_UPDATE, &reboot, int, 1);

	return 0;
}

int rtk_print(int level, char *module, char *msg)
{

#if 0	// print to stdout
	printf(msg);
	return 0;
#else
	rtk_print_cfg cfg;

	cfg.level = level;

	strncpy(cfg.module, module, sizeof(cfg.module) - 1);
	cfg.module[sizeof(cfg.module) - 1] = '\0';

	strncpy(cfg.msg, msg, sizeof(cfg.msg) - 1);
	cfg.msg[sizeof(cfg.msg) - 1] = '\0';

	SETSOCKOPT(VOIP_MGR_PRINT, &cfg, rtk_print_cfg, 1);

	return 0;//cfg.ret_val;
#endif
}

int rtk_cp3_measure(st_CP3_VoIP_param* cp3_voip_param)
{

	SETSOCKOPT(VOIP_MGR_COP3_CONIFG, cp3_voip_param, st_CP3_VoIP_param, 1);

	return 0;//cp3_voip_param->ret_val;
}

int32 rtk_SetDspIdToDsp(unsigned char cpuid)
{
    TstVoipValue stVoipValue;

    stVoipValue.value = cpuid;

    SETSOCKOPT(VOIP_MGR_SET_DSP_ID_TO_DSP, &stVoipValue, TstVoipValue, 1);

    return 0;
}

int32 rtk_SetDspPhyId(unsigned char cpuid)
{
    TstVoipValue stVoipValue;

    stVoipValue.value = cpuid;

    SETSOCKOPT(VOIP_MGR_SET_DSP_PHY_ID, &stVoipValue, TstVoipValue, 1);

    return 0;
}

int32 rtk_CheckDspAllSoftwareReady(unsigned char cpuid)
{
    TstVoipValue stVoipValue;

    stVoipValue.value = cpuid;

    SETSOCKOPT(VOIP_MGR_CHECK_DSP_ALL_SW_READY, &stVoipValue, TstVoipValue, 1);

    return stVoipValue.value1;
}

uint8 rtk_CompleteDeferInitialzation( void )
{
    TstVoipValue stVoipValue;

	// Don't need any data!

    SETSOCKOPT(VOIP_MGR_COMPLETE_DEFER_INIT, &stVoipValue, TstVoipValue, 1);

	return 0;
}

static void __attribute__ ((constructor)) voip_manager_init(void)
{
#ifdef __mips__
	if( ( g_VoIP_Mgr_FD = open( VOIP_MGR_IOCTL_DEV_NAME, O_RDWR ) ) < 0 )
		fprintf( stderr, "Open " VOIP_MGR_IOCTL_DEV_NAME " fail\n" );
#endif	
	rtk_Get_VoIP_Feature();
}

static void __attribute__ ((destructor)) voip_manager_fini(void)
{
#ifdef __mips__
	close( g_VoIP_Mgr_FD );
#endif
}

