#include <netinet/in.h>

//#include "../../linux-2.4.18/net/rtk_voip/voip_manager/voip_mgr_netfilter.h"
#include "voip_manager.h"
#include "voip_ioctl.h"

int main(int argc, char *argv[])
{
	int c, quit;
	TstVoipCfg stVoipCfg;
	TstVoipValue stVoipValue;
	TstVoipMgrSession stVoipMgrSession;
	TstVoipPayLoadTypeConfig stVoipPayLoadTypeConfig;
	TstVoipSlicRing stVoipSlicRing;
	TstVoipPlayToneConfig cfg;
	TstVoipRtpSessionState stVoipRtpSessionState;

	if (argc != 6) {
		printf("Usage: rtptalk src_ip dst_ip codec channel rtp_port_offset\n");
		printf("codec:\n");
		printf("\t 0: rtpPayloadPCMU\n");
		printf("\t 3: rtpPayloadGSM\n");
		printf("\t 4: rtpPayloadG723\n");
		printf("\t 8: rtpPayloadPCMA\n");
		printf("\t18: rtpPayloadG729\n");
		return -1;
	}

#if 0
	stVoipSlicRing.ch_id = 0;
	stVoipSlicRing.ring_set = 1;
	SETSOCKOPT(VOIP_MGR_SLIC_RING, &stVoipSlicRing, TstVoipSlicRing, 1);
	sleep(1);
	stVoipSlicRing.ch_id = 0;
	stVoipSlicRing.ring_set = 0;
	SETSOCKOPT(VOIP_MGR_SLIC_RING, &stVoipSlicRing, TstVoipSlicRing, 1);
#endif
	// start session
	stVoipCfg.ch_id = atoi(argv[4]);
	stVoipCfg.enable = 1;
	SETSOCKOPT(VOIP_MGR_PCM_CFG, &stVoipCfg, TstVoipCfg, 1);

	stVoipCfg.ch_id = atoi(argv[4]);
	stVoipCfg.m_id = 0;
	stVoipCfg.enable = 1;
	SETSOCKOPT(VOIP_MGR_CTRL_RTPSESSION, &stVoipCfg, TstVoipCfg, 1);

	stVoipValue.ch_id = atoi(argv[4]);
	stVoipValue.m_id = 0;
	stVoipValue.value = 7;
	SETSOCKOPT(VOIP_MGR_SET_SLIC_TX_GAIN, &stVoipValue, TstVoipValue, 1);
	stVoipValue.value = 7;
	SETSOCKOPT(VOIP_MGR_SET_SLIC_RX_GAIN, &stVoipValue, TstVoipValue, 1);

	stVoipCfg.ch_id = atoi(argv[4]);
	stVoipCfg.m_id = 0;
	stVoipCfg.enable = 1;
	SETSOCKOPT(VOIP_MGR_DTMF_CFG, &stVoipCfg, TstVoipCfg, 1);
	SETSOCKOPT(VOIP_MGR_CTRL_TRANSESSION_ID, &stVoipCfg, TstVoipCfg, 1);

#if 1
	stVoipMgrSession.ch_id = atoi(argv[4]);
	stVoipMgrSession.m_id = 0;
	stVoipMgrSession.ip_src_addr = htonl(inet_addr(argv[2]));
	stVoipMgrSession.ip_dst_addr = htonl(inet_addr(argv[1]));
	stVoipMgrSession.udp_src_port = htons(0x3333 + atoi(argv[5]));
	stVoipMgrSession.udp_dst_port = htons(0x3333 + atoi(argv[5]));
	stVoipMgrSession.protocol = 0x11; // udp
	stVoipMgrSession.rfc2833_payload_type_local = 96;
	stVoipMgrSession.rfc2833_payload_type_remote = 96;
	stVoipMgrSession.rtp_redundant_payload_type_local = 0;
	stVoipMgrSession.rtp_redundant_payload_type_remote = 0;
	stVoipMgrSession.rtp_redundant_max_Audio = 0;
	stVoipMgrSession.rtp_redundant_max_RFC2833 = 0;
	stVoipMgrSession.SID_payload_type_local = 0;
	stVoipMgrSession.SID_payload_type_remote = 0;
	stVoipMgrSession.init_randomly = 1;
	stVoipMgrSession.init_seqno = 0;
	stVoipMgrSession.init_SSRC = 0;
	stVoipMgrSession.init_timestamp = 0;
	SETSOCKOPT(VOIP_MGR_SET_SESSION, &stVoipMgrSession, TstVoipMgrSession, 1);

	stVoipPayLoadTypeConfig.ch_id = atoi(argv[4]);
	stVoipPayLoadTypeConfig.m_id = 0;
	//stVoipPayLoadTypeConfig.uPktFormat = rtpPayloadPCMA;
	stVoipPayLoadTypeConfig.local_pt = atoi(argv[3]);
	stVoipPayLoadTypeConfig.remote_pt = atoi(argv[3]);
	stVoipPayLoadTypeConfig.uPktFormat = atoi(argv[3]);
	stVoipPayLoadTypeConfig.nG723Type = 0;
	stVoipPayLoadTypeConfig.nFramePerPacket = 2;
	stVoipPayLoadTypeConfig.nFramePerPacket_vbd = 2;
	stVoipPayLoadTypeConfig.bVAD = 0;
	stVoipPayLoadTypeConfig.bPLC = 1;
	SETSOCKOPT(VOIP_MGR_SETRTPPAYLOADTYPE, &stVoipPayLoadTypeConfig, TstVoipPayLoadTypeConfig, 1);

#if 0
	stVoipCfg.ch_id = atoi(argv[4]);
	stVoipCfg.m_id = 0;
	stVoipCfg.enable = 1;
	SETSOCKOPT(VOIP_MGR_RTP_CFG, &stVoipCfg, TstVoipCfg, 1);
#else
	stVoipRtpSessionState.ch_id = atoi(argv[4]);;
	stVoipRtpSessionState.m_id = 0;
	stVoipRtpSessionState.state = rtp_session_sendrecv;
	SETSOCKOPT(VOIP_MGR_SETRTPSESSIONSTATE, &stVoipRtpSessionState, TstVoipRtpSessionState, 1);
#endif

#else

    	cfg.ch_id = 0;
    	cfg.m_id = 0;
    	cfg.nTone = 12;
    	cfg.bFlag = 1;
    	cfg.path = 0;
	SETSOCKOPT(VOIP_MGR_SETPLAYTONE, &cfg, TstVoipPlayToneConfig, 1);
#endif

	return 0;
}
