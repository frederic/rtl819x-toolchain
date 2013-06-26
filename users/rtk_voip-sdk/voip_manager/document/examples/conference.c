#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "voip_manager.h"

#define MAX_SESSION  2	//(SESS_NUM / VOIP_CH_NUM)

enum {
	STATE_IDLE = 0,
	STATE_RTP
};

int main(int argc, char *argv[])
{
	int chid, ssid;
	SIGNSTATE val;
	rtp_config_t rtp_config[CON_CH_NUM][MAX_SESSION];
	payloadtype_config_t codec_config[CON_CH_NUM][MAX_SESSION];
	TstVoipMgr3WayCfg stVoipMgr3WayCfg;
	int ActiveSession[CON_CH_NUM];
	int session_state[CON_CH_NUM][MAX_SESSION];
	int dtmf_val[SIGN_HASH + 1] = {0,1,2,3,4,5,6,7,8,9,0,10,11};
	char dial_val[SIGN_HASH + 1] = {'\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '.', '#'};
	char dial_code[CON_CH_NUM][256];
	int dial_index[CON_CH_NUM];
	int i, j;
	char *src_ip;
	int  src_port;

	if (argc == 2)
	{
		src_ip = argv[1];
		src_port = 9000;
	}
	else if (argc == 3)
	{
		src_ip = argv[1];
		src_port = atoi(argv[2]);
	}
	else
	{
		printf("usage: %s src_ip [src_port]\n", argv[0]);
		return 0;
	}

	memset( dial_index, 0, sizeof( dial_index ) );	// avoid warning: "may be used uninitialized" 
	
	// init
	for (i=0; i<CON_CH_NUM; i++)
	{
		if( !RTK_VOIP_IS_SLIC_CH( i, g_VoIP_Feature ) )
			continue;
			
		rtk_InitDSP(i);
		rtk_Set_Flash_Hook_Time(i, 0, 300);		// 0~300 ms
		rtk_Set_flush_fifo(i);					// flush kernel fifo before app run
		rtk_SetDTMFMODE(i, DTMF_RFC2833);		// set dtmf mode
		ActiveSession[i] = 0;
		for (j=0; j<MAX_SESSION; j++)
		{
			session_state[i][j] = STATE_IDLE;
		}
	}

main_loop:

	for (chid=0; chid<CON_CH_NUM; chid++)
	{
		if( !RTK_VOIP_IS_SLIC_CH( i, g_VoIP_Feature ) )
			continue;
			
		rtk_GetFxsEvent(chid, &val);
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
		{
			ssid = ActiveSession[chid];
			if (session_state[chid][ssid] == STATE_RTP)
			{
				rtk_SetRTPRFC2833(chid, ssid, dtmf_val[val], 100);
			}
			else if (val == SIGN_HASH)
			{
				char *pPort;
				char *dest_ip;
				int dest_port;

				dial_code[chid][dial_index[chid]] = 0;

				// 192.168.1.1..9002 => ip: 192.168.1.1, rtp port:9002
				pPort = strstr(dial_code[chid], "..");	
				if (pPort)
				{
					*pPort = 0;
					pPort += 2;
					dest_ip = dial_code[chid];
					dest_port = atoi(pPort);
				}
				else
				{
					dest_ip = dial_code[chid];
					dest_port = 9000;
				}
				
				// set rtp session
				rtp_config[chid][ssid].chid = chid;		// use channel 0
				rtp_config[chid][ssid].sid = ssid;
				rtp_config[chid][ssid].isTcp = 0;		// use udp
				rtp_config[chid][ssid].extIp = inet_addr(src_ip);
				rtp_config[chid][ssid].remIp = inet_addr(dest_ip);
				rtp_config[chid][ssid].extPort = htons(src_port + chid * 2);
				rtp_config[chid][ssid].remPort = htons(dest_port);
#ifdef SUPPORT_VOICE_QOS
				rtp_config[chid][ssid].tos = 0;
#endif
				rtp_config[chid][ssid].rfc2833_payload_type_local = 96;
				rtp_config[chid][ssid].rfc2833_payload_type_remote = 96;
				rtp_config[chid][ssid].rfc2833_fax_modem_pt_local = 101;
				rtp_config[chid][ssid].rfc2833_fax_modem_pt_remote = 101;
#ifdef SUPPORT_RTP_REDUNDANT
				rtp_config[chid][ssid].rtp_redundant_payload_type_local = 0;
				rtp_config[chid][ssid].rtp_redundant_payload_type_remote = 0;
				rtp_config[chid][ssid].rtp_redundant_max_Audio = 0;
				rtp_config[chid][ssid].rtp_redundant_max_RFC2833 = 0;
#endif
				rtp_config[chid][ssid].SID_payload_type_local = 0;
				rtp_config[chid][ssid].SID_payload_type_remote = 0;
				rtp_config[chid][ssid].init_randomly = 1;
				rtp_config[chid][ssid].init_seqno = 0;
				rtp_config[chid][ssid].init_SSRC = 0;
				rtp_config[chid][ssid].init_timestamp = 0;

				rtk_SetRtpConfig(&rtp_config[chid][ssid]);

				// set rtp payload, and other session parameters.
				codec_config[chid][ssid].chid = chid;		// use channel 0
				codec_config[chid][ssid].sid = ssid;
				codec_config[chid][ssid].local_pt = 0;		// PCMU use pt 0
				codec_config[chid][ssid].remote_pt = 0;		// PCMU use pt 0
				codec_config[chid][ssid].uPktFormat = rtpPayloadPCMU;
				codec_config[chid][ssid].local_pt_vbd = rtpPayloadUndefined;
				codec_config[chid][ssid].remote_pt_vbd = rtpPayloadUndefined;
				codec_config[chid][ssid].uPktFormat_vbd = rtpPayloadUndefined;
				codec_config[chid][ssid].nG723Type = 0;
				codec_config[chid][ssid].nFramePerPacket = 1;
				codec_config[chid][ssid].nFramePerPacket_vbd = 1;
				codec_config[chid][ssid].bVAD = 0;
				codec_config[chid][ssid].bPLC = 1;
				codec_config[chid][ssid].nJitterDelay = 4;
				codec_config[chid][ssid].nMaxDelay = 13;
				codec_config[chid][ssid].nJitterFactor = 7;
				codec_config[chid][ssid].nG726Packing = 2;//pack right
				if (codec_config[chid][ssid].uPktFormat == rtpPayloadG722)
					codec_config[chid][ssid].nPcmMode = 3;	// wide-band
				else
					codec_config[chid][ssid].nPcmMode = 2;	// narrow-band
				rtk_SetRtpPayloadType(&codec_config[chid][ssid]);

				// start rtp (channel number = 0, session number = 0)
				rtk_SetRtpSessionState(chid, ssid, rtp_session_sendrecv);

				session_state[chid][ssid] = STATE_RTP;
			
				printf("%s:%d -> %s:%d\n",
					src_ip, src_port + chid * 2, dest_ip, dest_port);
			}
			else
			{
				// stop dial tone in current session
				rtk_SetPlayTone(chid, ssid, DSPCODEC_TONE_HOLD, 0, DSPCODEC_TONEDIRECTION_LOCAL);
				dial_code[chid][dial_index[chid]++] = dial_val[val];
			}
			break;
		}

		case SIGN_OFFHOOK:
		{
			// do rtk_SLIC_Offhook_Action at first
			rtk_Offhook_Action(chid);
			rtk_SetTranSessionID(chid, 0);
			ActiveSession[chid] = 0;
			dial_index[chid] = 0;
			for (i=0; i<MAX_SESSION; i++)
			{
				session_state[chid][i] = STATE_IDLE;
			}
			rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_DIAL, 1, DSPCODEC_TONEDIRECTION_LOCAL);	// play dial tone
			break;
		}

		case SIGN_ONHOOK:
			for (i=0; i<MAX_SESSION; i++)
			{
				rtk_SetRtpSessionState(chid, i, rtp_session_inactive);
				rtk_SetPlayTone(chid, i, DSPCODEC_TONE_HOLD, 0, DSPCODEC_TONEDIRECTION_LOCAL); // stop tone
			}
			// clear resource count
			rtk_SetTranSessionID(chid, 255);	
			// close conference
			memset(&stVoipMgr3WayCfg, 0, sizeof(stVoipMgr3WayCfg));
			stVoipMgr3WayCfg.ch_id = chid;
			stVoipMgr3WayCfg.enable = 0;
			rtk_SetConference(&stVoipMgr3WayCfg);
			// do rtk_SLIC_Onhook_Action at last
			rtk_Onhook_Action(chid);		
			break;

		case SIGN_FLASHHOOK:
		{
			dial_index[chid] = 0;
			ssid = ActiveSession[chid];
			if (session_state[chid][ssid] == STATE_RTP)
			{
				// check another session state
				if (session_state[chid][!ssid] == STATE_IDLE) 
				{
					// hold current session
					rtk_Hold_Rtp(chid, ssid, 1);
					// change to new session 
					ssid = !ssid;
					ActiveSession[chid] = ssid;
					rtk_SetTranSessionID(chid, ssid);
					// play dial tone in new session
					rtk_SetPlayTone(chid, ssid, DSPCODEC_TONE_DIAL, 1, DSPCODEC_TONEDIRECTION_LOCAL);
				}
				else
				{
					// resume another session
					rtk_Hold_Rtp(chid, !ssid, 0);
					// do conference
					memset(&stVoipMgr3WayCfg, 0, sizeof(stVoipMgr3WayCfg));
					stVoipMgr3WayCfg.ch_id = chid;
					stVoipMgr3WayCfg.enable = 1;
					for (i=0; i<MAX_SESSION; i++)
					{
						stVoipMgr3WayCfg.rtp_cfg[i].ch_id = chid;
						stVoipMgr3WayCfg.rtp_cfg[i].m_id = rtp_config[chid][i].sid;
						stVoipMgr3WayCfg.rtp_cfg[i].ip_src_addr = rtp_config[chid][i].remIp;
						stVoipMgr3WayCfg.rtp_cfg[i].ip_dst_addr = rtp_config[chid][i].extIp;
						stVoipMgr3WayCfg.rtp_cfg[i].udp_src_port = rtp_config[chid][i].remPort;
						stVoipMgr3WayCfg.rtp_cfg[i].udp_dst_port = rtp_config[chid][i].extPort;
#ifdef SUPPORT_VOICE_QOS
						stVoipMgr3WayCfg.rtp_cfg[i].tos = rtp_config[chid][i].tos;
#endif
						stVoipMgr3WayCfg.rtp_cfg[i].rfc2833_payload_type_local = 
							rtp_config[chid][i].rfc2833_payload_type_local;
						stVoipMgr3WayCfg.rtp_cfg[i].rfc2833_payload_type_remote = 
							rtp_config[chid][i].rfc2833_payload_type_remote;
						stVoipMgr3WayCfg.rtp_cfg[i].local_pt = codec_config[chid][i].local_pt;
						stVoipMgr3WayCfg.rtp_cfg[i].remote_pt = codec_config[chid][i].remote_pt;
						stVoipMgr3WayCfg.rtp_cfg[i].uPktFormat = codec_config[chid][i].uPktFormat;
						stVoipMgr3WayCfg.rtp_cfg[i].nG723Type = codec_config[chid][i].nG723Type;
						stVoipMgr3WayCfg.rtp_cfg[i].nFramePerPacket = codec_config[chid][i].nFramePerPacket; 
						stVoipMgr3WayCfg.rtp_cfg[i].nFramePerPacket_vbd = codec_config[chid][i].nFramePerPacket_vbd;
						stVoipMgr3WayCfg.rtp_cfg[i].bVAD = codec_config[chid][i].bVAD;
						stVoipMgr3WayCfg.rtp_cfg[i].bPLC = codec_config[chid][i].bPLC;
						stVoipMgr3WayCfg.rtp_cfg[i].nJitterDelay = codec_config[chid][i].nJitterDelay;
						stVoipMgr3WayCfg.rtp_cfg[i].nMaxDelay = codec_config[chid][i].nMaxDelay;
						stVoipMgr3WayCfg.rtp_cfg[i].nJitterFactor = codec_config[chid][i].nJitterFactor;
					}
					rtk_SetConference(&stVoipMgr3WayCfg);
				}
			}
			else
			{
				if (session_state[chid][!ssid] == STATE_RTP) 
				{
					// stop dial tone in current session
					rtk_SetPlayTone(chid, ssid, DSPCODEC_TONE_HOLD, 0, DSPCODEC_TONEDIRECTION_LOCAL);
					// change to old session 
					ssid = !ssid;
					ActiveSession[chid] = ssid;
					rtk_SetTranSessionID(chid, ssid);
					// resume old session
					rtk_Hold_Rtp(chid, ssid, 0);
				}
				else
				{
					// do nothing if no rtp session exist
				}
			}
			break;
		}

		default:
			break;
		}

		usleep(100000 / CON_CH_NUM); // 100ms
	}

	goto main_loop;

	return 0;
}

