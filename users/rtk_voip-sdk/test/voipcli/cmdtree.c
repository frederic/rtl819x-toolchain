#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>

#include "voip_manager.h"
#include "cmdtree.h"

#define ANAME( sn, fn )	#fn, ( unsigned long )&( ( ( sn * )0 )->fn ), sizeof( ( ( sn * )0 )->fn )

#define NID( x )		x, #x
#define NAT( x )		a ## x, sizeof( x )

/* ================================================================ */
/* Arguments */
/* ================================================================ */


static const args_t aTstVoipFDT_UnsignedChar[] = {
	{ "uchar", 0, sizeof( unsigned char ), ATYPE_IN },
	{ NULL, 0, 0, ATYPE_NONE },
};

static const args_t aTstVoipFDT_Int[] = {
	{ "int", 0, sizeof( int ), ATYPE_IN },
	{ NULL, 0, 0, ATYPE_NONE },
};

static const args_t aTstVoipFDT_2xUint[] = {
	{ "uint32", 0, sizeof( uint32 ), ATYPE_IN },
	{ "uint32", 4, sizeof( uint32 ), ATYPE_IN },
	{ NULL, 0, 0, ATYPE_NONE },
};

static const args_t aTstVoipCfg[] = {
	{ ANAME( TstVoipCfg, ch_id ), ATYPE_IN },
	{ ANAME( TstVoipCfg, m_id ), ATYPE_IN },
	{ ANAME( TstVoipCfg, t_id ), ATYPE_IN },
	{ ANAME( TstVoipCfg, enable ), ATYPE_IN },
	{ ANAME( TstVoipCfg, cfg ), ATYPE_IN },
	{ ANAME( TstVoipCfg, cfg2 ), ATYPE_IN },
	{ ANAME( TstVoipCfg, cfg3 ), ATYPE_IN },
	{ NULL, 0, 0, ATYPE_NONE },
};

static const args_t aTstVoipValue[] = {
	{ ANAME( TstVoipValue, ch_id ), ATYPE_IN },
	{ ANAME( TstVoipValue, m_id ), ATYPE_IN },
	{ ANAME( TstVoipValue, value ), ATYPE_IN },
	{ ANAME( TstVoipValue, value1 ), ATYPE_IN },
	{ ANAME( TstVoipValue, value2 ), ATYPE_IN },
	{ ANAME( TstVoipValue, value3 ), ATYPE_IN },
	{ ANAME( TstVoipValue, value4 ), ATYPE_IN },
	{ ANAME( TstVoipValue, value5 ), ATYPE_IN },
	{ ANAME( TstVoipValue, value6 ), ATYPE_IN },
	{ ANAME( TstVoipValue, value7 ), ATYPE_IN },
	{ NULL, 0, 0, ATYPE_NONE },
};

static const args_t aTstVoipEvent[] = {
	{ ANAME( TstVoipEvent, ch_id ), ATYPE_IN },
	{ ANAME( TstVoipEvent, type ), ATYPE_IN },
	{ ANAME( TstVoipEvent, mask ), ATYPE_IN },
	{ ANAME( TstVoipEvent, id ), ATYPE_OUT },
	{ ANAME( TstVoipEvent, p0 ), ATYPE_OUT },
	{ ANAME( TstVoipEvent, p1 ), ATYPE_OUT },
	{ ANAME( TstVoipEvent, time ), ATYPE_OUT },
	{ NULL, 0, 0, ATYPE_NONE },
};

static const args_t aTstFlushVoipEvent[] = {
	{ ANAME( TstFlushVoipEvent, ch_id ), ATYPE_IN },
	{ ANAME( TstFlushVoipEvent, type ), ATYPE_IN },
	{ ANAME( TstFlushVoipEvent, mask ), ATYPE_IN },
	{ NULL, 0, 0, ATYPE_NONE },
};

static const args_t aTstVoipPayLoadTypeConfig[] = {
	{ ANAME( TstVoipPayLoadTypeConfig, ch_id ), ATYPE_IN },
	{ ANAME( TstVoipPayLoadTypeConfig, m_id ), ATYPE_IN },
	{ ANAME( TstVoipPayLoadTypeConfig, local_pt ), ATYPE_IN },
	{ ANAME( TstVoipPayLoadTypeConfig, remote_pt ), ATYPE_IN },
	{ ANAME( TstVoipPayLoadTypeConfig, uPktFormat ), ATYPE_IN },
	{ ANAME( TstVoipPayLoadTypeConfig, nG723Type ), ATYPE_IN },
	{ ANAME( TstVoipPayLoadTypeConfig, nFramePerPacket ), ATYPE_IN },
	{ ANAME( TstVoipPayLoadTypeConfig, nFramePerPacket_vbd ), ATYPE_IN },
	{ ANAME( TstVoipPayLoadTypeConfig, bVAD ), ATYPE_IN },
	{ ANAME( TstVoipPayLoadTypeConfig, bPLC ), ATYPE_IN },
	{ ANAME( TstVoipPayLoadTypeConfig, result ), ATYPE_IN },
	{ ANAME( TstVoipPayLoadTypeConfig, nJitterDelay ), ATYPE_IN },
	{ ANAME( TstVoipPayLoadTypeConfig, nMaxDelay ), ATYPE_IN },
	{ ANAME( TstVoipPayLoadTypeConfig, nJitterFactor ), ATYPE_IN },
	{ ANAME( TstVoipPayLoadTypeConfig, nG726Packing ), ATYPE_IN },
	{ ANAME( TstVoipPayLoadTypeConfig, bT38ParamEnable ), ATYPE_IN },
	{ ANAME( TstVoipPayLoadTypeConfig, nT38MaxBuffer ), ATYPE_IN },
	{ ANAME( TstVoipPayLoadTypeConfig, nT38RateMgt ), ATYPE_IN },
	{ ANAME( TstVoipPayLoadTypeConfig, nT38MaxRate ), ATYPE_IN },
	{ ANAME( TstVoipPayLoadTypeConfig, bT38EnableECM ), ATYPE_IN },
	{ ANAME( TstVoipPayLoadTypeConfig, nT38ECCSignal ), ATYPE_IN },
	{ ANAME( TstVoipPayLoadTypeConfig, nT38ECCData ), ATYPE_IN },
	{ ANAME( TstVoipPayLoadTypeConfig, bT38EnableSpoof ), ATYPE_IN },
	{ ANAME( TstVoipPayLoadTypeConfig, nT38DuplicateNum ), ATYPE_IN },
	{ ANAME( TstVoipPayLoadTypeConfig, nPcmMode ), ATYPE_IN },
	{ NULL, 0, 0, ATYPE_NONE },
};

static const args_t aTstVoipThresVadCngConfig[] = {
	{ ANAME( TstVoipThresVadCngConfig, ch_id ), ATYPE_IN },
	{ ANAME( TstVoipThresVadCngConfig, m_id ), ATYPE_IN },
	{ ANAME( TstVoipThresVadCngConfig, nThresVAD ), ATYPE_IN },
	{ ANAME( TstVoipThresVadCngConfig, nThresCNG ), ATYPE_IN },
	{ NULL, 0, 0, ATYPE_NONE },
};

static const args_t aTstVoipCID[] = {
	{ ANAME( TstVoipCID, ch_id ), ATYPE_IN },
	{ ANAME( TstVoipCID, daa_id ), ATYPE_IN },
	{ ANAME( TstVoipCID, cid_state ), ATYPE_IN },
	{ ANAME( TstVoipCID, cid_mode ), ATYPE_IN },
	{ ANAME( TstVoipCID, cid_gain ), ATYPE_IN },
	{ ANAME( TstVoipCID, cid_msg_type ), ATYPE_IN },
	{ ANAME( TstVoipCID, string ), ATYPE_IN_STR },
	{ ANAME( TstVoipCID, string2 ), ATYPE_IN_STR },
	{ ANAME( TstVoipCID, cid_name ), ATYPE_IN_STR },
	{ ANAME( TstVoipCID, cid_dtmf_mode ), ATYPE_IN },
	{ NULL, 0, 0, ATYPE_NONE },
};

static const args_t aTstVoipMgrSession[] = {
	{ ANAME( TstVoipMgrSession, ip_src_addr ), ATYPE_IN_IA4 },
	{ ANAME( TstVoipMgrSession, ip_dst_addr ), ATYPE_IN_IA4 },
	{ ANAME( TstVoipMgrSession, udp_src_port ), ATYPE_IN },
	{ ANAME( TstVoipMgrSession, udp_dst_port ), ATYPE_IN },
	{ ANAME( TstVoipMgrSession, protocol ), ATYPE_IN },
	{ ANAME( TstVoipMgrSession, ch_id ), ATYPE_IN },
	{ ANAME( TstVoipMgrSession, m_id ), ATYPE_IN },
	{ ANAME( TstVoipMgrSession, result ), ATYPE_IN },
	{ ANAME( TstVoipMgrSession, tos ), ATYPE_IN },
	{ ANAME( TstVoipMgrSession, rfc2833_dtmf_pt_local ), ATYPE_IN },
	{ ANAME( TstVoipMgrSession, rfc2833_dtmf_pt_remote ), ATYPE_IN },
	{ ANAME( TstVoipMgrSession, rfc2833_fax_modem_pt_local ), ATYPE_IN },
	{ ANAME( TstVoipMgrSession, rfc2833_fax_modem_pt_remote ), ATYPE_IN },
#ifdef CONFIG_RTK_VOIP_SRTP
	{ ANAME( TstVoipMgrSession, remoteSrtpKey ), ATYPE_IN_BIN },
	{ ANAME( TstVoipMgrSession, localSrtpKey ), ATYPE_IN_BIN },
	{ ANAME( TstVoipMgrSession, remoteCryptAlg ), ATYPE_IN },
	{ ANAME( TstVoipMgrSession, localCryptAlg ), ATYPE_IN },
#endif /*CONFIG_RTK_VOIP_SRTP*/
#ifdef SUPPORT_RTP_REDUNDANT
	{ ANAME( TstVoipMgrSession, rtp_redundant_payload_type_local ), ATYPE_IN },
	{ ANAME( TstVoipMgrSession, rtp_redundant_payload_type_remote ), ATYPE_IN },
	{ ANAME( TstVoipMgrSession, rtp_redundant_max_Audio ), ATYPE_IN },
	{ ANAME( TstVoipMgrSession, rtp_redundant_max_RFC2833 ), ATYPE_IN },
#endif
	{ ANAME( TstVoipMgrSession, SID_payload_type_local ), ATYPE_IN },
	{ ANAME( TstVoipMgrSession, SID_payload_type_remote ), ATYPE_IN },
	{ ANAME( TstVoipMgrSession, init_randomly ), ATYPE_IN },
	{ ANAME( TstVoipMgrSession, init_seqno ), ATYPE_IN },
	{ ANAME( TstVoipMgrSession, init_SSRC ), ATYPE_IN },
	{ ANAME( TstVoipMgrSession, init_timestamp ), ATYPE_IN },
	{ NULL, 0, 0, ATYPE_NONE },
};

static const args_t aTstVoipRtpSessionState[] = {
	{ ANAME( TstVoipRtpSessionState, ch_id ), ATYPE_IN },
	{ ANAME( TstVoipRtpSessionState, m_id ), ATYPE_IN },
	{ ANAME( TstVoipRtpSessionState, state ), ATYPE_IN },
	{ NULL, 0, 0, ATYPE_NONE },
};

static const args_t aTstVoipRtpStatistics[] = {
	{ ANAME( TstVoipRtpStatistics, ch_id ), ATYPE_IN },
	{ ANAME( TstVoipRtpStatistics, bResetStatistics ), ATYPE_IN },
	{ ANAME( TstVoipRtpStatistics, nRxRtpStatsCountByte ), ATYPE_OUT },
	{ ANAME( TstVoipRtpStatistics, nRxRtpStatsCountPacket ), ATYPE_OUT },
	{ ANAME( TstVoipRtpStatistics, nRxRtpStatsLostPacket ), ATYPE_OUT },
	{ ANAME( TstVoipRtpStatistics, nTxRtpStatsCountByte ), ATYPE_OUT },
	{ ANAME( TstVoipRtpStatistics, nTxRtpStatsCountPacket ), ATYPE_OUT },
	{ NULL, 0, 0, ATYPE_NONE },
};

static const args_t aTstVoipSessionStatistics[] = {
	{ ANAME( TstVoipSessionStatistics, ch_id ), ATYPE_IN },
	{ ANAME( TstVoipSessionStatistics, m_id ), ATYPE_IN },
	{ ANAME( TstVoipSessionStatistics, bResetStatistics ), ATYPE_IN },
	{ ANAME( TstVoipSessionStatistics, nRxSilencePacket ), ATYPE_OUT },
	{ ANAME( TstVoipSessionStatistics, nTxSilencePacket ), ATYPE_OUT },
	{ ANAME( TstVoipSessionStatistics, nAvgPlayoutDelay ), ATYPE_OUT },
	{ ANAME( TstVoipSessionStatistics, nCurrentJitterBuf ), ATYPE_OUT },
	{ ANAME( TstVoipSessionStatistics, nEarlyPacket ), ATYPE_OUT },
	{ ANAME( TstVoipSessionStatistics, nLatePacket ), ATYPE_OUT },
	{ ANAME( TstVoipSessionStatistics, nSilenceSpeech ), ATYPE_OUT },
	{ NULL, 0, 0, ATYPE_NONE },
};

static const args_t aTstVoipRtcpSession[] = {
	{ ANAME( TstVoipRtcpSession, ip_src_addr ), ATYPE_IN },
	{ ANAME( TstVoipRtcpSession, ip_dst_addr ), ATYPE_IN },
	{ ANAME( TstVoipRtcpSession, rtcp_src_port ), ATYPE_IN },
	{ ANAME( TstVoipRtcpSession, rtcp_dst_port ), ATYPE_IN },
	{ ANAME( TstVoipRtcpSession, protocol ), ATYPE_IN },
	{ ANAME( TstVoipRtcpSession, ch_id ), ATYPE_IN },
	{ ANAME( TstVoipRtcpSession, m_id ), ATYPE_IN },
	{ ANAME( TstVoipRtcpSession, enableXR ), ATYPE_IN },
	{ ANAME( TstVoipRtcpSession, tx_interval ), ATYPE_IN },
	{ NULL, 0, 0, ATYPE_NONE },
};

static const args_t aTstVoipRtcpLogger[] = {
	{ ANAME( TstVoipRtcpLogger, ch_id ), ATYPE_IN },
	{ ANAME( TstVoipRtcpLogger, m_id ), ATYPE_IN },
	{ ANAME( TstVoipRtcpLogger, TX_packet_count ), ATYPE_IN },
	{ ANAME( TstVoipRtcpLogger, TX_loss_rate_max ), ATYPE_IN },
	{ ANAME( TstVoipRtcpLogger, TX_loss_rate_min ), ATYPE_IN },
	{ ANAME( TstVoipRtcpLogger, TX_loss_rate_avg ), ATYPE_IN },
	{ ANAME( TstVoipRtcpLogger, TX_loss_rate_cur ), ATYPE_IN },
	{ ANAME( TstVoipRtcpLogger, TX_jitter_max ), ATYPE_IN },
	{ ANAME( TstVoipRtcpLogger, TX_jitter_min ), ATYPE_IN },
	{ ANAME( TstVoipRtcpLogger, TX_jitter_avg ), ATYPE_IN },
	{ ANAME( TstVoipRtcpLogger, TX_jitter_cur ), ATYPE_IN },
	{ ANAME( TstVoipRtcpLogger, TX_round_trip_max ), ATYPE_IN },
	{ ANAME( TstVoipRtcpLogger, TX_round_trip_min ), ATYPE_IN },
	{ ANAME( TstVoipRtcpLogger, TX_round_trip_avg ), ATYPE_IN },
	{ ANAME( TstVoipRtcpLogger, TX_round_trip_cur ), ATYPE_IN },
	{ ANAME( TstVoipRtcpLogger, TX_MOS_LQ_max ), ATYPE_IN },
	{ ANAME( TstVoipRtcpLogger, TX_MOS_LQ_min ), ATYPE_IN },
	{ ANAME( TstVoipRtcpLogger, TX_MOS_LQ_avg ), ATYPE_IN },
	{ ANAME( TstVoipRtcpLogger, TX_MOS_LQ_cur ), ATYPE_IN },
	{ ANAME( TstVoipRtcpLogger, TX_MOS_LQ_max_x10 ), ATYPE_IN },
	{ ANAME( TstVoipRtcpLogger, TX_MOS_LQ_min_x10 ), ATYPE_IN },
	{ ANAME( TstVoipRtcpLogger, TX_MOS_LQ_avg_x10 ), ATYPE_IN },
	{ ANAME( TstVoipRtcpLogger, TX_MOS_LQ_cur_x10 ), ATYPE_IN },
	{ ANAME( TstVoipRtcpLogger, TX_MOS_LQ_avg_x10 ), ATYPE_IN },
	{ ANAME( TstVoipRtcpLogger, RX_packet_count ), ATYPE_IN },
	{ ANAME( TstVoipRtcpLogger, RX_loss_rate_max ), ATYPE_IN },
	{ ANAME( TstVoipRtcpLogger, RX_loss_rate_min ), ATYPE_IN },
	{ ANAME( TstVoipRtcpLogger, RX_loss_rate_avg ), ATYPE_IN },
	{ ANAME( TstVoipRtcpLogger, RX_loss_rate_cur ), ATYPE_IN },
	{ ANAME( TstVoipRtcpLogger, RX_jitter_max ), ATYPE_IN },
	{ ANAME( TstVoipRtcpLogger, RX_jitter_min ), ATYPE_IN },
	{ ANAME( TstVoipRtcpLogger, RX_jitter_avg ), ATYPE_IN },
	{ ANAME( TstVoipRtcpLogger, RX_jitter_cur ), ATYPE_IN },
	{ ANAME( TstVoipRtcpLogger, RX_round_trip_max ), ATYPE_IN },
	{ ANAME( TstVoipRtcpLogger, RX_round_trip_min ), ATYPE_IN },
	{ ANAME( TstVoipRtcpLogger, RX_round_trip_avg ), ATYPE_IN },
	{ ANAME( TstVoipRtcpLogger, RX_round_trip_cur ), ATYPE_IN },
	{ ANAME( TstVoipRtcpLogger, RX_MOS_LQ_max ), ATYPE_IN },
	{ ANAME( TstVoipRtcpLogger, RX_MOS_LQ_min ), ATYPE_IN },
	{ ANAME( TstVoipRtcpLogger, RX_MOS_LQ_avg ), ATYPE_IN },
	{ ANAME( TstVoipRtcpLogger, RX_MOS_LQ_cur ), ATYPE_IN },
	{ ANAME( TstVoipRtcpLogger, RX_MOS_LQ_avg_x10 ), ATYPE_IN },
	{ NULL, 0, 0, ATYPE_NONE },
};

static const args_t aTstVoip2833[] = {
	{ ANAME( TstVoip2833, ch_id ), ATYPE_IN },
	{ ANAME( TstVoip2833, m_id ), ATYPE_IN },
	{ ANAME( TstVoip2833, digit ), ATYPE_IN },
	{ ANAME( TstVoip2833, duration ), ATYPE_IN },
	{ ANAME( TstVoip2833, bEnable ), ATYPE_IN },
	{ ANAME( TstVoip2833, volume ), ATYPE_IN },
	{ NULL, 0, 0, ATYPE_NONE },
};

static const args_t aTstVoipPlayToneConfig[] = {
	{ ANAME( TstVoipPlayToneConfig, ch_id ), ATYPE_IN },
	{ ANAME( TstVoipPlayToneConfig, m_id ), ATYPE_IN },
	{ ANAME( TstVoipPlayToneConfig, nTone ), ATYPE_IN },
	{ ANAME( TstVoipPlayToneConfig, bFlag ), ATYPE_IN },
	{ ANAME( TstVoipPlayToneConfig, path ), ATYPE_IN },
	{ NULL, 0, 0, ATYPE_NONE },
};

static const args_t aTstVoipToneCfg[] = {
	{ ANAME( TstVoipToneCfg, toneType ), ATYPE_IN },
	{ ANAME( TstVoipToneCfg, cycle ), ATYPE_IN },
	{ ANAME( TstVoipToneCfg, cadNUM ), ATYPE_IN },
	{ ANAME( TstVoipToneCfg, CadOn0 ), ATYPE_IN },
	{ ANAME( TstVoipToneCfg, CadOn1 ), ATYPE_IN },
	{ ANAME( TstVoipToneCfg, CadOn2 ), ATYPE_IN },
	{ ANAME( TstVoipToneCfg, CadOn3 ), ATYPE_IN },
	{ ANAME( TstVoipToneCfg, CadOff0 ), ATYPE_IN },
	{ ANAME( TstVoipToneCfg, CadOff1 ), ATYPE_IN },
	{ ANAME( TstVoipToneCfg, CadOff2 ), ATYPE_IN },
	{ ANAME( TstVoipToneCfg, CadOff3 ), ATYPE_IN },
	{ ANAME( TstVoipToneCfg, PatternOff ), ATYPE_IN },
	{ ANAME( TstVoipToneCfg, ToneNUM ), ATYPE_IN },
	{ ANAME( TstVoipToneCfg, Freq1 ), ATYPE_IN },
	{ ANAME( TstVoipToneCfg, Freq2 ), ATYPE_IN },
	{ ANAME( TstVoipToneCfg, Freq3 ), ATYPE_IN },
	{ ANAME( TstVoipToneCfg, Freq4 ), ATYPE_IN },
	{ ANAME( TstVoipToneCfg, Gain1 ), ATYPE_IN },
	{ ANAME( TstVoipToneCfg, Gain2 ), ATYPE_IN },
	{ ANAME( TstVoipToneCfg, Gain3 ), ATYPE_IN },
	{ ANAME( TstVoipToneCfg, Gain4 ), ATYPE_IN },
	{ NULL, 0, 0, ATYPE_NONE },
};

static const args_t aTstVoipdistonedet_parm[] = {
	{ ANAME( TstVoipdistonedet_parm, distone_num ), ATYPE_IN },
	{ ANAME( TstVoipdistonedet_parm, tone1_accuracy ), ATYPE_IN },
	{ ANAME( TstVoipdistonedet_parm, tone1_level ), ATYPE_IN },
	{ ANAME( TstVoipdistonedet_parm, tone1_frequency1 ), ATYPE_IN },
	{ ANAME( TstVoipdistonedet_parm, tone1_frequency2 ), ATYPE_IN },
	{ ANAME( TstVoipdistonedet_parm, tone1_distone_on_low_limit ), ATYPE_IN },
	{ ANAME( TstVoipdistonedet_parm, tone1_distone_on_up_limit ), ATYPE_IN },
	{ ANAME( TstVoipdistonedet_parm, tone1_distone_off_low_limit ), ATYPE_IN },
	{ ANAME( TstVoipdistonedet_parm, tone1_distone_off_up_limit ), ATYPE_IN },
	{ ANAME( TstVoipdistonedet_parm, tone2_accuracy ), ATYPE_IN },
	{ ANAME( TstVoipdistonedet_parm, tone2_level ), ATYPE_IN },
	{ ANAME( TstVoipdistonedet_parm, tone2_frequency1 ), ATYPE_IN },
	{ ANAME( TstVoipdistonedet_parm, tone2_frequency2 ), ATYPE_IN },
	{ ANAME( TstVoipdistonedet_parm, tone2_distone_on_low_limit ), ATYPE_IN },
	{ ANAME( TstVoipdistonedet_parm, tone2_distone_on_up_limit ), ATYPE_IN },
	{ ANAME( TstVoipdistonedet_parm, tone2_distone_off_low_limit ), ATYPE_IN },
	{ ANAME( TstVoipdistonedet_parm, tone2_distone_off_up_limit ), ATYPE_IN },
	{ NULL, 0, 0, ATYPE_NONE },
};

static const args_t aTstVoipSlicHook[] = {
	{ ANAME( TstVoipSlicHook, ch_id ), ATYPE_IN },
	{ ANAME( TstVoipSlicHook, change ), ATYPE_IN },
	{ ANAME( TstVoipSlicHook, hook_status ), ATYPE_IN },
	{ NULL, 0, 0, ATYPE_NONE },
};

static const args_t aTstVoipSlicRing[] = {
	{ ANAME( TstVoipSlicRing, ch_id ), ATYPE_IN },
	{ ANAME( TstVoipSlicRing, ring_set ), ATYPE_IN },
	{ NULL, 0, 0, ATYPE_NONE },
};

static const args_t aTstVoipSlicTone[] = {
	{ ANAME( TstVoipSlicTone, ch_id ), ATYPE_IN },
	{ ANAME( TstVoipSlicTone, tone2play ), ATYPE_IN },
	{ NULL, 0, 0, ATYPE_NONE },
};

static const args_t aTstVoipSlicRestart[] = {
	{ ANAME( TstVoipSlicRestart, ch_id ), ATYPE_IN },
	{ ANAME( TstVoipSlicRestart, codec_law ), ATYPE_IN },
	{ NULL, 0, 0, ATYPE_NONE },
};

static const args_t aTstVoipSlicReg[] = {
	{ ANAME( TstVoipSlicReg, ch_id ), ATYPE_IN },
	{ ANAME( TstVoipSlicReg, reg_num ), ATYPE_IN },
	{ ANAME( TstVoipSlicReg, reg_len ), ATYPE_IN },
	{ ANAME( TstVoipSlicReg, reg_ary ), ATYPE_IN_BIN },
	{ NULL, 0, 0, ATYPE_NONE },
};

static const args_t aTstVoipHook[] = {
	{ ANAME( TstVoipHook, ch_id ), ATYPE_IN },
	{ ANAME( TstVoipHook, flash_time ), ATYPE_IN },
	{ ANAME( TstVoipHook, flash_time_min ), ATYPE_IN },
	{ ANAME( TstVoipHook, dummy ), ATYPE_IN },
	{ NULL, 0, 0, ATYPE_NONE },
};

static const args_t aTstVoipSlicRam[] = {
	{ ANAME( TstVoipSlicRam, ch_id ), ATYPE_IN },
	{ ANAME( TstVoipSlicRam, reg_num ), ATYPE_IN },
	{ ANAME( TstVoipSlicRam, reg_val ), ATYPE_IN },
	{ NULL, 0, 0, ATYPE_NONE },
};

static const args_t aTstVoipGPIO[] = {
	{ ANAME( TstVoipGPIO, action ), ATYPE_IN },
	{ ANAME( TstVoipGPIO, pid ), ATYPE_IN },
	{ ANAME( TstVoipGPIO, value ), ATYPE_IN },
	{ ANAME( TstVoipGPIO, result ), ATYPE_IN },
	{ NULL, 0, 0, ATYPE_NONE },
};

static const args_t aTstVoipLedDisplay[] = {
	{ ANAME( TstVoipLedDisplay, ch_id ), ATYPE_IN },
	{ ANAME( TstVoipLedDisplay, led_id ), ATYPE_IN },
	{ ANAME( TstVoipLedDisplay, mode ), ATYPE_IN },
	{ NULL, 0, 0, ATYPE_NONE },
};

static const args_t aTstVoipSlicRelay[] = {
	{ ANAME( TstVoipSlicRelay, ch_id ), ATYPE_IN },
	{ ANAME( TstVoipSlicRelay, close ), ATYPE_IN },
	{ NULL, 0, 0, ATYPE_NONE },
};

static const args_t aTstVoipBusDataFormat[] = {
	{ ANAME( TstVoipBusDataFormat, ch_id ), ATYPE_IN },
	{ ANAME( TstVoipBusDataFormat, format ), ATYPE_IN },
	{ NULL, 0, 0, ATYPE_NONE },
};

static const args_t aTstVoipPcmTimeslot[] = {
	{ ANAME( TstVoipPcmTimeslot, ch_id ), ATYPE_IN },
	{ ANAME( TstVoipPcmTimeslot, timeslot1 ), ATYPE_IN },
	{ ANAME( TstVoipPcmTimeslot, timeslot2 ), ATYPE_IN },
	{ NULL, 0, 0, ATYPE_NONE },
};

static const args_t aTstVoipSingleValue[] = {
	{ ANAME( TstVoipSingleValue, value ), ATYPE_IN },
	{ NULL, 0, 0, ATYPE_NONE },
};

static const args_t aTstVoipSwitch[] = {
	{ ANAME( TstVoipSwitch, phy ), ATYPE_IN },
	{ ANAME( TstVoipSwitch, reg ), ATYPE_IN },
	{ ANAME( TstVoipSwitch, value ), ATYPE_IN },
	{ ANAME( TstVoipSwitch, read_write ), ATYPE_IN },
	{ NULL, 0, 0, ATYPE_NONE },
};

static const args_t aTstVoipSwitch_VLAN_tag[] = {
	{ ANAME( TstVoipSwitch_VLAN_tag, enable ), ATYPE_IN },
	{ ANAME( TstVoipSwitch_VLAN_tag, vlanId ), ATYPE_IN },
	{ ANAME( TstVoipSwitch_VLAN_tag, priority ), ATYPE_IN },
	{ ANAME( TstVoipSwitch_VLAN_tag, cfi ), ATYPE_IN },
	{ NULL, 0, 0, ATYPE_NONE },
};

static const args_t aTstVoipSwitch_2_VLAN_tag[] = {
	{ ANAME( TstVoipSwitch_2_VLAN_tag, enable ), ATYPE_IN },
	{ ANAME( TstVoipSwitch_2_VLAN_tag, vlanIdVoice ), ATYPE_IN },
	{ ANAME( TstVoipSwitch_2_VLAN_tag, priorityVoice ), ATYPE_IN },
	{ ANAME( TstVoipSwitch_2_VLAN_tag, cfiVoice ), ATYPE_IN },
	{ ANAME( TstVoipSwitch_2_VLAN_tag, vlanIdData ), ATYPE_IN },
	{ ANAME( TstVoipSwitch_2_VLAN_tag, priorityData ), ATYPE_IN },
	{ ANAME( TstVoipSwitch_2_VLAN_tag, cfiData ), ATYPE_IN },
	{ NULL, 0, 0, ATYPE_NONE },
};

static const args_t aTstVoipSwitch_3_VLAN_tag[] = {
	{ ANAME( TstVoipSwitch_3_VLAN_tag, enable ), ATYPE_IN },
	{ ANAME( TstVoipSwitch_3_VLAN_tag, vlanIdVoice ), ATYPE_IN },
	{ ANAME( TstVoipSwitch_3_VLAN_tag, priorityVoice ), ATYPE_IN },
	{ ANAME( TstVoipSwitch_3_VLAN_tag, cfiVoice ), ATYPE_IN },
	{ ANAME( TstVoipSwitch_3_VLAN_tag, vlanIdData ), ATYPE_IN },
	{ ANAME( TstVoipSwitch_3_VLAN_tag, priorityData ), ATYPE_IN },
	{ ANAME( TstVoipSwitch_3_VLAN_tag, cfiData ), ATYPE_IN },
	{ ANAME( TstVoipSwitch_3_VLAN_tag, vlanIdVideo ), ATYPE_IN },
	{ ANAME( TstVoipSwitch_3_VLAN_tag, priorityVideo ), ATYPE_IN },
	{ ANAME( TstVoipSwitch_3_VLAN_tag, cfiVideo ), ATYPE_IN },
	{ NULL, 0, 0, ATYPE_NONE },
};

static const args_t aTstVoipCloneMAC[] = {
	{ ANAME( TstVoipCloneMAC, CloneMACAddress ), ATYPE_IN_BIN },
	{ NULL, 0, 0, ATYPE_NONE },
};

static const args_t aTstVoipBandwidthMgr[] = {
	{ ANAME( TstVoipBandwidthMgr, port ), ATYPE_IN },
	{ ANAME( TstVoipBandwidthMgr, dir ), ATYPE_IN },
	{ ANAME( TstVoipBandwidthMgr, ban ), ATYPE_IN },
	{ NULL, 0, 0, ATYPE_NONE },
};

static const args_t aTstVoipPortLinkStatus[] = {
	{ ANAME( TstVoipPortLinkStatus, status ), ATYPE_IN },
	{ NULL, 0, 0, ATYPE_NONE },
};

#ifdef CONFIG_RTK_VOIP_IP_PHONE
static const args_t aTstVoicePath_t[] = {
	{ ANAME( TstVoicePath_t, vpath ), ATYPE_IN },
	{ NULL, 0, 0, ATYPE_NONE },
};

static const args_t aTstLedCtl[] = {
	{ ANAME( TstLedCtl, led ), ATYPE_IN },
	{ NULL, 0, 0, ATYPE_NONE },
};

static const args_t aTstMiscCtl_t[] = {
	{ ANAME( TstMiscCtl_t, buildno ), ATYPE_OUT },
	{ ANAME( TstMiscCtl_t, builddate ), ATYPE_OUT },
	{ NULL, 0, 0, ATYPE_NONE },
};
#endif

static const args_t aTstDtmfDetPara[] = {
	{ ANAME( TstDtmfDetPara, ch_id ), ATYPE_IN },
	{ ANAME( TstDtmfDetPara, m_id ), ATYPE_IN },
	{ ANAME( TstDtmfDetPara, t_id ), ATYPE_IN },
	{ ANAME( TstDtmfDetPara, enable ), ATYPE_IN },
	{ ANAME( TstDtmfDetPara, dir ), ATYPE_IN },
	{ ANAME( TstDtmfDetPara, thres ), ATYPE_IN },
	{ ANAME( TstDtmfDetPara, on_time ), ATYPE_IN },
	{ ANAME( TstDtmfDetPara, fore_twist ), ATYPE_IN },
	{ ANAME( TstDtmfDetPara, rev_twist ), ATYPE_IN },
	{ NULL, 0, 0, ATYPE_NONE },
};

typedef union {
	TstVoipCfg stVoipCfg;
	TstVoipValue stVoipValue;
	TstVoipPayLoadTypeConfig stVoipPayLoadTypeConfig;
	TstVoipCID stVoipCID;
	TstVoipMgrSession stVoipMgrSession;
	TstVoipRtpSessionState stVoipRtpSessionState;
	TstVoipRtpStatistics stVoipRtpStatistics;
	TstVoip2833 stVoip2833;
	TstVoipPlayToneConfig stVoipPlayToneConfig;
	TstVoipToneCfg stVoipToneCfg;
	TstVoipdistonedet_parm stVoipdistonedet_parm;
	TstVoipSlicHook stVoipSlicHook;
	TstVoipSlicRing stVoipSlicRing;
	TstVoipSlicTone stVoipSlicTone;
	TstVoipSlicRestart stVoipSlicRestart;
	TstVoipSlicReg stVoipSlicReg;
	TstVoipHook stVoipHook;
	TstVoipSlicRam stVoipSlicRam;
	TstVoipSingleValue stVoipSingleValue;
	TstVoipSwitch stVoipSwitch;
	TstVoipSwitch_VLAN_tag stVoipSwitch_VLAN_tag;
	TstVoipSwitch_2_VLAN_tag stVoipSwitch_2_VLAN_tag;
	TstVoipCloneMAC stVoipCloneMAC;
	TstVoipBandwidthMgr stVoipBandwidthMgr;
	TstVoipPortLinkStatus stVoipPortLinkStatus;
#ifdef CONFIG_RTK_VOIP_IP_PHONE
	TstVoicePath_t stVoicePath;
	TstLedCtl stLedCtl;
	TstMiscCtl_t stMiscCtl;
#endif
} argsu_t;

unsigned long aBufferSize = sizeof( argsu_t );
unsigned char aBuffer[ sizeof( argsu_t ) ];

/* ================================================================ */
/* Internal node */
/* ================================================================ */


static const node_t nProtocolRTP[] = {
	{ NID( VOIP_MGR_SET_SESSION ), NTYPE_LEAF, NULL, NAT( TstVoipMgrSession ) },
	{ NID( VOIP_MGR_UNSET_SESSION ), NTYPE_LEAF, NULL, NAT( TstVoipCfg ) },
	{ NID( VOIP_MGR_SETRTPSESSIONSTATE ), NTYPE_LEAF, NULL, NAT( TstVoipRtpSessionState ) },
	{ NID( VOIP_MGR_RTP_CFG ), NTYPE_LEAF, NULL, NAT( TstVoipCfg ) },
	{ NID( VOIP_MGR_HOLD ), NTYPE_LEAF, NULL, NAT( TstVoipCfg ) },
	{ NID( VOIP_MGR_CTRL_RTPSESSION ), NTYPE_LEAF, NULL, NAT( TstVoipCfg ) },
	{ NID( VOIP_MGR_CTRL_TRANSESSION_ID ), NTYPE_LEAF, NULL, NAT( TstVoipCfg ) },
	//{ NID( VOIP_MGR_SETCONFERENCE ), NTYPE_LEAF, NULL, NAT( TstVoipMgr3WayCfg ) },
	{ NID( VOIP_MGR_GET_RTP_STATISTICS ), NTYPE_LEAF, NULL, NAT( TstVoipRtpStatistics ) },
	{ NID( VOIP_MGR_GET_SESSION_STATISTICS ), NTYPE_LEAF, NULL, NAT( TstVoipSessionStatistics ) },
	{ NID( 0 ), NTYPE_NONE, NULL, NULL, 0 },
};

static const node_t nProtocolRTCP[] = {
	{ NID( VOIP_MGR_SET_RTCP_SESSION ), NTYPE_LEAF, NULL, NAT( TstVoipCfg ) },
	{ NID( VOIP_MGR_UNSET_RTCP_SESSION ), NTYPE_LEAF, NULL, NAT( TstVoipRtcpSession ) },
	//{ NID( VOIP_MGR_SET_RTCP_TX_INTERVAL ), NTYPE_LEAF, NULL, NAT( TstVoipValue ) },
	{ NID( VOIP_MGR_GET_RTCP_LOGGER ), NTYPE_LEAF, NULL, NAT( TstVoipRtcpLogger ) },
	{ NID( 0 ), NTYPE_NONE, NULL, NULL, 0 },
};

static const node_t nDspGeneral[] = {
	{ NID( VOIP_MGR_ON_HOOK_RE_INIT ), NTYPE_LEAF, NULL, NAT( TstVoipCfg ) },
	{ NID( VOIP_MGR_SET_VOICE_GAIN ), NTYPE_LEAF, NULL, NAT( TstVoipValue ) },
	{ NID( VOIP_MGR_ENERGY_DETECT ), NTYPE_LEAF, NULL, NAT( TstVoipValue ) },
	{ NID( VOIP_MGR_GET_VOIP_EVENT ), NTYPE_LEAF, NULL, NAT( TstVoipEvent ) },
	{ NID( VOIP_MGR_FLUSH_VOIP_EVENT ), NTYPE_LEAF, NULL, NAT( TstFlushVoipEvent ) },
	{ NID( 0 ), NTYPE_NONE, NULL, NULL, 0 },
};

static const node_t nDspCodec[] = {
	{ NID( VOIP_MGR_SETRTPPAYLOADTYPE ), NTYPE_LEAF, NULL, NAT( TstVoipPayLoadTypeConfig ) },
	{ NID( VOIP_MGR_DSPCODECSTOP ), NTYPE_LEAF, NULL, NAT( TstVoipValue ) },
	{ NID( VOIP_MGR_SET_VAD_CNG_THRESHOLD ), NTYPE_LEAF, NULL, NAT( TstVoipThresVadCngConfig ) },
	{ NID( 0 ), NTYPE_NONE, NULL, NULL, 0 },
};

static const node_t nDspFaxModem[] = {
	{ NID( VOIP_MGR_FAX_OFFHOOK ), NTYPE_LEAF, NULL, NAT( TstVoipCfg ) },
	{ NID( VOIP_MGR_FAX_END_DETECT ), NTYPE_LEAF, NULL, NAT( TstVoipCfg ) },
	{ NID( VOIP_MGR_SET_FAX_MODEM_DET ), NTYPE_LEAF, NULL, NAT( TstVoipCfg ) },
	{ NID( VOIP_MGR_SET_FAX_SILENCE_DET ), NTYPE_LEAF, NULL, NAT( TstVoipCfg ) },
	{ NID( 0 ), NTYPE_NONE, NULL, NULL, 0 },
};

static const node_t nDspLEC[] = {
	{ NID( VOIP_MGR_SET_ECHO_TAIL_LENGTH ), NTYPE_LEAF, NULL, NAT( TstVoipValue ) },
	{ NID( VOIP_MGR_SET_G168_LEC_CFG ), NTYPE_LEAF, NULL, NAT( TstVoipCfg ) },
	{ NID( VOIP_MGR_SET_VBD_EC ), NTYPE_LEAF, NULL, NAT( TstVoipCfg ) },
	{ NID( 0 ), NTYPE_NONE, NULL, NULL, 0 },
};
static const node_t nDspDTMF[] = {
	{ NID( VOIP_MGR_DTMF_CFG ), NTYPE_LEAF, NULL, NAT( TstVoipCfg ) },
	{ NID( VOIP_MGR_SET_DTMF_MODE ), NTYPE_LEAF, NULL, NAT( TstVoipCfg ) },
	{ NID( VOIP_MGR_SEND_RFC2833_PKT_CFG ), NTYPE_LEAF, NULL, NAT( TstVoip2833 ) },
	{ NID( VOIP_MGR_SEND_RFC2833_BY_AP ), NTYPE_LEAF, NULL, NAT( TstVoipCfg ) },
	{ NID( VOIP_MGR_LIMIT_MAX_RFC2833_DTMF_DURATION ), NTYPE_LEAF, NULL, NAT( TstVoip2833 ) },
	{ NID( VOIP_MGR_SET_RFC2833_TX_VOLUME ), NTYPE_LEAF, NULL, NAT( TstVoip2833 ) },
	{ NID( VOIP_MGR_FAX_MODEM_RFC2833_CFG ), NTYPE_LEAF, NULL, NAT( TstVoipCfg ) },
	{ NID( VOIP_MGR_RFC2833_PKT_INTERVAL_CFG ), NTYPE_LEAF, NULL, NAT( TstVoipCfg ) },
	{ NID( VOIP_MGR_DTMF_DET_PARAM ), NTYPE_LEAF, NULL, NAT( TstDtmfDetPara ) },
	{ NID( VOIP_MGR_PLAY_SIP_INFO ), NTYPE_LEAF, NULL, NAT( TstVoipValue ) },
	{ NID( 0 ), NTYPE_NONE, NULL, NULL, 0 },
};

static const node_t nDspCallerID[] = {
	{ NID( VOIP_MGR_DTMF_CID_GEN_CFG ), NTYPE_LEAF, NULL, NAT( TstVoipCID ) },
	{ NID( VOIP_MGR_GET_CID_STATE_CFG ), NTYPE_LEAF, NULL, NAT( TstVoipCID ) },
	{ NID( VOIP_MGR_FSK_CID_GEN_CFG ), NTYPE_LEAF, NULL, NAT( TstVoipCID ) },
	{ NID( VOIP_MGR_SET_FSK_VMWI_STATE ), NTYPE_LEAF, NULL, NAT( TstVoipCID ) },
	{ NID( VOIP_MGR_SET_FSK_AREA ), NTYPE_LEAF, NULL, NAT( TstVoipCID ) },
	{ NID( VOIP_MGR_FSK_ALERT_GEN_CFG ), NTYPE_LEAF, NULL, NAT( TstVoipCID ) },
	{ NID( VOIP_MGR_SET_CID_DTMF_MODE ), NTYPE_LEAF, NULL, NAT( TstVoipCID ) },
	{ NID( VOIP_MGR_SET_CID_DET_MODE ), NTYPE_LEAF, NULL, NAT( TstVoipCfg ) },
	{ NID( VOIP_MGR_GET_FSK_CID_STATE_CFG ), NTYPE_LEAF, NULL, NAT( TstVoipCID ) },
	{ NID( VOIP_MGR_SET_CID_FSK_GEN_MODE ), NTYPE_LEAF, NULL, NAT( TstVoipCfg ) },
	{ NID( VOIP_MGR_FSK_CID_VMWI_GEN_CFG ), NTYPE_LEAF, NULL, NAT( TstVoipCID ) },
	{ NID( VOIP_MGR_STOP_CID ), NTYPE_LEAF, NULL, NAT( TstVoipCfg ) },
	{ NID( 0 ), NTYPE_NONE, NULL, NULL, 0 },
};

static const node_t nDspTone[] = {
	{ NID( VOIP_MGR_SETPLAYTONE ), NTYPE_LEAF, NULL, NAT( TstVoipPlayToneConfig ) },
	{ NID( VOIP_MGR_SET_COUNTRY ), NTYPE_LEAF, NULL, NAT( TstVoipValue ) },
	{ NID( VOIP_MGR_SET_TONE_OF_CUSTOMIZE ), NTYPE_LEAF, NULL, NAT( TstVoipValue ) },
	{ NID( VOIP_MGR_SET_CUST_TONE_PARAM ), NTYPE_LEAF, NULL, NAT( TstVoipToneCfg ) },
	{ NID( VOIP_MGR_USE_CUST_TONE ), NTYPE_LEAF, NULL, NAT( TstVoipValue ) },
	{ NID( VOIP_MGR_SET_DIS_TONE_DET ), NTYPE_LEAF, NULL, NAT( TstVoipdistonedet_parm ) },
	{ NID( 0 ), NTYPE_NONE, NULL, NULL, 0 },
};

static const node_t nDspAGC[] = {
	{ NID( VOIP_MGR_SET_SPK_AGC ), NTYPE_LEAF, NULL, NAT( TstVoipValue ) },
	{ NID( VOIP_MGR_SET_SPK_AGC_LVL ), NTYPE_LEAF, NULL, NAT( TstVoipValue ) },
	{ NID( VOIP_MGR_SET_SPK_AGC_GUP ), NTYPE_LEAF, NULL, NAT( TstVoipValue ) },
	{ NID( VOIP_MGR_SET_SPK_AGC_GDOWN ), NTYPE_LEAF, NULL, NAT( TstVoipValue ) },
	{ NID( VOIP_MGR_SET_MIC_AGC ), NTYPE_LEAF, NULL, NAT( TstVoipValue ) },
	{ NID( VOIP_MGR_SET_MIC_AGC_LVL ), NTYPE_LEAF, NULL, NAT( TstVoipValue ) },
	{ NID( VOIP_MGR_SET_MIC_AGC_GUP ), NTYPE_LEAF, NULL, NAT( TstVoipValue ) },
	{ NID( VOIP_MGR_SET_MIC_AGC_GDOWN ), NTYPE_LEAF, NULL, NAT( TstVoipValue ) },
	{ NID( 0 ), NTYPE_NONE, NULL, NULL, 0 },
};

static const node_t nDspPluseDial[] = {
	{ NID( VOIP_MGR_SET_PULSE_DIGIT_DET ), NTYPE_LEAF, NULL, NAT( TstVoipCfg ) },
	{ NID( VOIP_MGR_SET_DIAL_MODE ), NTYPE_LEAF, NULL, NAT( TstVoipCfg ) },
	{ NID( VOIP_MGR_GET_DIAL_MODE ), NTYPE_LEAF, NULL, NAT( TstVoipCfg ) },
	{ NID( VOIP_MGR_PULSE_DIAL_GEN_CFG ), NTYPE_LEAF, NULL, NAT( TstVoipValue ) },
	{ NID( VOIP_MGR_GEN_PULSE_DIAL ), NTYPE_LEAF, NULL, NAT( TstVoipValue ) },
	{ NID( 0 ), NTYPE_NONE, NULL, NULL, 0 },
};

static const node_t nDriverPCM[] = {
	{ NID( VOIP_MGR_PCM_CFG ), NTYPE_LEAF, NULL, NAT( TstVoipCfg ) },
	{ NID( VOIP_MGR_SET_BUS_DATA_FORMAT ), NTYPE_LEAF, NULL, NAT( TstVoipBusDataFormat ) },
	{ NID( VOIP_MGR_SET_PCM_TIMESLOT ), NTYPE_LEAF, NULL, NAT( TstVoipPcmTimeslot ) },
	{ NID( VOIP_MGR_SET_PCM_LOOP_MODE ), NTYPE_LEAF, NULL, NAT( TstVoipValue ) },
	{ NID( 0 ), NTYPE_NONE, NULL, NULL, 0 },
};

static const node_t nDriverSLIC[] = {
	{ NID( VOIP_MGR_SLIC_RING ), NTYPE_LEAF, NULL, NAT( TstVoipSlicRing ) },
	{ NID( VOIP_MGR_SLIC_TONE ), NTYPE_LEAF, NULL, NAT( TstVoipSlicTone ) },
	{ NID( VOIP_MGR_SLIC_RESTART ), NTYPE_LEAF, NULL, NAT( TstVoipSlicRestart ) },
	{ NID( VOIP_MGR_GET_SLIC_REG_VAL ), NTYPE_LEAF, NULL, NAT( TstVoipSlicReg ) },
	{ NID( VOIP_MGR_SET_SLIC_TX_GAIN ), NTYPE_LEAF, NULL, NAT( TstVoipValue ) },
	{ NID( VOIP_MGR_SET_SLIC_RX_GAIN ), NTYPE_LEAF, NULL, NAT( TstVoipValue ) },
	{ NID( VOIP_MGR_SET_FLASH_HOOK_TIME ), NTYPE_LEAF, NULL, NAT( TstVoipHook ) },
	{ NID( VOIP_MGR_SET_SLIC_RING_CADENCE ), NTYPE_LEAF, NULL, NAT( TstVoipValue ) },
	{ NID( VOIP_MGR_SET_SLIC_REG_VAL ), NTYPE_LEAF, NULL, NAT( TstVoipSlicReg ) },
	{ NID( VOIP_MGR_GET_SLIC_STAT ), NTYPE_LEAF, NULL, NAT( TstVoipCfg ) },
	{ NID( VOIP_MGR_SLIC_ONHOOK_ACTION ), NTYPE_LEAF, NULL, NAT( TstVoipCfg ) },
	{ NID( VOIP_MGR_SLIC_OFFHOOK_ACTION ), NTYPE_LEAF, NULL, NAT( TstVoipCfg ) },
	{ NID( VOIP_MGR_LINE_CHECK ), NTYPE_LEAF, NULL, NAT( TstVoipValue ) },
	{ NID( VOIP_MGR_HOOK_FIFO_IN ), NTYPE_LEAF, NULL, NAT( TstVoipCfg ) },
	{ NID( VOIP_MGR_GET_SLIC_RAM_VAL ), NTYPE_LEAF, NULL, NAT( TstVoipSlicRam ) },
	{ NID( VOIP_MGR_SET_SLIC_RAM_VAL ), NTYPE_LEAF, NULL, NAT( TstVoipSlicRam ) },
	{ NID( VOIP_MGR_SET_RING_DETECTION ), NTYPE_LEAF, NULL, NAT( TstVoipValue ) },
	{ NID( VOIP_MGR_SET_FXS_FXO_LOOPBACK ), NTYPE_LEAF, NULL, NAT( TstVoipCfg ) },
	{ NID( VOIP_MGR_SET_SLIC_LINE_VOLTAGE ), NTYPE_LEAF, NULL, NAT( TstVoipValue ) },
	{ NID( VOIP_MGR_GEN_SLIC_CPC ), NTYPE_LEAF, NULL, NAT( TstVoipCfg ) },
	{ NID( 0 ), NTYPE_NONE, NULL, NULL, 0 },
};

static const node_t nDriverDAA[] = {
	{ NID( VOIP_MGR_DAA_RING ), NTYPE_LEAF, NULL, NAT( TstVoipCfg ) },
	{ NID( VOIP_MGR_DAA_OFF_HOOK ), NTYPE_LEAF, NULL, NAT( TstVoipCfg ) },
	{ NID( VOIP_MGR_DAA_ON_HOOK ), NTYPE_LEAF, NULL, NAT( TstVoipCfg ) },
	{ NID( VOIP_MGR_SET_DAA_TX_GAIN ), NTYPE_LEAF, NULL, NAT( TstVoipValue ) },
	{ NID( VOIP_MGR_SET_DAA_RX_GAIN ), NTYPE_LEAF, NULL, NAT( TstVoipValue ) },
	{ NID( VOIP_MGR_SET_DAA_ISR_FLOW ), NTYPE_LEAF, NULL, NAT( TstVoipValue ) },
	{ NID( VOIP_MGR_GET_DAA_ISR_FLOW ), NTYPE_LEAF, NULL, NAT( TstVoipValue ) },
	{ NID( VOIP_MGR_SET_DAA_PCM_HOLD_CFG ), NTYPE_LEAF, NULL, NAT( TstVoipValue ) },
	{ NID( VOIP_MGR_GET_DAA_BUSY_TONE_STATUS ), NTYPE_LEAF, NULL, NAT( TstVoipValue ) },
	{ NID( VOIP_MGR_GET_DAA_CALLER_ID ), NTYPE_LEAF, NULL, NAT( TstVoipCID ) },
	{ NID( VOIP_MGR_GET_DAA_USED_BY_WHICH_SLIC ), NTYPE_LEAF, NULL, NAT( TstVoipValue ) },
	{ NID( VOIP_MGR_FXO_ON_HOOK ), NTYPE_LEAF, NULL, NAT( TstVoipCfg ) },
	{ NID( VOIP_MGR_FXO_OFF_HOOK ), NTYPE_LEAF, NULL, NAT( TstVoipCfg ) },
	{ NID( 0 ), NTYPE_NONE, NULL, NULL, 0 },
};

static const node_t nDriverGPIO[] = {
	{ NID( VOIP_MGR_GPIO ), NTYPE_LEAF, NULL, NAT( TstVoipGPIO ) },
	{ NID( VOIP_MGR_SET_LED_DISPLAY ), NTYPE_LEAF, NULL, NAT( TstVoipLedDisplay ) },
	{ NID( VOIP_MGR_SET_SLIC_RELAY ), NTYPE_LEAF, NULL, NAT( TstVoipSlicRelay ) },
	{ NID( 0 ), NTYPE_NONE, NULL, NULL, 0 },
};

static const node_t nDriverDECT[] = {
	{ NID( VOIP_MGR_DECT_SET_POWER ), NTYPE_LEAF, NULL, NAT( TstVoipSingleValue ) },
	{ NID( VOIP_MGR_DECT_GET_POWER ), NTYPE_LEAF, NULL, NAT( TstVoipSingleValue ) },
	{ NID( VOIP_MGR_DECT_GET_PAGE ), NTYPE_LEAF, NULL, NAT( TstVoipSingleValue ) },
	{ NID( VOIP_MGR_DECT_SET_LED ), NTYPE_LEAF, NULL, NAT( TstVoipValue ) },
	{ NID( 0 ), NTYPE_NONE, NULL, NULL, 0 },
};

static const node_t nDriverNetwork[] = {
	{ NID( VOIP_MGR_8305_SWITCH_VAL ), NTYPE_LEAF, NULL, NAT( TstVoipSwitch ) },
	{ NID( VOIP_MGR_WAN_VLAN_TAG ), NTYPE_LEAF, NULL, NAT( TstVoipSwitch_VLAN_tag ) },
	{ NID( VOIP_MGR_BRIDGE_MODE ), NTYPE_LEAF, NULL, aTstVoipFDT_UnsignedChar, sizeof( unsigned char ) },
	{ NID( VOIP_MGR_SET_DSCP_PRIORITY ), NTYPE_LEAF, NULL, aTstVoipFDT_Int, sizeof( int ) },
	{ NID( VOIP_MGR_WAN_2_VLAN_TAG ), NTYPE_LEAF, NULL, NAT( TstVoipSwitch_2_VLAN_tag ) },
	{ NID( VOIP_MGR_WAN_3_VLAN_TAG ), NTYPE_LEAF, NULL, NAT( TstVoipSwitch_3_VLAN_tag ) },
	{ NID( VOIP_MGR_SET_WAN_CLONE_MAC ), NTYPE_LEAF, NULL, NAT( TstVoipCloneMAC ) },
	{ NID( VOIP_MGR_BANDWIDTH_MGR ), NTYPE_LEAF, NULL, NAT( TstVoipBandwidthMgr ) },
	{ NID( VOIP_MGR_GET_PORT_LINK_STATUS ), NTYPE_LEAF, NULL, NAT( TstVoipPortLinkStatus ) },
	{ NID( VOIP_MGR_SET_RTP_TOS ), NTYPE_LEAF, NULL, aTstVoipFDT_Int, sizeof( int ) },
	{ NID( 0 ), NTYPE_NONE, NULL, NULL, 0 },
};

static const node_t nMisc[] = {
	{ NID( VOIP_MGR_SIP_REGISTER ), NTYPE_LEAF, NULL, NAT( TstVoipCfg ) },
	{ NID( VOIP_MGR_GET_FEATURE ), NTYPE_LEAF, NULL, aTstVoipFDT_2xUint, 2 * sizeof( uint32 ) },
	{ NID( VOIP_MGR_VOIP_RESOURCE_CHECK ), NTYPE_LEAF, NULL, NAT( TstVoipCfg ) },
	{ NID( VOIP_MGR_SET_FW_UPDATE ), NTYPE_LEAF, NULL, aTstVoipFDT_Int, sizeof( int ) },
	{ NID( 0 ), NTYPE_NONE, NULL, NULL, 0 },
};

#ifdef CONFIG_RTK_VOIP_IP_PHONE
static const node_t nIPPhone[] = {
	{ NID( VOIP_MGR_CTL_VOICE_PATH ), NTYPE_LEAF, NULL, NAT( TstVoicePath_t ) },
	{ NID( VOIP_MGR_CTL_LED ), NTYPE_LEAF, NULL, NAT( TstLedCtl ) },
	{ NID( VOIP_MGR_CTL_MISC ), NTYPE_LEAF, NULL, NAT( TstMiscCtl_t ) },
	{ NID( 0 ), NTYPE_NONE, NULL, NULL, 0 },
};
#endif

enum {		/* enum for dummy group ID */
	DUMMY_GROUP_ID = 1,
	VOIP_PROTOCOL_RTP,
	VOIP_PROTOCOL_RTCP,
	VOIP_DSP_GENERAL,
	VOIP_DSP_CODEC,
	VOIP_DSP_FAXMODEM,
	VOIP_DSP_LEC,
	VOIP_DSP_DTMF,
	VOIP_DSP_CALLERID,
	VOIP_DSP_TONE,
	VOIP_DSP_AGC,
	VOIP_DSP_PLUSEDIAL,
	VOIP_DRIVER_PCM,
	VOIP_DRIVER_SLIC,
	VOIP_DRIVER_DAA,
	VOIP_DRIVER_GPIO,
	VOIP_DRIVER_DECT,
	VOIP_DRIVER_NETWORK,
	VOIP_MISC,
#ifdef CONFIG_RTK_VOIP_IP_PHONE
	VOIP_IP_PHONE,
#endif
};

const node_t nRoot[] = {
	{ NID( VOIP_PROTOCOL_RTP ), NTYPE_INODE, nProtocolRTP, NULL, 0 },
	{ NID( VOIP_PROTOCOL_RTCP ), NTYPE_INODE, nProtocolRTCP, NULL, 0 },
	{ NID( VOIP_DSP_GENERAL ), NTYPE_INODE, nDspGeneral, NULL, 0 },
	{ NID( VOIP_DSP_CODEC ), NTYPE_INODE, nDspCodec, NULL, 0 },
	{ NID( VOIP_DSP_FAXMODEM ), NTYPE_INODE, nDspFaxModem, NULL, 0 },
	{ NID( VOIP_DSP_LEC ), NTYPE_INODE, nDspLEC, NULL, 0 },
	{ NID( VOIP_DSP_DTMF ), NTYPE_INODE, nDspDTMF, NULL, 0 },
	{ NID( VOIP_DSP_CALLERID ), NTYPE_INODE, nDspCallerID, NULL, 0 },
	{ NID( VOIP_DSP_TONE ), NTYPE_INODE, nDspTone, NULL, 0 },
	{ NID( VOIP_DSP_AGC ), NTYPE_INODE, nDspAGC, NULL, 0 },
	{ NID( VOIP_DSP_PLUSEDIAL ), NTYPE_INODE, nDspPluseDial, NULL, 0 },
	{ NID( VOIP_DRIVER_PCM ), NTYPE_INODE, nDriverPCM, NULL, 0 },
	{ NID( VOIP_DRIVER_SLIC ), NTYPE_INODE, nDriverSLIC, NULL, 0 },
	{ NID( VOIP_DRIVER_DAA ), NTYPE_INODE, nDriverDAA, NULL, 0 },
	{ NID( VOIP_DRIVER_GPIO ), NTYPE_INODE, nDriverGPIO, NULL, 0 },
	{ NID( VOIP_DRIVER_DECT ), NTYPE_INODE, nDriverDECT, NULL, 0 },
	{ NID( VOIP_DRIVER_NETWORK ), NTYPE_INODE, nDriverNetwork, NULL, 0 },
	{ NID( VOIP_MISC ), NTYPE_INODE, nMisc, NULL, 0 },
#ifdef CONFIG_RTK_VOIP_IP_PHONE
	{ NID( VOIP_IP_PHONE ), NTYPE_INODE, nIPPhone, NULL, 0 },
#endif
	{ NID( 0 ), NTYPE_NONE, NULL, NULL, 0 },
};


/* ================================================================ */
/* IOCTL */
/* ================================================================ */

#include "voip_ioctl.h"

int CmdNodeExecute( const node_t *pNode )
{
	if( pNode ->type == NTYPE_LEAF )
		;
	else
		return -3;
	
	SETSOCKOPT( pNode ->id, aBuffer, pNode ->asize, 1 );
	
	return 0;
}

