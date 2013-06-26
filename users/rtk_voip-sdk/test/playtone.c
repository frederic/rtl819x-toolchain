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
	stVoipCfg.ch_id = 0;
	stVoipCfg.enable = 1;
	SETSOCKOPT(VOIP_MGR_PCM_CFG, &stVoipCfg, TstVoipCfg, 1);
	
	stVoipCfg.ch_id = 0;
	stVoipCfg.m_id = 0;
	stVoipCfg.enable = 1;
	SETSOCKOPT(VOIP_MGR_CTRL_RTPSESSION, &stVoipCfg, TstVoipCfg, 1);
#if 0
	stVoipValue.ch_id = 0;
	stVoipValue.m_id = 0;
	stVoipValue.value = 7;
	SETSOCKOPT(VOIP_MGR_SET_SLIC_TX_GAIN, &stVoipValue, TstVoipValue, 1);
	stVoipValue.value = 7;
	SETSOCKOPT(VOIP_MGR_SET_SLIC_RX_GAIN, &stVoipValue, TstVoipValue, 1);

	stVoipCfg.ch_id = 0;
	stVoipCfg.m_id = 0;
	stVoipCfg.enable = 1;
	SETSOCKOPT(VOIP_MGR_DTMF_CFG, &stVoipCfg, TstVoipCfg, 1);
	SETSOCKOPT(VOIP_MGR_CTRL_TRANSESSION_ID, &stVoipCfg, TstVoipCfg, 1);
#endif
#if 0	
	stVoipMgrSession.ch_id = 0;
	stVoipMgrSession.m_id = 0;
	stVoipMgrSession.ip_src_addr = htonl(inet_addr("172.21.69.43"));
	stVoipMgrSession.ip_dst_addr = htonl(inet_addr("172.21.69.91"));
	stVoipMgrSession.udp_src_port = htons(0x3333);
	stVoipMgrSession.udp_dst_port = htons(0x3333);
	stVoipMgrSession.protocol = 0x11; // udp
	SETSOCKOPT(VOIP_MGR_SET_SESSION, &stVoipMgrSession, TstVoipMgrSession, 1);

	stVoipPayLoadTypeConfig.ch_id = 0;
	stVoipPayLoadTypeConfig.m_id = 0;
	stVoipPayLoadTypeConfig.uPktFormat = rtpPayloadPCMA;
	stVoipPayLoadTypeConfig.nG723Type = 0;
	stVoipPayLoadTypeConfig.nFramePerPacket = 1;
	stVoipPayLoadTypeConfig.bVAD = 0;
	stVoipPayLoadTypeConfig.bPLC = 1;
	SETSOCKOPT(VOIP_MGR_SETRTPPAYLOADTYPE, &stVoipPayLoadTypeConfig, TstVoipPayLoadTypeConfig, 1);

	stVoipCfg.ch_id = 0;
	stVoipCfg.m_id = 0;
	stVoipCfg.enable = 1;
	SETSOCKOPT(VOIP_MGR_RTP_CFG, &stVoipCfg, TstVoipCfg, 1);
#endif

    	cfg.ch_id = 0;
    	cfg.m_id = 0;
    	cfg.nTone = 12;
    	cfg.bFlag = 1;
    	cfg.path = 0;
	SETSOCKOPT(VOIP_MGR_SETPLAYTONE, &cfg, TstVoipPlayToneConfig, 1);

	return 0;
}
