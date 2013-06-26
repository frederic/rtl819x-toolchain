#include <asm/uaccess.h>
#include "rtk_voip.h"
#include "voip_types.h"
#include "voip_control.h"
#include "voip_init.h"
#include "voip_limit.h"
#include "voip_mgr_define.h"
#include "ipc_arch_tx.h"
#include "con_mux.h"

#include "voip_mgr_transfer_id.h"
#include "voip_mgr_help_host.h"

//int slic_ch_num = SLIC_CH_NUM;
//int voip_ch_num = VOIP_CH_NUM;
//int sess_num = SESS_NUM;
//int rtcp_sid_offset = RTCP_SID_OFFSET;
//int pcm_period = PCM_PERIOD;
//uint64 gVoipFeature;

/* PCM Channel Info */
//char chanEnabled[MAX_VOIP_CH_NUM] = {0};

/************************************************************************/

#if 0
extern uint32 API_OpenSid_Dsp(uint32 chid, uint32 mid, uint32 dsp_id);
extern uint32 API_GetSid_Dsp(uint32 chid, uint32 mid, uint32 dsp_id);
extern uint32 API_GetMid_Dsp(uint32 chid, uint32 sid, uint32 dsp_id);
extern uint32 API_CloseSid_Dsp(uint32 chid, uint32 mid, uint32 dsp_id);
#endif

/* T.38 Info */
//t38_running_state_t t38RunningState[MAX_VOIP_CH_NUM]; // need T.38 info for Host in rx trap.

/* RTP Statistic Info */
//unsigned char RtcpOpen[MAX_SESS_NUM] = {0};
//unsigned char RtpOpen[MAX_SESS_NUM] = {0};
//uint32 rtpHold[MAX_SESS_NUM];
//uint32 rtpConfigOK[MAX_SESS_NUM];
//uint32 nRxRtpStatsLostPacket[MAX_VOIP_CH_NUM];

int32 Host_pktRx( uint32 chid, uint32 sid, void* packet, uint32 pktLen, uint32 flags )
{

	// Send RTP packet to DSP by voice packet.
	TstTxPktCtrl stTxPktCtrl = { .seq_no = -1, .resend_flag = 0 };
	unsigned char rtp_read_tmp[1500];
	unsigned int chid_dsp/*, sid_dsp*/, mid;
	int isRtcp = 0;
	voice_rtp2dsp_content_t * const voice_rtp2dsp_content = 
			( voice_rtp2dsp_content_t * )rtp_read_tmp;
	
	if( sid >= RTCP_SID_OFFSET ) {
		sid -= RTCP_SID_OFFSET;
		isRtcp = 1;
	} 
	
	stTxPktCtrl.dsp_cpuid = API_get_DSP_ID(0, chid);
	
	/* Transfer Host chid, sid to DSP ID, chid */	
	chid_dsp = API_get_DSP_CH(0, chid);
	mid = API_GetMid(chid, sid);
	//sid_dsp = API_GetSid_Dsp(chid_dsp, mid, stTxPktCtrl.dsp_id);

#if 1
	voice_rtp2dsp_content ->chid = chid_dsp;
	voice_rtp2dsp_content ->mid = mid + ( isRtcp ? RTCP_MID_OFFSET : 0 );
	voice_rtp2dsp_content ->flags = flags;
#else	
	*(uint32*)rtp_read_tmp= chid_dsp;
	*(uint32*)((unsigned char*)rtp_read_tmp+4)= sid_dsp;
	*(uint32*)((unsigned char*)rtp_read_tmp+8)= flags;
#endif
	
	if (pktLen > (1500-12))
	{
		PRINT_R("Error! no enought buf size, %s-%s-%d\n", __FILE__, __FUNCTION__, __LINE__);
		PRINT_R("Need %d bytes, sid=%d\n", pktLen, sid);
	}
	//memcpy(&rtp_read_tmp[12], packet, pktLen);
	memcpy(voice_rtp2dsp_content ->voice, packet, pktLen);
	
	ipc_pkt_tx_final( 0, IPC_PROT_VOICE_TO_DSP, 
						(unsigned char*)voice_rtp2dsp_content, 
						pktLen + SIZE_OF_VOICE_RTP2DSP_CONT_HEADER, 
						&stTxPktCtrl, NULL);
	
	return SUCCESS;
}

void ipc_Host2DSP_T38Tx( uint32 chid_pkt, uint32 sid_pkt, void* packet, uint32 pktLen )
{
	/* Send T.38 packet to DSP */
	TstTxPktCtrl stTxPktCtrl;
	unsigned char t38_read_tmp[1500];
	unsigned int chid, mid, /*sid,*/ len;
	voice_content_t * const voice_content =
				( voice_content_t * )t38_read_tmp;

	stTxPktCtrl.dsp_cpuid = API_get_DSP_ID(0, chid_pkt);
	stTxPktCtrl.seq_no = -1;
	stTxPktCtrl.resend_flag = 0;
	
	/* Transfer Host chid, sid to DSP ID, chid */
	chid = API_get_DSP_CH(0, chid_pkt);
	mid = API_GetMid(chid_pkt, sid_pkt);
	//sid = API_GetSid(chid, mid);

#if 1
	voice_content ->chid = chid;
	voice_content ->mid = mid;
#else	
	*(uint32*)t38_read_tmp = chid; // chid
	*(uint32*)((unsigned char*)t38_read_tmp+4) = sid; // sid
#endif
	
	len = pktLen;
	
	if ( len > (1500-8))
	{
		PRINT_R("Error! no enought buf size, %s-%s-%d\n", __FILE__, __FUNCTION__, __LINE__);
		PRINT_R("Need %d bytes, sid=%d\n", len, sid_pkt);
	}
	//memcpy(&t38_read_tmp[8], skb->data + sizeof(struct udphdr), len);
	memcpy(voice_content ->voice, packet, len);
	
	ipc_pkt_tx_final( 0, IPC_PROT_T38_TO_DSP, 
						(unsigned char*)voice_content, 
						len + SIZE_OF_VOICE_CONT_HEADER, 
						&stTxPktCtrl, NULL);
	//printk("T%d ", ptr->c_id);
}

/************************************************************************/
static void read_mgr_chid( uint32 *mgr_chid, void *user, unsigned int len, const voip_mgr_entry_t *p_entry )
{
	unsigned char *field;
	
	if( p_entry ->field_offset + p_entry ->field_size > len ) {
		PRINT_R( "Bad field offset + size (%d > %d) in reading!\n",
				 p_entry ->field_offset + p_entry ->field_size, len );
		return;
	}
	
	field = ( unsigned char * )user;
	field += p_entry ->field_offset;
	
	switch( p_entry ->field_size ) {
	case 1:
		*mgr_chid = *( ( uint8 * )field );
		break;
	case 2:
		*mgr_chid = *( ( uint16 * )field );
		break;
	case 4:
		*mgr_chid = *( ( uint32 * )field );
		break;
	default:
		PRINT_R( "Bad field_size=%u in reading!\n", p_entry ->field_size );
	}
}

static void write_mgr_chid( uint32 mgr_chid, void *user, unsigned int len, const voip_mgr_entry_t *p_entry )
{
	unsigned char *field;
	
	if( p_entry ->field_offset + p_entry ->field_size > len ) {
		PRINT_R( "Bad field offset + size (%d > %d) in writing!\n",
				 p_entry ->field_offset + p_entry ->field_size, len );
		return;
	}
	
	field = ( unsigned char * )user;
	field += p_entry ->field_offset;
	
	switch( p_entry ->field_size ) {
	case 1:
		*( ( uint8 * )field ) = mgr_chid;
		break;
	case 2:
		*( ( uint16 * )field ) = mgr_chid;
		break;
	case 4:
		*( ( uint32 * )field ) = mgr_chid;
		break;
	default:
		PRINT_R( "Bad field_size=%u in writing!\n", p_entry ->field_size );
	}
}

#define HSND_PRINTK		PRINT_Y

#define USER_BUFFER_SIZE		1500
static unsigned char user_buffer[ USER_BUFFER_SIZE ];

int do_voip_mgr_ctl_in_host( int cmd, void *user, unsigned int len, const voip_mgr_entry_t *p_entry )
{
	uint32 flags = p_entry ->flags;
	int ret = 0;
	uint32 mgr_chid;
	int copy_to_user_buffer_ok = 0;
	
	static int entry = 0;

#if 0	
	switch( cmd ) {
	case VOIP_MGR_CHECK_DSP_ALL_SW_READY:
	case IPC_EVT_FAX_MODEM_PASS_FIFO:
		break;
	
	default:
		HSND_PRINTK( ">%d %d\n", cmd, len );
		break;
	}
#endif
	
	//PRINT_MSG( "%s\n", p_entry ->cmd_name );
	
	// check re-entry. Because we have only one copy of user_buffer 
	if( entry ) {
		PRINT_R( "host voip_mgr_ctl re-entry!!\n" );
	}
	
	entry = 1;
	
	// check type size 
	if( ( flags & MF_AUTOFW ) ) {
		if( cmd == VOIP_MGR_PLAY_IVR )
			;	// structure fill header only 
		else {
			if( ( flags & MF_CHANNEL ) && ( p_entry ->type_size != len ) ) 
				PRINT_R( "cmd %d field size is different %d!=%d\n", cmd, p_entry ->type_size, len );
		}
		if( len > USER_BUFFER_SIZE ) {
			PRINT_R( "cmd %d length is too long %d>%d\n", cmd, len, USER_BUFFER_SIZE );
			return -EFBIG;
		}
	}
	
	// check if SND locates in host 
	if( ( flags & ( MF_AUTOFW | MF_CHANNEL | MF_SNDCMD ) )
			   == ( MF_AUTOFW | MF_CHANNEL | MF_SNDCMD ) )
	{
		// channel: M_NORMAL_SND_MGR, M_FETCH_SND_MGR  
		copy_from_user( user_buffer, user, len );
		copy_to_user_buffer_ok = 1;
		
		read_mgr_chid( &mgr_chid, user_buffer, len, p_entry );
		
		// if SND locates host 
		if( snd_locate_host_cch( mgr_chid ) ) {
			flags &= ~MF_AUTOFW;
			flags |= MF_BODY;
		}
	} else if( ( flags & ( MF_AUTOFW | MF_CHANNEL | MF_SNDCMD ) )
					  == ( MF_AUTOFW |      0     | MF_SNDCMD ) )
	{
		// no channel: M_NOCHANNEL_SND_MGR
		
		// if SND locates host 
		flags |= MF_BODY;
	}
	
	// main process 
	if( flags & MF_AUTOFW ) {
		
		if( !copy_to_user_buffer_ok )
			copy_from_user( user_buffer, user, len );
		
		if( flags & MF_CHANNEL ) {
			uint32 dsp_cpuid, dsp_cch;
			
			//mgr_chid = stVoipCfg.ch_id;
			read_mgr_chid( &mgr_chid, user_buffer, len, p_entry );
			//stVoipCfg.ch_id = API_get_DSP_CH(cmd, stVoipCfg.ch_id);	// convert to DSP chid
			API_get_DSP_info( cmd, mgr_chid, &dsp_cpuid, &dsp_cch );
			PRINT_MSG( "%s ch_id=%d(%d:%d)\n", p_entry ->cmd_name, mgr_chid, dsp_cpuid, dsp_cch );
			write_mgr_chid( dsp_cch, user_buffer, len, p_entry );
			//ret = ethernetDspSentL2ControlPacket(cmd, mgr_chid, &stVoipCfg, sizeof(TstVoipCfg));
			ret = ipcSentControlPacket(cmd, mgr_chid, user_buffer, len );
			//stVoipCfg.ch_id = mgr_chid;
			//write_mgr_chid( mgr_chid, user_buffer, len, p_entry );	// move to below 
			//stVoipCfg.ret_val = ret;
		} else {
			PRINT_MSG( "%s\n", p_entry ->cmd_name );
			ret = ipcSentControlPacketNoChannel( cmd, user_buffer, len );
		}
	}
	
	if( flags & MF_BODY ) {
		// do_mgr function do copy_from_user by itself 
		ret = p_entry ->do_mgr( cmd, user, len, 0 );
	}
	
	if( flags & MF_AUTOFW ) {
		if( flags & MF_FETCH ) {
			unsigned short dsp_id;
			uint32 dsp_chid;
			
			//ethernetDspCheckL2RespPacket(cmd, &stSlicReg, &dsp_id);
			ipcCheckRespPacket(cmd, user_buffer, &dsp_id);
			//stSlicReg.ch_id = API_get_Host_CH( dsp_id, stSlicReg.ch_id);/* Get Host chid */
			read_mgr_chid( &dsp_chid, user_buffer, len, p_entry );
			mgr_chid = API_get_Host_CH( dsp_id, dsp_chid );/* Get Host chid */
			write_mgr_chid( mgr_chid, user_buffer, len, p_entry );
			
			//stSlicReg.ret_val = ret; // update ret_val must after check response ack      	
			
			copy_to_user( user, user_buffer, len );
		} else if( flags & MF_CHANNEL ) {
			write_mgr_chid( mgr_chid, user_buffer, len, p_entry );
		}
	}
	
	entry = 0;
	
	return ret;
}

/**********************************************/

#if 0
void __init voip_mgr_help_host_init(void)
{
	extern void __init ipc_arch_is_host_init(void);

	//int i = 0, j = 0;
	
	PRINT_Y("=== IPC: Host Init ===\n");	
	
	ipc_arch_is_host_init();

#if 0	
	for (j=0; j<CONFIG_RTK_VOIP_DSP_DEVICE_NR; j++)
	{
		for (i=0; i<SessNumPerDsp[j]; i++)
		{
			API_OpenSid_Dsp(i, 0, j); // mid 0
			API_OpenSid_Dsp(i, 1, j); // mid 1
			PRINT_Y("Open sid%d\n", i);
		}
	}
#endif
	
	PRINT_Y("=== Done ===\n");
	
}
#endif

#if 0
void __exit voip_mgr_help_host_exit(void)
{
	PRINT_Y("============= IPC: Host exit ============\n");
	
	PRINT_Y("==> Done !\n");
}
#endif

