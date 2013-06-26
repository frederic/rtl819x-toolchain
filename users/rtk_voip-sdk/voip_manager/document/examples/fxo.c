#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "voip_manager.h"

// test option:
// 	- FXO_DIAL_FXS0 has defined
//		* fxo ring will dial fxs0 automatically
//	- FXO_DIAL_FXS0 has not defined
//		* fxo ring will prompt user dial voip number via IVR
#define FXO_DIAL_FXS0_AUTO

#define FUNCKEY_FXO_0	".0"
#define FUNCKEY_FXO_1	".1"
#define FUNCKEY_FXO_2	".2"
#define FUNCKEY_FXO_3	".3"
#define FUNCKEY_FXS_0 	".4"
#define FUNCKEY_FXS_1 	".5"

#define CHANNEL_IS_FXS(chid)	RTK_VOIP_IS_SLIC_CH( chid, g_VoIP_Feature )
#define CHANNEL_IS_FXO(chid)	RTK_VOIP_IS_DAA_CH( chid, g_VoIP_Feature )

enum {
	FXO_NO_DAA = 0,
	FXO_VIRTUAL_DAA,
	FXO_DAA
};

enum {
	STATE_IDLE = 0,
	STATE_RING,
	STATE_CONNECT,
	STATE_DAA_RING,			// virtual daa state
	STATE_DAA_CONNECT		// virtual daa state
};

enum {
	DIAL_NONE = 0,
	DIAL_FXO_0,
	DIAL_FXO_1,
	DIAL_FXO_2,
	DIAL_FXO_3,
	DIAL_FXS_0,
	DIAL_FXS_1,
	DIAL_OTHER,
};

typedef struct channel_s channel_t;

struct channel_s {
	// phone info
	int chid;
	int state;
	char dial_code[101];
	int dial_index;
	// rtp info
	unsigned long ip;
	unsigned short port;
	// remote channel (just peer simulation)
	channel_t *remote;
};

void channel_init(channel_t *channel, int chid);
void channel_reset(channel_t *channel, int chid);
int channel_input(channel_t *channel, SIGNSTATE val);
void channel_dial_local(channel_t *channel, channel_t *remote_channel);
void channel_dial_out(channel_t *channel);
void channel_switch_to_fxo(channel_t *channel);	// virtual daa
void channel_rtp_start(channel_t *channel);
void channel_t38_start(channel_t *channel);
void channel_rtp_stop(channel_t *channel);

// global variable
int fxo_type;
channel_t channels[CON_CH_NUM];

void channel_init(channel_t *channel, int chid)
{
	memset(channel, 0, sizeof(*channel));
	// phone
	channel->chid = chid;
	channel->state = STATE_IDLE;
	channel->dial_code[0] = 0;
	channel->dial_index = 0;
	// rtp
	channel->ip = ntohl(inet_addr("127.0.0.1"));
	channel->port = 9000 + chid;
	// reomte 
	channel->remote = NULL;
}

int channel_input(channel_t *channel, SIGNSTATE val)
{
	if (channel->dial_index >= sizeof(channel->dial_code) - 1) 
	{
		printf("%s: out of buffer\n", __FUNCTION__);
		return DIAL_NONE;
	}

	if (val >= SIGN_KEY1 && val <= SIGN_KEY9) 
		channel->dial_code[channel->dial_index++] = (char) '0' + val;
	else if (val == SIGN_KEY0)
		channel->dial_code[channel->dial_index++] = '0';
	else if (val == SIGN_STAR)
		channel->dial_code[channel->dial_index++] = '.';
	else if (val == SIGN_HASH)
		return DIAL_OTHER;

	channel->dial_code[channel->dial_index] = 0;

	// check local dial
	if (strcmp(channel->dial_code, FUNCKEY_FXO_0) == 0)
		return DIAL_FXO_0;
	else if (strcmp(channel->dial_code, FUNCKEY_FXO_1) == 0)
		return DIAL_FXO_1;
	else if (strcmp(channel->dial_code, FUNCKEY_FXO_2) == 0)
		return DIAL_FXO_2;
	else if (strcmp(channel->dial_code, FUNCKEY_FXO_3) == 0)
		return DIAL_FXO_3;
	else if (strcmp(channel->dial_code, FUNCKEY_FXS_0) == 0)
		return DIAL_FXS_0;
	else if (strcmp(channel->dial_code, FUNCKEY_FXS_1) == 0)
		return DIAL_FXS_1;

	return DIAL_NONE;
}

void channel_dial_local(channel_t *channel, channel_t *remote_channel)
{
	if (remote_channel->chid == channel->chid)
	{
		printf("%s: couldn't dial itself\n", __FUNCTION__);
		rtk_SetPlayTone(channel->chid, 0, DSPCODEC_TONE_BUSY, 1, DSPCODEC_TONEDIRECTION_LOCAL);
		return;
	}

	// 1. create local rtp
	channel->remote = remote_channel;
	channel_rtp_start(channel);
	channel->state = STATE_CONNECT;

	// 2. signal remote channel
	remote_channel->remote = channel;

	if (CHANNEL_IS_FXS(remote_channel->chid))
	{
		rtk_SetPlayTone(channel->chid, 0, DSPCODEC_TONE_RINGING, 1, DSPCODEC_TONEDIRECTION_LOCAL);
		// signal fxs
		rtk_SetRingFXS(remote_channel->chid, 1);
	}
	else
	{	
		// signal fxo
		rtk_FXO_RingOn(remote_channel->chid);
	}

	remote_channel->state = STATE_RING;
}

void channel_resetup_local_t38(channel_t *channel)
{
	channel_rtp_stop(channel);
	channel_t38_start(channel);
}

void channel_dial_out(channel_t *channel)
{
	printf("%s: dial %s done.\n", __FUNCTION__, channel->dial_code);
	rtk_SetPlayTone(channel->chid, 0, DSPCODEC_TONE_BUSY, 1, DSPCODEC_TONEDIRECTION_LOCAL);
}

void channel_switch_to_fxo(channel_t *channel)
{
	if (rtk_DAA_off_hook(channel->chid) == 0)
	{
		printf("%s: ok.\n", __FUNCTION__);
	}
	else	// pstn out failed
	{
		printf("%s: failed.\n", __FUNCTION__);
		rtk_SetPlayTone(channel->chid, 0, DSPCODEC_TONE_BUSY, 1, DSPCODEC_TONEDIRECTION_LOCAL);
	}
}

void channel_rtp_start(channel_t *channel)
{
	rtp_config_t rtp_config;
	payloadtype_config_t codec_config;

	// set rtp session
	rtp_config.chid = channel->chid;
	rtp_config.sid = 0;
	rtp_config.isTcp = 0;		// use udp
	rtp_config.extIp = htonl(channel->ip);
	rtp_config.remIp = htonl(channel->remote->ip);
	rtp_config.extPort = htons(channel->port);
	rtp_config.remPort = htons(channel->remote->port);
#ifdef SUPPORT_VOICE_QOS
	rtp_config.tos = 0;
#endif
	rtp_config.rfc2833_payload_type_local = 0;		// use in-band
	rtp_config.rfc2833_payload_type_remote = 0;		// use in-band
	rtp_config.rfc2833_fax_modem_pt_local = 0;		// use in-band
	rtp_config.rfc2833_fax_modem_pt_remote = 0;		// use in-band
#ifdef SUPPORT_RTP_REDUNDANT
	rtp_config.rtp_redundant_payload_type_local = 0;
	rtp_config.rtp_redundant_payload_type_remote = 0;
	rtp_config.rtp_redundant_max_Audio = 0;
	rtp_config.rtp_redundant_max_RFC2833 = 0;
#endif
	rtp_config.SID_payload_type_local = 0;
	rtp_config.SID_payload_type_remote = 0;
	rtp_config.init_randomly = 1;
	rtp_config.init_seqno = 0;
	rtp_config.init_SSRC = 0;
	rtp_config.init_timestamp = 0;
	rtk_SetRtpConfig(&rtp_config);

	// set rtp payload, and other session parameters.
	codec_config.chid = channel->chid;
	codec_config.sid = 0;
	codec_config.local_pt = 0;
	codec_config.remote_pt = 0;
	codec_config.uPktFormat = rtpPayloadPCMU;
	codec_config.local_pt_vbd = rtpPayloadUndefined;
	codec_config.remote_pt_vbd = rtpPayloadUndefined;
	codec_config.uPktFormat_vbd = rtpPayloadUndefined;
	codec_config.nG723Type = 0;
	codec_config.nFramePerPacket = 1;
	codec_config.nFramePerPacket_vbd = 1;
	codec_config.bVAD = 0;
	codec_config.bPLC = 1;
	codec_config.nJitterDelay = 4;
	codec_config.nMaxDelay = 13;
	codec_config.nJitterFactor = 7;
	codec_config.nG726Packing = 2;//pack right
	if (codec_config.uPktFormat == rtpPayloadG722)
		codec_config.nPcmMode = 3;	// wide-band
	else
		codec_config.nPcmMode = 2;	// narrow-band
	rtk_SetRtpPayloadType(&codec_config);

	// start rtp 
	rtk_SetRtpSessionState(channel->chid, 0, rtp_session_sendrecv);
}

void channel_t38_start(channel_t *channel)
{
	t38udp_config_t t38udp_config;
	t38_payloadtype_config_t t38_config;

	// set rtp session
	t38udp_config.chid = channel->chid;
	t38udp_config.sid = 0;
	t38udp_config.isTcp = 0;		// use udp
	t38udp_config.extIp = htonl(channel->ip);
	t38udp_config.remIp = htonl(channel->remote->ip);
	t38udp_config.extPort = htons(channel->port);
	t38udp_config.remPort = htons(channel->remote->port);
#ifdef SUPPORT_VOICE_QOS
	t38udp_config.tos = 0;
#endif
	
	rtk_SetT38UdpConfig(&t38udp_config);

	// set rtp payload, and other session parameters.
	t38_config.chid = channel->chid;
	t38_config.sid = 0;
	t38_config.bT38ParamEnable = 0;
	t38_config.nT38MaxBuffer = 0;
	t38_config.nT38RateMgt = 0;
	t38_config.nT38MaxRate = 0;
	t38_config.bT38EnableECM = 0;
	t38_config.nT38ECCSignal = 0;
	t38_config.nT38ECCData = 0;
	t38_config.bT38EnableSpoof = 0;
	t38_config.nT38DuplicateNum = 0;
	t38_config.nPcmMode = 2;	// narrow-band
	rtk_SetT38PayloadType(&t38_config);

	// start rtp 
	//rtk_SetRtpSessionState(channel->chid, 0, rtp_session_sendrecv);
}

void channel_rtp_stop(channel_t *channel)
{
	rtk_SetRtpSessionState(channel->chid, 0, rtp_session_inactive);
}

void channel_handle_input(channel_t *channel, SIGNSTATE val)
{
	int ret;

	rtk_SetPlayTone(channel->chid, 0, DSPCODEC_TONE_HOLD, 0, DSPCODEC_TONEDIRECTION_LOCAL);
	ret = channel_input(channel, val);
	switch (ret)
	{
	case DIAL_FXO_0:
		if (fxo_type == FXO_NO_DAA)
		{
			printf("%s: no fxo\n", __FUNCTION__);
			rtk_SetPlayTone(channel->chid, 0, DSPCODEC_TONE_BUSY, 1, DSPCODEC_TONEDIRECTION_LOCAL);
			return;
		}
		if (fxo_type == FXO_VIRTUAL_DAA)
		{
			channel_switch_to_fxo(channel);							// switch to fxo
		}
		else
		{
			channel_dial_local(channel, &channels[RTK_VOIP_DAA_CH_OFFSET( g_VoIP_Feature )+0]);	// dial fxo 0
		}
		break;
	case DIAL_FXO_1:
		if (fxo_type == FXO_NO_DAA)
		{
			printf("%s: no fxo\n", __FUNCTION__);
			rtk_SetPlayTone(channel->chid, 0, DSPCODEC_TONE_BUSY, 1, DSPCODEC_TONEDIRECTION_LOCAL);
			return;
		}
		if (fxo_type == FXO_VIRTUAL_DAA)
		{
			channel_switch_to_fxo(channel);							// switch to fxo
		}
		else
		{
			channel_dial_local(channel, &channels[RTK_VOIP_DAA_CH_OFFSET( g_VoIP_Feature )+1]);	// dial fxo 1
		}
		break;
	case DIAL_FXO_2:
		if (fxo_type == FXO_NO_DAA)
		{
			printf("%s: no fxo\n", __FUNCTION__);
			rtk_SetPlayTone(channel->chid, 0, DSPCODEC_TONE_BUSY, 1, DSPCODEC_TONEDIRECTION_LOCAL);
			return;
		}
		if (fxo_type == FXO_VIRTUAL_DAA)
		{
			channel_switch_to_fxo(channel);							// switch to fxo
		}
		else
		{
			channel_dial_local(channel, &channels[RTK_VOIP_DAA_CH_OFFSET( g_VoIP_Feature )+2]);	// dial fxo 2
		}
		break;
	case DIAL_FXO_3:
		if (fxo_type == FXO_NO_DAA)
		{
			printf("%s: no fxo\n", __FUNCTION__);
			rtk_SetPlayTone(channel->chid, 0, DSPCODEC_TONE_BUSY, 1, DSPCODEC_TONEDIRECTION_LOCAL);
			return;
		}
		if (fxo_type == FXO_VIRTUAL_DAA)
		{
			channel_switch_to_fxo(channel);							// switch to fxo
		}
		else
		{
			channel_dial_local(channel, &channels[RTK_VOIP_DAA_CH_OFFSET( g_VoIP_Feature )+3]);	// dial fxo 3
		}
		break;
	case DIAL_FXS_0:
		channel_dial_local(channel, &channels[0]);	// dial fxs 0
		break;
	case DIAL_FXS_1:
		if (RTK_VOIP_SLIC_NUM( g_VoIP_Feature ) < 2)
		{
			printf("%s: no fxs 1\n", __FUNCTION__);
			rtk_SetPlayTone(channel->chid, 0, DSPCODEC_TONE_BUSY, 1, DSPCODEC_TONEDIRECTION_LOCAL);
			return;
		}
		channel_dial_local(channel, &channels[1]);	// dial fxs 1
		break;
	case DIAL_OTHER:
		channel_dial_out(channel);
		break;
	default:
		// nothing
		break;
	}
}

void channel_handle_onhook(channel_t *channel)
{
	switch (channel->state)
	{
	case STATE_CONNECT:
		channel_rtp_stop(channel);
		if (CHANNEL_IS_FXS(channel->chid))
		{
			// fxs
		}
		else
		{
			// daa
			rtk_FXO_onhook(channel->chid);
		}

		// check remote channel is terminated 
		if (channel->remote->remote == NULL) 
			break;

		// signal remote channel
		if (CHANNEL_IS_FXS(channel->remote->chid))
		{
			if(channel->remote->state == STATE_RING)
			{
				channel->remote->state = STATE_IDLE;
				rtk_SetRingFXS(channel->remote->chid, 0);
			}
			else if(channel->remote->state == STATE_CONNECT)
			{
				rtk_SetPlayTone(channel->remote->chid, 0, DSPCODEC_TONE_BUSY, 1, DSPCODEC_TONEDIRECTION_LOCAL);
				channel_rtp_stop(channel->remote);
			}
		}
		else
		{
			rtk_FXO_Busy(channel->remote->chid);
		}
		break;
	case STATE_DAA_CONNECT:
		// virtual daa
		rtk_DAA_on_hook(channel->chid);
		break;
	default:
		break;
	}

#ifdef CONFIG_RTK_VOIP_IVR
	rtk_IvrStopPlaying(channel->chid); // clear ivr
#endif
	rtk_SetTranSessionID(channel->chid, 255);	// clear resource count
	rtk_SetPlayTone(channel->chid, 0, DSPCODEC_TONE_HOLD, 0, DSPCODEC_TONEDIRECTION_LOCAL);	// clear tone
	channel_init(channel, channel->chid);
}

void channel_handle_offhook(channel_t *channel)
{
	// active session 0, and record resource + 1
	rtk_SetTranSessionID(channel->chid, 0);

	switch (channel->state)
	{
	case STATE_IDLE:
		if (CHANNEL_IS_FXS(channel->chid))
		{
			// fxs
			rtk_SetPlayTone(channel->chid, 0, DSPCODEC_TONE_DIAL, 1, DSPCODEC_TONEDIRECTION_LOCAL);
		}
		else
		{
			char caller_id[21];
			char caller_date[9];
			char caller_name[51];

		#ifdef FXO_DIAL_FXS0_AUTO
			rtk_Get_DAA_CallerID(channel->chid, caller_id, caller_date, caller_name);

			if (caller_id[0])
				printf("caller id is %s\n", caller_id);
			else
				printf("no caller id detect\n");

			if (caller_name[0])
				printf("caller name is %s\n", caller_name);
				
#if 1	// Gen FSK CID

			//rtk_Set_CID_FSK_GEN_MODE(channel->chid - 4, 1); // soft gen
			rtk_Set_FSK_Area(channel->chid - 4, 0x108/*area*/);   /* area -> 0:Bellcore 1:ETSI 2:BT 3:NTT */
			rtk_Gen_CID_And_FXS_Ring(channel->chid - 4, 1/*fsk*/, caller_id, caller_date, caller_name, 0/*type1*/, 0);
#endif

//			channel_dial_local(channel, &channels[0]);	// dial fxs 0
			channel_dial_local(channel, &channels[channel->chid - 4]);	// dial fxs 0
		#else
			char text[] = {IVR_TEXT_ID_PLZ_ENTER_NUMBER, '\0'};

			usleep(500000); // after phone off-hook, wait for a second, and then play IVR
			rtk_FXO_offhook(channel->chid);
		#ifdef CONFIG_RTK_VOIP_IVR
			rtk_IvrStartPlaying(channel->chid, text);
		#else
			// use dial tone to replace if no IVR
			rtk_SetPlayTone(channel->chid, 0, DSPCODEC_TONE_DIAL, 1, DSPCODEC_TONEDIRECTION_LOCAL);
		#endif
		#endif
		}
		break;
	case STATE_RING:
		if (CHANNEL_IS_FXS(channel->chid))
		{
			// fxs
			rtk_SetRingFXS(channel->chid, 0);
			rtk_SetPlayTone(channel->remote->chid, 0, DSPCODEC_TONE_RINGING, 0, DSPCODEC_TONEDIRECTION_LOCAL);
			channel_rtp_start(channel);
			channel->state = STATE_CONNECT;
		}
		else
		{
			// daa
			rtk_FXO_offhook(channel->chid);
			channel_rtp_start(channel);
			channel->state = STATE_CONNECT;
		}
				
		// ack remote channel
		if (CHANNEL_IS_FXS(channel->remote->chid))
		{
			// fxs
		}
		else
		{
			// daa
			rtk_FXO_offhook(channel->remote->chid);
		}
		break;
	case STATE_DAA_RING:
		// virtual daa
		rtk_DAA_off_hook(channel->chid);
		channel->state = STATE_DAA_CONNECT;
		break;
	default:
		break;
	}
}

int main(void)
{
	int i;
	uint32 fax[CON_CH_NUM] = {0};
	int fax_clear[CON_CH_NUM] = {[0 ... CON_CH_NUM-1] = 1};
	uint32 fax_end[CON_CH_NUM] = {0};
	int fax_end_clear[CON_CH_NUM] = {[0 ... CON_CH_NUM-1] = 1};

	SIGNSTATE val;
	channel_t *channel;

	if ((g_VoIP_Feature & DAA_TYPE_MASK) == NO_DAA)
		fxo_type = FXO_NO_DAA;
	else if ((g_VoIP_Feature & DAA_TYPE_MASK) == VIRTUAL_DAA)
		fxo_type = FXO_VIRTUAL_DAA;
	else
		fxo_type = FXO_DAA;

	// init 
	for (i=0; i<CON_CH_NUM; i++)
	{
		// init dsp
		rtk_InitDSP(i);

		// init phone
		rtk_Set_Voice_Gain(i, 0, 0);	// 0db
		rtk_Set_echoTail(i, 4, 1);			// 4ms, NLP on

		// init fxs & fxo
		if (CHANNEL_IS_FXS(i))
		{
			// fxs
			rtk_Set_Flash_Hook_Time(i, 0, 300);		// 0~300 ms
			rtk_Set_SLIC_Ring_Cadence(i, 1500, 1500);	// 1500 ms
			rtk_Set_CID_DTMF_MODE(i, 0x40, 80, 80, 300, 300);	// priori 1st ring, auto AC, auto ring, digit duration = 80ms, inter digit pause = 80ms
		}
		else
		{
			// fxo
			rtk_Set_FXO_Ring_Detection(300, 300, 4500);
			rtk_Set_CID_Det_Mode(i,
				2,										// auto detect mode (NOT NTT)
				CID_DTMF
			);
		}

		// set dtmf mode
		rtk_SetDTMFMODE(i, DTMF_INBAND);

		// set fax modem det mode
		rtk_SetFaxModemDet(i, 0); //0:auto-hi-speed-fax


		// flush kernel fifo before app run
		rtk_Set_flush_fifo(i);

		// init local data
		channel_init(&channels[i], i);

		rtk_Onhook_Reinit(i);
	}

	// main loop
main_loop:

	for (i=0; i<CON_CH_NUM; i++)
	{
		channel = &channels[i];

		if (CHANNEL_IS_FXS(i))
			rtk_GetFxsEvent(i, &val);
		else
			rtk_GetFxoEvent(i, &val);

		// Thlin add: once detect CED, change codec to T.38
		/*************** FAX Det ****************/
		if (CHANNEL_IS_FXS(i))
		{
			rtk_GetFaxModemEvent(i, &fax[i], NULL, 0);

			if (((fax[i] == 1) || (fax[i] == 2))	//1:low-speed fax, 2: high-speed fax, 3:local-modem, 4: remote-modem
				&& (fax_clear[i] ==1))
			{
				printf("Get FAX event, ch=%d\n", i);
				channel_resetup_local_t38(channel);
				channel_resetup_local_t38(channel->remote);
				fax_clear[i] = 0;
			}
		}
		
		uint32 dis = 0;
		rtk_GetFaxDisDetect(i, &dis); 
		if (dis == 1)
		{
			printf("Get FAX DIS event, ch=%d\n", i);
		}
		
		dis = 0;
		rtk_GetFaxDisTxDetect(i, &dis); 
		if (dis == 1)
		{
			printf("Get FAX DIS TX event, ch=%d\n", i);
		}
		
		dis = 0;
		rtk_GetFaxDisRxDetect(i, &dis); 
		if (dis == 1)
		{
			printf("Get FAX DIS RX event, ch=%d\n", i);
		}

		rtk_GetFaxEndDetect(i, &fax_end[i]); 
		if ((fax_end[i] == 1) && (fax_end_clear[i] == 1))
		{
			printf("Get FAX End event, ch=%d\n", i);
			fax_end_clear[i] = 0;
		}

		/****************************************/

		switch (val)
		{
		case SIGN_KEY1:
		case SIGN_KEY2:
		case SIGN_KEY3:
		case SIGN_KEY4:
		case SIGN_KEY5:
		case SIGN_KEY6:
		case SIGN_KEY7:
		case SIGN_KEY8:
		case SIGN_KEY9:
		case SIGN_KEY0:
		case SIGN_STAR:
		case SIGN_HASH:
			if (channel->state == STATE_IDLE)
			{
				channel_handle_input(channel, val);
			}
			break;
		case SIGN_ONHOOK:
			channel_handle_onhook(channel);
			rtk_Onhook_Action(i);	// rtk_Onhook_Action have to be the last step in SIGN_ONHOOK
			fax_clear[i] = 1;
			fax_end_clear[i] = 1;
			break;
		case SIGN_OFFHOOK:
			rtk_Offhook_Action(i);	// rtk_Offhook_Action have to be the first step in SIGN_OFFHOOK
			channel_handle_offhook(channel);
			break;
    	case SIGN_RING_ON:		// virtual daa ring on
			if (channel->state == STATE_IDLE)
			{
				channel->state = STATE_DAA_RING;
				rtk_SetRingFXS(channel->chid, 1);
			}
			break;
		case SIGN_RING_OFF: 	// virtual daa ring off
			if (channel->state == STATE_DAA_RING)
			{
				rtk_SetRingFXS(channel->chid, 0);
				channel->state = STATE_IDLE;
			}
			break;
		case SIGN_FLASHHOOK:
			break;
		case SIGN_OFFHOOK_2: // off-hook but no event
			break;
		case SIGN_NONE:
			break;
		default:
			printf("unknown(%d)\n", val);
			break;
		}

		usleep(100000 / CON_CH_NUM); // 100ms
	}

	goto main_loop;

	return 0;
}
