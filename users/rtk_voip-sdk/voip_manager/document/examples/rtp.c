#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "voip_manager.h"

/*
 *  RTP sample code.
 *  Send RTP and RTPC to specific IP address and port number when 
 *  off-hook handset. Press flash-hook to hold/resume RTP. 
 *  User hears a tone ,sounds like du du du, when the call is held.
 *  ATA sends RFC 2833 packets when user press phone digit key.
 *  Please killall solar_monitor before running this sample code.
 *
 */
#ifdef __ECOS
#include <commands.h>
shell_old_cmd( "rtp", "", "", user_rtp_main );
#endif

#ifdef __ECOS
CMD_DECL( user_rtp_main )
#else
int main(int argc, char *argv[])
#endif
{
	SIGNSTATE val;
	rtp_config_t rtp_config;
	rtcp_config_t rtcp_config;
	payloadtype_config_t codec_config;
	int ActiveSession;
	int bStartRTP;
	int dtmf_val[SIGN_HASH + 1] = {0,1,2,3,4,5,6,7,8,9,0,10,11};
	int chid = 0;

	if (argc != 6)
	{
		printf("usage: %s chid src_ip dest_ip src_port dest_port\n", argv[0]);
		return 0;
	}

	// init channel
	chid = atoi(argv[1]);

	// init dsp
	rtk_InitDSP(chid);

	rtk_SetDTMFMODE(chid, DTMF_RFC2833);	// set dtmf mode

	// init flag
	bStartRTP = 0;
	ActiveSession = 0;

	while (1)
	{
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
			if (bStartRTP && ActiveSession == 0)
				rtk_SetRTPRFC2833(chid, 0, dtmf_val[val], 100);
			break;

		case SIGN_OFFHOOK:
			// call rtk_Offhook_Action at first
			rtk_Offhook_Action(chid);
			ActiveSession = 0;
			rtk_SetTranSessionID(chid, ActiveSession);

			// set RTP session
			memset(&rtp_config, 0, sizeof(rtp_config));
			rtp_config.chid 			= chid;					// use channel 
			rtp_config.sid 				= ActiveSession;
			rtp_config.isTcp 			= 0;					// use udp
			rtp_config.extIp 			= inet_addr(argv[2]);
			rtp_config.remIp 			= inet_addr(argv[3]);
			rtp_config.extPort 			= htons(atoi(argv[4]));
			rtp_config.remPort 			= htons(atoi(argv[5]));
#ifdef SUPPORT_VOICE_QOS
			rtp_config.tos 				= 0;
#endif
			rtp_config.rfc2833_payload_type_local  = 96;
			rtp_config.rfc2833_payload_type_remote = 96;
			rtp_config.rfc2833_fax_modem_pt_local = 101;
			rtp_config.rfc2833_fax_modem_pt_remote = 101;
#ifdef SUPPORT_RTP_REDUNDANT
			rtp_config.rtp_redundant_payload_type_local  = 0;
			rtp_config.rtp_redundant_payload_type_remote = 0;
			rtp_config.rtp_redundant_max_Audio = 0;
			rtp_config.rtp_redundant_max_RFC2833 = 0;
#endif
			rtp_config.SID_payload_type_local  = 0;
			rtp_config.SID_payload_type_remote = 0;
			rtp_config.init_randomly = 1;
			rtp_config.init_seqno = 0;
			rtp_config.init_SSRC = 0;
			rtp_config.init_timestamp = 0;
			rtk_SetRtpConfig(&rtp_config);

			// set RTCP session
			rtcp_config.chid	= rtp_config.chid;
			rtcp_config.sid		= rtp_config.sid;
			rtcp_config.remIp	= rtp_config.remIp;
			rtcp_config.remPort	= rtp_config.remPort + 1;
			rtcp_config.extIp	= rtp_config.extIp;
			rtcp_config.extPort	= rtp_config.extPort + 1;
#ifdef SUPPORT_VOICE_QOS
			rtcp_config.tos		= rtp_config.tos;
#endif
			
			rtcp_config.txInterval = 500;	// send rtcp every 500ms
			rtcp_config.enableXR = 1;
			
			rtk_SetRtcpConfig(&rtcp_config);


			// set rtp payload, and other session parameters.
			codec_config.chid 				= chid;
			codec_config.sid 				= ActiveSession;
			codec_config.local_pt 			= 18;					// G729 use static pt 18
			codec_config.remote_pt 			= 18;					// G729 use static pt 18
			codec_config.uPktFormat 		= rtpPayloadG729;
			codec_config.local_pt_vbd 		= rtpPayloadUndefined;
			codec_config.remote_pt_vbd 		= rtpPayloadUndefined;
			codec_config.uPktFormat_vbd 	= rtpPayloadUndefined;
			codec_config.nG723Type 			= 0;
			codec_config.nFramePerPacket 	= 1;
			codec_config.nFramePerPacket_vbd = 1;
			codec_config.bVAD 				= 0;
			codec_config.bPLC 				= 1;
			codec_config.nJitterDelay 		= 4;
			codec_config.nMaxDelay 			= 13;
			codec_config.nJitterFactor 		= 7;
			codec_config.nG726Packing 		= 2;					//pack right

			if (codec_config.uPktFormat == rtpPayloadG722)
				codec_config.nPcmMode = 3;	// wide-band
			else
				codec_config.nPcmMode = 2;	// narrow-band

			rtk_SetRtpPayloadType(&codec_config);

			// start rtp (channel number = 0, session number = 0)
			rtk_SetRtpSessionState(chid, ActiveSession, rtp_session_sendrecv);
			bStartRTP = 1;
			printf("%s:%d -> %s:%d\n", argv[2], atoi(argv[4]), argv[3], atoi(argv[5]));

			break;

		case SIGN_ONHOOK:
			// close rtp
			rtk_SetRtpSessionState(chid, 0, rtp_session_inactive);
			rtk_SetRtpSessionState(chid, 1, rtp_session_inactive);
			bStartRTP = 0;

			// call rtk_Offhook_Action at last
			rtk_Onhook_Action(chid);
			break;

		case SIGN_FLASHHOOK:
			ActiveSession = !ActiveSession;
			rtk_SetTranSessionID(chid, ActiveSession);
			if (ActiveSession == 1)
			{
			    // Get RTP statistics
			    TstVoipRtpStatistics pRtpSt;
				rtk_Get_Rtp_Statistics(chid, 0, &pRtpSt);

				printf("\n== RTP 0 Statics ========================\n");
				printf("Rx RTP bytes	= %ld\n",pRtpSt.nRxRtpStatsCountByte);
				printf("Rx RTP packets	= %ld\n",pRtpSt.nRxRtpStatsCountPacket);
				printf("Rx packet loss	= %ld\n",pRtpSt.nRxRtpStatsLostPacket);
				printf("Tx RTP bytes	= %ld\n",pRtpSt.nTxRtpStatsCountByte);
				printf("Tx RTP packets 	= %ld\n",pRtpSt.nTxRtpStatsCountPacket);
				printf("=========================================\n");


				// hold session
				rtk_Hold_Rtp(chid, 0, 1);

				// play a tone. Sounds like "du du du".
				rtk_SetPlayTone(chid, 1, DSPCODEC_TONE_CONFIRMATION, 1, DSPCODEC_TONEDIRECTION_LOCAL);
			}
			else
			{
				// resume session
				rtk_Hold_Rtp(chid, 0, 0);

				// stop tone
				rtk_SetPlayTone(chid, 1, DSPCODEC_TONE_HOLD, 0, DSPCODEC_TONEDIRECTION_LOCAL);
			}
			break;

		default:
			break;
		}

		usleep(100000); // 100ms
	}

	return 0;
}

