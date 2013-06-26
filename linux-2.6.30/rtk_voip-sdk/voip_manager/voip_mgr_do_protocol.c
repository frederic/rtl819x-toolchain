#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <asm/uaccess.h>
#include <net/sock.h>

#define TYPEDEF_H //ALIGN(x) is the same name of kernel

#include "rtk_voip.h"
#include "voip_types.h"
#include "voip_control.h"
#include "voip_params.h"
#include "voip_mgr_define.h"
#include "voip_mgr_netfilter.h"
#include "voip_mgr_do_protocol.h"
#include "../voip_rx/rtk_trap.h"
#ifdef T38_STAND_ALONE_HANDLER
#include "../voip_drivers/t38_handler.h"
#endif /* T38_STAND_ALONE_HANDLER */


#if ! defined (AUDIOCODES_VOIP)

#include "dsp_main.h"
#include "silence_det.h"

#else

#include "RTK_AC49xApi_Interface.h"

#endif


// variables 
#if ! defined (AUDIOCODES_VOIP)
#ifdef PCM_LOOP_MODE_CONTROL
TstVoipLoopBcakInfo LoopBackInfo[DSP_RTK_SS_NUM]={0};
int loop_3way[DSP_RTK_CH_NUM]={0};
#endif
#endif

#if ! defined (AUDIOCODES_VOIP) && defined( VOIP_RESOURCE_CHECK )
extern int resource_weight[DSP_RTK_SS_NUM];// = {0};
#endif
//TstTwoChannelCfg astTwoChannelCfg[VOIP_CH_NUM];	// pkshih: unused now

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
#include "con_mux.h"
#endif

#ifdef CONFIG_RTK_VOIP_IPC_ARCH
#include "ipc_arch_tx.h"
#endif


#if ! defined (AUDIOCODES_VOIP)

extern long talk_flag[];
extern unsigned char rfc2833_dtmf_pt_local[];
extern unsigned char rfc2833_dtmf_pt_remote[];
extern unsigned char rfc2833_fax_modem_pt_local[];
extern unsigned char rfc2833_fax_modem_pt_remote[];
extern struct RTK_TRAP_profile *filter[];
extern unsigned int CurrentDtmfMode[];

extern unsigned char RtcpOpen[];
extern unsigned char RtpOpen[];

extern int g_SIP_Info_play[];		/* 0: stop 1: start play */
extern int g_SIP_Info_tone_buf[][10];
extern int g_SIP_Info_time_buf[][10];
extern int g_SIP_Info_buf_w[];
extern int g_SIP_Info_buf_r[];

extern int DSP_init_done ;

#else


char dtmf_mode[MAX_DSP_AC_CH_NUM] = {0};
static int IsMode3[MAX_DSP_AC_CH_NUM] = {0};
extern struct RTK_TRAP_profile *filter[];


extern int32 Ac49xTxPacketCB( uint32 channel, uint32 mid, void* packet, uint32 pktLen, uint32 flags );


#endif	/* !AUDIOCODES_VOIP */

extern unsigned char rfc2833_payload_type_local[];
extern unsigned char rfc2833_payload_type_remote[];

extern uint32 rtpConfigOK[];
extern uint32 rtpHold[];


//static int ret;


#ifdef AUDIOCODES_VOIP
/************************************************************************/
/*									*/
/*  voip_mgr_set_rtp_1: This function register RTP session              */
/*									*/
/************************************************************************/

int voip_mgr_set_rtp_1(TstVoipMgrRtpCfg stVoipMgrRtpCfg)
{
	//int ret = ( int )NULL;
	uint32 s_id_rtp, s_id_rtcp;
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8651
	int32 result;
	voip_rtp_port_t rtp_pair;
#endif
	unsigned long flags;
	TstVoipMgrSession stVoipMgrSession;

	stVoipMgrSession.ch_id = stVoipMgrRtpCfg.ch_id;
	stVoipMgrSession.m_id = stVoipMgrRtpCfg.m_id;
	stVoipMgrSession.ip_src_addr = stVoipMgrRtpCfg.ip_src_addr;
	stVoipMgrSession.ip_dst_addr = stVoipMgrRtpCfg.ip_dst_addr;
	stVoipMgrSession.udp_src_port = stVoipMgrRtpCfg.udp_src_port;
	stVoipMgrSession.udp_dst_port = stVoipMgrRtpCfg.udp_dst_port;
	stVoipMgrSession.protocol = stVoipMgrRtpCfg.protocol;
	stVoipMgrSession.tos = stVoipMgrRtpCfg.tos;

	stVoipMgrSession.protocol = UDP_PROTOCOL;

	s_id_rtp = (2*stVoipMgrRtpCfg.ch_id+PROTOCOL__RTP-1);
	s_id_rtcp = (2*stVoipMgrRtpCfg.ch_id+PROTOCOL__RTCP-1);
	PRINT_MSG("SET RTP(%d)\n", s_id_rtp);
	//PRINT_MSG("VOIP_MGR_SETRTPPAYLOADTYPE(ch=%d, mid=%d)\n", stVoipMgrRtpCfg.ch_id, stVoipMgrRtpCfg.m_id);
	//PRINT_MSG("VOIP_MGR_RTP_CFG:ch_id = %d, state = %d\n", stVoipMgrRtpCfg.ch_id, stVoipMgrRtpCfg.state);


	#if 1
	PRINT_MSG("ch_id = %d\n", stVoipMgrSession.ch_id);
	PRINT_MSG("m_id = %d\n", stVoipMgrSession.m_id);
	PRINT_MSG("Rtp s_id = %d\n", s_id_rtp);
	PRINT_MSG("Rtp ip_src_addr = %d.%d.%d.%d\n", ((unsigned char *)(&stVoipMgrSession.ip_src_addr))[0], ((unsigned char *)(&stVoipMgrSession.ip_src_addr))[1], ((unsigned char *)(&stVoipMgrSession.ip_src_addr))[2], ((unsigned char *)(&stVoipMgrSession.ip_src_addr))[3]);
	PRINT_MSG("Rtp ip_dst_addr = %d.%d.%d.%d\n", ((unsigned char *)(&stVoipMgrSession.ip_dst_addr))[0], ((unsigned char *)(&stVoipMgrSession.ip_dst_addr))[1], ((unsigned char *)(&stVoipMgrSession.ip_dst_addr))[2], ((unsigned char *)(&stVoipMgrSession.ip_dst_addr))[3]);
	PRINT_MSG("Rtp udp_src_port = %d\n", ntohs(stVoipMgrSession.udp_src_port));
	PRINT_MSG("Rtp udp_dst_port = %d\n", ntohs(stVoipMgrSession.udp_dst_port));
	PRINT_MSG("protocol = 0x%x\n", stVoipMgrSession.protocol);
	#endif

	if(filter[s_id_rtp]!=0)
	{
         	PRINT_MSG("rtp: s_id %d in used, please unregister first\n", s_id_rtp);
		return -1;
        }
	if(filter[s_id_rtcp]!=0)
	{
         	PRINT_MSG("rtcp: s_id %d in used, please unregister first\n", s_id_rtcp);
		return -1;
        }

	save_flags(flags); cli();
	#if defined(CONFIG_RTK_VOIP_DRIVERS_PCM8651) && !defined(CONFIG_RTK_VOIP_RX)
	rtp_pair.isTcp = FALSE;
	rtp_pair.remIp = stVoipMgrRtpCfg.ip_src_addr;
	rtp_pair.remPort = stVoipMgrRtpCfg.udp_src_port;
	rtp_pair.extIp = stVoipMgrRtpCfg.ip_dst_addr;
	rtp_pair.extPort = stVoipMgrRtpCfg.udp_dst_port;
	rtp_pair.chid = stVoipMgrRtpCfg.ch_id;
	/* rtp register */
	rtp_pair.mid = s_id_rtp;
	result = voip_register_RTPport( /*chid*/ rtp_pair.chid, /*mid*/ rtp_pair.mid, &rtp_pair );
	if(result < 0 )
		PRINT_MSG("865x register RTP failed\n");
	else
	{
		rfc2833_dtmf_pt_local[s_id_rtp] = stVoipMgrRtpCfg.rfc2833_dtmf_pt_local;
        	rfc2833_dtmf_pt_remote[s_id_rtp] = stVoipMgrRtpCfg.rfc2833_dtmf_pt_remote;
		PRINT_MSG("865x register RTP success\n");

        	/* Check if local GW is RFC2833 mode, and remote GW support RFC2833, then set ACMW to RFC2833 mode. */
        	if ( dtmf_mode[stVoipMgrRtpCfg.ch_id] == 0 )/*RFC2833*/
		{
			if (rfc2833_dtmf_pt_remote[s_id_rtp] != 0)
			{
				RtkAc49xApiSetIbsTransferMode(stVoipMgrRtpCfg.ch_id, IBS_TRANSFER_MODE__RELAY_ENABLE_VOICE_MUTE);
				//RtkAc49xApiSetDtmfErasureMode(stVoipMgrRtpCfg.ch_id, DTMF_ERASURE_MODE__ERASE_1_COMPONENT);
			}
			else
			{
				RtkAc49xApiSetIbsTransferMode(stVoipMgrRtpCfg.ch_id, IBS_TRANSFER_MODE__TRANSPARENT_THROUGH_VOICE);
			}
		}
		/*******************************************************************************************************/
	}
	/* rtcp register (rtcp port = rtp port + 1) */
	rtp_pair.remPort = rtp_pair.remPort + 1;
	rtp_pair.extPort = rtp_pair.extPort + 1;
	rtp_pair.mid = s_id_rtcp;
	result = voip_register_RTPport( /*chid*/ rtp_pair.chid, /*mid*/ rtp_pair.mid, &rtp_pair );
	if(result < 0 )
		PRINT_MSG("865x register RTP failed\n");
	else
		PRINT_MSG("865x register RTP success\n");
	#else
	/* rtp register */
	stVoipMgrSession.m_id = PROTOCOL__RTP;
	rtk_trap_register(&stVoipMgrSession, stVoipMgrSession.ch_id, stVoipMgrSession.m_id, s_id_rtp,  ( int32(*)(uint8, int32, void *, uint32, uint32 ) )Ac49xTxPacketCB);
	/* rtcp register (rtcp port = rtp port + 1) */
	stVoipMgrSession.udp_src_port = stVoipMgrSession.udp_src_port + 1;
	stVoipMgrSession.udp_dst_port = stVoipMgrSession.udp_dst_port + 1;
	stVoipMgrSession.m_id = PROTOCOL__RTCP;
	rtk_trap_register(&stVoipMgrSession, stVoipMgrSession.ch_id, stVoipMgrSession.m_id, s_id_rtcp, ( int32(*)(uint8, int32, void *, uint32, uint32 ) )Ac49xTxPacketCB);
	rfc2833_dtmf_pt_local[s_id_rtp] = stVoipMgrRtpCfg.rfc2833_dtmf_pt_local;
	rfc2833_dtmf_pt_remote[s_id_rtp] = stVoipMgrRtpCfg.rfc2833_dtmf_pt_remote;
	
	/* Check if local GW is RFC2833 mode, and remote GW support RFC2833, then set ACMW to RFC2833 mode. */
	if ( dtmf_mode[stVoipMgrRtpCfg.ch_id] == 0 )/*RFC2833*/
	{
		if (rfc2833_dtmf_pt_remote[s_id_rtp] != 0)
		{
			RtkAc49xApiSetIbsTransferMode(stVoipMgrRtpCfg.ch_id, IBS_TRANSFER_MODE__RELAY_ENABLE_VOICE_MUTE);
			//RtkAc49xApiSetDtmfErasureMode(stVoipMgrRtpCfg.ch_id, DTMF_ERASURE_MODE__ERASE_1_COMPONENT);
		}
		else if (rfc2833_dtmf_pt_remote[s_id_rtp] == 0)
		{
			RtkAc49xApiSetIbsTransferMode(stVoipMgrRtpCfg.ch_id, IBS_TRANSFER_MODE__TRANSPARENT_THROUGH_VOICE);
		}
	}
	/*******************************************************************************************************/
	#endif

	restore_flags(flags);
	return 0;
}


/********************************************************************************/
/*										*/
/*  voip_mgr_set_rtp_2: This function set the payload type and active RTP       */
/*  Berfor calling voip_mgr_set_rtp_2, RTP session must be register.		*/
/*  I.e. voip_mgr_set_rtp_1 must be called.					*/
/*										*/
/********************************************************************************/

int voip_mgr_set_rtp_2(TstVoipMgrRtpCfg stVoipMgrRtpCfg)
{
  	//int ret = ( int )NULL;
  	int ret = -1;
  	uint32 s_id_rtp, s_id_rtcp;
	unsigned long flags;
	extern uint32 gSetByassMode[];

	s_id_rtp = (2*stVoipMgrRtpCfg.ch_id+PROTOCOL__RTP-1);
	s_id_rtcp = (2*stVoipMgrRtpCfg.ch_id+PROTOCOL__RTCP-1);
	
	save_flags(flags); cli();

#ifdef T38_STAND_ALONE_HANDLER
	if( stVoipMgrRtpCfg.uPktFormat == rtpPayloadT38_Virtual )
	{
		T38_API_Initialize( stVoipMgrRtpCfg.ch_id, NULL );
		PRINT_MSG("MGR: Initialize T38(%d)\n", stVoipMgrRtpCfg.ch_id);
		t38RunningState[ stVoipMgrRtpCfg.ch_id ] = T38_START;
		enable_silence_det( stVoipMgrRtpCfg.ch_id, 1 );
	}
	else
	{
		enable_silence_det( stVoipMgrRtpCfg.ch_id, 0 );
		t38RunningState[ stVoipMgrRtpCfg.ch_id ] = T38_STOP;
		RtkAc49xApiSetRtpChannelConfiguration(stVoipMgrRtpCfg.ch_id, s_id_rtp, stVoipMgrRtpCfg.remote_pt, stVoipMgrRtpCfg.nG723Type, stVoipMgrRtpCfg.bVAD);
		
		if (CHANNEL_STATE__ACTIVE_RTP != RtkAc49xGetChannelState(stVoipMgrRtpCfg.ch_id))
			RtkAc49xApiSetVoiceJBDelay(stVoipMgrRtpCfg.ch_id, 10*stVoipMgrRtpCfg.nMaxDelay, 10*stVoipMgrRtpCfg.nJitterDelay, stVoipMgrRtpCfg.nJitterFactor);
		
		if (gSetByassMode[stVoipMgrRtpCfg.ch_id] == 1) // fax bypass
		{
			RtkAc49xApiSetIntoBypassMode(stVoipMgrRtpCfg.ch_id, FAX_BYPASS);
		}
		else if (gSetByassMode[stVoipMgrRtpCfg.ch_id] == 2) // modem bypass
		{
			RtkAc49xApiSetIntoBypassMode(stVoipMgrRtpCfg.ch_id, MODEM_BYPASS);
		}

	}
#else
		RtkAc49xApiSetRtpChannelConfiguration(stVoipMgrRtpCfg.ch_id, s_id_rtp, stVoipMgrRtpCfg.remote_pt, stVoipMgrRtpCfg.nG723Type, stVoipMgrRtpCfg.bVAD);
		
		if (CHANNEL_STATE__ACTIVE_RTP != RtkAc49xGetChannelState(stVoipMgrRtpCfg.ch_id))
			RtkAc49xApiSetVoiceJBDelay(stVoipMgrRtpCfg.ch_id, 10*stVoipMgrRtpCfg.nMaxDelay, 10*stVoipMgrRtpCfg.nJitterDelay, stVoipMgrRtpCfg.nJitterFactor);
		
		if (gSetByassMode[stVoipMgrRtpCfg.ch_id] == 1) // fax bypass
		{
			RtkAc49xApiSetIntoBypassMode(stVoipMgrRtpCfg.ch_id, FAX_BYPASS);
		}
		else if (gSetByassMode[stVoipMgrRtpCfg.ch_id] == 2) // modem bypass
		{
			RtkAc49xApiSetIntoBypassMode(stVoipMgrRtpCfg.ch_id, MODEM_BYPASS);
		}

#endif


     	if(stVoipMgrRtpCfg.state ==1)
      	{
      		if (rtpConfigOK[s_id_rtp] == 0)
			ret = RtkAc49xApiActiveRegularRtp(stVoipMgrRtpCfg.ch_id, s_id_rtp); /* return 0 : success */

		if (ret == 0)
		{
			rtpConfigOK[s_id_rtp] = 1;
			rtpConfigOK[s_id_rtcp] = 1;
			PRINT_MSG(".open RTP(%d, %d)\n", stVoipMgrRtpCfg.ch_id, s_id_rtp);
		}
	}
	else
	{
		if (rtpConfigOK[s_id_rtp] == 1)
			ret = RtkAc49xApiCloseRegularRtp(stVoipMgrRtpCfg.ch_id, s_id_rtp); /* return 0 : success */

		if (ret == 0)
		{
			rtpConfigOK[s_id_rtp] = 0;
			rtpConfigOK[s_id_rtcp] = 0;
			PRINT_MSG(".close RTP(%d, %d)\n", stVoipMgrRtpCfg.ch_id, s_id_rtp);
		}
	}

	restore_flags(flags);
	return 0;

}

/********************************************************************************/
/*										*/
/*  voip_mgr_unset_rtp_1: This function un-register RTP session			*/
/*										*/
/********************************************************************************/

/////// voip_mgr_set_rtp_2: This function set the payload type and active RTP ///////
int voip_mgr_unset_rtp_1(int chid, int mid)
{
	uint32 s_id_rtp, s_id_rtcp;
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8651
	int32 result;
	voip_rtp_port_t rtp_pair;
#endif
	//int ret = ( int )NULL;
	unsigned long flags;

	s_id_rtp = (2*chid + PROTOCOL__RTP-1);
	s_id_rtcp = (2*chid + PROTOCOL__RTCP-1);
	PRINT_MSG("UNSET RTP(%d)\n", s_id_rtp);

	save_flags(flags); cli();

	#if defined(CONFIG_RTK_VOIP_DRIVERS_PCM8651) && !defined(CONFIG_RTK_VOIP_RX)
	rtp_pair.chid = stVoipCfg.ch_id;

	rtp_pair.mid = s_id_rtp;
	result = voip_register_RTPport( /*chid*/ rtp_pair.chid, /*mid*/ rtp_pair.mid, NULL );
	if( result < 0 )
		PRINT_MSG("Unregister 865x RTP port  failed\n");
        else
		PRINT_MSG("Unregister 865x RTP port  successfully\n");

	rtp_pair.mid = s_id_rtcp;
	result = voip_register_RTPport( /*chid*/ rtp_pair.chid, /*mid*/ rtp_pair.mid, NULL );
	if( result < 0 )
		PRINT_MSG("Unregister 865x RTP port  failed\n");
        else
		PRINT_MSG("Unregister 865x RTP port  successfully\n");
	#else
	rtk_trap_unregister(s_id_rtp);
	rtk_trap_unregister(s_id_rtcp);
	#endif

	restore_flags(flags);
	
	return 0;
}

/********************************************************************************/
/*										*/
/*  voip_mgr_unset_rtp_2: This function de-active the RTP			*/
/*  Usually, voip_mgr_unset_rtp_1 should be used to unrgister and then          */
/*  call the voip_mgr_unset_rtp_2 to deactive RTP.				*/
/*										*/
/********************************************************************************/

int voip_mgr_unset_rtp_2(int chid, int mid)
{
	uint32 s_id_rtp, s_id_rtcp;

	//int ret = ( int )NULL;
	int ret = -1;
	unsigned long flags;

	s_id_rtp = (2*chid + PROTOCOL__RTP-1);
	s_id_rtcp = (2*chid + PROTOCOL__RTCP-1);

	save_flags(flags); cli();


	if (rtpConfigOK[s_id_rtp] == 1)
		ret = RtkAc49xApiCloseRegularRtp(chid, s_id_rtp); /* return 0 : success */

	if (ret == 0)
	{
		rtpConfigOK[s_id_rtp] = 0;
		rtpConfigOK[s_id_rtcp] = 0;
		PRINT_MSG("close RTP(%d)\n", s_id_rtp);
	}

	restore_flags(flags);
	
	return 0;
}
#endif //AUDIOCODES_VOIP

/**
 * @ingroup VOIP_PROTOCOL_RTP
 * @brief Set RTP session trap with IP, port and so on 
 * @see VOIP_MGR_SET_SESSION TstVoipMgrSession rtk_trap_register()
 * @see do_mgr_VOIP_MGR_UNSET_SESSION()
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_SET_SESSION( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	unsigned long flags;
	uint32 s_id;
	TstVoipMgrSession stVoipMgrSession;
	int ret = 0;
	extern unsigned int CurrentRfc2833DtmfMode[];
	extern unsigned int CurrentRfc2833FaxModemMode[];

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	unsigned int mgr_chid;
#endif

	COPY_FROM_USER(&stVoipMgrSession, (TstVoipMgrSession *)user, sizeof(TstVoipMgrSession));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST

	s_id = API_GetSid(stVoipMgrSession.ch_id, stVoipMgrSession.m_id);
	PRINT_MSG("SET RTP SESSION(%d)\n", s_id);
	
	save_flags(flags); cli();

#ifdef SUPPORT_IP_ADDR_QOS
#if 1
	rtl8306_setAsicQosIPAddress(
		s_id%2,
		stVoipMgrSession.ip_src_addr,
		0xFFFFFFFF, 
		1);
	rtl8306_getAsicPhyReg(6, 0, 0, &i);
#else
	disable_eth8186_rx();
	rtl8306_disable_all_ports();
	rtl8306_setAsicQosIPAddress(
		s_id%2,
		stVoipMgrSession.ip_src_addr,
		0xFFFFFFFF,
		1);
	rtl8306_asicSoftReset();
	rtl8306_enable_all_ports();
	enable_eth8186_rx();
#endif
#endif

	if(filter[s_id]!=0) {
		PRINT_MSG("rtp: s_id %d in used, please unregister first\n", s_id);
		return 0;
	}
	
	
#if defined(CONFIG_RTK_VOIP_DRIVERS_PCM8651) && !defined(CONFIG_RTK_VOIP_RX)
	rtp_pair.isTcp = FALSE;
	rtp_pair.remIp = stVoipMgrSession.ip_src_addr;
	rtp_pair.remPort = stVoipMgrSession.udp_src_port;
	rtp_pair.extIp = stVoipMgrSession.ip_dst_addr;
	rtp_pair.extPort = stVoipMgrSession.udp_dst_port;
	rtp_pair.chid = stVoipMgrSession.ch_id;
	rtp_pair.mid = s_id;
	stVoipMgrSession.result = voip_register_RTPport( /*chid*/ rtp_pair.chid, /*mid*/ rtp_pair.mid, &rtp_pair );
	restore_flags(flags);

	if(stVoipMgrSession.result < 0 )
	{
		PRINT_MSG("865x register RTP failed\n");
	}
	else
	{
		PRINT_MSG("865x register RTP success\n");
	}
#else
	stVoipMgrSession.result = rtk_trap_register(&stVoipMgrSession, stVoipMgrSession.ch_id, stVoipMgrSession.m_id, s_id,  (int32(*)(uint8 , int32 , void *, uint32 , uint32 ))Host_pktRx);
	restore_flags(flags);

	if(stVoipMgrSession.result == -1)
		PRINT_MSG("rtk_trap_register fail, sid=%d\n", s_id);
	else if (stVoipMgrSession.result == 0)
		PRINT_MSG("rtk_trap_register success, sid=%d\n", s_id);
#endif

	
	// Send Control Packet and wait Response Packet
	mgr_chid = stVoipMgrSession.ch_id;
	stVoipMgrSession.ch_id = API_get_DSP_CH(cmd, stVoipMgrSession.ch_id);	// convert to DSP chid
	ret = ipcSentControlPacket(cmd, mgr_chid, &stVoipMgrSession, sizeof(TstVoipMgrSession));
	stVoipMgrSession.ch_id = mgr_chid;

#else //! CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST

	s_id = API_GetSid(stVoipMgrSession.ch_id, stVoipMgrSession.m_id);
	PRINT_MSG("SET RTP SESSION(%d)\n", s_id);
#if 0
	PRINT_MSG("ch_id = %d\n", stVoipMgrSession.ch_id);
	PRINT_MSG("m_id = %d\n", stVoipMgrSession.m_id);
	PRINT_MSG("Rtp s_id = %d\n", s_id);
	PRINT_MSG("Rtp ip_src_addr = %d.%d.%d.%d\n", ((unsigned char *)(&stVoipMgrSession.ip_src_addr))[0], ((unsigned char *)(&stVoipMgrSession.ip_src_addr))[1], ((unsigned char *)(&stVoipMgrSession.ip_src_addr))[2], ((unsigned char *)(&stVoipMgrSession.ip_src_addr))[3]);
	PRINT_MSG("Rtp ip_dst_addr = %d.%d.%d.%d\n", ((unsigned char *)(&stVoipMgrSession.ip_dst_addr))[0], ((unsigned char *)(&stVoipMgrSession.ip_dst_addr))[1], ((unsigned char *)(&stVoipMgrSession.ip_dst_addr))[2], ((unsigned char *)(&stVoipMgrSession.ip_dst_addr))[3]);
	PRINT_MSG("Rtp udp_src_port = %d\n", ntohs(stVoipMgrSession.udp_src_port));
	PRINT_MSG("Rtp udp_dst_port = %d\n", ntohs(stVoipMgrSession.udp_dst_port));
	PRINT_MSG("protocol = 0x%x\n", stVoipMgrSession.protocol);
#endif
	
	save_flags(flags); cli();

#ifdef SUPPORT_IP_ADDR_QOS
#if 1
	rtl8306_setAsicQosIPAddress(
		s_id%2,
		stVoipMgrSession.ip_src_addr,
		0xFFFFFFFF, 
		1);
	rtl8306_getAsicPhyReg(6, 0, 0, &i);
#else
	disable_eth8186_rx();
	rtl8306_disable_all_ports();
	rtl8306_setAsicQosIPAddress(
		s_id%2,
		stVoipMgrSession.ip_src_addr,
		0xFFFFFFFF,
		1);
	rtl8306_asicSoftReset();
	rtl8306_enable_all_ports();
	enable_eth8186_rx();
#endif
#endif

	if(filter[s_id]!=0) {
		PRINT_MSG("rtp: s_id %d in used, please unregister first\n", s_id);
		return 0;
	}

	RtpSession_renew(s_id, &stVoipMgrSession, NULL /*ignore*/ );

#if defined(CONFIG_RTK_VOIP_DRIVERS_PCM8651) && !defined(CONFIG_RTK_VOIP_RX)
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	result = stVoipMgrSession.result;
#else
	rtp_pair.isTcp = FALSE;
	rtp_pair.remIp = stVoipMgrSession.ip_src_addr;
	rtp_pair.remPort = stVoipMgrSession.udp_src_port;
	rtp_pair.extIp = stVoipMgrSession.ip_dst_addr;
	rtp_pair.extPort = stVoipMgrSession.udp_dst_port;
	rtp_pair.chid = stVoipMgrSession.ch_id;
	rtp_pair.mid = s_id;
	result = voip_register_RTPport( /*chid*/ rtp_pair.chid, /*mid*/ rtp_pair.mid, &rtp_pair );
#endif
	if(result < 0 )
	{
		PRINT_MSG("865x register RTP failed\n");
		RtpOpen[s_id] = 0;
	}
	else
	{
		PRINT_MSG("865x register RTP success\n");
		RtpOpen[s_id] = 1;                      
		rfc2833_dtmf_pt_local[s_id] = stVoipMgrSession.rfc2833_payload_type_local;
		rfc2833_dtmf_pt_remote[s_id] = stVoipMgrSession.rfc2833_payload_type_remote;
	}
#else
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP
	if(stVoipMgrSession.result == 0) //register success (info from Host)
#else
	if(!rtk_trap_register(&stVoipMgrSession, stVoipMgrSession.ch_id, stVoipMgrSession.m_id, s_id,  (int32(*)(uint8 , int32 , void *, uint32 , uint32 ))DSP_pktRx))
#endif
	{
		RtpOpen[s_id] = 1;
		
		//if ( Is_DAA_Channel(stVoipMgrSession.ch_id) != 1)
		{
			/* If talk_flag > 0, DTMF detection threshold will be pulled to higt level. */
			talk_flag[stVoipMgrSession.ch_id] = talk_flag[stVoipMgrSession.ch_id] + 1;
		}
	}                
	rfc2833_dtmf_pt_local[s_id] = stVoipMgrSession.rfc2833_dtmf_pt_local;
	rfc2833_dtmf_pt_remote[s_id] = stVoipMgrSession.rfc2833_dtmf_pt_remote;
	
	rfc2833_fax_modem_pt_local[s_id] = stVoipMgrSession.rfc2833_fax_modem_pt_local;
	rfc2833_fax_modem_pt_remote[s_id] = stVoipMgrSession.rfc2833_fax_modem_pt_remote;
	
	//PRINT_MSG(" rfc2833_dtmf_pt_local[%d] = %d\n", s_id, rfc2833_dtmf_pt_local[s_id]);
	//PRINT_MSG(" rfc2833_dtmf_pt_remote[%d] = %d\n", s_id, rfc2833_dtmf_pt_remote[s_id]);
	//PRINT_MSG(" rfc2833_fax_modem_pt_local[%d] = %d\n", s_id, rfc2833_fax_modem_pt_local[s_id]);
	//PRINT_MSG(" rfc2833_fax_modem_pt_remote[%d] = %d\n", s_id, rfc2833_fax_modem_pt_remote[s_id]);
	
	extern unsigned char dtmf_removal_flag[];
	
	if (rfc2833_dtmf_pt_local[s_id]!=0 && rfc2833_dtmf_pt_remote[s_id]!=0)
	{
		CurrentRfc2833DtmfMode[stVoipMgrSession.ch_id] = 1;
		dtmf_removal_flag[stVoipMgrSession.ch_id] = 1;
	}
	else
		CurrentRfc2833DtmfMode[stVoipMgrSession.ch_id] = 0;
	
	if (rfc2833_fax_modem_pt_local[s_id]!=0 && rfc2833_fax_modem_pt_remote[s_id]!=0)
		CurrentRfc2833FaxModemMode[stVoipMgrSession.ch_id] = 1;
	else
		CurrentRfc2833FaxModemMode[stVoipMgrSession.ch_id] = 0;
#endif
	g_SIP_Info_play[s_id] = 0; /* Init for SIP Info play tone */

#ifdef PCM_LOOP_MODE_CONTROL
	int s, s1, loop_cnt=0;
	
	if (pcm_get_LoopMode() != 2)
	{
		for (s=0; s<DSP_RTK_SS_NUM; s++)
		{
			if(LoopBackInfo[s].isLoopBack == 1)
			{
				loop_cnt++;
			}
		}
	
		if (stVoipMgrSession.ip_src_addr==stVoipMgrSession.ip_dst_addr)
		{
			if (loop_cnt < 2)
			{
				LoopBackInfo[s_id].chid = stVoipMgrSession.ch_id;
				LoopBackInfo[s_id].sid = s_id;
				LoopBackInfo[s_id].isLoopBack = 1;
				LoopBackInfo[s_id].ip_src_addr = stVoipMgrSession.ip_src_addr;
				LoopBackInfo[s_id].ip_dst_addr = stVoipMgrSession.ip_dst_addr;
				LoopBackInfo[s_id].udp_src_port = stVoipMgrSession.udp_src_port;
				LoopBackInfo[s_id].udp_dst_port = stVoipMgrSession.udp_dst_port;
				loop_cnt++;
			}
			else if (loop_cnt == 2)
			{
				LoopBackInfo[s_id].isLoopBack = 0;
			}
			else if (loop_cnt > 2)
			{
				LoopBackInfo[s_id].isLoopBack = 0;
				PRINT_R("Error: loop mode session count > 2.\n");
			}
		}
		else
		{
			LoopBackInfo[s_id].isLoopBack = 0;
		}
	
		if (LoopBackInfo[s_id].isLoopBack == 1)
		{
			if (loop_cnt >=2)
			{
				for (s=0; s<DSP_RTK_SS_NUM; s++)
				{
					if(LoopBackInfo[s].isLoopBack == 1)
					{
						for (s1=s+1; s1<DSP_RTK_SS_NUM; s1++)
						{
							if(LoopBackInfo[s1].isLoopBack == 1)
							{
								if (LoopBackInfo[s].udp_src_port == LoopBackInfo[s1].udp_dst_port)
								{
									int ch1, ch2;
									int m_id, sid_tmp;
									ch1 = chanInfo_GetChannelbySession(s);
									ch2 = chanInfo_GetChannelbySession(s1);
	
									if (loop_3way[ch1] == 1)
									{
										pcm_set_LoopMode(2, ch1, ch2);
	
										for (m_id=0; m_id<2; m_id++)
										{
											sid_tmp = API_GetSid(ch1, m_id);
											if (LoopBackInfo[sid_tmp].isLoopBack == 0)
											{
												chanInfo_SetTranSessionID(ch1, sid_tmp);
												//PRINT_MSG("SetTranSessionID %d\n", sid_tmp);
											}
										}
									}
									else if (loop_3way[ch2] == 1)
									{
										pcm_set_LoopMode(2, ch2, ch1);
	
										for (m_id=0; m_id<2; m_id++)
										{
											sid_tmp = API_GetSid(ch2, m_id);
											if (LoopBackInfo[sid_tmp].isLoopBack == 0)
											{
												chanInfo_SetTranSessionID(ch2, sid_tmp);
												//PRINT_MSG("SetTranSessionID %d\n", sid_tmp);
											}
										}
									}
									else
										pcm_set_LoopMode(2, ch1, ch2);
								}
	
							}
						}
					}
				}
			}
		}
	}

#endif // PCM_LOOP_MODE_CONTROL

	restore_flags(flags);
#if 0
	PRINT_MSG("chid = %d, talk_flag = %d \n", stVoipMgrSession.ch_id, talk_flag[stVoipMgrSession.ch_id]);
#endif
#endif

	return ret;
}
#else
int do_mgr_VOIP_MGR_SET_SESSION( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	unsigned long flags;
	TstVoipMgrSession stVoipMgrSession;	
	uint32 s_id_rtp, s_id_rtcp;
	int ret = 0;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	unsigned int mgr_chid;
#endif

	COPY_FROM_USER(&stVoipMgrSession, (TstVoipMgrSession *)user, sizeof(TstVoipMgrSession));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST

	stVoipMgrSession.protocol = UDP_PROTOCOL;

	s_id_rtp = (2*stVoipMgrSession.ch_id+PROTOCOL__RTP-1);
	s_id_rtcp = (2*stVoipMgrSession.ch_id+PROTOCOL__RTCP-1);
	PRINT_MSG("SET RTP(%d)\n", s_id_rtp);

	if(filter[s_id_rtp]!=0)
	{
		PRINT_MSG("rtp: s_id %d in used, please unregister first\n", s_id_rtp);
		//stVoipMgrSession.ret_val = 0;
		return COPY_TO_USER(user, &stVoipMgrSession, sizeof(TstVoipMgrSession), cmd, seq_no);
		//return 0;
	}
	if(filter[s_id_rtcp]!=0)
	{
		PRINT_MSG("rtcp: s_id %d in used, please unregister first\n", s_id_rtcp);
		//stVoipMgrSession.ret_val = 0;
		return COPY_TO_USER(user, &stVoipMgrSession, sizeof(TstVoipMgrSession), cmd, seq_no);
		//return 0;
	}

	save_flags(flags); cli();
#if defined(CONFIG_RTK_VOIP_DRIVERS_PCM8651) && !defined(CONFIG_RTK_VOIP_RX)
	rtp_pair.isTcp = FALSE;
	rtp_pair.remIp = stVoipMgrSession.ip_src_addr;
	rtp_pair.remPort = stVoipMgrSession.udp_src_port;
	rtp_pair.extIp = stVoipMgrSession.ip_dst_addr;
	rtp_pair.extPort = stVoipMgrSession.udp_dst_port;
	rtp_pair.chid = stVoipMgrSession.ch_id;
	/* rtp register */
	rtp_pair.mid = s_id_rtp;
	result = voip_register_RTPport( /*chid*/ rtp_pair.chid, /*mid*/ rtp_pair.mid, &rtp_pair );
	if(result < 0 )
		PRINT_MSG("865x register RTP failed\n");
	else
		PRINT_MSG("865x register RTP success\n");

	/* rtcp register (rtcp port = rtp port + 1) */
	rtp_pair.remPort = rtp_pair.remPort + 1;
	rtp_pair.extPort = rtp_pair.extPort + 1;
	rtp_pair.mid = s_id_rtcp;
	stVoipMgrSession.result = voip_register_RTPport( /*chid*/ rtp_pair.chid, /*mid*/ rtp_pair.mid, &rtp_pair );
	restore_flags(flags);
	if(stVoipMgrSession.result < 0 )
		PRINT_MSG("865x register RTP failed\n");
	else
		PRINT_MSG("865x register RTP success\n");
#else
	/* rtp register */
	stVoipMgrSession.m_id = PROTOCOL__RTP;
	rtk_trap_register(&stVoipMgrSession, stVoipMgrSession.ch_id, stVoipMgrSession.m_id, s_id_rtp,  ( int32(*)(uint8, int32, void *, uint32, uint32 ) )Ac49xTxPacketCB);
	/* rtcp register (rtcp port = rtp port + 1) */
	stVoipMgrSession.udp_src_port = stVoipMgrSession.udp_src_port + 1;
	stVoipMgrSession.udp_dst_port = stVoipMgrSession.udp_dst_port + 1;
	stVoipMgrSession.m_id = PROTOCOL__RTCP;
	stVoipMgrSession.result = rtk_trap_register(&stVoipMgrSession, stVoipMgrSession.ch_id, stVoipMgrSession.m_id, s_id_rtcp,  ( int32(*)(uint8, int32, void *, uint32, uint32 ) )Ac49xTxPacketCB);
	
	restore_flags(flags);
#endif
	// Send Control Packet and wait Response Packet
	mgr_chid = stVoipMgrSession.ch_id;
	stVoipMgrSession.ch_id = API_get_DSP_CH(cmd, stVoipMgrSession.ch_id);	// convert to DSP chid
	ret = ipcSentControlPacket(cmd, mgr_chid, &stVoipMgrSession, sizeof(TstVoipMgrSession));
	stVoipMgrSession.ch_id = mgr_chid;
	//stVoipMgrSession.ret_val = ret;


#else //!CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST

	stVoipMgrSession.protocol = UDP_PROTOCOL;

	s_id_rtp = (2*stVoipMgrSession.ch_id+PROTOCOL__RTP-1);
	s_id_rtcp = (2*stVoipMgrSession.ch_id+PROTOCOL__RTCP-1);
	PRINT_MSG("SET RTP(%d)\n", s_id_rtp);
	//PRINT_MSG("s_id_rtp = %d\n", s_id_rtp);
	//PRINT_MSG("s_id_rtcp = %d\n", s_id_rtcp);

	#if 0
	PRINT_MSG("ch_id = %d\n", stVoipMgrSession.ch_id);
	PRINT_MSG("m_id = %d\n", stVoipMgrSession.m_id);
	PRINT_MSG("Rtp s_id = %d\n", s_id);
	PRINT_MSG("Rtp ip_src_addr = %d.%d.%d.%d\n", ((unsigned char *)(&stVoipMgrSession.ip_src_addr))[0], ((unsigned char *)(&stVoipMgrSession.ip_src_addr))[1], ((unsigned char *)(&stVoipMgrSession.ip_src_addr))[2], ((unsigned char *)(&stVoipMgrSession.ip_src_addr))[3]);
	PRINT_MSG("Rtp ip_dst_addr = %d.%d.%d.%d\n", ((unsigned char *)(&stVoipMgrSession.ip_dst_addr))[0], ((unsigned char *)(&stVoipMgrSession.ip_dst_addr))[1], ((unsigned char *)(&stVoipMgrSession.ip_dst_addr))[2], ((unsigned char *)(&stVoipMgrSession.ip_dst_addr))[3]);
	PRINT_MSG("Rtp udp_src_port = %d\n", ntohs(stVoipMgrSession.udp_src_port));
	PRINT_MSG("Rtp udp_dst_port = %d\n", ntohs(stVoipMgrSession.udp_dst_port));
	PRINT_MSG("protocol = 0x%x\n", stVoipMgrSession.protocol);
	#endif

	if(filter[s_id_rtp]!=0)
	{
		PRINT_MSG("rtp: s_id %d in used, please unregister first\n", s_id_rtp);
		return 0;
	}
	if(filter[s_id_rtcp]!=0)
	{
		PRINT_MSG("rtcp: s_id %d in used, please unregister first\n", s_id_rtcp);
		return 0;
	}

	save_flags(flags); cli();
#if defined(CONFIG_RTK_VOIP_DRIVERS_PCM8651) && !defined(CONFIG_RTK_VOIP_RX)
	rtp_pair.isTcp = FALSE;
	rtp_pair.remIp = stVoipMgrSession.ip_src_addr;
	rtp_pair.remPort = stVoipMgrSession.udp_src_port;
	rtp_pair.extIp = stVoipMgrSession.ip_dst_addr;
	rtp_pair.extPort = stVoipMgrSession.udp_dst_port;
	rtp_pair.chid = stVoipMgrSession.ch_id;
	/* rtp register */
	rtp_pair.mid = s_id_rtp;
	result = voip_register_RTPport( /*chid*/ rtp_pair.chid, /*mid*/ rtp_pair.mid, &rtp_pair );
	if(result < 0 )
		PRINT_MSG("865x register RTP failed\n");
	else
	{
		rfc2833_dtmf_pt_local[s_id_rtp] = stVoipMgrSession.rfc2833_dtmf_pt_local;
                	rfc2833_dtmf_pt_remote[s_id_rtp] = stVoipMgrSession.rfc2833_dtmf_pt_remote;
		PRINT_MSG("865x register RTP success\n");

		/* Check if local GW is RFC2833 mode, and remote GW support RFC2833, then set ACMW to RFC2833 mode. */
		if ( dtmf_mode[stVoipMgrSession.ch_id] == 0 )/*RFC2833*/
		{
			if (rfc2833_dtmf_pt_remote[s_id_rtp] != 0)
			{
				RtkAc49xApiSetIbsTransferMode(stVoipMgrSession.ch_id, IBS_TRANSFER_MODE__RELAY_ENABLE_VOICE_MUTE);
				//RtkAc49xApiSetDtmfErasureMode(stVoipMgrSession.ch_id, DTMF_ERASURE_MODE__ERASE_1_COMPONENT);
			}
			else
			{
				RtkAc49xApiSetIbsTransferMode(stVoipMgrSession.ch_id, IBS_TRANSFER_MODE__TRANSPARENT_THROUGH_VOICE);
			}
		}
		/*******************************************************************************************************/
	}
	/* rtcp register (rtcp port = rtp port + 1) */
	rtp_pair.remPort = rtp_pair.remPort + 1;
	rtp_pair.extPort = rtp_pair.extPort + 1;
	rtp_pair.mid = s_id_rtcp;
	result = voip_register_RTPport( /*chid*/ rtp_pair.chid, /*mid*/ rtp_pair.mid, &rtp_pair );
	restore_flags(flags);
	if(result < 0 )
		PRINT_MSG("865x register RTP failed\n");
	else
		PRINT_MSG("865x register RTP success\n");
#else
	/* rtp register */
	stVoipMgrSession.m_id = PROTOCOL__RTP;
	rtk_trap_register(&stVoipMgrSession, stVoipMgrSession.ch_id, stVoipMgrSession.m_id, s_id_rtp,  ( int32(*)(uint8, int32, void *, uint32, uint32 ) )Ac49xTxPacketCB);
	/* rtcp register (rtcp port = rtp port + 1) */
	stVoipMgrSession.udp_src_port = stVoipMgrSession.udp_src_port + 1;
	stVoipMgrSession.udp_dst_port = stVoipMgrSession.udp_dst_port + 1;
	stVoipMgrSession.m_id = PROTOCOL__RTCP;
	rtk_trap_register(&stVoipMgrSession, stVoipMgrSession.ch_id, stVoipMgrSession.m_id, s_id_rtcp,  ( int32(*)(uint8, int32, void *, uint32, uint32 ) )Ac49xTxPacketCB);
	rfc2833_dtmf_pt_local[s_id_rtp] = stVoipMgrSession.rfc2833_dtmf_pt_local;
	rfc2833_dtmf_pt_remote[s_id_rtp] = stVoipMgrSession.rfc2833_dtmf_pt_remote;
	
	/* Check if local GW is RFC2833 mode, and remote GW support RFC2833, then set ACMW to RFC2833 mode. */
	if ( dtmf_mode[stVoipMgrSession.ch_id] == 0 )/*RFC2833*/
	{
		if (rfc2833_dtmf_pt_remote[s_id_rtp] != 0)
		{
			RtkAc49xApiSetIbsTransferMode(stVoipMgrSession.ch_id, IBS_TRANSFER_MODE__RELAY_ENABLE_VOICE_MUTE);
			//RtkAc49xApiSetDtmfErasureMode(stVoipMgrSession.ch_id, DTMF_ERASURE_MODE__ERASE_1_COMPONENT);
		}
		else if (rfc2833_dtmf_pt_remote[s_id_rtp] == 0)
		{
			RtkAc49xApiSetIbsTransferMode(stVoipMgrSession.ch_id, IBS_TRANSFER_MODE__TRANSPARENT_THROUGH_VOICE);
		}
	}
	/*******************************************************************************************************/
	restore_flags(flags);
#endif

#endif
	
	return ret;
}
#endif // !AUDIOCODES_VOIP

/**
 * @ingroup VOIP_PROTOCOL_RTP
 * @brief Unset RTP session trap 
 * @param TstVoipCfg.ch_id Channel ID 
 * @param TstVoipCfg.m_id Media ID 
 * @see VOIP_MGR_UNSET_SESSION TstVoipCfg rtk_trap_unregister()
 * @see do_mgr_VOIP_MGR_SET_SESSION()
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_UNSET_SESSION( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	extern int dsp_rtk_ch_num;
#else
	unsigned int mgr_chid;
#endif

	unsigned long flags;
	uint32 s_id;
	TstVoipCfg stVoipCfg;
	int ret;

	PRINT_MSG("UNSET RTP SESSION\n");
	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST

	s_id = API_GetSid(stVoipCfg.ch_id, stVoipCfg.m_id);
	PRINT_MSG("s_id = %d\n", s_id);
	
	save_flags(flags); cli();
#ifdef SUPPORT_IP_ADDR_QOS
#if 1
	rtl8306_setAsicQosIPAddress(
		s_id%2,
		0,
		0, 
		0);
	rtl8306_getAsicPhyReg(6, 0, 0, &i);
#else
	disable_eth8186_rx();
	rtl8306_disable_all_ports();
	rtl8306_setAsicQosIPAddress(
		s_id%2,
		0,
		0,
		0);
	rtl8306_asicSoftReset();
	rtl8306_enable_all_ports();
	enable_eth8186_rx();
#endif
#endif

#if defined(CONFIG_RTK_VOIP_DRIVERS_PCM8651) && !defined(CONFIG_RTK_VOIP_RX)
	rtp_pair.chid = stVoipCfg.ch_id;
	rtp_pair.mid = s_id;
	result = voip_register_RTPport( /*chid*/ rtp_pair.chid, /*mid*/ rtp_pair.mid, NULL );
	if( result < 0 )
	{
		PRINT_MSG("Unregister 865x RTP port  failed\n");
	}

#else
	rtk_trap_unregister(s_id);
            
#endif

	restore_flags(flags);

	// Send Control Packet and wait Response Packet
	mgr_chid = stVoipCfg.ch_id;
	stVoipCfg.ch_id = API_get_DSP_CH(cmd, stVoipCfg.ch_id);	// convert to DSP chid
	ret = ipcSentControlPacket(cmd, mgr_chid, &stVoipCfg, sizeof(TstVoipCfg));
	stVoipCfg.ch_id = mgr_chid;
	
#else //!CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	s_id = API_GetSid(stVoipCfg.ch_id, stVoipCfg.m_id);
	PRINT_MSG("s_id = %d\n", s_id);
	
	save_flags(flags); cli();
#ifdef SUPPORT_IP_ADDR_QOS
#if 1
	rtl8306_setAsicQosIPAddress(
		s_id%2,
		0,
		0, 
		0);
	rtl8306_getAsicPhyReg(6, 0, 0, &i);
#else
	disable_eth8186_rx();
	rtl8306_disable_all_ports();
	rtl8306_setAsicQosIPAddress(
		s_id%2,
		0,
		0,
		0);
	rtl8306_asicSoftReset();
	rtl8306_enable_all_ports();
	enable_eth8186_rx();
#endif
#endif

#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP
#if defined(CONFIG_RTK_VOIP_DRIVERS_PCM8651) && !defined(CONFIG_RTK_VOIP_RX)
	rtp_pair.chid = stVoipCfg.ch_id;
	rtp_pair.mid = s_id;
	result = voip_register_RTPport( /*chid*/ rtp_pair.chid, /*mid*/ rtp_pair.mid, NULL );
	if( result < 0 )
	{
		PRINT_MSG("Unregister 865x RTP port  failed\n");
	}
	else
		RtpOpen[s_id] = 0;
#else
	rtk_trap_unregister(s_id);
	RtpOpen[s_id] = 0;
#endif
#endif
	//if ( Is_DAA_Channel(stVoipCfg.ch_id) != 1)
	{
		if(stVoipCfg.ch_id < dsp_rtk_ch_num)
		{
			if(talk_flag[stVoipCfg.ch_id]>0)
			{
				talk_flag[stVoipCfg.ch_id] = talk_flag[stVoipCfg.ch_id] - 1;
			}
			else
			{
				talk_flag[stVoipCfg.ch_id] = 0;
			}
		}
	}

#ifdef VOIP_RESOURCE_CHECK
	resource_weight[s_id] = 0;
#endif

#ifdef PCM_LOOP_MODE_CONTROL
	if(LoopBackInfo[s_id].isLoopBack == 1)
	{
		LoopBackInfo[s_id].isLoopBack = 0;
		pcm_set_LoopMode(0,NULL, NULL);
	}
#endif

	extern int Reset_RFC2833_Trasmit(uint32 sid);
	Reset_RFC2833_Trasmit(s_id);
	
	restore_flags(flags);
#if 0
	PRINT_MSG("chid = %d, talk_flag = %d \n", stVoipCfg.ch_id, talk_flag[stVoipCfg.ch_id]);
#endif
#endif	
	return 0;
}
#else
int do_mgr_VOIP_MGR_UNSET_SESSION( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	unsigned long flags;
	TstVoipCfg stVoipCfg;
	uint32 s_id_rtp, s_id_rtcp;
	int ret = 0;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	unsigned int mgr_chid;
#endif

	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST

	// Send Control Packet and wait Response Packet
	mgr_chid = stVoipCfg.ch_id;
	stVoipCfg.ch_id = API_get_DSP_CH(cmd, stVoipCfg.ch_id);	// convert to DSP chid
	ret = ipcSentControlPacket(cmd, mgr_chid, &stVoipCfg, sizeof(TstVoipCfg));
	stVoipCfg.ch_id = mgr_chid;
	//stVoipCfg.ret_val = ret;
	
	s_id_rtp = (2*stVoipCfg.ch_id+PROTOCOL__RTP-1);
	s_id_rtcp = (2*stVoipCfg.ch_id+PROTOCOL__RTCP-1);
	PRINT_MSG("UNSET RTP(%d)\n", s_id_rtp);

	save_flags(flags); cli();
	
	#if defined(CONFIG_RTK_VOIP_DRIVERS_PCM8651) && !defined(CONFIG_RTK_VOIP_RX)
	rtp_pair.chid = stVoipCfg.ch_id;
	
	rtp_pair.mid = s_id_rtp;
	result = voip_register_RTPport( /*chid*/ rtp_pair.chid, /*mid*/ rtp_pair.mid, NULL );
	if( result < 0 )
		PRINT_MSG("Unregister 865x RTP port  failed\n");
	else
		PRINT_MSG("Unregister 865x RTP port  successfully\n");
	
	rtp_pair.mid = s_id_rtcp;
	result = voip_register_RTPport( /*chid*/ rtp_pair.chid, /*mid*/ rtp_pair.mid, NULL );
	if( result < 0 )
		PRINT_MSG("Unregister 865x RTP port  failed\n");
	else
		PRINT_MSG("Unregister 865x RTP port  successfully\n");
	#else
	rtk_trap_unregister(s_id_rtp);
	rtk_trap_unregister(s_id_rtcp);
	#endif
	
	restore_flags(flags);

#else //!CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	s_id_rtp = (2*stVoipCfg.ch_id+PROTOCOL__RTP-1);
	s_id_rtcp = (2*stVoipCfg.ch_id+PROTOCOL__RTCP-1);
	PRINT_MSG("UNSET RTP(%d)\n", s_id_rtp);
	//PRINT_MSG("s_id_rtp = %d\n", s_id_rtp);
	//PRINT_MSG("s_id_rtcp = %d\n", s_id_rtcp);
	save_flags(flags); cli();
	
	#if defined(CONFIG_RTK_VOIP_DRIVERS_PCM8651) && !defined(CONFIG_RTK_VOIP_RX)
	rtp_pair.chid = stVoipCfg.ch_id;
	
	rtp_pair.mid = s_id_rtp;
	result = voip_register_RTPport( /*chid*/ rtp_pair.chid, /*mid*/ rtp_pair.mid, NULL );
	if( result < 0 )
		PRINT_MSG("Unregister 865x RTP port  failed\n");
	else
		PRINT_MSG("Unregister 865x RTP port  successfully\n");
	
	rtp_pair.mid = s_id_rtcp;
	result = voip_register_RTPport( /*chid*/ rtp_pair.chid, /*mid*/ rtp_pair.mid, NULL );
	if( result < 0 )
		PRINT_MSG("Unregister 865x RTP port  failed\n");
	else
		PRINT_MSG("Unregister 865x RTP port  successfully\n");
	#else
	rtk_trap_unregister(s_id_rtp);
	rtk_trap_unregister(s_id_rtcp);
	#endif
	
	restore_flags(flags);
#endif	
	
	return ret;
}
#endif

/**
 * @ingroup VOIP_PROTOCOL_RTP
 * @brief Set RTP session state to be send or receive 
 * @see VOIP_MGR_SETRTPSESSIONSTATE TstVoipRtpSessionState 
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_SETRTPSESSIONSTATE( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	unsigned long flags;
	uint32 ch_id, m_id, s_id;
#endif
	TstVoipRtpSessionState stVoipRtpSessionState;
	int ret;

	COPY_FROM_USER(&stVoipRtpSessionState, (TstVoipRtpSessionState *)user, sizeof(TstVoipRtpSessionState));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	ch_id = stVoipRtpSessionState.ch_id;
	m_id = stVoipRtpSessionState.m_id;
	s_id = API_GetSid(ch_id, m_id);
#ifdef PCM_LOOP_MODE_CONTROL
 #ifndef CONFIG_RTK_VOIP_SILENCE
	if(LoopBackInfo[s_id].isLoopBack == 1)
	{
		return 0;
	}
 #endif
#endif
	PRINT_MSG("VOIP_MGR_SETRTPSESSIONSTATE:ch_id=%d, m_id=%d, s_id=%d, state=%d\n", stVoipRtpSessionState.ch_id, stVoipRtpSessionState.m_id, s_id, stVoipRtpSessionState.state);

#ifdef VOIP_RESOURCE_CHECK
	if ( resource_weight[s_id] == DEFAULT_WEIGHT )
	{
		stVoipRtpSessionState.state = rtp_session_inactive;
	}
#endif
	switch(stVoipRtpSessionState.state){
		case rtp_session_sendonly:	//pass through
		case rtp_session_recvonly: //pass through
		case rtp_session_sendrecv:
		{
			extern void DspcodecWriteSnyc( uint32 sid );
			DspcodecWriteSnyc( s_id );
	  		rtpConfigOK[s_id]=1;
	 		DSP_init_done = 1;
		}
			break;
		case rtp_session_inactive:
			rtpConfigOK[s_id]=0;
			//astTwoChannelCfg[stVoipRtpSessionState.ch_id].channel_enable = 0;	// pkshih: unused now
			break;
	}
	save_flags(flags); cli();
	RtpTerminal_SetSessionState(s_id, stVoipRtpSessionState.state);
	restore_flags(flags);
#endif
	return 0;
}
#else
int do_mgr_VOIP_MGR_SETRTPSESSIONSTATE( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	unsigned long flags;
	uint32 ch_id, m_id;
	TstVoipRtpSessionState stVoipRtpSessionState;
	uint32 s_id_rtp, s_id_rtcp;
	int ret;

	COPY_FROM_USER(&stVoipRtpSessionState, (TstVoipRtpSessionState *)user, sizeof(TstVoipRtpSessionState));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward 
#else
	ret = ( int )NULL;
	ch_id = stVoipRtpSessionState.ch_id;
	m_id = stVoipRtpSessionState.m_id;
	
	s_id_rtp = 2*ch_id + PROTOCOL__RTP - 1;
	s_id_rtcp = 2*ch_id + PROTOCOL__RTCP - 1;
	
	PRINT_MSG("VOIP_MGR_SETRTPSESSIONSTATE:ch_id=%d, m_id=%d, s_id=%d, state=%d\n", ch_id, m_id, s_id_rtp, stVoipRtpSessionState.state);
	
	save_flags(flags); cli();

	switch(stVoipRtpSessionState.state)
	{
		case rtp_session_sendonly://pass through
		case rtp_session_recvonly: //pass through
		case rtp_session_sendrecv:
		{
			if (rtpConfigOK[s_id_rtp] == 0)
			ret = RtkAc49xApiActiveRegularRtp(ch_id, s_id_rtp); /* return 0 : success */
	
			if (ret == 0)
			{
				rtpConfigOK[s_id_rtp] = 1;
				rtpConfigOK[s_id_rtcp] = 1;
				PRINT_MSG("open RTP(%d, %d)\n", ch_id, s_id_rtp);
			}
		}
			break;
	
		case rtp_session_inactive:
	
			if (rtpConfigOK[s_id_rtp] == 1)
			ret = RtkAc49xApiCloseRegularRtp(ch_id, s_id_rtp); /* return 0 : success */
	
			if (ret == 0)
			{
				rtpConfigOK[s_id_rtp] = 0;
				rtpConfigOK[s_id_rtcp] = 0;
				PRINT_MSG("close RTP(%d, %d)\n", ch_id, s_id_rtp);
			}
			break;
	}
	
	restore_flags(flags);
#endif	
	
	return 0;
}
#endif

/**
 * @ingroup VOIP_PROTOCOL_RTP
 * @brief Turn on/off RTP session 
 * @param TstVoipCfg.ch_id Channel ID 
 * @param TstVoipCfg.m_id Media ID 
 * @param TstVoipCfg.enable Enable or disable RTP  
 * @see VOIP_MGR_RTP_CFG TstVoipCfg 
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_RTP_CFG( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	uint32 s_id;
#endif
	TstVoipCfg stVoipCfg;
	int ret;
	
	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward 
#else
	s_id = API_GetSid(stVoipCfg.ch_id, stVoipCfg.m_id);
	PRINT_MSG("VOIP_MGR_RTP_CFG:ch_id = %d, m_id = %d, s_id=%d, enable = %d\n", stVoipCfg.ch_id,  stVoipCfg.m_id, s_id, stVoipCfg.enable);
	if(stVoipCfg.enable ==1){
		extern void DspcodecWriteSnyc( uint32 sid );
		DspcodecWriteSnyc( s_id );
		
		rtpConfigOK[s_id]=1;
		DSP_init_done = 1;
	} else if(stVoipCfg.enable ==0) {
		rtpConfigOK[s_id]=0;
		//DSP_init_done = 0;
	}
#endif
	return 0;
}
#else
int do_mgr_VOIP_MGR_RTP_CFG( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipCfg stVoipCfg;
	uint32 s_id_rtp, s_id_rtcp;
	int ret;

	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;
	
	PRINT_MSG("VOIP_MGR_RTP_CFG:ch_id = %d, enable = %d\n", stVoipCfg.ch_id, stVoipCfg.enable);

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward 
#else
	ret = ( int )NULL;
	
	s_id_rtp = 2*stVoipCfg.ch_id + PROTOCOL__RTP - 1;
	s_id_rtcp = 2*stVoipCfg.ch_id + PROTOCOL__RTCP - 1;
	
	
	if(stVoipCfg.enable ==1)
	{
		if (rtpConfigOK[s_id_rtp] == 0)
			ret = RtkAc49xApiActiveRegularRtp(stVoipCfg.ch_id, s_id_rtp); /* return 0 : success */
	
		if (ret == 0)
		{
			rtpConfigOK[s_id_rtp] = 1;
			rtpConfigOK[s_id_rtcp] = 1;
			PRINT_MSG("open RTP(%d, %d)\n", stVoipCfg.ch_id, s_id_rtp);
		}
	}
	else
	{
		if (rtpConfigOK[s_id_rtp] == 1)
			ret = RtkAc49xApiCloseRegularRtp(stVoipCfg.ch_id, s_id_rtp); /* return 0 : success */
	
		if (ret == 0)
		{
			rtpConfigOK[s_id_rtp] = 0;
			rtpConfigOK[s_id_rtcp] = 0;
			PRINT_MSG("close RTP(%d, %d)\n", stVoipCfg.ch_id, s_id_rtp);
		}
	}
#endif
	
	return 0;
}
#endif

/**
 * @ingroup VOIP_PROTOCOL_RTP
 * @brief Hold RTP session 
 * @param TstVoipCfg.ch_id Channel ID 
 * @param TstVoipCfg.m_id Media ID 
 * @param TstVoipCfg.enable Hold (1) or release (0) session 
 * @see VOIP_MGR_HOLD TstVoipCfg 
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_HOLD( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	unsigned long flags;
	uint32 s_id;
#endif
	TstVoipCfg stVoipCfg;
	int ret;
	
	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward 
#else
	s_id = API_GetSid(stVoipCfg.ch_id, stVoipCfg.m_id);
	PRINT_MSG("VOIP_MGR_HOLD:ch_id = %d, m_id = %d, s_id=%d, enable = %d\n", stVoipCfg.ch_id,  stVoipCfg.m_id, s_id, stVoipCfg.enable);
	save_flags(flags); cli();
	if(stVoipCfg.enable ==1){
		rtpConfigOK[s_id]=0;
		rtpHold[s_id]=1;
	} else if(stVoipCfg.enable ==0) {
		rtpConfigOK[s_id]=1;
		rtpHold[s_id]=0;
	}
	restore_flags(flags);
#endif
	return 0;
}
#else
int do_mgr_VOIP_MGR_HOLD( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	//PRINT_MSG("This IO Ctrl is NOT support at AudioCodes solution.\n");
	return NO_COPY_TO_USER( cmd, seq_no );
	//return 0;
}
#endif

/**
 * @ingroup VOIP_PROTOCOL_RTP
 * @brief Open / close a session ID by channel and media ID  
 * @param TstVoipCfg.ch_id Channel ID 
 * @param TstVoipCfg.m_id Media ID 
 * @see VOIP_MGR_CTRL_RTPSESSION TstVoipCfg 
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_CTRL_RTPSESSION( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	//extern int hook_det_start(uint32 cch);
	unsigned long flags;
	TstVoipCfg stVoipCfg;
	int ret;

	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

	PRINT_MSG("VOIP_MGR_CTRL_RTPSESSION:ch_id = %d, m_id = %d, enable = %d\n", stVoipCfg.ch_id, stVoipCfg.m_id, stVoipCfg.enable);

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward, and run this body 
#endif
	save_flags(flags); cli();
//#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP
//	hook_det_start(stVoipCfg.ch_id); //DSP only
//#endif
	if(stVoipCfg.enable ==1){
		API_OpenSid(stVoipCfg.ch_id, stVoipCfg.m_id);
	} else if(stVoipCfg.enable ==0) {
		API_CloseSid(stVoipCfg.ch_id, stVoipCfg.m_id);
	}
	restore_flags(flags);
	return 0;
}
#else
int do_mgr_VOIP_MGR_CTRL_RTPSESSION( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	//PRINT_MSG("This IO Ctrl is NOT support at AudioCodes solution.\n");
	return NO_COPY_TO_USER( cmd, seq_no );
	//return 0;
}
#endif

/**
 * @ingroup VOIP_PROTOCOL_RTP
 * @brief Set / get transaction (active) session ID 
 * @param TstVoipCfg.ch_id Channel ID 
 * @param TstVoipCfg.m_id Media ID 
 * @param [out] TstVoipCfg.t_id Transaction ID 
 * @param TstVoipCfg.enable Set (1) or get (0) transaction session ID 
 * @see VOIP_MGR_CTRL_TRANSESSION_ID TstVoipCfg 
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_CTRL_TRANSESSION_ID( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	unsigned long flags;
	uint32 temp;
#endif
	uint32 s_id;
	TstVoipCfg stVoipCfg;
	
	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));
	
	s_id = API_GetSid(stVoipCfg.ch_id, stVoipCfg.m_id);
	PRINT_MSG("VOIP_MGR_CTRL_TRANSESSION_ID:ch_id = %d, m_id = %d, s_id=%d, enable = %d\n", stVoipCfg.ch_id, stVoipCfg.m_id, s_id, stVoipCfg.enable);

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward 
#else
	
	//extern channel_config_t chanInfo[];
	save_flags(flags); cli();
	
	if(stVoipCfg.enable ==1)
	{
		chanInfo_SetTranSessionID(stVoipCfg.ch_id, s_id);
	}
	else if(stVoipCfg.enable ==0)
	{
		temp = chanInfo_GetTranSessionID(stVoipCfg.ch_id);
		stVoipCfg.t_id = temp;
	}
	restore_flags(flags);
#endif
	return COPY_TO_USER(user, &stVoipCfg, sizeof(TstVoipCfg), cmd, seq_no);
	//return 0;
}
#else
int do_mgr_VOIP_MGR_CTRL_TRANSESSION_ID( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipCfg stVoipCfg;
	
	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));
	return COPY_TO_USER(user, &stVoipCfg, sizeof(TstVoipCfg), cmd, seq_no);
	//PRINT_MSG("This IO Ctrl is NOT support at AudioCodes solution.\n");
	return 0;
}
#endif

/**
 * @ingroup VOIP_PROTOCOL_RTP
 * @brief Set a channel to be 3 way conference 
 * @see VOIP_MGR_SETCONFERENCE TstVoipMgr3WayCfg 
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_SETCONFERENCE( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	unsigned long flags;
#endif
	//uint32 s_id;
	TstVoipMgr3WayCfg stVoipMgr3WayCfg;
	int ret;

	COPY_FROM_USER(&stVoipMgr3WayCfg, (TstVoipMgr3WayCfg *)user, sizeof(TstVoipMgr3WayCfg));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

	PRINT_MSG("VOIP_MGR_SETCONFERENCE:ch_id = %d, enable = %d\n", stVoipMgr3WayCfg.ch_id, stVoipMgr3WayCfg.enable);

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward 
#else
	save_flags(flags); cli();
	//extern channel_config_t chanInfo[];

#ifdef PCM_LOOP_MODE_CONTROL

	if (stVoipMgr3WayCfg.enable == 1)
	{
		int sid0=0, sid1;
		for(sid0=0; sid0<DSP_RTK_SS_NUM; sid0++)
		{
	
	
			if(LoopBackInfo[sid0].isLoopBack == 1)
			{
				//printk("---> sid%d loop back\n", sid0);
				loop_3way[stVoipMgr3WayCfg.ch_id] = 1;
	
				for (sid1=sid0+1; sid1<DSP_RTK_SS_NUM; sid1++)
				{
					if(LoopBackInfo[sid1].isLoopBack == 1)
					{
						//printk("---> sid%d loop back\n", sid1);
						if (LoopBackInfo[sid0].udp_src_port == LoopBackInfo[sid1].udp_dst_port)
						{
							int mate_ch;
							if (stVoipMgr3WayCfg.ch_id == chanInfo_GetChannelbySession(sid0))
							{
								mate_ch = chanInfo_GetChannelbySession(sid1);
							}
							else
							{
								mate_ch = chanInfo_GetChannelbySession(sid0);
							}
	
							pcm_set_LoopMode(2, stVoipMgr3WayCfg.ch_id, mate_ch);
	
							int m_id, sid_tmp;
							for (m_id=0; m_id<2; m_id++)
							{
								sid_tmp = API_GetSid(stVoipMgr3WayCfg.ch_id, m_id);
								if (LoopBackInfo[sid_tmp].isLoopBack != 1)
								{
									chanInfo_SetTranSessionID(stVoipMgr3WayCfg.ch_id, sid_tmp);
									PRINT_MSG("SetTranSessionID %d\n", sid_tmp);
								}
							}
	
	
						}
						//else
							//printk("--->src=%d, dst=%d\n", ntohs(LoopBackInfo[sid0].udp_src_port), ntohs(LoopBackInfo[sid1].udp_dst_port));
	
					}
				}
			}
		}
	}
	else
	{
		loop_3way[stVoipMgr3WayCfg.ch_id] = 0;
	}
	
	if (loop_3way[stVoipMgr3WayCfg.ch_id] == 0)
	{
		chanInfo_SetConference(stVoipMgr3WayCfg.ch_id, stVoipMgr3WayCfg.enable);
	}

#else
	chanInfo_SetConference(stVoipMgr3WayCfg.ch_id, stVoipMgr3WayCfg.enable);
#endif
	restore_flags(flags);
	
#endif
	return 0;
}
#else
#define ACMW_NEW_3WAY //2x3wau support
int do_mgr_VOIP_MGR_SETCONFERENCE( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipMgr3WayCfg stVoipMgr3WayCfg;
	int ret;
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	int mmid;
	int isMode3[2] = {0};
#endif
	
	COPY_FROM_USER(&stVoipMgr3WayCfg, (TstVoipMgr3WayCfg *)user, sizeof(TstVoipMgr3WayCfg));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

	PRINT_MSG("VOIP_MGR_SETCONFERENCE:ch_id = %d, enable = %d\n", stVoipMgr3WayCfg.ch_id, stVoipMgr3WayCfg.enable);

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward 
#else
	
	/* Check which mid session is mode3. Note: If src ip = dst ip, this session is mode3. */
	for (mmid=0; mmid<=1; mmid++)
		{
		if (stVoipMgr3WayCfg.rtp_cfg[mmid].ip_src_addr == stVoipMgr3WayCfg.rtp_cfg[mmid].ip_dst_addr)
		{
			isMode3[mmid] = 1;
			PRINT_MSG("isMode3[%d]=%d\n", mmid, isMode3[mmid]);
		}
	}
	
	if(stVoipMgr3WayCfg.enable == 1)
	{
		PRINT_MSG("<<<< 3-way conf. enable >>>>\n");

		IsMode3[stVoipMgr3WayCfg.ch_id] = isMode3[0] | isMode3[1];

		////////// Register RTP Session //////////
		for (mmid=0; mmid<=1; mmid++)
		{
			///// The rule to decide the mid(0, 1) use which sid resource((rtp,rtcp)-(0,1), (2,3)) ////
			///// If mid 0 is mode3, then it always use the sid (2,3), and mid 1 use sid (0,1) ////////
			///// If mid 1 is mode3, then it always use the sid (2,3), and mid 0 use sid (0,1) ////////
			///// If both mid is NOT mode3, then mid 1 always use sid (2,3), mid 0 always use sid (0,1).
			if (isMode3[mmid] == 1)
				stVoipMgr3WayCfg.rtp_cfg[mmid].ch_id = stVoipMgr3WayCfg.rtp_cfg[mmid].ch_id^1;
			else if ((stVoipMgr3WayCfg.rtp_cfg[mmid].m_id == 1) && isMode3[mmid^1] == 0)
		#ifndef ACMW_NEW_3WAY
				stVoipMgr3WayCfg.rtp_cfg[mmid].ch_id = stVoipMgr3WayCfg.rtp_cfg[mmid].ch_id^1;
		#else
			{
				//printk("==> 1\n");
				//if (stVoipMgr3WayCfg.rtp_cfg[mmid].ch_id == 0)
				//	stVoipMgr3WayCfg.rtp_cfg[mmid].ch_id = 2;
				//else if (stVoipMgr3WayCfg.rtp_cfg[mmid].ch_id == 1)
				//	stVoipMgr3WayCfg.rtp_cfg[mmid].ch_id = 3;
				stVoipMgr3WayCfg.rtp_cfg[mmid].ch_id =
					(ACMW_MAX_NUM_CH/2) + stVoipMgr3WayCfg.rtp_cfg[mmid].ch_id;
			}
		#endif
			//////////////////////////////////////////////////////////////////////////////////////////
	
			if (isMode3[mmid] == 0)
			{
				//printk("==> 2\n");
				voip_mgr_unset_rtp_1(stVoipMgr3WayCfg.rtp_cfg[mmid].ch_id, 0 /*don't care here*/);
				voip_mgr_unset_rtp_2(stVoipMgr3WayCfg.rtp_cfg[mmid].ch_id, 0 /*don't care here*/);
				voip_mgr_set_rtp_1(stVoipMgr3WayCfg.rtp_cfg[mmid]);
			}
			else if (isMode3[mmid] == 1)
			{
				//printk("==> 3\n");
				// If mmid is mode3, just de-active RTP, don't unregister sid session used by mmid.
				voip_mgr_unset_rtp_2(stVoipMgr3WayCfg.rtp_cfg[mmid].ch_id, 0 /*don't care here*/);
			}
	
		}
	
	
	
		////////// Active 3-Way Conference ///////////
		RtkAc49xApiOnhookAction(stVoipMgr3WayCfg.ch_id);
		if (IsMode3[stVoipMgr3WayCfg.ch_id] == 1)
			RtkAc49xApiOnhookAction(stVoipMgr3WayCfg.ch_id^1);
		RtkAc49xApiOffhookAction(stVoipMgr3WayCfg.ch_id);			//main
		RtkAc49xApiOffhookAction((ACMW_MAX_NUM_CH/2) + stVoipMgr3WayCfg.ch_id);	//mate
	
		//printk("==> 4\n");
	
	
		for (mmid=0; mmid<=1; mmid++)
		{
			stVoipMgr3WayCfg.rtp_cfg[mmid].state = 1; //for open RTP
	
			PRINT_MSG("isMode3[%d]=%d\n", mmid, isMode3[mmid]);
			if (isMode3[mmid] != 1)
			{
				if (IsMode3[stVoipMgr3WayCfg.ch_id] == 1)
				{
					if (stVoipMgr3WayCfg.rtp_cfg[mmid].m_id == 1)
						stVoipMgr3WayCfg.rtp_cfg[mmid].ch_id = stVoipMgr3WayCfg.ch_id;
				}
				voip_mgr_set_rtp_2(stVoipMgr3WayCfg.rtp_cfg[mmid]);
			}
		}
	
		if (IsMode3[stVoipMgr3WayCfg.ch_id] == 1)
			RtkAc49xApiActiveOrDeactive3WayConference(stVoipMgr3WayCfg.ch_id, _3_WAY_CONFERENCE_MODE__3);
		else
			RtkAc49xApiActiveOrDeactive3WayConference(stVoipMgr3WayCfg.ch_id, _3_WAY_CONFERENCE_MODE__1);
		
		//printk("==> 5\n");
	
	}
	else if(stVoipMgr3WayCfg.enable == 0)
	{
		PRINT_MSG("<<<< 3-way conf. disable >>>>\n");

		RtkAc49xApiActiveOrDeactive3WayConference(stVoipMgr3WayCfg.ch_id, _3_WAY_CONFERENCE_MODE__DISABLE);
	
		voip_mgr_unset_rtp_1(stVoipMgr3WayCfg.ch_id, 0 /*don't care here*/);
		voip_mgr_unset_rtp_2(stVoipMgr3WayCfg.ch_id, 0 /*don't care here*/);
	
		if (IsMode3[stVoipMgr3WayCfg.ch_id] == 0)
		{
	#ifndef ACMW_NEW_3WAY
			voip_mgr_unset_rtp_1(stVoipMgr3WayCfg.ch_id^1, 0 /*don't care here*/);
			voip_mgr_unset_rtp_2(stVoipMgr3WayCfg.ch_id^1, 0 /*don't care here*/);
	#else
			//if (stVoipMgr3WayCfg.ch_id == 0)
			//	stVoipMgr3WayCfg.ch_id = 2;
			//else if (stVoipMgr3WayCfg.ch_id == 1)
			//	stVoipMgr3WayCfg.ch_id = 3;
			stVoipMgr3WayCfg.ch_id =
				(ACMW_MAX_NUM_CH/2) + stVoipMgr3WayCfg.ch_id;

			voip_mgr_unset_rtp_1(stVoipMgr3WayCfg.ch_id, 0 /*don't care here*/);
			voip_mgr_unset_rtp_2(stVoipMgr3WayCfg.ch_id, 0 /*don't care here*/);
	#endif
		}
		else if (IsMode3[stVoipMgr3WayCfg.ch_id] == 1)
		{
			voip_mgr_unset_rtp_2(stVoipMgr3WayCfg.ch_id^1, 0 /*don't care here*/);
		}
	
		RtkAc49xApiOnhookAction(stVoipMgr3WayCfg.ch_id);			//main
		RtkAc49xApiOnhookAction((ACMW_MAX_NUM_CH/2) + stVoipMgr3WayCfg.ch_id);	//mate
		RtkAc49xApiOffhookAction(stVoipMgr3WayCfg.ch_id);			//main
	
		if (IsMode3[stVoipMgr3WayCfg.ch_id] == 1)
		{
			RtkAc49xApiOffhookAction(stVoipMgr3WayCfg.ch_id^1);//must
	
			for (mmid=0; mmid<=1; mmid++)
			{
				if ((isMode3[mmid] == 1) && (stVoipMgr3WayCfg.rtp_cfg[mmid].ip_src_addr != 0))
				{
					stVoipMgr3WayCfg.rtp_cfg[mmid].state = 1; //for open RTP
					stVoipMgr3WayCfg.rtp_cfg[mmid].ch_id = stVoipMgr3WayCfg.rtp_cfg[mmid].ch_id^1;
					voip_mgr_set_rtp_2(stVoipMgr3WayCfg.rtp_cfg[mmid]);
				}
			}
	
		}
	
		IsMode3[stVoipMgr3WayCfg.ch_id] = 0;
	}
#endif
	return 0;
}
#endif

/**
 * @ingroup VOIP_PROTOCOL_RTP
 * @brief Get RTP statistics 
 * @see VOIP_MGR_GET_RTP_STATISTICS TstVoipRtpStatistics 
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_GET_RTP_STATISTICS( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	extern void ResetRtpStatsCount( uint32 chid );
	extern uint32 nRxRtpStatsCountByte[MAX_DSP_RTK_CH_NUM];
	extern uint32 nRxRtpStatsCountPacket[MAX_DSP_RTK_CH_NUM];
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	extern uint32 nRxRtpStatsLostPacket[MAX_DSP_RTK_CH_NUM];
#endif
	extern uint32 nTxRtpStatsCountByte[MAX_DSP_RTK_CH_NUM];
	extern uint32 nTxRtpStatsCountPacket[MAX_DSP_RTK_CH_NUM];

	uint32 ch_id;
	TstVoipRtpStatistics stVoipRtpStatistics;
	int ret = 0;
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	unsigned short dsp_id;
	unsigned int mgr_chid;
#endif
	
	COPY_FROM_USER(&stVoipRtpStatistics, (TstVoipRtpStatistics *)user, sizeof(TstVoipRtpStatistics));

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	
	if( ( ch_id = stVoipRtpStatistics.ch_id ) > DSP_RTK_CH_NUM )
	{
		//stVoipRtpStatistics.ret_val = 0;
		return COPY_TO_USER(user, &stVoipRtpStatistics, sizeof(TstVoipRtpStatistics), cmd, seq_no);
		//return 0;	/* unexpected chid */
	}

	if( stVoipRtpStatistics.bResetStatistics )	/* reset statistics? */
		ResetRtpStatsCount( ch_id );

	// Send Control Packet and wait Response Packet
	mgr_chid = stVoipRtpStatistics.ch_id;
	stVoipRtpStatistics.ch_id = API_get_DSP_CH(cmd, stVoipRtpStatistics.ch_id);	// convert to DSP chid
	ret = ipcSentControlPacket(cmd, mgr_chid, &stVoipRtpStatistics, sizeof(TstVoipRtpStatistics));
	
	// Ckeck Response Packet (need for copy_to_user)
	ipcCheckRespPacket(cmd, &stVoipRtpStatistics, &dsp_id);
	stVoipRtpStatistics.ch_id = API_get_Host_CH( dsp_id, stVoipRtpStatistics.ch_id);/* Get Host chid */

	//stVoipRtpStatistics.nRxRtpStatsLostPacket info is from DSP
	stVoipRtpStatistics.nRxRtpStatsCountByte 	= nRxRtpStatsCountByte[ ch_id ];	//info from Host
	stVoipRtpStatistics.nRxRtpStatsCountPacket 	= nRxRtpStatsCountPacket[ ch_id ];	//info from Host
	stVoipRtpStatistics.nTxRtpStatsCountByte 	= nTxRtpStatsCountByte[ ch_id ];	//info from Host
	stVoipRtpStatistics.nTxRtpStatsCountPacket 	= nTxRtpStatsCountPacket[ ch_id ];	//info from Host
	
#else

	if( ( ch_id = stVoipRtpStatistics.ch_id ) > DSP_RTK_CH_NUM )
	{
		return 0;	/* unexpected chid */
	}
	
	if( stVoipRtpStatistics.bResetStatistics )	/* reset statistics? */
		ResetRtpStatsCount( ch_id );
	
	/* ok. copy statistics data */
	stVoipRtpStatistics.nRxRtpStatsCountByte 	= nRxRtpStatsCountByte[ ch_id ];	//info from Host
	stVoipRtpStatistics.nRxRtpStatsCountPacket 	= nRxRtpStatsCountPacket[ ch_id ];	//info from Host
	stVoipRtpStatistics.nRxRtpStatsLostPacket 	= nRxRtpStatsLostPacket[ ch_id ];	//info from DSP
	stVoipRtpStatistics.nTxRtpStatsCountByte 	= nTxRtpStatsCountByte[ ch_id ];	//info from Host
	stVoipRtpStatistics.nTxRtpStatsCountPacket 	= nTxRtpStatsCountPacket[ ch_id ];	//info from Host
	//stVoipRtpStatistics.ret_val = 0;
#endif
	return COPY_TO_USER(user, &stVoipRtpStatistics, sizeof(TstVoipRtpStatistics), cmd, seq_no);
	
	return ret;
}
#else
int do_mgr_VOIP_MGR_GET_RTP_STATISTICS( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	extern uint32 nRxRtpStatsCountByte[];
	extern uint32 nRxRtpStatsCountPacket[];
	extern uint32 nRxRtpStatsLostPacket[];
	extern uint32 nTxRtpStatsCountByte[];
	extern uint32 nTxRtpStatsCountPacket[];
	extern uint32 gRtcpStatsUpdOk[];

	uint32 ch_id;
	TstVoipRtpStatistics stVoipRtpStatistics;

	COPY_FROM_USER(&stVoipRtpStatistics, (TstVoipRtpStatistics *)user, sizeof(TstVoipRtpStatistics));

//#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
#if 0

	// Send Control Packet and wait Response Packet
	mgr_chid = stVoipRtpStatistics.ch_id;
	stVoipRtpStatistics.ch_id = API_get_DSP_CH(cmd, stVoipRtpStatistics.ch_id);	// convert to DSP chid
	ret = ipcSentControlPacket(cmd, mgr_chid, &stVoipRtpStatistics, sizeof(TstVoipRtpStatistics));
	
	// Ckeck Response Packet (need for copy_to_user)
	unsigned short dsp_id;
	ipcCheckRespPacket(cmd, &stVoipRtpStatistics, &dsp_id);
	stVoipRtpStatistics.ch_id = API_get_Host_CH( dsp_id, stVoipRtpStatistics.ch_id);/* Get Host chid */

	//stVoipRtpStatistics.ret_val = ret; // update ret_val must after check response ack
#else
	
	if( ( ch_id = stVoipRtpStatistics.ch_id ) > DSP_AC_CH_NUM )
	{
		return 0;	/* unexpected chid */
	}
	
	if( stVoipRtpStatistics.bResetStatistics )	/* reset statistics? */
	{
		RtkAc49xApiResetRtpStatistics(ch_id);
	}
	
	RtkAc49xApiGetRtpStatistics(ch_id);
	
	PRINT_MSG("wait");
	while(!gRtcpStatsUpdOk[ch_id])
	{
		PRINT_MSG(".");
	}
	
	
	/* ok. copy statistics data */
	stVoipRtpStatistics.nRxRtpStatsCountByte 	= nRxRtpStatsCountByte[ ch_id ];
	stVoipRtpStatistics.nRxRtpStatsCountPacket 	= nRxRtpStatsCountPacket[ ch_id ];
	stVoipRtpStatistics.nRxRtpStatsLostPacket 	= nRxRtpStatsLostPacket[ ch_id ];
	stVoipRtpStatistics.nTxRtpStatsCountByte 	= nTxRtpStatsCountByte[ ch_id ];
	stVoipRtpStatistics.nTxRtpStatsCountPacket 	= nTxRtpStatsCountPacket[ ch_id ];
	
	gRtcpStatsUpdOk[ch_id] = 0;
	
	//PRINT_MSG("CH%d-(%d, %d, %d, %d, %d)\n", stVoipValue.ch_id, nRxRtpStatsCountByte[stVoipValue.ch_id], nRxRtpStatsCountPacket[stVoipValue.ch_id],
	// nRxRtpStatsLostPacket[stVoipValue.ch_id], nTxRtpStatsCountByte[stVoipValue.ch_id], nTxRtpStatsCountPacket[stVoipValue.ch_id]);
#endif
	return COPY_TO_USER(user, &stVoipRtpStatistics, sizeof(TstVoipRtpStatistics), cmd, seq_no);
	
	//return 0;
}
#endif

/**
 * @ingroup VOIP_PROTOCOL_RTP
 * @brief Get statistics by session 
 * @see VOIP_MGR_GET_SESSION_STATISTICS TstVoipSessionStatistics 
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_GET_SESSION_STATISTICS( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	extern void ResetSessionRxStatistics( uint32 sid );
	extern void ResetSessionTxStatistics( uint32 sid );
	extern void JbcStatistics_Reset( uint32 ssid );
	extern uint32 JbcStatistics_GetJitterLength( uint32 sid );
	extern uint32 JbcStatistics_GetPlayoutDelay( uint32 sid );
	extern uint32 JbcStatistics_GetEarlyPacket( uint32 ssid );
	extern uint32 JbcStatistics_GetLatePacket( uint32 ssid );
	extern uint32 JbcStatistics_GetSilenceSpeech( uint32 ssid );

#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	extern uint32 nRxSilencePacket[MAX_DSP_RTK_SS_NUM];
	extern uint32 nTxSilencePacket[MAX_DSP_RTK_SS_NUM];
	
	unsigned char sid;
#endif

	TstVoipSessionStatistics stVoipSessionStatistics;
	
	COPY_FROM_USER(&stVoipSessionStatistics, (TstVoipSessionStatistics *)user, sizeof(TstVoipSessionStatistics));
	
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward 
#else
	
	sid = API_GetSid(stVoipSessionStatistics.ch_id, stVoipSessionStatistics.m_id);

	if( sid  >= DSP_RTK_SS_NUM ) {
		return 0;	/* unexpected sid */
	}
	
	if( stVoipSessionStatistics.bResetStatistics ) {
		ResetSessionRxStatistics( sid );
		ResetSessionTxStatistics( sid );
		JbcStatistics_Reset( sid );
	}
	
	stVoipSessionStatistics.nRxSilencePacket = nRxSilencePacket[ sid ];
	stVoipSessionStatistics.nTxSilencePacket = nTxSilencePacket[ sid ];
	stVoipSessionStatistics.nAvgPlayoutDelay = JbcStatistics_GetPlayoutDelay( sid );
	stVoipSessionStatistics.nCurrentJitterBuf = JbcStatistics_GetJitterLength( sid );
	stVoipSessionStatistics.nEarlyPacket = JbcStatistics_GetEarlyPacket( sid );
	stVoipSessionStatistics.nLatePacket = JbcStatistics_GetLatePacket( sid );
	stVoipSessionStatistics.nSilenceSpeech = JbcStatistics_GetSilenceSpeech( sid );
#endif
	
	return COPY_TO_USER(user, &stVoipSessionStatistics, sizeof(TstVoipSessionStatistics), cmd, seq_no);
	
	//return 0;
}
#else
int do_mgr_VOIP_MGR_GET_SESSION_STATISTICS( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipSessionStatistics stVoipSessionStatistics;
	
	COPY_FROM_USER(&stVoipSessionStatistics, (TstVoipSessionStatistics *)user, sizeof(TstVoipSessionStatistics));
	
	stVoipSessionStatistics.nRxSilencePacket = 1;
	stVoipSessionStatistics.nTxSilencePacket = 2;
	stVoipSessionStatistics.nAvgPlayoutDelay = 3;
	stVoipSessionStatistics.nCurrentJitterBuf = 4;
	stVoipSessionStatistics.nEarlyPacket = 5;
	stVoipSessionStatistics.nLatePacket = 6;
	
	return COPY_TO_USER(user, &stVoipSessionStatistics, sizeof(TstVoipSessionStatistics), cmd, seq_no);
	
	//return 0;
}
#endif

/**
 * @ingroup VOIP_PROTOCOL_RTCP
 * @brief Set RTCP session trap with IP, port and so on 
 * @see VOIP_MGR_SET_RTCP_SESSIO TstVoipRtcpSession rtk_trap_register()
 * @see do_mgr_VOIP_MGR_UNSET_RTCP_SESSION()
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_SET_RTCP_SESSION( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	unsigned long flags;
	uint32 s_id;
	TstVoipRtcpSession stVoipRtcpSession;
	int ret;
	
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	unsigned int mgr_chid;
#endif


#ifdef SUPPORT_RTCP
	PRINT_MSG("SET RTCP SESSION\n");
	COPY_FROM_USER(&stVoipRtcpSession, (TstVoipRtcpSession *)user, sizeof(TstVoipRtcpSession));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST

	save_flags(flags); cli();
	
	s_id = API_GetSid(stVoipRtcpSession.ch_id, stVoipRtcpSession.m_id);
	
	PRINT_MSG("sid=%d\n", s_id);
	
	if(filter[s_id+ RTCP_SID_OFFSET]!=0) {
		PRINT_MSG("rtcp: s_id %d in used, please unregister first\n", s_id+ RTCP_SID_OFFSET);
		return 0;
	}
	

#if defined(CONFIG_RTK_VOIP_DRIVERS_PCM8651) && !defined(CONFIG_RTK_VOIP_RX)
	//thlin: need test for 8651
	rtp_pair.isTcp = FALSE;
	rtp_pair.remIp = stVoipRtcpSession.ip_src_addr;
	rtp_pair.remPort = stVoipRtcpSession.rtcp_src_port;
	rtp_pair.extIp = stVoipRtcpSession.ip_dst_addr;
	rtp_pair.extPort = stVoipRtcpSession.rtcp_dst_port;
	rtp_pair.chid = stVoipRtcpSession.ch_id;
	rtp_pair.mid = s_id+ RTCP_SID_OFFSET;
	result = voip_register_RTPport(rtp_pair.chid, rtp_pair.mid, &rtp_pair );
	if(result < 0 )
	{
		PRINT_MSG("865x register RTCP failed\n");
	}
	else
	{
		PRINT_MSG("865x register RTCP success\n");
	}
#else
	stVoipRtcpSession.result = rtk_trap_register((TstVoipMgrSession * )&stVoipRtcpSession, stVoipRtcpSession.ch_id, stVoipRtcpSession.m_id, s_id+ RTCP_SID_OFFSET,  (int32(*)(uint8 , int32 , void *, uint32 , uint32 ))Host_pktRx);//register ok, return 0.
	
	if(stVoipRtcpSession.result == -1)
		PRINT_MSG("rtk_trap_register RTCP fail, sid=%d\n", s_id);
	else if (stVoipRtcpSession.result == 0)
		PRINT_MSG("rtk_trap_register success, sid=%d\n", s_id);
#endif
	restore_flags(flags);

	// Send Control Packet and wait Response Packet
	mgr_chid = stVoipRtcpSession.ch_id;
	stVoipRtcpSession.ch_id = API_get_DSP_CH(cmd, stVoipRtcpSession.ch_id);	// convert to DSP chid
	ret = ipcSentControlPacket(cmd, mgr_chid, &stVoipRtcpSession, sizeof(TstVoipRtcpSession));
	stVoipRtcpSession.ch_id = mgr_chid;

#else //!CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST

	s_id = API_GetSid(stVoipRtcpSession.ch_id, stVoipRtcpSession.m_id);
#if 0
	PRINT_MSG("ch_id = %d\n", stVoipRtcpSession.ch_id);
	PRINT_MSG("m_id = %d\n", stVoipRtcpSession.m_id);
	PRINT_MSG("Rtcp s_id = %d\n", s_id+ RTCP_SID_OFFSET);
	PRINT_MSG("Rtcp ip_src_addr = %d.%d.%d.%d\n", ((unsigned char *)(&stVoipRtcpSession.ip_src_addr))[0], ((unsigned char *)(&stVoipRtcpSession.ip_src_addr))[1], ((unsigned char *)(&stVoipRtcpSession.ip_src_addr))[2], ((unsigned char *)(&stVoipRtcpSession.ip_src_addr))[3]);
	PRINT_MSG("Rtcp ip_dst_addr = %d.%d.%d.%d\n", ((unsigned char *)(&stVoipRtcpSession.ip_dst_addr))[0], ((unsigned char *)(&stVoipRtcpSession.ip_dst_addr))[1], ((unsigned char *)(&stVoipRtcpSession.ip_dst_addr))[2], ((unsigned char *)(&stVoipRtcpSession.ip_dst_addr))[3]);
	PRINT_MSG("Rtcp udp_src_port = %d\n", ntohs(stVoipRtcpSession.rtcp_src_port));
	PRINT_MSG("Rtcp udp_dst_port = %d\n", ntohs(stVoipRtcpSession.rtcp_dst_port));
	PRINT_MSG("protocol = 0x%x\n", stVoipRtcpSession.protocol);
#endif
	save_flags(flags); cli();
	
	if(filter[s_id+ RTCP_SID_OFFSET]!=0) {
		PRINT_MSG("rtcp: s_id %d in used, please unregister first\n", s_id+ RTCP_SID_OFFSET);
		return 0;
	}
	
	RtpSession_renew(s_id+ RTCP_SID_OFFSET, NULL /*ignore*/, &stVoipRtcpSession );

#if defined(CONFIG_RTK_VOIP_DRIVERS_PCM8651) && !defined(CONFIG_RTK_VOIP_RX)
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP
	result = stVoipRtcpSession.result;
#else
	//thlin: need test for 8651
	rtp_pair.isTcp = FALSE;
	rtp_pair.remIp = stVoipRtcpSession.ip_src_addr;
	rtp_pair.remPort = stVoipRtcpSession.rtcp_src_port;
	rtp_pair.extIp = stVoipRtcpSession.ip_dst_addr;
	rtp_pair.extPort = stVoipRtcpSession.rtcp_dst_port;
	rtp_pair.chid = stVoipRtcpSession.ch_id;
	rtp_pair.mid = s_id+ RTCP_SID_OFFSET;
	result = voip_register_RTPport(rtp_pair.chid, rtp_pair.mid, &rtp_pair );
#endif
	if(result < 0 )
	{
		PRINT_MSG("865x register RTCP failed\n");
		RtcpOpen[s_id] = 0;
	}
	else
	{
		PRINT_MSG("865x register RTCP success\n");
		RtcpOpen[s_id] = 1;
	}
#else
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP
	if (!rtk_trap_register((TstVoipMgrSession * )&stVoipRtcpSession, stVoipRtcpSession.ch_id, stVoipRtcpSession.m_id, s_id+ RTCP_SID_OFFSET,  (int32(*)(uint8 , int32 , void *, uint32 , uint32 ))DSP_pktRx))//register ok, return 0.
		RtcpOpen[s_id] = 1;
#endif
#endif
	restore_flags(flags);
#endif
	
#else
	return NO_COPY_TO_USER( cmd, seq_no );
#endif	//SUPPORT_RTCP
	return 0;
}
#else
int do_mgr_VOIP_MGR_SET_RTCP_SESSION( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	/* For ACMW, RTCP is bonded with RTP. This IO Ctrl not need to implement.*/
	// move VOIP_MGR_SET_RTCP_TX_INTERVAL to here!! 
	TstVoipRtcpSession stVoipRtcpSession;
	
	COPY_FROM_USER(&stVoipRtcpSession, (TstVoipRtcpSession *)user, sizeof(TstVoipRtcpSession));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef SUPPORT_RTCP
#ifdef CONFIG_RTK_VOIP_ETHERNET_DSP_IS_HOST
	// Host auto forward 
#else
	save_flags(flags); cli();
	/* If RtcpMeanTxInterval is equal to 0, then RTCP Tx is disable.*/
	RtkAc49xApiSetRtcpTxInterval(stVoipRtcpSession.ch_id, stVoipRtcpSession.tx_interval);
	restore_flags(flags);
#endif
#else
#endif

	return 0;
}
#endif

/**
 * @ingroup VOIP_PROTOCOL_RTCP
 * @brief Unset RTCP session trap 
 * @param TstVoipCfg.ch_id Channel ID 
 * @param TstVoipCfg.m_id Media ID 
 * @see VOIP_MGR_UNSET_RTCP_SESSION TstVoipCfg rtk_trap_register()
 * @see do_mgr_VOIP_MGR_SET_RTCP_SESSION()
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_UNSET_RTCP_SESSION( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	extern unsigned short RtcpMeanTxInterval;
#endif

	unsigned long flags;
	uint32 s_id;
	TstVoipCfg stVoipCfg;
	int ret = 0;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	unsigned int mgr_chid;
#endif

#ifdef SUPPORT_RTCP
	PRINT_MSG("UNSET RTCP SESSION\n");
	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST

	s_id = API_GetSid(stVoipCfg.ch_id, stVoipCfg.m_id);

	PRINT_MSG("s_id = %d\n", s_id+ RTCP_SID_OFFSET);
	save_flags(flags); cli();
#if defined(CONFIG_RTK_VOIP_DRIVERS_PCM8651) && !defined(CONFIG_RTK_VOIP_RX)
	//thlin: need test for 8651
	rtp_pair.chid = stVoipCfg.ch_id;
	rtp_pair.mid = s_id+ RTCP_SID_OFFSET;
	result = voip_register_RTPport( /*chid*/ rtp_pair.chid, /*mid*/ rtp_pair.mid, NULL );
	if( result < 0 )
	{
		PRINT_MSG("Unregister 865x RTCP port  failed\n");
	}

#else
	rtk_trap_unregister(s_id+ RTCP_SID_OFFSET);
#endif
	restore_flags(flags);

	// Send Control Packet and wait Response Packet
	mgr_chid = stVoipCfg.ch_id;
	stVoipCfg.ch_id = API_get_DSP_CH(cmd, stVoipCfg.ch_id);	// convert to DSP chid
	ret = ipcSentControlPacket(cmd, mgr_chid, &stVoipCfg, sizeof(TstVoipCfg));
	stVoipCfg.ch_id = mgr_chid;

#else //!CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	s_id = API_GetSid(stVoipCfg.ch_id, stVoipCfg.m_id);
	//extern unsigned short RtcpMeanTxInterval;
	if (RtcpOpen[s_id] != 0)
	{
		//if (RtcpMeanTxInterval != 0)	// move to RtcpTx_transmitRTCPBYE() 
			RtcpTx_transmitRTCPBYE(s_id);
	}
	PRINT_MSG("s_id = %d\n", s_id+ RTCP_SID_OFFSET);
	save_flags(flags); cli();
#if defined(CONFIG_RTK_VOIP_DRIVERS_PCM8651) && !defined(CONFIG_RTK_VOIP_RX)
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	//thlin: need test for 8651
	rtp_pair.chid = stVoipCfg.ch_id;
	rtp_pair.mid = s_id+ RTCP_SID_OFFSET;
	result = voip_register_RTPport( /*chid*/ rtp_pair.chid, /*mid*/ rtp_pair.mid, NULL );
	if( result < 0 )
	{
		PRINT_MSG("Unregister 865x RTCP port  failed\n");
	}
	else
		RtcpOpen[s_id] = 0;
#endif
#else
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	rtk_trap_unregister(s_id+ RTCP_SID_OFFSET);
#endif
	RtcpOpen[s_id] = 0;
#endif
	restore_flags(flags);
#endif
#else
	return NO_COPY_TO_USER( cmd, seq_no );
#endif
	return ret;
}
#else
int do_mgr_VOIP_MGR_UNSET_RTCP_SESSION( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	/* For ACMW, RTCP is bonded with RTP. This IO Ctrl not need to implement. */
	return NO_COPY_TO_USER( cmd, seq_no );
	//return 0;
}
#endif

/**
 * @ingroup VOIP_PROTOCOL_RTCP
 * @brief Configure RTCP Tx interval 
 * @param TstVoipValue.value5 RTCP interval 
 * @see VOIP_MGR_SET_RTCP_TX_INTERVAL TstVoipValue 
 */
#if 0
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_SET_RTCP_TX_INTERVAL( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifdef SUPPORT_RTCP
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	extern unsigned short RtcpMeanTxInterval;
	unsigned long flags;
#endif
#endif
	TstVoipValue stVoipValue;
	int ret;

	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef SUPPORT_RTCP
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward 
#else
	save_flags(flags); cli();
	/* If RtcpMeanTxInterval is equal to 0, then RTCP Tx is disable.*/
	RtcpMeanTxInterval = stVoipValue.value5;
	restore_flags(flags);
#endif
#else
#endif
	return 0;
}
#else
int do_mgr_VOIP_MGR_SET_RTCP_TX_INTERVAL( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	unsigned long flags;
	TstVoipValue stVoipValue;
	int ret;

	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef SUPPORT_RTCP
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward 
#else
	save_flags(flags); cli();
	/* If RtcpMeanTxInterval is equal to 0, then RTCP Tx is disable.*/
	RtkAc49xApiSetRtcpTxInterval(stVoipValue.ch_id, stVoipValue.value5);
	restore_flags(flags);
#endif
#else
#endif
	return 0;
}
#endif
#endif

/**
 * @ingroup VOIP_PROTOCOL_RTCP
 * @brief Get RTCP logger data  
 * @param ch_id Channel ID
 * @param m_id Media ID 
 * @param TX_* Log data for TX 
 * @param RX_* Log data for RX 
 * @param *_loss_rate_* Loss rate (RTCP)
 * @param *_jitter_* Interarrival jitter (RTCP)
 * @param *_round_trip_* Round trip delay (RTCP XR)
 * @param *_MOS_LQ_* MOS LQ (RTCP XR)
 * @param *_max / *_min / *_avg / *_cur are means maximum / minimum / average / current respectively. 
 * @see VOIP_MGR_GET_RTCP_LOGGER TstVoipRtcpLogger 
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_GET_RTCP_LOGGER( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifndef CONFIG_RTK_VOIP_ETHERNET_DSP_IS_HOST
	extern int RtcpTx_getLogger( uint32 sid, TstVoipRtcpLogger *pLogger );
	extern int RtcpRx_getLogger( uint32 sid, TstVoipRtcpLogger *pLogger );
	
	TstVoipRtcpLogger stVoipRtcpLogger;
	uint32 s_id;
	uint32 ch_id, m_id;
	
	COPY_FROM_USER(&stVoipRtcpLogger, (TstVoipRtcpLogger *)user, sizeof(TstVoipRtcpLogger));
	
#ifdef SUPPORT_RTCP
	s_id = API_GetSid(stVoipRtcpLogger.ch_id, stVoipRtcpLogger.m_id);
	
	if( s_id == SESSION_NULL ) {
		PRINT_MSG( "VOIP_MGR_GET_RTCP_LOGGER ch_id=%d m_id=%d s_id=%d\n", stVoipRtcpLogger.ch_id, stVoipRtcpLogger.m_id, s_id );
		goto label_done;
	}
	
	// clean logger data, but preserve channel and media ID 
	ch_id = stVoipRtcpLogger.ch_id;
	m_id = stVoipRtcpLogger.m_id;
	memset( &stVoipRtcpLogger, 0, sizeof( stVoipRtcpLogger ) );
	stVoipRtcpLogger.ch_id = ch_id;
	stVoipRtcpLogger.m_id = m_id;
	
	RtcpTx_getLogger( s_id, &stVoipRtcpLogger );
	RtcpRx_getLogger( s_id, &stVoipRtcpLogger );

label_done:
#endif	
	
	return COPY_TO_USER( user, &stVoipRtcpLogger, sizeof(TstVoipRtcpLogger), cmd, seq_no );
#else
	// Host auto forward
#endif
}
#else
int do_mgr_VOIP_MGR_GET_RTCP_LOGGER( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipRtcpLogger stVoipRtcpLogger;
	
	COPY_FROM_USER(&stVoipRtcpLogger, (TstVoipRtcpLogger *)user, sizeof(TstVoipRtcpLogger));
	
	return COPY_TO_USER( user, &stVoipRtcpLogger, sizeof(TstVoipRtcpLogger), cmd, seq_no );
}
#endif


